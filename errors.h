#ifndef _ERRORS_H_
#define _ERRORS_H_

#include <string>
#include "token.h"

namespace errors {
	static const std::string expected_closing_paren = "expected closing parenthesis";
	static const std::string expected_closing_func_call = "expected closing parenthesis";
	static const std::string expected_expression = "expected expression";
	static const std::string expected_argument_delimiter = "expected comma";
}

void printError(const TokenMetaData& meta, std::string error);

#endif
