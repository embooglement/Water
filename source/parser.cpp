#include <iostream>
#include <iterator>
#include <utility>
#include <stdexcept>
#include <functional>

#include "parser.h"
#include "errors.h"
#include "iohelpers.h"

using namespace std;

struct ParserHelper {
	// <block> ::= <statement>* | "{" <statement>* "}"
	static shared_ptr<ASTNode> parseBlock(Parser& p, TokenStream& tokens, bool is_global_block = false) {
		if (tokens.empty()) {
			return nullptr;
		}

		vector<shared_ptr<ASTNode>> statements;

		auto token = tokens.get();
		auto block_meta = token.meta();
		bool has_open_brace = isBuiltin(token.text(), Builtin::OpenBlock);

		if (has_open_brace) {
			if (is_global_block) {
				has_open_brace = false;

				auto block = parseBlock(p, tokens);
				if (!block) {
					return nullptr;
				}

				statements.push_back(move(block));
			} else {
				tokens.eat();

				if (tokens.empty()) {
					p.error(block_meta, errors::expected_close_block);
					return nullptr;
				}

				token = tokens.get();
			}
		}

		p.pushScope();

		while (true) {
			if (tokens.empty()) {
				break;
			}

			shared_ptr<ASTNode> statement = nullptr;
			token = tokens.get();
			auto token_text = token.text();

			if (has_open_brace && isBuiltin(token_text, Builtin::CloseBlock)) {
				break;
			} else if (isBuiltin(token_text, Builtin::OpenBlock)) {
				statement = parseBlock(p, tokens);
			} else {
				statement = parseStatement(p, tokens);
			}

			if (!statement) {
				break;
			}

			statements.push_back(move(statement));
		}

		p.popScope();

		if (has_open_brace) {
			if (tokens.empty()) {
				p.error(token.meta(), errors::expected_close_block);
				return nullptr;
			}

			token = tokens.get();

			if (!isBuiltin(token.text(), Builtin::CloseBlock)) {
				p.error(token.meta(), errors::expected_close_block);
				return nullptr;
			}

			tokens.eat();
		}

		return make_shared<BlockNode>(block_meta, has_open_brace, statements);
	}

	// <if-statement> ::= "if" "(" <expr> ")" <block-or-statement> ["else" <block-or-statement>]
	static shared_ptr<ASTNode> parseIfStatement(Parser& p, TokenStream& tokens) {
		auto token = tokens.get();
		auto if_meta = token.meta();

		tokens.eat();

		if (tokens.empty()) {
			p.error(token.meta(), errors::expected_open_control_flow_condition);
			return nullptr;
		}

		token = tokens.get();
		if (!isBuiltin(token.text(), Builtin::OpenControlFlowCondition)) {
			p.error(token.meta(), errors::expected_open_control_flow_condition);
			return nullptr;
		}

		tokens.eat();

		auto condition = parseExpression(p, tokens);
		if (!condition) {
			return nullptr;
		}

		if (tokens.empty()) {
			p.error(condition->meta(), errors::expected_close_control_flow_condition);
			return nullptr;
		}

		token = tokens.get();
		if (!isBuiltin(token.text(), Builtin::CloseControlFlowCondition)) {
			p.error(token.meta(), errors::expected_close_control_flow_condition);
			return nullptr;
		}

		tokens.eat();

		if (tokens.empty()) {
			p.error(token.meta(), errors::expected_statement);
			return nullptr;
		}

		shared_ptr<ASTNode> then_block;

		// TODO: refactor this whole bit into a helper
		token = tokens.get();
		if (isBuiltin(token.text(), Builtin::OpenBlock)) {
			then_block = parseBlock(p, tokens);
		} else {
			then_block = parseStatement(p, tokens);
		}

		if (!then_block) {
			p.error(token.meta(), errors::expected_statement);
			return nullptr;
		}

		if (tokens.empty()) {
			return make_shared<IfStatementNode>(if_meta, condition, then_block, nullptr);
		}

		token = tokens.get();
		if (!isBuiltin(token.text(), Builtin::ElseStatement)) {
			return make_shared<IfStatementNode>(if_meta, condition, then_block, nullptr);
		}

		tokens.eat();

		shared_ptr<ASTNode> else_block;
		token = tokens.get();
		if (isBuiltin(token.text(), Builtin::OpenBlock)) {
			else_block = parseBlock(p, tokens);
		} else {
			else_block = parseStatement(p, tokens);
		}

		if (!else_block) {
			p.error(token.meta(), errors::expected_statement);
			return nullptr;
		}

		return make_shared<IfStatementNode>(if_meta, condition, then_block, else_block);
	}

	// <while-statement> ::= "while" "(" <expr> ")" <block-or-statement>
	static shared_ptr<ASTNode> parseWhileStatement(Parser& p, TokenStream& tokens) {
		auto token = tokens.get();
		auto while_meta = token.meta();

		tokens.eat();

		if (tokens.empty()) {
			p.error(token.meta(), errors::expected_open_control_flow_condition);
			return nullptr;
		}

		token = tokens.get();
		if (!isBuiltin(token.text(), Builtin::OpenControlFlowCondition)) {
			p.error(token.meta(), errors::expected_open_control_flow_condition);
			return nullptr;
		}

		tokens.eat();

		auto condition = parseExpression(p, tokens);
		if (!condition) {
			return nullptr;
		}

		if (tokens.empty()) {
			p.error(condition->meta(), errors::expected_close_control_flow_condition);
			return nullptr;
		}

		token = tokens.get();
		if (!isBuiltin(token.text(), Builtin::CloseControlFlowCondition)) {
			p.error(token.meta(), errors::expected_close_control_flow_condition);
			return nullptr;
		}

		tokens.eat();
		shared_ptr<ASTNode> loop_block;

		p.pushLoopState(true);
		token = tokens.get();

		if (isBuiltin(token.text(), Builtin::OpenBlock)) {
			loop_block = parseBlock(p, tokens);
		} else {
			loop_block = parseStatement(p, tokens);
		}

		p.popLoopState();

		if (!loop_block) {
			return nullptr;
		}

		return make_shared<WhileStatementNode>(while_meta, move(condition), move(loop_block));
	}

	// <loop-control> ::= "break" | "continue"
	static shared_ptr<ASTNode> parseLoopControlStatement(Parser& p, TokenStream& tokens) {
		auto token = tokens.get();
		auto token_text = token.text();

		tokens.eat();

		if (!p.inLoop()) {
			p.error(token.meta(), errors::unexpected_loop_control_statement);
			return nullptr;
		}

		if (isBuiltin(token_text, Builtin::BreakStatement)) {
			return make_shared<BreakNode>(token.meta());
		} else if (isBuiltin(token_text, Builtin::ContinueStatement)) {
			return make_shared<ContinueNode>(token.meta());
		}

		return nullptr;
	}

	// <statement> ::= <expr>; | <declaration>; | <assignment>; | (<assignment>); | <control-statement>
	static shared_ptr<ASTNode> parseStatement(Parser& p, TokenStream& tokens) {
		if (tokens.empty()) {
			return nullptr;
		}

		auto token = tokens.get();
		auto token_text = token.text();

		bool require_semicolon = true;
		shared_ptr<ASTNode> statement = nullptr;

		if (isBuiltin(token_text, Builtin::IfStatement)) {
			require_semicolon = false;
			statement = parseIfStatement(p, tokens);
		} else if (isBuiltin(token_text, Builtin::WhileStatement)) {
			require_semicolon = false;
			statement = parseWhileStatement(p, tokens);
		} else if (isBuiltin(token_text, Builtin::VariableDeclarator) || isBuiltin(token_text, Builtin::ConstantDeclarator)) {
			statement = parseDeclaration(p, tokens);
		} else if (isBuiltin(token_text, Builtin::BreakStatement) || isBuiltin(token_text, Builtin::ContinueStatement)) {
			statement = parseLoopControlStatement(p, tokens);
		} else {
			statement = parseExpression(p, tokens);
		}

		if (!statement) {
			return nullptr;
		}

		if (!require_semicolon) {
			return statement;
		}

		if (tokens.empty()) {
			p.error(statement->meta(), errors::expected_statement_delimiter);
			return nullptr;
		}

		token = tokens.get();
		if (!isBuiltin(token.text(), Builtin::StatementDelimiter)) {
			p.error(token.meta(), errors::expected_statement_delimiter);
			return nullptr;
		}

		tokens.eat();
		return statement;
	}

	// <func-decl> ::= "func" "(" <id>* ")" <block>
	static shared_ptr<ASTNode> parseFunctionDeclaration(Parser& p, TokenStream& tokens) {
		// TODO: pass identifiers from declarations into here somehow

		if (tokens.empty()) {
			return nullptr;
		}

		auto token = tokens.get();
		auto function_decl_meta = token.meta();

		if (!isBuiltin(token.text(), Builtin::FunctionDeclaration)) {
			p.error(token.meta(), errors::expected_function_declaration);
			return nullptr;
		}

		tokens.eat();
		token = tokens.get();

		if (!isBuiltin(token.text(), Builtin::FunctionOpenArgumentList)) {
			p.error(token.meta(), errors::expected_argument_list);
			return nullptr;
		}

		tokens.eat();

		vector<string> arguments;
		bool require_identifier = false;

		p.pushScope(true);
		auto&& scope = p.scope();

		while (true) {
			if (tokens.empty()) {
				if (require_identifier) {
					p.error(token.meta(), errors::expected_identifier);
				} else {
					p.error(token.meta(), errors::expected_close_func_declaration);
				}

				return nullptr;
			}

			token = tokens.get();

			if (isBuiltin(token.text(), Builtin::FunctionCloseArgumentList)) {
				if (require_identifier) {
					p.error(token.meta(), errors::expected_identifier);
					return nullptr;
				}

				tokens.eat();
				break;
			}

			if (token.type() != TokenType::Identifier) {
				p.error(token.meta(), errors::expected_identifier);
				return nullptr;
			}

			require_identifier = false;
			auto&& identifier = token.text();

			arguments.push_back(identifier);
			// TODO: eventually allow for let and var in argument declarations
			scope.add(identifier, { false });

			tokens.eat();

			if (tokens.empty()) {
				p.error(token.meta(), errors::expected_close_func_declaration);
				return nullptr;
			}

			auto next_token = tokens.get();
			if (isBuiltin(next_token.text(), Builtin::ArgumentDelimiter)) {
				require_identifier = true;
				tokens.eat();
			}
		}

		if (tokens.empty()) {
			p.error(token.meta(), errors::expected_open_block);
			return nullptr;
		}

		token = tokens.get();

		if (!isBuiltin(token.text(), Builtin::OpenBlock)) {
			p.error(token.meta(), errors::expected_open_block);
			return nullptr;
		}

		p.pushLoopState(false);

		auto body = parseBlock(p, tokens); // TODO: figure out closure scope

		p.popLoopState();
		p.popScope();

		return make_shared<FunctionDeclarationNode>(function_decl_meta, "", arguments, body);
	}

	// <paren-expr> ::= "(" <expr> ")"
	static shared_ptr<ASTNode> parseParenthesesExpression(Parser& p, TokenStream& tokens) {
		if (tokens.empty()) {
			return nullptr;
		}

		auto token = tokens.get();
		if (!isBuiltin(token.text(), Builtin::OpenParen)) {
			p.error(token.meta(), errors::expected_open_paren);
			return nullptr;
		}

		tokens.eat();
		if (tokens.empty()) {
			p.error(token.meta(), errors::expected_expression);
			return nullptr;
		}

		auto expr = parseExpression(p, tokens);
		if (!expr) {
			p.error(token.meta(), errors::expected_expression);
			return nullptr;
		}

		token = tokens.get();
		if (!isBuiltin(token.text(), Builtin::CloseParen)) {
			p.error(token.meta(), errors::expected_close_paren);
			return nullptr;
		}

		tokens.eat();

		return expr;
	}

	static shared_ptr<ASTNode> parseFunctionCall(Parser& p, TokenStream& tokens, shared_ptr<ASTNode> lhs) {
		if (tokens.empty()) {
			p.error(lhs->meta(), errors::expected_open_func_call);
			return nullptr;
		}

		auto token = tokens.get();
		auto call_meta = token.meta();

		if (!isBuiltin(token.text(), Builtin::OpenFunctionCall)) {
			p.error(token.meta(), errors::expected_open_func_call);
			return nullptr;
		}

		tokens.eat();
		if (tokens.empty()) {
			p.error(token.meta(), errors::expected_close_func_call);
			return nullptr;
		}

		vector<shared_ptr<ASTNode>> arguments;

		while (true) {
			auto argument = parseExpression(p, tokens);
			if (!argument) {
				break;
			}

			arguments.push_back(argument);

			token = tokens.get();
			const auto& token_text = token.text();

			if (isBuiltin(token_text, Builtin::CloseFunctionCall)) {
				break;
			}

			if (!isBuiltin(token_text, Builtin::ArgumentDelimiter)) {
				p.error(token.meta(), errors::expected_argument_delimiter);
				return nullptr;
			}

			tokens.eat();
			if (tokens.empty()) {
				p.error(token.meta(), errors::expected_close_func_call);
			}
		}

		token = tokens.get();
		if (!isBuiltin(token.text(), Builtin::CloseFunctionCall)) {
			p.error(token.meta(), errors::expected_close_func_call);
			return nullptr;
		}

		tokens.eat();
		return make_shared<FunctionCallNode>(call_meta, lhs, arguments);
	}

	// <array-literal> ::= "[" <expr>,* "]"
	static shared_ptr<ASTNode> parseArrayLiteral(Parser& p, TokenStream& tokens) {
		if (tokens.empty()) {
			return nullptr;
		}

		auto token = tokens.get();
		if (!isBuiltin(token.text(), Builtin::OpenArrayLiteral)) {
			p.error(token.meta(), errors::expected_open_array_literal);
			return nullptr;
		}

		auto array_meta = token.meta();
		vector<shared_ptr<ASTNode>> elements;

		tokens.eat();

		while (tokens.hasNext()) {
			token = tokens.get();

			if (isBuiltin(token.text(), Builtin::CloseArrayLiteral)) {
				break;
			}

			auto expr = parseExpression(p, tokens);
			elements.push_back(move(expr));

			if (tokens.empty()) {
				p.error(token.meta(), errors::expected_close_array_literal);
				return nullptr;
			}

			token = tokens.get();
			const auto& token_text = token.text();

			if (isBuiltin(token_text, Builtin::ElementDelimiter)) {
				tokens.eat();
			} else if (!isBuiltin(token_text, Builtin::CloseArrayLiteral)) {
				p.error(token.meta(), errors::expected_element_delimiter);
				return nullptr;
			}
		}

		if (tokens.empty()) {
			p.error(token.meta(), errors::expected_close_array_literal);
			return nullptr;
		}

		token = tokens.get();
		if (!isBuiltin(token.text(), Builtin::CloseArrayLiteral)) {
			p.error(token.meta(), errors::expected_close_array_literal);
			return nullptr;
		}

		tokens.eat();
		return make_shared<ArrayLiteralNode>(array_meta, move(elements));
	}

	// <subscript-expr> ::= <expr> "[" <expr> "]"
	static shared_ptr<ASTNode> parseSubscript(Parser& p, TokenStream& tokens, shared_ptr<ASTNode> lhs) {
		if (tokens.empty()) {
			p.error(lhs->meta(), errors::expected_open_subscript);
			return nullptr;
		}

		auto token = tokens.get();
		auto subscript_meta = token.meta();

		if (!isBuiltin(token.text(), Builtin::OpenSubscript)) {
			p.error(token.meta(), errors::expected_open_subscript);
			return nullptr;
		}

		tokens.eat();

		if (tokens.empty()) {
			p.error(token.meta(), errors::expected_close_subscript);
			return nullptr;
		}

		token = tokens.get();
		auto index = parseExpression(p, tokens);

		if (!index) {
			p.error(token.meta(), errors::expected_expression);
			return nullptr;
		}

		if (tokens.empty()) {
			p.error(token.meta(), errors::expected_close_subscript);
			return nullptr;
		}

		token = tokens.get();

		if (!isBuiltin(token.text(), Builtin::CloseSubscript)) {
			p.error(token.meta(), errors::expected_close_subscript);
			return nullptr;
		}

		tokens.eat();
		return make_shared<SubscriptNode>(subscript_meta, move(lhs), move(index));
	}

	// <member-access> ::= <expr> "." <identifier>
	static shared_ptr<ASTNode> parseAccessMember(Parser& p, TokenStream& tokens, shared_ptr<ASTNode> lhs) {
		if (tokens.empty()) {
			p.error(lhs->meta(), errors::expected_access_member);
			return nullptr;
		}

		auto token = tokens.get();
		auto access_meta = token.meta();

		if (!isBuiltin(token.text(), Builtin::AccessMember)) {
			p.error(token.meta(), errors::expected_access_member);
			return nullptr;
		}

		tokens.eat();
		token = tokens.get();

		if (token.type() != TokenType::Identifier) {
			p.error(token.meta(), errors::expected_identifier);
			return nullptr;
		}

		tokens.eat();

		return make_shared<AccessMemberNode>(access_meta, move(lhs), token.text());
	}

	// <expr-primary> ::= <number-literal> | <string-literal> | <boolean-literal> | <function-decl> | <function-call>
	static shared_ptr<ASTNode> parseExpressionPrimary(Parser& p, TokenStream& tokens) {
		if (tokens.empty()) {
			return nullptr;
		}

		shared_ptr<ASTNode> expr;
		auto token = tokens.get();
		auto token_text = token.text();

		switch (token.type()) {
			case TokenType::Builtin: {
				if (isBuiltin(token_text, Builtin::TrueLiteral)) {
					tokens.eat();
					expr = make_shared<BooleanLiteralNode>(token.meta(), true);
				} else if (isBuiltin(token_text, Builtin::FalseLiteral)) {
					tokens.eat();
					expr = make_shared<BooleanLiteralNode>(token.meta(), false);
				} else if (isBuiltin(token_text, Builtin::NullLiteral)) {
					tokens.eat();
					expr = make_shared<NullLiteralNode>(token.meta());
				} else if (isBuiltin(token_text, Builtin::FunctionDeclaration)) {
					expr = parseFunctionDeclaration(p, tokens);
				} else if (isBuiltin(token_text, Builtin::Return)) {
					auto return_meta = token.meta();
					tokens.eat();

					auto rhs = parseExpression(p, tokens);
					expr = make_shared<ReturnNode>(return_meta, rhs);
				} else if (isBuiltin(token_text, Builtin::OpenParen)) {
					expr = parseParenthesesExpression(p, tokens);
				} else if (isBuiltin(token_text, Builtin::OpenArrayLiteral)) {
					expr = parseArrayLiteral(p, tokens);
				}
			} break;
			case TokenType::Identifier: {
				// TODO: handle closures
				tokens.eat();
				if (p.scope().contains(token_text)) {
					expr = make_shared<IdentifierNode>(token.meta(), token_text);
				} else {
					p.error(token.meta(), errors::undeclared_identifier + token_text);
					return nullptr;
				}
			} break;
			case TokenType::NumberLiteral: {
				tokens.eat();
				expr = make_shared<NumberLiteralNode>(token.meta(), token_text);
			} break;
			case TokenType::StringLiteral: {
				tokens.eat();
				expr = make_shared<StringLiteralNode>(token.meta(), token_text);
			} break;
			default:
				break;
		}

		if (!expr) {
			return nullptr;
		}

		if (tokens.empty()) {
			return expr;
		}

		while (tokens.hasNext()) {
			token = tokens.get();
			token_text = token.text();

			if (isBuiltin(token.text(), Builtin::OpenFunctionCall)) {
				expr = parseFunctionCall(p, tokens, expr);
			} else if (isBuiltin(token_text, Builtin::OpenSubscript)) {
				expr = parseSubscript(p, tokens, expr);
			} else if (isBuiltin(token_text, Builtin::AccessMember)) {
				expr = parseAccessMember(p, tokens, expr);
			} else {
				break;
			}
		}

		return expr;
	}

	// <unary-op> ::= "-" <expr> | "not" <expr> | ...
	static shared_ptr<ASTNode> parseUnaryOperator(Parser& p, TokenStream& tokens, shared_ptr<ASTNode> lhs) {
		if (tokens.empty()) {
			return lhs;
		}

		auto token = tokens.get();
		if (token.type() != TokenType::Builtin) {
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

	// <bin-op> ::= <expr> "+" <expr> | <expr> "-" <expr> | ...
	static shared_ptr<ASTNode> parseBinaryOperator(Parser& p, TokenStream& tokens, shared_ptr<ASTNode> lhs, int min_precedence) {
		while (true) {
			if (tokens.empty()) {
				return lhs;
			}

			auto token = tokens.get();
			if (token.type() != TokenType::Builtin) {
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

			if (isAssignmentOperator(op_info)) {
				if (!lhs->isLValue()) {
					p.error(lhs->meta(), errors::expected_lvalue);
					return nullptr;
				}
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

	// <declaration> ::= "var" <identifier> | "var" <identifier> = <expr>
	static shared_ptr<DeclarationNode> parseDeclaration(Parser& p, TokenStream& tokens) {
		if (tokens.empty()) {
			return nullptr;
		}

		auto token = tokens.get();
		auto declaration_meta = token.meta();
		auto token_text = token.text();
		bool is_const;

		if (isBuiltin(token_text, Builtin::VariableDeclarator)) {
			is_const = false;
		} else if (isBuiltin(token_text, Builtin::ConstantDeclarator)) {
			is_const = true;
		} else {
			return nullptr;
		}

		tokens.eat();

		if (tokens.empty()) {
			p.error(token.meta(), errors::expected_identifier);
			return nullptr;
		}

		token = tokens.get();

		if (token.type() != TokenType::Identifier) {
			p.error(token.meta(), errors::expected_identifier);
			return nullptr;
		}

		auto id = token.text();
		auto&& scope = p.scope();

		bool added_variable = scope.add(id, { is_const });
		if (!added_variable) {
			p.error(token.meta(), errors::redeclaration + id);
			return nullptr;
		}

		tokens.eat();

		shared_ptr<ASTNode> expr = nullptr;

		if (tokens.empty() && is_const) {
			p.error(token.meta(), errors::expected_declaration_expression);
			return nullptr;
		}

		if (!tokens.empty()) {
			token = tokens.get();

			if (isBuiltin(token.text(), Builtin::VariableDeclarationOperator)) {
				tokens.eat();

				if (tokens.empty()) {
					p.error(token.meta(), errors::expected_expression);
					return nullptr;
				}

				expr = parseExpression(p, tokens);
			} else if (is_const) {
				p.error(token.meta(), errors::expected_declaration_expression);
				return nullptr;
			}
		}

		return make_shared<DeclarationNode>(declaration_meta, is_const, id, expr);
	}
};

// <top> ::= <block>
pair<shared_ptr<ASTNode>, int> Parser::parse(TokenStream& tokens) {
	_error_count = 0;

	if (tokens.empty()) {
		return { nullptr, 0 };
	}

	// TODO: this could probably be cleaner
	_scope = ParserScope::getGlobalScope();

	auto root = ParserHelper::parseBlock(*this, tokens, true);
	return { root, _error_count };
}

void Parser::error(const TokenMetaData& meta, const string& error) {
	++_error_count;
	printError(meta, error);
}

ParserScope& Parser::scope() {
	return *_scope;
}

void Parser::pushScope(bool can_overshadow) {
	_scope = make_shared<ParserScope>(_scope, can_overshadow);
}

void Parser::popScope() {
	if (_scope) {
		_scope = _scope->parent();
	}
}

bool Parser::inLoop() const {
	return _in_loop.top();
}

void Parser::pushLoopState(bool in_loop) {
	_in_loop.push(in_loop);
}

void Parser::popLoopState() {
	if (!_in_loop.empty()) {
		_in_loop.pop();
	}
}

ParserScope::ParserScope(shared_ptr<ParserScope> parent, bool can_overshadow)
	: _parent(parent), _can_overshadow(can_overshadow) {}

pair<IdentifierInfo, bool> ParserScope::get(string identifier) const {
	auto it = _vars.find(identifier);
	if (it != end(_vars)) {
		return { it->second, true };
	}

	if (_parent) {
		return _parent->get(move(identifier));
	}

	return { {}, false };
}

bool ParserScope::contains(string identifier) const {
	return get(move(identifier)).second;
}

bool ParserScope::add(string identifier, IdentifierInfo info) {
	function<bool(ParserScope*)> can_add = [&identifier, &can_add](ParserScope* scope) -> bool {
		if (!scope || scope->_can_overshadow) {
			return true;
		}

		if (scope->_vars.find(identifier) != end(scope->_vars)) {
			return false;
		}

		return can_add(scope->parent().get());
	};

	if (can_add(this)) {
		_vars.emplace(move(identifier), info);
		return true;
	}

	return false;
}

shared_ptr<ParserScope> ParserScope::parent() {
	return _parent;
}

void ParserScope::output(ostream& out, int indent) {
	out << io::indent(indent) << "scope: " << boolalpha << _can_overshadow << endl;

	for (auto p : _vars) {
		out << io::indent(indent) << p.first << ": " << boolalpha << p.second.is_const << endl;
	}

	if (_parent) {
		_parent->output(out, indent + 1);
	}

	out << endl;
}

shared_ptr<ParserScope>& ParserScope::getGlobalScope() {
	return global_scope;
}

void ParserScope::addToGlobalScope(string identifier, IdentifierInfo info) {
	global_scope->_vars.emplace(move(identifier), info);
}

shared_ptr<ParserScope> ParserScope::global_scope = make_shared<ParserScope>(nullptr, true);
