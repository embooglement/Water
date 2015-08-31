#ifndef _ERRORS_H_
#define _ERRORS_H_

#include <string>
#include "token.h"

namespace errors {
	// TODO: organize these errors and make them nicer
	static const std::string expected_access_member = "expected period";
	static const std::string expected_open_paren = "expected opening parenthesis";
	static const std::string expected_close_paren = "expected closing parenthesis";
	static const std::string expected_open_func_call = "expected opening parenthesis";
	static const std::string expected_close_func_call = "expected closing parenthesis";
	static const std::string expected_close_func_declaration = "expected closing parenthesis";
	static const std::string expected_open_control_flow_condition = "expected opening parenthesis";
	static const std::string expected_close_control_flow_condition = "expected closing parenthesis";
	static const std::string expected_expression = "expected expression";
	static const std::string expected_statement = "expected statement";
	static const std::string expected_identifier = "expected identifier";
	static const std::string expected_argument_list = "expected argument list";
	static const std::string expected_function_declaration = "expected function declaration";
	static const std::string expected_statement_delimiter = "expected semicolon";
	static const std::string expected_argument_delimiter = "expected comma";
	static const std::string expected_open_block = "expected opening curly brace";
	static const std::string expected_close_block = "expected closing curly brace";
	static const std::string expected_close_double_quote = "expected closing double quote";
	static const std::string expected_close_single_quote = "expected closing single quote";
	static const std::string expected_open_array_literal = "expected opening square brace";
	static const std::string expected_close_array_literal = "expected closing square brace";
	static const std::string expected_open_object_literal = "expected opening curly brace";
	static const std::string expected_close_object_literal = "expected closing curly brace";
	static const std::string expected_object_seperator = "expected a colon";
	static const std::string expected_object_key = "expected identifier or string literal";
	static const std::string redeclared_object_key = "key included in object multiple times: ";
	static const std::string expected_open_subscript = "expected opening square brace";
	static const std::string expected_close_subscript = "expected closing square brace";
	static const std::string expected_element_delimiter = "expected a comma";
	static const std::string expected_lvalue = "expected lvalue expression";
	static const std::string unexpected_token = "unexpected token";
	static const std::string unexpected_loop_control_statement = "unexpected loop control statement";
	static const std::string redeclaration = "redeclaraton of variable: ";
	static const std::string undeclared_identifier = "undeclared identifier: ";
	static const std::string expected_declaration_expression = "constants must be assigned to when declared";
	static const std::string assigning_constant = "left hand side is immutable, and cannot be assigned to";
}

void printError(const TokenMetaData& meta, std::string error);

#endif
