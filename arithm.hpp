#ifndef __FF__ARITHM__
#define __FF__ARITHM__
#include <string>
#include <iostream>
namespace ff {
	namespace arithm {
		struct data_t {
			enum class type_t {
				OP, DOUBLE
			};

			enum class operator_t {
				MUL, ADD, DIV, SUB
			};
	
			data_t(double d);
			data_t(char c);

			double number() const;
			data_t::operator_t op() const;
			data_t::type_t type() const;

			data_t operator+(const data_t& that) const;
			data_t operator-(const data_t& that) const;
			data_t operator*(const data_t& that) const;
			data_t operator/(const data_t& that) const;

		private:
			union {
				data_t::operator_t op_;
				double num_;
			};

			data_t::type_t type_;
		}; // data_t
		
		struct node_t {
			static node_t* parse(const char* exp);

			data_t  eval() const;
			bool    ok() const;

			template <typename TStream>
			void print_node(TStream& out, const char* pretext = nullptr) {
				if (nullptr != pretext)
					out << pretext;
				switch (d_.type()) {
					case data_t::type_t::DOUBLE : out << d_.number() << std::endl; break;
					case data_t::type_t::OP :
						switch(d_.op()) {
							case data_t::operator_t::MUL : out << "*" << std::endl;  break;
							case data_t::operator_t::ADD : out << "+" << std::endl;  break;
							case data_t::operator_t::DIV : out << "/" << std::endl;  break;
							case data_t::operator_t::SUB : out << "-" << std::endl;  break;
						} break;
				}
				if (left()) left()->print_node<TStream>(out, pretext);
				if (right()) right()->print_node<TStream>(out, pretext);
			}

			node_t(data_t d, node_t* left = nullptr, node_t* right = nullptr);
			node_t* add_children(node_t* left = nullptr, node_t* right = nullptr);
			node_t* left();
			node_t* right();

		private:
			data_t  d_;
			node_t* left_;
			node_t* right_;
			mutable std::string error_;
		};
	}
}

#endif
