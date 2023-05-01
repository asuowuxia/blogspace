// arithmetic types 2.cpp : Defines the entry point for the console application.

# include <afx.h>
# include "arithmetic_types.h"

int _tmain(int /*argc*/, _TCHAR* /*argv[]*/)
{
	if (arithmetic_types_module_test(100, std::cout) == 1)
		 std::cout << "***** module tests passed" << std::endl;
	else std::cout << "***** module test(s) failed" << std::endl;
	return 0;
}

