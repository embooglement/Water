#include <utility>
#include <boost/optional.hpp>

#include "parser.h"
#include "errors.h"

using namespace std;

boost::optional<Token> getTokenWithType(TokenStream& tokens, TokenType type) {
	if (tokens.empty()) {
		return boost::none;
	}

	auto token = tokens.get();
	if (token.type() != type) {
		return boost::none;
	}

	return token;
}

boost::optional<Token> getTokenWithBuiltin(TokenStream& tokens, Builtin builtin) {
	if (tokens.empty()) {
		return boost::none;
	}

	auto token = tokens.get();
	if (!isBuiltin(token.text(), builtin)) {
		return boost::none;
	}

	return token;
}

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
		auto scope = p.scope();

		while (tokens.hasNext()) {
			token = tokens.get();
			if (has_open_brace && isBuiltin(token.text(), Builtin::CloseBlock)) {
				break;
			}

			shared_ptr<ASTNode> statement = parseBlockOrStatement(p, tokens);
			if (!statement) {
				break;
			}

			statements.push_back(move(statement));
		}

		p.popScope();

		if (has_open_brace) {
			auto token_opt = getTokenWithBuiltin(tokens, Builtin::CloseBlock);
			if (!token_opt) {
				p.error(tokens.meta(), errors::expected_close_block);
				return nullptr;
			}

			tokens.eat();
		}

		return make_shared<BlockNode>(block_meta, scope, has_open_brace, move(statements));
	}

	// <if-statement> ::= "if" "(" <expr> ")" <block-or-statement> ["else" <block-or-statement>]
	static shared_ptr<ASTNode> parseIfStatement(Parser& p, TokenStream& tokens) {
		auto token = tokens.get();
		auto if_meta = token.meta();

		tokens.eat();

		auto token_opt = getTokenWithBuiltin(tokens, Builtin::OpenControlFlowCondition);
		if (!token_opt) {
			p.error(tokens.meta(), errors::expected_open_control_flow_condition);
			return nullptr;
		}

		tokens.eat();

		auto condition = parseExpression(p, tokens);
		if (!condition) {
			return nullptr;
		}

		token_opt = getTokenWithBuiltin(tokens, Builtin::CloseControlFlowCondition);
		if (!token_opt) {
			p.error(tokens.meta(), errors::expected_close_control_flow_condition);
			return nullptr;
		}

		tokens.eat();

		if (tokens.empty()) {
			p.error(tokens.meta(), errors::expected_statement);
			return nullptr;
		}

		shared_ptr<ASTNode> then_block = parseBlockOrStatement(p, tokens);
		if (!then_block) {
			p.error(tokens.meta(), errors::expected_statement);
			return nullptr;
		}

		token_opt = getTokenWithBuiltin(tokens, Builtin::ElseStatement);
		if (!token_opt) {
			return make_shared<IfStatementNode>(if_meta, p.scope(), condition, then_block, nullptr);
		}

		tokens.eat();

		shared_ptr<ASTNode> else_block = parseBlockOrStatement(p, tokens);
		if (!else_block) {
			p.error(tokens.meta(), errors::expected_statement);
			return nullptr;
		}

		return make_shared<IfStatementNode>(if_meta, p.scope(), condition, then_block, else_block);
	}

	// <while-statement> ::= "while" "(" <expr> ")" <block-or-statement>
	static shared_ptr<ASTNode> parseWhileStatement(Parser& p, TokenStream& tokens) {
		auto token = tokens.get();
		auto while_meta = token.meta();

		tokens.eat();

		auto token_opt = getTokenWithBuiltin(tokens, Builtin::OpenControlFlowCondition);
		if (!token_opt) {
			p.error(tokens.meta(), errors::expected_open_control_flow_condition);
			return nullptr;
		}

		tokens.eat();

		auto condition = parseExpression(p, tokens);
		if (!condition) {
			p.error(tokens.meta(), errors::expected_expression);
			return nullptr;
		}

		token_opt = getTokenWithBuiltin(tokens, Builtin::CloseControlFlowCondition);
		if (!token_opt) {
			p.error(tokens.meta(), errors::expected_close_control_flow_condition);
			return nullptr;
		}

		tokens.eat();

		p.pushLoopState(true);
		shared_ptr<ASTNode> loop_block = parseBlockOrStatement(p, tokens);
		p.popLoopState();

		if (!loop_block) {
			p.error(tokens.meta(), errors::expected_statement);
			return nullptr;
		}

		return make_shared<WhileStatementNode>(while_meta, p.scope(), move(condition), move(loop_block));
	}

	static shared_ptr<ASTNode> parseForStatement(Parser& p, TokenStream& tokens) {
		auto token_opt = getTokenWithBuiltin(tokens, Builtin::ForStatement);
		if (!token_opt) {
			p.error(tokens.meta(), errors::expected_for_statement);
			return nullptr;
		}

		auto for_meta = token_opt->meta();
		tokens.eat();

		token_opt = getTokenWithBuiltin(tokens, Builtin::OpenControlFlowCondition);
		if (!token_opt) {
			p.error(tokens.meta(), errors::expected_open_control_flow_condition);
			return nullptr;
		}

		tokens.eat();

		if (tokens.empty()) {
			p.error(tokens.meta(), errors::expected_declaration);
			return nullptr;
		}

		bool iter_is_const = true;
		auto token = tokens.get();
		auto token_text = token.text();

		if (isBuiltin(token_text, Builtin::VariableDeclarator)) {
			iter_is_const = false;
			tokens.eat();
		} else if (isBuiltin(token_text, Builtin::ConstantDeclarator)) {
			tokens.eat();
		}

		token_opt = getTokenWithType(tokens, TokenType::Identifier);
		if (!token_opt) {
			p.error(tokens.meta(), errors::expected_identifier);
			return nullptr;
		}

		auto identifier = token_opt->text();

		bool added_variable = p.scope()->add(identifier, { iter_is_const });
		if (!added_variable) {
			p.error(tokens.meta(), errors::redeclaration + identifier);
			return nullptr;
		}

		tokens.eat();

		token_opt = getTokenWithBuiltin(tokens, Builtin::ForSeperator);
		if (!token_opt) {
			p.error(tokens.meta(), errors::expected_for_seperator);
			return nullptr;
		}

		tokens.eat();

		auto array_expr = parseExpression(p, tokens);
		if (!array_expr) {
			p.error(tokens.meta(), errors::expected_expression);
			return nullptr;
		}

		token_opt = getTokenWithBuiltin(tokens, Builtin::CloseControlFlowCondition);
		if (!token_opt) {
			p.error(tokens.meta(), errors::expected_close_control_flow_condition);
			return nullptr;
		}

		tokens.eat();

		p.pushLoopState(true);
		shared_ptr<ASTNode> loop_block = parseBlockOrStatement(p, tokens);
		p.popLoopState();

		if (!loop_block) {
			p.error(tokens.meta(), errors::expected_statement);
			return nullptr;
		}

		return make_shared<ForStatementNode>(for_meta, p.scope(), iter_is_const, move(identifier), move(array_expr), move(loop_block));
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
			return make_shared<BreakNode>(token.meta(), p.scope());
		} else if (isBuiltin(token_text, Builtin::ContinueStatement)) {
			return make_shared<ContinueNode>(token.meta(), p.scope());
		}

		return nullptr;
	}

	// <statement> ::= <expr>; | <declaration>; | <assignment>; | (<assignment>); | <control-statement>
	static shared_ptr<ASTNode> parseStatement(Parser& p, TokenStream& tokens) {
		if (tokens.empty()) {
			p.error(tokens.meta(), errors::expected_statement);
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
		} else if (isBuiltin(token_text, Builtin::ForStatement)) {
			require_semicolon = false;
			statement = parseForStatement(p, tokens);
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

		auto token_opt = getTokenWithBuiltin(tokens, Builtin::StatementDelimiter);
		if (!token_opt) {
			p.error(tokens.meta(), errors::expected_statement_delimiter);
			return nullptr;
		}

		tokens.eat();
		return statement;
	}

	static shared_ptr<ASTNode> parseBlockOrStatement(Parser& p, TokenStream& tokens) {
		if (tokens.empty()) {
			p.error(tokens.meta(), errors::expected_statement);
			return nullptr;
		}

		auto token = tokens.get();
		if (isBuiltin(token.text(), Builtin::OpenBlock)) {
			return parseBlock(p, tokens);
		} else {
			return parseStatement(p, tokens);
		}
	}

	// <func-decl> ::= "func" "(" <id>* ")" <block>
	static shared_ptr<ASTNode> parseFunctionDeclaration(Parser& p, TokenStream& tokens) {
		// TODO: pass identifiers from declarations into here somehow

		auto token_opt = getTokenWithBuiltin(tokens, Builtin::FunctionDeclaration);
		if (!token_opt) {
			p.error(tokens.meta(), errors::expected_function_declaration);
			return nullptr;
		}

		auto function_decl_meta = token_opt->meta();

		tokens.eat();

		token_opt = getTokenWithBuiltin(tokens, Builtin::FunctionOpenArgumentList);
		if (!token_opt) {
			p.error(tokens.meta(), errors::expected_argument_list);
			return nullptr;
		}

		tokens.eat();

		vector<string> arguments;
		bool require_identifier = false;

		p.pushScope(true);
		auto scope = p.scope();

		while (true) {
			// TODO: I think that getTokenWithBuiltin can be used here
			if (tokens.empty()) {
				if (require_identifier) {
					p.error(tokens.meta(), errors::expected_identifier);
				} else {
					p.error(tokens.meta(), errors::expected_close_func_declaration);
				}

				return nullptr;
			}

			auto token = tokens.get();

			if (isBuiltin(token.text(), Builtin::FunctionCloseArgumentList)) {
				if (require_identifier) {
					p.error(token.meta(), errors::expected_identifier);
					return nullptr;
				}

				tokens.eat();
				break;
			}

			bool is_const = true;

			if (isBuiltin(token.text(), Builtin::VariableDeclarator)) {
				is_const = false;
				tokens.eat();
			} else if (isBuiltin(token.text(), Builtin::ConstantDeclarator)) {
				tokens.eat();
			}

			token_opt = getTokenWithType(tokens, TokenType::Identifier);
			if (!token_opt) {
				p.error(tokens.meta(), errors::expected_identifier);
				return nullptr;
			}

			require_identifier = false;
			auto&& identifier = token_opt->text();

			arguments.push_back(identifier);
			scope->add(identifier, { is_const });

			tokens.eat();

			if (tokens.empty()) {
				p.error(tokens.meta(), errors::expected_close_func_declaration);
				return nullptr;
			}

			auto next_token = tokens.get();
			if (isBuiltin(next_token.text(), Builtin::ArgumentDelimiter)) {
				require_identifier = true;
				tokens.eat();
			}
		}

		token_opt = getTokenWithBuiltin(tokens, Builtin::OpenBlock);
		if (!token_opt) {
			p.error(tokens.meta(), errors::expected_open_block);
			return nullptr;
		}

		p.pushLoopState(false);

		auto body = parseBlock(p, tokens); // TODO: figure out closure scope

		p.popLoopState();

		scope->add(return_value_alias, { false });
		p.popScope();

		return make_shared<FunctionDeclarationNode>(function_decl_meta, move(scope), "", arguments, body);
	}

	// <paren-expr> ::= "(" <expr> ")"
	static shared_ptr<ASTNode> parseParenthesesExpression(Parser& p, TokenStream& tokens) {
		auto token_opt = getTokenWithBuiltin(tokens, Builtin::OpenParen);
		if (!token_opt) {
			p.error(tokens.meta(), errors::expected_open_paren);
			return nullptr;
		}

		tokens.eat();

		auto expr = parseExpression(p, tokens);
		if (!expr) {
			p.error(tokens.meta(), errors::expected_expression);
			return nullptr;
		}

		token_opt = getTokenWithBuiltin(tokens, Builtin::CloseParen);
		if (!token_opt) {
			p.error(tokens.meta(), errors::expected_close_paren);
			return nullptr;
		}

		tokens.eat();

		return expr;
	}

	static shared_ptr<ASTNode> parseFunctionCall(Parser& p, TokenStream& tokens, shared_ptr<ASTNode> lhs) {
		auto token_opt = getTokenWithBuiltin(tokens, Builtin::OpenFunctionCall);
		if (!token_opt) {
			p.error(tokens.meta(), errors::expected_open_func_call);
			return nullptr;
		}

		auto call_meta = token_opt->meta();
		tokens.eat();

		if (tokens.empty()) {
			p.error(call_meta, errors::expected_close_func_call);
			return nullptr;
		}

		vector<shared_ptr<ASTNode>> arguments;

		while (true) {
			auto argument = parseExpression(p, tokens);
			if (!argument) {
				break;
			}

			arguments.push_back(argument);

			if (tokens.empty()) {
				p.error(tokens.meta(), errors::expected_close_func_call);
				return nullptr;
			}

			auto token = tokens.get();
			const auto& token_text = token.text();

			if (isBuiltin(token_text, Builtin::CloseFunctionCall)) {
				break;
			}

			if (!isBuiltin(token_text, Builtin::ArgumentDelimiter)) {
				p.error(token.meta(), errors::expected_argument_delimiter);
				return nullptr;
			}

			tokens.eat();
		}

		token_opt = getTokenWithBuiltin(tokens, Builtin::CloseFunctionCall);
		if (!token_opt) {
			p.error(tokens.meta(), errors::expected_close_func_call);
			return nullptr;
		}

		tokens.eat();
		return make_shared<FunctionCallNode>(call_meta, p.scope(), lhs, arguments);
	}

	// <array-literal> ::= "[" <expr>,* "]"
	static shared_ptr<ASTNode> parseArrayLiteral(Parser& p, TokenStream& tokens) {
		auto token_opt = getTokenWithBuiltin(tokens, Builtin::OpenArrayLiteral);
		if (!token_opt) {
			p.error(tokens.meta(), errors::expected_open_array_literal);
			return nullptr;
		}

		p.pushLoopState(false);

		auto array_meta = token_opt->meta();
		vector<shared_ptr<ASTNode>> elements;

		tokens.eat();

		while (tokens.hasNext()) {
			auto token = tokens.get();
			if (isBuiltin(token.text(), Builtin::CloseArrayLiteral)) {
				break;
			}

			auto expr = parseExpression(p, tokens);
			if (!expr) {
				p.error(token.meta(), errors::expected_expression);
				return nullptr;
			}

			elements.push_back(move(expr));

			if (tokens.empty()) {
				p.error(tokens.meta(), errors::expected_close_array_literal);
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

		token_opt = getTokenWithBuiltin(tokens, Builtin::CloseArrayLiteral);
		if (!token_opt) {
			p.error(tokens.meta(), errors::expected_close_array_literal);
			return nullptr;
		}

		tokens.eat();
		p.popLoopState();

		return make_shared<ArrayLiteralNode>(array_meta, p.scope(), move(elements));
	}

	static shared_ptr<ASTNode> parseObjectLiteral(Parser& p, TokenStream& tokens) {
		auto token_opt = getTokenWithBuiltin(tokens, Builtin::OpenObjectLiteral);
		if (!token_opt) {
			p.error(tokens.meta(), errors::expected_open_object_literal);
			return nullptr;
		}

		auto obj_meta = token_opt->meta();

		tokens.eat();
		p.pushLoopState(false);

		auto scope = p.scope();
		unordered_map<string, shared_ptr<ASTNode>> members;

		while (tokens.hasNext()) {
			auto token = tokens.get();
			auto token_meta = token.meta();

			if (isBuiltin(token.text(), Builtin::CloseObjectLiteral)) {
				break;
			}

			string key;
			switch (token.type()) {
				case TokenType::Identifier:
				case TokenType::StringLiteral:
					key = token.text();
					break;
				default:
					p.error(token_meta, errors::expected_object_key);
					return nullptr;
			}

			if (members.find(key) != end(members)) {
				p.error(token.meta(), errors::redeclared_object_key + key);
				return nullptr;
			}

			tokens.eat();

			token_opt = getTokenWithBuiltin(tokens, Builtin::KeyValueSeperator);
			if (!token_opt) {
				p.error(tokens.meta(), errors::expected_object_seperator);
				return nullptr;
			}

			token_meta = token_opt->meta();

			tokens.eat();

			auto expr = parseExpression(p, tokens);
			if (!expr) {
				p.error(token_meta, errors::expected_expression);
				return nullptr;
			}

			members.emplace(move(key), move(expr));

			if (tokens.empty()) {
				p.error(token.meta(), errors::expected_close_object_literal);
				return nullptr;
			}

			token = tokens.get();
			const auto& token_text = token.text();

			if (isBuiltin(token_text, Builtin::ElementDelimiter)) {
				tokens.eat();
			} else if (!isBuiltin(token_text, Builtin::CloseObjectLiteral)) {
				p.error(token.meta(), errors::expected_close_object_literal);
				return nullptr;
			}
		}

		token_opt = getTokenWithBuiltin(tokens, Builtin::CloseObjectLiteral);
		if (!token_opt) {
			p.error(tokens.meta(), errors::expected_close_object_literal);
			return nullptr;
		}

		tokens.eat();

		p.popLoopState();

		return make_shared<ObjectLiteralNode>(obj_meta, move(scope), move(members));
	}

	// <subscript-expr> ::= <expr> "[" <expr> "]"
	static shared_ptr<ASTNode> parseSubscript(Parser& p, TokenStream& tokens, shared_ptr<ASTNode> lhs) {
		auto token_opt = getTokenWithBuiltin(tokens, Builtin::OpenSubscript);
		if (!token_opt) {
			p.error(tokens.meta(), errors::expected_open_subscript);
			return nullptr;
		}

		auto subscript_meta = token_opt->meta();
		tokens.eat();

		if (tokens.empty()) {
			p.error(tokens.meta(), errors::expected_close_subscript);
			return nullptr;
		}

		auto index = parseExpression(p, tokens);

		if (!index) {
			p.error(tokens.meta(), errors::expected_expression);
			return nullptr;
		}

		token_opt = getTokenWithBuiltin(tokens, Builtin::CloseSubscript);
		if (!token_opt) {
			p.error(tokens.meta(), errors::expected_close_subscript);
			return nullptr;
		}

		tokens.eat();
		return make_shared<SubscriptNode>(subscript_meta, p.scope(), move(lhs), move(index));
	}

	// <member-access> ::= <expr> "." <identifier>
	static shared_ptr<ASTNode> parseAccessMember(Parser& p, TokenStream& tokens, shared_ptr<ASTNode> lhs) {
		auto token_opt = getTokenWithBuiltin(tokens, Builtin::AccessMember);
		if (!token_opt) {
			p.error(tokens.meta(), errors::expected_access_member);
			return nullptr;
		}

		auto access_meta = token_opt->meta();
		tokens.eat();

		token_opt = getTokenWithType(tokens, TokenType::Identifier);
		if (!token_opt) {
			p.error(tokens.meta(), errors::expected_identifier);
			return nullptr;
		}

		tokens.eat();

		return make_shared<AccessMemberNode>(access_meta, p.scope(), move(lhs), token_opt->text());
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
					expr = make_shared<BooleanLiteralNode>(token.meta(), p.scope(), true);
				} else if (isBuiltin(token_text, Builtin::FalseLiteral)) {
					tokens.eat();
					expr = make_shared<BooleanLiteralNode>(token.meta(), p.scope(), false);
				} else if (isBuiltin(token_text, Builtin::NullLiteral)) {
					tokens.eat();
					expr = make_shared<NullLiteralNode>(token.meta(), p.scope());
				} else if (isBuiltin(token_text, Builtin::FunctionDeclaration)) {
					expr = parseFunctionDeclaration(p, tokens);
				} else if (isBuiltin(token_text, Builtin::Return)) {
					auto return_meta = token.meta();
					tokens.eat();

					auto rhs = parseExpression(p, tokens);
					expr = make_shared<ReturnNode>(return_meta, p.scope(), rhs);
				} else if (isBuiltin(token_text, Builtin::OpenParen)) {
					expr = parseParenthesesExpression(p, tokens);
				} else if (isBuiltin(token_text, Builtin::OpenArrayLiteral)) {
					expr = parseArrayLiteral(p, tokens);
				} else if (isBuiltin(token_text, Builtin::OpenObjectLiteral)) {
					expr = parseObjectLiteral(p, tokens);
				}
			} break;
			case TokenType::Identifier: {
				// TODO: handle closures
				tokens.eat();
				if (p.scope()->contains(token_text)) {
					expr = make_shared<IdentifierNode>(token.meta(), p.scope(), token_text);
				} else {
					p.error(token.meta(), errors::undeclared_identifier + token_text);
					return nullptr;
				}
			} break;
			case TokenType::NumberLiteral: {
				tokens.eat();
				expr = make_shared<NumberLiteralNode>(token.meta(), p.scope(), token_text);
			} break;
			case TokenType::StringLiteral: {
				tokens.eat();
				expr = make_shared<StringLiteralNode>(token.meta(), p.scope(), token_text);
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

		if (!lhs) {
			if (tokens.empty()) {
				// TODO: move this error message to errors.h
				p.error(tokens.meta(), "expression required for prefix unary operator");
				return nullptr;
			}

			auto expr = parseUnaryOperator(p, tokens, nullptr);
			expr = parseBinaryOperator(p, tokens, expr, op_info.precedence);

			return make_shared<UnaryOperatorNode>(token.meta(), p.scope(), op, expr);
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

				if (lhs->isConst(p.scope())) {
					p.error(lhs->meta(), errors::assigning_constant);
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

			lhs = make_shared<BinaryOperatorNode>(token.meta(), p.scope(), op, lhs, rhs);
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

		auto token_opt = getTokenWithType(tokens, TokenType::Identifier);
		if (!token_opt) {
			p.error(declaration_meta, errors::expected_identifier);
			return nullptr;
		}

		auto id = token_opt->text();
		auto scope = p.scope();

		bool added_variable = scope->add(id, { is_const });
		if (!added_variable) {
			p.error(tokens.meta(), errors::redeclaration + id);
			return nullptr;
		}

		tokens.eat();

		shared_ptr<ASTNode> expr = nullptr;

		if (tokens.empty() && is_const) {
			p.error(tokens.meta(), errors::expected_declaration_expression);
			return nullptr;
		}

		if (tokens.hasNext()) {
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

		return make_shared<DeclarationNode>(declaration_meta, move(scope), is_const, id, expr);
	}
};

// <top> ::= <block>
pair<shared_ptr<ASTNode>, int> Parser::parse(TokenStream& tokens) {
	_error_count = 0;

	if (tokens.empty()) {
		return { nullptr, 0 };
	}

	// TODO: this could probably be cleaner
	_scope = Scope::getGlobalScope();

	auto root = ParserHelper::parseBlock(*this, tokens, true);
	return { root, _error_count };
}

void Parser::error(const TokenMetaData& meta, const string& error) {
	++_error_count;
	printError(meta, error);
}

std::shared_ptr<Scope> Parser::scope() {
	return _scope;
}

void Parser::pushScope(bool is_function_scope) {
	_scope = make_shared<Scope>(_scope, is_function_scope);
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
