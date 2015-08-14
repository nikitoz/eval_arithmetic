#include "arithm.hpp"
#include <cstring>

namespace ff { namespace arithm {

/*
Grammar:
	ExpSumMinus ::= ExpMulDiv Op2 ExpSumMinus | ExpMulDiv;
	ExpMulDiv   ::= Exp3 Op1 ExpMulDiv | ExpSingle;
	ExpSingle   ::= Ob ExpSumMinus Cb | Num;
	Op1  ::= '*' ws | '/' ws;
	Op2  ::= '+' ws | '-' ws;
	Ob   ::= '(' ws;
	Cb   ::= ')' ws;
	Num  ::= d*\.d* ws;
	ws   ::= s*;
*/

node_t fail_node('f');
node_t succ_node('s');

data_t::data_t(double d) : num_(d), type_(data_t::type_t::DOUBLE) {}
data_t::data_t(char c)   : type_ (data_t::type_t::OP) {
	switch(c) {
		case '+' : op_ = operator_t::ADD; break;
		case '-' : op_ = operator_t::SUB; break;
		case '*' : op_ = operator_t::MUL; break;
		case '/' : op_ = operator_t::DIV; break;
	}
}
double data_t::number() const {	return num_; }
data_t::operator_t data_t::op() const { return op_; }
data_t::type_t data_t::type() const { return type_; }
data_t data_t::operator+(const data_t& that) const { return data_t(this->number() + that.number()); }
data_t data_t::operator-(const data_t& that) const { return data_t(this->number() - that.number()); }
data_t data_t::operator*(const data_t& that) const { return data_t(this->number() * that.number()); }
data_t data_t::operator/(const data_t& that) const { return data_t(this->number() / that.number()); }

node_t::node_t(data_t d, node_t* left, node_t* right)
	: d_(d), left_(left), right_(right)
{ }

node_t* node_t::add_children(node_t* left, node_t* right) {
	left_ = left;
	right_ = right;
	return this;
}

node_t* node_t::left()  { return left_; }
node_t* node_t::right() { return right_; }
bool    node_t::ok() const { return false == error_.empty(); }

data_t node_t::eval() const {
	switch(d_.type()) {
		case data_t::type_t::DOUBLE : return d_.number();
		case data_t::type_t::OP :
			switch(d_.op()) {
				case data_t::operator_t::MUL : return left_->eval() * right_->eval();
				case data_t::operator_t::ADD : return left_->eval() + right_->eval();
				case data_t::operator_t::SUB : return left_->eval() - right_->eval();
				case data_t::operator_t::DIV : return left_->eval() / right_->eval();
			}
	}
	return data_t('F');
}

bool is_failed(const node_t* nd) {
	return &fail_node == nd;
}

bool is_succ(const node_t* nd) {
	return &succ_node == nd;
}

bool is_eps(const char c) {
	return '\0' == c || ')' == c;
}

bool not_sum_diff(const char* str, size_t s) {
	return str[s] != '+' && str[s] != '-';
}

bool not_mul_div(const char* str, size_t s) {
	return str[s] != '*' && str[s] != '/';
}

node_t* ws(const char* str, size_t& s) {
	while (str[s] == ' ' || str[s] == '\t' || str[s] == '\n') ++s;
	return &succ_node;
}

node_t* op_mul_div(const char* str, size_t& s) {
	const char val = str[s];
	if ((str[s] == '*' || str[s] == '/') && is_succ(ws(str, ++s)))
		return new node_t(val);
	return &fail_node;
}

node_t* op_sum_diff(const char* str, size_t& s) {
	const char val = str[s];
	if ((str[s] == '+' || str[s] == '-') && is_succ(ws(str, ++s)))
		return new node_t(val);
	return &fail_node;
}

node_t* cb(const char* str, size_t& s) {
	if (str[s] == ')' && is_succ(ws(str, ++s)))
		return &succ_node;
	return &fail_node;
}

node_t* ob(const char* str, size_t& s) {
	if (str[s] == '(' && is_succ(ws(str, ++s)))
		return &succ_node;
	return &fail_node;
}


node_t* num(const char* str, size_t& s) {
	// parse d*
	double d = 0.0;
	const bool neg = str[0] == '-' ? true : false;
	if (neg) ++s;
		
	while (str[s] >= '0' && str[s] <= '9') {
		d *= 10;
		d += str[s] - '0';
		++s;
	}

	if (str[s] != '.') return &fail_node;
	++s;
	int m = 10;
	while (str[s] >= '0' && str[s] <= '9') {
		d += (str[s] - '0') / m;
		m *= 10;
		++s;
	}
	ws(str, s);
	return new node_t(d);
}

node_t* exp_sum_diff(const char* str, size_t& s);
node_t* exp_single(const char* str, size_t& s) {
	if (str[s] == '(' && is_succ(ob(str, s))) {
		node_t* e = exp_sum_diff(str, s);
		if (!is_failed(e) && !is_failed(cb(str, s)))
			return e;
	} else {
		return num(str, s);
	}
	return &fail_node;
}

//  ExpMulDiv ::= Exp3 Op1 ExpMulDiv | ExpSingle;
node_t* exp_mul_div(const char* str, size_t& s) {
	node_t* e3 = exp_single(str, s);
	if (is_failed(e3))
		return &fail_node;
	if (is_eps(str[s]) || not_mul_div(str, s))
		return e3;
	node_t* op = op_mul_div(str, s);
	if (!is_failed(op)) {
		node_t* e = exp_mul_div(str, s);
		if (!is_failed(e))
			return op->add_children(e3, e);
	}
	return &fail_node;
}

// ExpSumMinus ::= ExpMulDiv Op2 ExpSumMinus | ExpMulDiv;
node_t* exp_sum_diff(const char* str, size_t& s) {
	node_t* e2 = exp_mul_div(str, s);
	if (is_failed(e2)) return &fail_node;
	if (is_eps(str[s]))
		return e2;
	node_t* op = op_sum_diff(str, s);
	if (!is_failed(op)) {
		node_t* e = exp_sum_diff(str, s);
		if (!is_failed(e))
			return op->add_children(e2, e);
	}
	return &fail_node;
}

node_t* parse_helper(const char* str) {
	const size_t len = strlen(str);
	size_t start = 0;
	ws(str, start);
	node_t* retval = exp_sum_diff(str, start);
	if (str[start] == '\0')
		return retval;

	return &fail_node;
}

node_t* node_t::parse(const char* expr) {
	if (nullptr == expr)
		return nullptr;
	return parse_helper(expr);
}
}
}
