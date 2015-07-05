#ifndef _ERRORS_H_
#define _ERRORS_H_

#include <string>
#include "token.h"

namespace errors {
	static const std::string expected_close_paren = "expected closing parenthesis";
	static const std::string expected_close_func_call = "expected closing parenthesis";
	static const std::string expected_open_control_flow_condition = "expected opening parenthesis";
	static const std::string expected_close_control_flow_condition = "expected closing parenthesis";
	static const std::string expected_expression = "expected expression";
	static const std::string expected_statement = "expected statement";
	static const std::string expected_statement_delimiter = "expected semicolon";
	static const std::string expected_argument_delimiter = "expected comma";
	static const std::string expected_open_block = "expected opening curly brace";
	static const std::string expected_close_block = "expected closing curly brace";
	static const std::string unexpected_token = "unexpected token";
}

void printError(const TokenMetaData& meta, std::string error);

#endif