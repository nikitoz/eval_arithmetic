#include "eval_arithm.h"
#include <iostream>
#include <stdio.h>

using namespace ff;
int main() {
	std::cout << eval_arithmetic_expression("2.0 + 1.0") << std::endl;
	std::cout << eval_arithmetic_expression("2.0 + 1.0*(2.0-1.0 ) + 3.0/4.0") << std::endl;
	return 0;
}
