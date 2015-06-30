#include <iostream>
#include <iterator>
#include <utility>
#include <stdexcept>

#include "parser.h"
#include "errors.h"

using namespace std;

struct ParserHelper {
	// <block> ::= <statement>* | "{" <statement>* "}"
	static shared_ptr<BlockNode> parseBlock(Parser& p, TokenStream& tokens) {
		return nullptr;
	}

	// <statement> ::= <expr>; | <declaration>; | <assignment>; | (<assignment>); | <control-statement>
	static shared_ptr<StatementNode> parseStatement(Parser& p, TokenStream& tokens) {
		return nullptr;
	}

	static shared_ptr<ASTNode> parseExpressionPrimary(Parser& p, TokenStream& tokens) {
		if (tokens.empty()) {
			return nullptr;
		}

		shared_ptr<ASTNode> expr;
		auto token = tokens.get();
		bool input_ended = false;
		bool allow_function_call = false;

		switch (token.type()) {
			case TokenType::Keyword: {
				if (token.text() == "print") {
					allow_function_call = true;
					tokens.eat();
					input_ended = tokens.empty();
					expr = make_shared<IdentifierNode>(token.meta(), "print");
					break;
				}
			}
			case TokenType::Operator: {
				if (isBuiltin(token.text(), Builtin::OpenParen)) {
					tokens.eat();
					if (tokens.empty()) {
						p.error(token.meta(), errors::expected_expression);
						return nullptr;
					}

					expr = parseExpression(p, tokens);
					if (!expr) {
						p.error(token.meta(), errors::expected_expression);
						return nullptr;
					}

					auto next_token = tokens.get();
					if (!isBuiltin(next_token.text(), Builtin::CloseParen)) {
						p.error(next_token.meta(), errors::expected_closing_paren);
						return nullptr;
					}

					tokens.eat();
					input_ended = tokens.empty();
				} else {
					return parseUnaryOperator(p, tokens, nullptr);
				}
			} break;
			case TokenType::Identifier: {
				allow_function_call = true;
				tokens.eat();
				input_ended = tokens.empty();
				expr = make_shared<IdentifierNode>(token.meta(), token.text());
			} break;
			case TokenType::NumberLiteral: {
				tokens.eat();
				input_ended = tokens.empty();
				expr = make_shared<NumberLiteralNode>(token.meta(), token.text());
			} break;
			case TokenType::StringLiteral: {
				tokens.eat();
				input_ended = tokens.empty();
				expr = make_shared<StringLiteralNode>(token.meta(), token.text());
			} break;
			default:
				break;
		}

		if (!expr) {
			return nullptr;
		}

		if (input_ended) {
			return expr;
		}

		auto next_token = tokens.get();
		if (allow_function_call && isBuiltin(next_token.text(), Builtin::OpenFunctionCall)) {
			tokens.eat();
			if (tokens.empty()) {
				p.error(next_token.meta(), errors::expected_closing_func_call);
				return nullptr;
			}

			vector<shared_ptr<ASTNode>> arguments;

			while (true) {
				auto argument = parseExpression(p, tokens);
				if (!argument) {
					break;
				}

				arguments.push_back(argument);

				next_token = tokens.get();
				if (isBuiltin(next_token.text(), Builtin::CloseFunctionCall)) {
					break;
				}

				if (!isBuiltin(next_token.text(), Builtin::ArgumentDelimiter)) {
					p.error(next_token.meta(), errors::expected_argument_delimiter);
					return nullptr;
				}

				tokens.eat();
				if (tokens.empty()) {
					p.error(next_token.meta(), errors::expected_closing_func_call);
				}
			}

			next_token = tokens.get();
			if (!isBuiltin(next_token.text(), Builtin::CloseFunctionCall)) {
				p.error(next_token.meta(), errors::expected_closing_func_call);
				return nullptr;
			}

			tokens.eat();
			expr = make_shared<FunctionCallNode>(token.meta(), expr, arguments);
		}

		return expr;
	}

	static shared_ptr<ASTNode> parseUnaryOperator(Parser& p, TokenStream& tokens, shared_ptr<ASTNode> lhs) {
		if (tokens.empty()) {
			return lhs;
		}

		auto token = tokens.get();
		auto token_type = token.type();
		if (token_type != TokenType::Operator && token_type != TokenType::Keyword) {
			return parseExpressionPrimary(p, tokens);
		}

		auto op = getUnaryBuiltin(token.text());
		if (op == Builtin::Invalid) {
			return parseExpressionPrimary(p, tokens);
		}

		auto op_info = getBuiltinInfo(op);
		if (!op_info.is_operator) {
			return parseExpressionPrimary(p, tokens);
		}

		tokens.eat();
		bool input_ended = tokens.empty();

		if (!lhs) {
			if (input_ended) {
				p.error(token.meta(), "expression required for prefix unary operator");
				return nullptr;
			}

			auto expr = parseUnaryOperator(p, tokens, nullptr);
			expr = parseBinaryOperator(p, tokens, expr, op_info.precedence);

			return make_shared<UnaryOperatorNode>(token.meta(), op, expr);
		} else {
			// TODO: postfix operators
			return nullptr;
		}
	}

	static shared_ptr<ASTNode> parseBinaryOperator(Parser& p, TokenStream& tokens, shared_ptr<ASTNode> lhs, int min_precedence) {
		while (true) {
			if (tokens.empty()) {
				return lhs;
			}

			auto token = tokens.get();
			auto token_type = token.type();

			if (token_type != TokenType::Operator && token_type != TokenType::Keyword) {
				return lhs;
			}

			auto op = getBinaryBuiltin(token.text());
			if (op == Builtin::Invalid) {
				return lhs;
			}

			auto op_info = getBuiltinInfo(op);
			if (!op_info.is_operator) {
				return lhs;
			}

			int token_precedence = op_info.precedence;
			if (token_precedence < min_precedence) {
				return lhs;
			}

			tokens.eat();
			if (tokens.empty()) {
				p.error(token.meta(), errors::expected_expression);
				return nullptr;
			}

			auto rhs = parseExpressionPrimary(p, tokens);
			if (!rhs) {
				p.error(token.meta(), errors::expected_expression);
				return nullptr;
			}

			while (true) {
				if (tokens.empty()) {
					break;
				}

				auto next_token = tokens.get();
				auto next_op = getBinaryBuiltin(next_token.text());
				auto next_op_info = getBuiltinInfo(next_op);

				if (!next_op_info.is_binary) {
					break;
				}

				auto op_info = getBuiltinInfo(next_op);
				auto next_token_precedence = op_info.precedence;

				if (op_info.binding_direction == BindingDirection::LeftAssociative) {
					if (next_token_precedence <= token_precedence) {
						break;
					}
				} else {
					if (next_token_precedence < token_precedence) {
						break;
					}
				}

				rhs = parseBinaryOperator(p, tokens, rhs, next_token_precedence);
				if (!rhs) {
					p.error(next_token.meta(), errors::expected_expression);
					return nullptr;
				}
			}

			lhs = make_shared<BinaryOperatorNode>(token.meta(), op, lhs, rhs);
		}
	}

	// <expr> ::= (<expr>) | <identifier> | <number-literal> | <string-literal> | <func-call> | <bin-op-expr> | <unary-op-expr>
	static shared_ptr<ASTNode> parseExpression(Parser& p, TokenStream& tokens) {
		if (tokens.empty()) {
			return nullptr;
		}

		auto expr = parseUnaryOperator(p, tokens, nullptr);
		return parseBinaryOperator(p, tokens, expr, 0);
	}

	// <assignment> ::= <expr> <assignment-bin-op> <expr> | <assignment-unary-op> <lvalue>
	static shared_ptr<AssignmentNode> parseAssignment(Parser& p, TokenStream& tokens) {
		return nullptr;
	}

	// <declaration> ::= "var" <identifier> | "var" <identifier> = <expr>
	static shared_ptr<DeclarationNode> parseDeclaration(Parser& p, TokenStream& tokens) {
		return nullptr;
	}

	// <control-statement> ::= <if-statement> | <while-statement> | <for-statement>
	static shared_ptr<ControlFlowNode> parseControlFlow(Parser& p, TokenStream& tokens) {
		return nullptr;
	}
};

// <top> ::= <block>
pair<shared_ptr<ASTNode>, int> Parser::parse(TokenStream& tokens) {
	_error_count = 0;

	if (tokens.empty()) {
		return { nullptr, 0 };
	}

	auto root = ParserHelper::parseExpression(*this, tokens);
	return { root, _error_count };
}

void Parser::error(const TokenMetaData& meta, const string& error) {
	++_error_count;
	printError(meta, error);
}
