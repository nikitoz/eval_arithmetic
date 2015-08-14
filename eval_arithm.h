#ifndef __FF__EVAL__ARITHM__
#define __FF__EVAL__ARITHM__

#include "arithm.hpp"
#include <memory>

namespace ff {
double eval_arithmetic_expression(const char* expression) {
	std::unique_ptr<arithm::node_t> nd(arithm::node_t::parse(expression));
	return nd->eval().number();
}
}
#endif
