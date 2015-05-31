#include <iostream>
#include <iterator>
#include <utility>

#include "parser.h"
#include "errors.h"

using namespace std;

bool eat_token(TokenIter& iter, const TokenIter& end) {
	++iter;
	return iter == end;
}

struct ParserHelper {
	// <block> ::= <statement>* | "{" <statement>* "}"
	static shared_ptr<BlockNode> parseBlock(Parser& p, TokenIter& iter) {
		return nullptr;
	}

	// <statement> ::= <expr>; | <declaration>; | <assignment>; | (<assignment>); | <control-statement>
	static shared_ptr<StatementNode> parseStatement(Parser& p, TokenIter& iter) {
		return nullptr;
	}

	static shared_ptr<ASTNode> parseExpressionPrimary(Parser& p, TokenIter& iter, const TokenIter& end) {
		shared_ptr<ASTNode> expr;
		auto token = *iter;
		bool input_ended = false;
		bool allow_function_call = false;

		switch (token.type()) {
			case TokenType::Keyword:
			case TokenType::Operator: {
				if (isBuiltin(token.text(), Builtin::OpenParen)) {
					if (eat_token(iter, end)) {
						p.error(token.meta(), errors::expected_expression);
						return nullptr;
					}

					expr = parseExpression(p, iter, end);
					if (!expr) {
						p.error(token.meta(), errors::expected_expression);
						return nullptr;
					}

					auto next_token = *iter;
					if (!isBuiltin(next_token.text(), Builtin::CloseParen)) {
						p.error(next_token.meta(), errors::expected_closing_paren);
						return nullptr;
					}

					input_ended = eat_token(iter, end);
				}
			} break;
			case TokenType::Identifier: {
				allow_function_call = true;
				input_ended = eat_token(iter, end);
				expr = make_shared<IdentifierNode>(token.meta(), token.text());
			} break;
			case TokenType::NumberLiteral: {
				input_ended = eat_token(iter, end);
				expr = make_shared<NumberLiteralNode>(token.meta(), token.text());
			} break;
			case TokenType::StringLiteral: {
				input_ended = eat_token(iter, end);
				expr = make_shared<StringLiteralNode>(token.meta(), token.text());
			} break;
			default:
				break;
		}

		if (!expr) {
			p.error(token.meta(), errors::expected_expression);
			return nullptr;
		}

		if (input_ended) {
			return expr;
		}

		auto next_token = *iter;

		if (allow_function_call && isBuiltin(next_token.text(), Builtin::OpenFunctionCall)) {
			if (eat_token(iter, end)) {
				p.error(next_token.meta(), errors::expected_closing_func_call);
				return nullptr;
			}

			auto open_paren_token = next_token;
			vector<shared_ptr<ASTNode>> arguments;

			while (true) {
				auto argument = parseExpression(p, iter, end);
				if (!argument) {
					break;
				}

				arguments.push_back(argument);

				next_token = *iter;
				if (isBuiltin(next_token.text(), Builtin::CloseFunctionCall)) {
					break;
				}

				if (!isBuiltin(next_token.text(), Builtin::ArgumentDelimiter)) {
					p.error(next_token.meta(), errors::expected_argument_delimiter);
					return nullptr;
				}

				if (eat_token(iter, end)) {
					p.error(next_token.meta(), errors::expected_closing_func_call);
				}
			}

			next_token = *iter;
			if (!isBuiltin(next_token.text(), Builtin::CloseFunctionCall)) {
				p.error(next_token.meta(), errors::expected_closing_func_call);
				return nullptr;
			}

			eat_token(iter, end);
			expr = make_shared<FunctionCallNode>(token.meta(), expr, arguments);
		}

		return expr;
	}

	// <expr> ::= (<expr>) | <identifier> | <number-literal> | <string-literal> | <func-call> | <bin-op-expr> | <unary-op-expr>
	static shared_ptr<ASTNode> parseExpression(Parser& p, TokenIter& iter, const TokenIter& end) {
		auto expr = parseExpressionPrimary(p, iter, end);
		return parseOperator(p, iter, end, expr, 0);
	}

	static shared_ptr<ASTNode> parseOperator(Parser& p, TokenIter& iter, const TokenIter& end, shared_ptr<ASTNode> lhs, int min_precedence) {
		while (true) {
			auto token = *iter;
			auto op = getBuiltin(token.text());
			auto op_info = getOperatorInfo(op);

			if (!op_info.is_binary) {
				return lhs;
			}

			int token_precedence = op_info.precedence;
			if (token_precedence < min_precedence) {
				return lhs;
			}

			if (eat_token(iter, end)) {
				p.error(token.meta(), errors::expected_expression);
				return nullptr;
			}

			auto rhs = parseExpressionPrimary(p, iter, end);
			if (!rhs) {
				p.error(token.meta(), errors::expected_expression);
				return nullptr;
			}

			while (true) {
				auto next_token = *iter;
				auto next_op = getBuiltin(next_token.text());
				auto next_op_info = getOperatorInfo(next_op);

				if (!next_op_info.is_binary) {
					break;
				}


				auto op_info = getOperatorInfo(next_op);
				auto next_token_precedence = op_info.precedence;

				if (op_info.left_associative) {
					if (next_token_precedence <= token_precedence) {
						break;
					}
				} else {
					if (next_token_precedence < token_precedence) {
						break;
					}
				}

				rhs = parseOperator(p, iter, end, rhs, next_token_precedence);
				if (!rhs) {
					p.error(next_token.meta(), errors::expected_expression);
					return nullptr;
				}
			}

			lhs = make_shared<BinaryOperatorNode>(token.meta(), op, lhs, rhs);
		}
	}

	// <assignment> ::= <expr> <assignment-bin-op> <expr> | <assignment-unary-op> <lvalue>
	static shared_ptr<AssignmentNode> parseAssignment(Parser& p, TokenIter& iter) {
		return nullptr;
	}

	// <declaration> ::= "var" <identifier> | "var" <identifier> = <expr>
	static shared_ptr<DeclarationNode> parseDeclaration(Parser& p, TokenIter& iter) {
		return nullptr;
	}

	// <control-statement> ::= <if-statement> | <while-statement> | <for-statement>
	static shared_ptr<ControlFlowNode> parseControlFlow(Parser& p, TokenIter& iter) {
		return nullptr;
	}
};

// <top> ::= <block>
pair<shared_ptr<ASTNode>, int> Parser::parse(TokenIter begin, TokenIter end) {
	_error_count = 0;

	if (begin == end) {
		return { nullptr, 0 };
	}

	auto root = ParserHelper::parseExpression(*this, begin, end);
	return { root, _error_count };
}

void Parser::error(const TokenMetaData& meta, const string& error) {
	++_error_count;
	printError(meta, error);
}
