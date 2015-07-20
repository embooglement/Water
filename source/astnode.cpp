#include <cmath>
#include "astnode.h"
#include "runtime_errors.h"
#include "iohelpers.h"

using namespace std;

/* ===== ASTNode ===== */

ASTNode::ASTNode(const TokenMetaData& meta)
	: _meta(meta) {}

const TokenMetaData& ASTNode::meta() const {
	return _meta;
}

shared_ptr<Value> ASTNode::evaluate(shared_ptr<Scope>& scope) const {
	throw InterpretorError("evaluate not implemented");
}

/* ===== IdentifierNode ===== */

IdentifierNode::IdentifierNode(const TokenMetaData& meta, string identifier)
	: ASTNode(meta), _identifier(move(identifier)) {}

void IdentifierNode::output(ostream& out, int indent) const {
	out << io::indent(indent) << _identifier;
}

shared_ptr<Value> IdentifierNode::evaluate(shared_ptr<Scope>& scope) const {
	return scope->get(_identifier);
}

const string& IdentifierNode::str() const {
	return _identifier;
}

/* ===== NumberLiteralNode ===== */

NumberLiteralNode::NumberLiteralNode(const TokenMetaData& meta, string number)
	: ASTNode(meta), _number(stod(move(number))) {}

void NumberLiteralNode::output(ostream& out, int indent) const {
	out << io::indent(indent) << _number;
}

shared_ptr<Value> NumberLiteralNode::evaluate(shared_ptr<Scope>& scope) const {
	return make_shared<NumberValue>(true, _number);
}

/* ===== StringLiteralNode ===== */

StringLiteralNode::StringLiteralNode(const TokenMetaData& meta, string str)
	: ASTNode(meta), _str(move(str)) {}

void StringLiteralNode::output(ostream& out, int indent) const {
	out << io::indent(indent) << "\"" << _str << "\"";
}

shared_ptr<Value> StringLiteralNode::evaluate(shared_ptr<Scope>& scope) const {
	return make_shared<StringValue>(true, _str);
}

/* ===== BooleanLiteralNode ===== */
BooleanLiteralNode::BooleanLiteralNode(const TokenMetaData& meta, bool boolean)
	: ASTNode(meta), _boolean(boolean) {}

void BooleanLiteralNode::output(ostream& out, int indent) const {
	out << io::indent(indent) << (_boolean ? "true" : "false");
}

shared_ptr<Value> BooleanLiteralNode::evaluate(shared_ptr<Scope>& scope) const {
	return make_shared<BooleanValue>(true, _boolean);
}

/* ===== BinaryOperatorNode ===== */

BinaryOperatorNode::BinaryOperatorNode(const TokenMetaData& meta, Builtin op, shared_ptr<ASTNode> left, shared_ptr<ASTNode> right)
	: ASTNode(meta), _op(op), _left(move(left)), _right(move(right)) {}

void BinaryOperatorNode::output(ostream& out, int indent) const {
	out << io::indent(indent) << "(" << getBuiltinString(_op) << "\n";

	_left->output(out, indent + 1);
	out << "\n";
	_right->output(out, indent + 1);
	out << "\n";

	out << io::indent(indent) << ")";
}

shared_ptr<Value> BinaryOperatorNode::evaluate(shared_ptr<Scope>& scope) const {
	// Short circuit evaluate logical operators
	switch (_op) {
		case Builtin::LogicalAnd:
			if (toBoolean(_left->evaluate(scope))) {
				return make_shared<BooleanValue>(true, toBoolean(_right->evaluate(scope)));
			} else {
				return make_shared<BooleanValue>(true, false);
			}
		case Builtin::LogicalOr:
			if (toBoolean(_left->evaluate(scope))) {
				return make_shared<BooleanValue>(true, true);
			} else {
				return make_shared<BooleanValue>(true, toBoolean(_right->evaluate(scope)));
			}
		default:
			break;
	}

	auto lhs = _left->evaluate(scope);
	auto rhs = _right->evaluate(scope);

	switch (_op) {
		// Arithmetic
		case Builtin::Addition:
			return make_shared<NumberValue>(true, toNumber(lhs) + toNumber(rhs));
		case Builtin::Subtraction:
			return make_shared<NumberValue>(true, toNumber(lhs) - toNumber(rhs));
		case Builtin::Multiplication:
			return make_shared<NumberValue>(true, toNumber(lhs) * toNumber(rhs));
		case Builtin::Division:
			return make_shared<NumberValue>(true, toNumber(lhs) / toNumber(rhs));
			break;
		case Builtin::Modulus:
			return make_shared<NumberValue>(true, fmod(toNumber(lhs), toNumber(rhs)));
		case Builtin::Exponent:
			return make_shared<NumberValue>(true, pow(toNumber(lhs), toNumber(rhs)));

		// Comparisons
		case Builtin::LessThan:
			return make_shared<BooleanValue>(true, toNumber(lhs) < toNumber(rhs));
		case Builtin::LessThanOrEqual:
			return make_shared<BooleanValue>(true, toNumber(lhs) <= toNumber(rhs));
		case Builtin::GreaterThan:
			return make_shared<BooleanValue>(true, toNumber(lhs) > toNumber(rhs));
		case Builtin::GreaterThanOrEqual:
			return make_shared<BooleanValue>(true, toNumber(lhs) >= toNumber(rhs));
		case Builtin::EqualTo:
			return make_shared<BooleanValue>(true, toNumber(lhs) == toNumber(rhs));
		case Builtin::NotEqualTo:
			return make_shared<BooleanValue>(true, toNumber(lhs) != toNumber(rhs));

		default:
			throw InterpretorError("operator not implemented");
	}

	return nullptr;
}

/* ===== UnaryOperatorNode ===== */

UnaryOperatorNode::UnaryOperatorNode(const TokenMetaData& meta, Builtin op, shared_ptr<ASTNode> expr)
	: ASTNode(meta), _op(op), _expr(move(expr)) {}

void UnaryOperatorNode::output(ostream& out, int indent) const {
	out << io::indent(indent) << "(" << getBuiltinString(_op) << "\n";

	_expr->output(out, indent + 1);
	out << "\n";

	out << io::indent(indent) << ")";
}

shared_ptr<Value> UnaryOperatorNode::evaluate(shared_ptr<Scope>& scope) const {
	auto expr = _expr->evaluate(scope);

	switch (_op) {
		// Arithmetic
		case Builtin::Negation:
			return make_shared<NumberValue>(true, -toNumber(expr));

		// Logical
		case Builtin::LogicalNot:
			return make_shared<BooleanValue>(true, !toBoolean(expr));

		default:
			throw InterpretorError("operator not implemented");
	}
}

/* ===== FunctionCallNode ===== */

FunctionCallNode::FunctionCallNode(const TokenMetaData& meta, shared_ptr<ASTNode> caller, vector<shared_ptr<ASTNode>> arguments)
	: ASTNode(meta), _caller(move(caller)), _arguments(move(arguments)) {}

void FunctionCallNode::output(ostream& out, int indent) const {
	out << io::indent(indent) << "(";
	_caller->output(out, 0);
	out << endl;

	for (auto&& argument : _arguments) {
		argument->output(out, indent + 1);
		out << "\n";
	}

	out << io::indent(indent) << ")";
}

shared_ptr<Value> FunctionCallNode::evaluate(shared_ptr<Scope>& scope) const {
	auto caller = _caller->evaluate(scope);
	if (caller->type() != ValueType::Function) {
		throw TypeError("Expression is not of type Function");
	}

	auto func = static_pointer_cast<FunctionValue>(caller);
	vector<shared_ptr<Value>> arguments;

	for (auto&& argument_node : _arguments) {
		arguments.push_back(argument_node->evaluate(scope));
	}

	return func->call(scope, move(arguments));
}

/* ===== BlockNode ===== */

BlockNode::BlockNode(const TokenMetaData& meta, bool is_new_scope, vector<shared_ptr<ASTNode>> statements)
	: ASTNode(meta), _is_new_scope(is_new_scope), _statements(move(statements)) {}

bool BlockNode::isNewScope() const {
	return _is_new_scope;
}

void BlockNode::output(ostream& out, int indent) const {
	out << io::indent(indent);

	if (isNewScope()) {
		out << "(block" << endl;
	} else {
		out << "(global scope" << endl;
	}

	for (auto&& statement : _statements) {
		statement->output(out, indent + 1);
		out << endl;
	}

	out << io::indent(indent) << ")";
}

shared_ptr<Value> BlockNode::evaluate(shared_ptr<Scope>& scope) const {
	shared_ptr<Scope> block_scope;

	if (isNewScope()) {
		block_scope = scope->createNestedScope();
	} else {
		block_scope = scope;
	}

	for (auto&& statement : _statements) {
		auto val = statement->evaluate(block_scope);

		if (val && val->type() == ValueType::Sentinel) {
			auto sentinel = static_pointer_cast<SentinelValue>(val);
			if (sentinel->isReturn()) {
				return val;
			}
		}
	}

	return nullptr;
}

/* ===== IfStatementNode ===== */

IfStatementNode::IfStatementNode(const TokenMetaData& meta, shared_ptr<ASTNode> condition, shared_ptr<ASTNode> then_statement, shared_ptr<ASTNode> else_statement)
	: ASTNode(meta), _condition(move(condition)), _then(move(then_statement)), _else(move(else_statement)) {}

void IfStatementNode::output(ostream& out, int indent) const {
	out << io::indent(indent) << "(if" << endl;

	{
		out << io::indent(indent + 1) << "(condition" << endl;
		_condition->output(out, indent + 2);
		out << endl;
		out << io::indent(indent + 1) << ")" << endl;
	}

	{
		out << io::indent(indent + 1) << "(then" << endl;
		_then->output(out, indent + 2);
		out << endl;
		out << io::indent(indent + 1) << ")" << endl;
	}

	if (_else) {
		out << io::indent(indent + 1) << "(else" << endl;
		_else->output(out, indent + 2);
		out << endl;
		out << io::indent(indent + 1) << ")" << endl;
	}

	out << io::indent(indent) << ")";
}

shared_ptr<Value> IfStatementNode::evaluate(shared_ptr<Scope>& scope) const {
	auto condition = _condition->evaluate(scope);
	if (!condition) {
		throw InterpretorError("condition is null");
	}

	if (condition->type() != ValueType::Boolean) {
		throw TypeError("Condition is not of type Boolean");
	}

	auto if_block_scope = scope->createNestedScope();
	bool condition_value = static_pointer_cast<BooleanValue>(condition)->valueOf();
	if (condition_value) {
		return _then->evaluate(if_block_scope);
	} else if (_else) {
		return _else->evaluate(if_block_scope);
	}

	return nullptr;
}

/* ===== DeclarationNode ===== */

DeclarationNode::DeclarationNode(const TokenMetaData& meta, bool is_const, string identifier, shared_ptr<ASTNode> expr)
	: ASTNode(meta), _is_const(is_const), _identifier(move(identifier)), _expr(move(expr)) {}

void DeclarationNode::output(ostream& out, int indent) const {
	out << io::indent(indent) << "(decl ";

	if (_is_const) {
		out << "const ";
	}

	out << _identifier << endl;

	_expr->output(out, indent + 1);
	out << endl;

	out << io::indent(indent) << ")";
}

shared_ptr<Value> DeclarationNode::evaluate(shared_ptr<Scope>& scope) const {
	if (_expr) {
		scope->add(_identifier, _expr->evaluate(scope));
	}

	return nullptr;
}

/* ===== FunctionDeclarationNode ===== */

FunctionDeclarationNode::FunctionDeclarationNode(const TokenMetaData& meta, string identifier, vector<string> argument_names, shared_ptr<ASTNode> body)
	: ASTNode(meta), _identifier(move(identifier)), _argument_names(move(argument_names)), _body(move(body)) {}

void FunctionDeclarationNode::output(ostream& out, int indent) const {
	out << io::indent(indent) << "(decl func";

	if (!_identifier.empty()) {
		out << " " << _identifier;
	}

	out << endl << io::indent(indent + 1) << "(";

	int arguments_count = _argument_names.size();

	for (int i = 0; i < arguments_count; ++i) {
		out << _argument_names[i];

		if (i + 1 < arguments_count) {
			out << " ";
		}
	}

	out << ")" << endl;

	_body->output(out, indent + 1);
	out << endl;

	out << io::indent(indent) << ")";
}

shared_ptr<Value> FunctionDeclarationNode::evaluate(shared_ptr<Scope>& scope) const {
	return make_shared<UserDefinedFunctionValue>(_identifier, _argument_names, _body);
}

/* ===== ReturnNode ===== */

ReturnNode::ReturnNode(const TokenMetaData& meta, shared_ptr<ASTNode> expr)
	: ASTNode(meta), _expr(move(expr)) {}

void ReturnNode::output(ostream& out, int indent) const {
	if (!_expr) {
		out << io::indent(indent) << "(return)";
		return;
	}

	out << io::indent(indent) << "(return" << endl;

	_expr->output(out, indent + 1);
	out << endl;

	out << io::indent(indent) << ")";
}

shared_ptr<Value> ReturnNode::evaluate(shared_ptr<Scope>& scope) const {
	if (_expr) {
		auto val = _expr->evaluate(scope); // TODO: check for sentinelvalue?
		if (val) {
			scope->update(return_value_alias, move(val));
		}
	}

	return make_shared<SentinelValue>();
}
