#include <cmath>
#include "astnode.h"
#include "runtime_errors.h"

using namespace std;

/* ===== Helpers ===== */

void indentOutput(ostream& out, int indent) {
	for (int i = 0; i < indent; ++i) {
		out << "    ";
	}
}

/* ===== Exceptions ===== */

class EvaluationError : public runtime_error {
public:
	EvaluationError(const string& error_message)
		: runtime_error(error_message) {}
};

/* ===== ASTNode ===== */

ASTNode::ASTNode(const TokenMetaData& meta)
	: _meta(meta) {}

const TokenMetaData& ASTNode::meta() const {
	return _meta;
}

shared_ptr<Value> ASTNode::evaluate(shared_ptr<Scope> scope) const {
	throw EvaluationError("evaluate not implemented");
}

/* ===== IdentifierNode ===== */

IdentifierNode::IdentifierNode(const TokenMetaData& meta, const string& identifier)
	: ASTNode(meta), _identifier(identifier) {}

void IdentifierNode::output(ostream& out, int indent) const {
	indentOutput(out, indent);
	out << _identifier;
}

shared_ptr<Value> IdentifierNode::evaluate(shared_ptr<Scope> scope) const {
	return scope->get(_identifier);
}

const string& IdentifierNode::str() const {
	return _identifier;
}

/* ===== NumberLiteralNode ===== */

NumberLiteralNode::NumberLiteralNode(const TokenMetaData& meta, const string& number)
	: ASTNode(meta), _number(stod(number)) {}

void NumberLiteralNode::output(ostream& out, int indent) const {
	indentOutput(out, indent);
	out << _number;
}

shared_ptr<Value> NumberLiteralNode::evaluate(shared_ptr<Scope> scope) const {
	return make_shared<NumberValue>(true, _number);
}

/* ===== StringLiteralNode ===== */

StringLiteralNode::StringLiteralNode(const TokenMetaData& meta, const string& str)
	: ASTNode(meta), _str(str) {}

void StringLiteralNode::output(ostream& out, int indent) const {
	indentOutput(out, indent);
	out << "\"" << _str << "\"";
}

shared_ptr<Value> StringLiteralNode::evaluate(shared_ptr<Scope> scope) const {
	return make_shared<StringValue>(true, _str);
}

/* ===== BooleanLiteralNode ===== */
BooleanLiteralNode::BooleanLiteralNode(const TokenMetaData& meta, bool boolean)
	: ASTNode(meta), _boolean(boolean) {}

void BooleanLiteralNode::output(ostream& out, int indent) const {
	indentOutput(out, indent);
	out << (_boolean ? "true" : "false");
}

shared_ptr<Value> BooleanLiteralNode::evaluate(shared_ptr<Scope> scope) const {
	return make_shared<BooleanValue>(true, _boolean);
}

/* ===== BinaryOperatorNode ===== */

BinaryOperatorNode::BinaryOperatorNode(const TokenMetaData& meta, Builtin op, shared_ptr<ASTNode> left, shared_ptr<ASTNode> right)
	: ASTNode(meta), _op(op), _left(left), _right(right) {}

void BinaryOperatorNode::output(ostream& out, int indent) const {
	indentOutput(out, indent);
	out << "(" << getBuiltinString(_op) << "\n";

	_left->output(out, indent + 1);
	out << "\n";
	_right->output(out, indent + 1);
	out << "\n";

	indentOutput(out, indent);
	out << ")";
}

shared_ptr<Value> BinaryOperatorNode::evaluate(shared_ptr<Scope> scope) const {
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
			throw EvaluationError("operator not implemented");
	}

	return nullptr;
}

/* ===== UnaryOperatorNode ===== */

UnaryOperatorNode::UnaryOperatorNode(const TokenMetaData& meta, Builtin op, shared_ptr<ASTNode> expr)
	: ASTNode(meta), _op(op), _expr(expr) {}

void UnaryOperatorNode::output(ostream& out, int indent) const {
	indentOutput(out, indent);
	out << "(" << getBuiltinString(_op) << "\n";

	_expr->output(out, indent + 1);
	out << "\n";

	indentOutput(out, indent);
	out << ")";
}

shared_ptr<Value> UnaryOperatorNode::evaluate(shared_ptr<Scope> scope) const {
	auto expr = _expr->evaluate(scope);

	switch (_op) {
		// Arithmetic
		case Builtin::Negation:
			return make_shared<NumberValue>(true, -toNumber(expr));

		// Logical
		case Builtin::LogicalNot:
			return make_shared<BooleanValue>(true, !toBoolean(expr));

		default:
			throw EvaluationError("operator not implemented");
	}
}

/* ===== FunctionCallNode ===== */

FunctionCallNode::FunctionCallNode(const TokenMetaData& meta, shared_ptr<ASTNode> caller, vector<shared_ptr<ASTNode>> arguments)
	: ASTNode(meta), _caller(caller), _arguments(arguments) {}

void FunctionCallNode::output(ostream& out, int indent) const {
	indentOutput(out, indent);

	out << "(";
	_caller->output(out, 0);
	out << "\n";

	for (auto&& argument : _arguments) {
		argument->output(out, indent + 1);
		out << "\n";
	}

	indentOutput(out, indent);
	out << ")";
}

shared_ptr<Value> FunctionCallNode::evaluate(shared_ptr<Scope> scope) const {
	auto id = static_pointer_cast<IdentifierNode>(_caller);
	if (id->str() == "print") {
		for (auto&& argument : _arguments) {
			auto expr = argument->evaluate(scope);
			if (expr) {
				expr->output(cout);
				cout << " ";
			} else {
				cout << "(undefined) ";
			}
		}

		cout << endl;
	}

	return nullptr;
}

/* ===== BlockNode ===== */

BlockNode::BlockNode(const TokenMetaData& meta, bool is_new_scope, vector<shared_ptr<ASTNode>> statements)
	: ASTNode(meta), _is_new_scope(is_new_scope), _statements(move(statements)) {}

bool BlockNode::isNewScope() const {
	return _is_new_scope;
}

void BlockNode::output(ostream& out, int indent) const {
	indentOutput(out, indent);

	if (isNewScope()) {
		out << "(block" << endl;
	} else {
		out << "(global scope" << endl;
	}

	for (auto&& statement : _statements) {
		statement->output(out, indent + 1);
		out << endl;
	}

	indentOutput(out, indent);
	out << ")";
}

shared_ptr<Value> BlockNode::evaluate(shared_ptr<Scope> scope) const {
	// TODO: only need to push new state if this is not the global block
	shared_ptr<Scope> block_scope;

	if (isNewScope()) {
		block_scope = scope->push();
	} else {
		block_scope = scope;
	}

	for (auto&& statement : _statements) {
		statement->evaluate(block_scope);
	}

	return nullptr;
}

/* ===== IfStatementNode ===== */

IfStatementNode::IfStatementNode(const TokenMetaData& meta, shared_ptr<ASTNode> condition, shared_ptr<ASTNode> then_statement, shared_ptr<ASTNode> else_statement)
	: ASTNode(meta), _condition(condition), _then(then_statement), _else(else_statement) {}

void IfStatementNode::output(ostream& out, int indent) const {
	indentOutput(out, indent);
	out << "(if" << endl;

	{
		indentOutput(out, indent + 1);
		out << "(condition" << endl;
		_condition->output(out, indent + 2);
		out << endl;
		indentOutput(out, indent + 1);
		out << ")" << endl;
	}

	{
		indentOutput(out, indent + 1);
		out << "(then" << endl;
		_then->output(out, indent + 2);
		out << endl;
		indentOutput(out, indent + 1);
		out << ")" << endl;
	}

	if (_else) {
		indentOutput(out, indent + 1);
		out << "(else" << endl;
		_else->output(out, indent + 2);
		out << endl;
		indentOutput(out, indent + 1);
		out << ")" << endl;
	}

	indentOutput(out, indent);
	out << ")";
}

shared_ptr<Value> IfStatementNode::evaluate(shared_ptr<Scope> scope) const {
	auto condition = _condition->evaluate(scope);

	if (!condition) {
		cout << "condition is null" << endl;
	}

	if (condition->type() != ValueType::Boolean) {
		throw EvaluationError("type of condition is not Boolean");
	}

	bool condition_value = static_pointer_cast<BooleanValue>(condition)->valueOf();
	if (condition_value) {
		_then->evaluate(scope->push());
	} else {
		_else->evaluate(scope->push());
	}

	return nullptr;
}

/* ===== DeclarationNode ===== */

DeclarationNode::DeclarationNode(const TokenMetaData& meta, bool is_const, string identifier, shared_ptr<ASTNode> expr)
	: ASTNode(meta), _is_const(is_const), _identifier(move(identifier)), _expr(move(expr)) {}

void DeclarationNode::output(ostream& out, int indent) const {
	indentOutput(out, indent);
	out << "(decl ";

	if (_is_const) {
		out << "const ";
	}

	out << _identifier << endl;

	_expr->output(out, indent + 1);
	out << endl;

	indentOutput(out, indent);
	out << ")";
}

shared_ptr<Value> DeclarationNode::evaluate(shared_ptr<Scope> scope) const {
	if (_expr) {
		auto expr = _expr->evaluate(scope);
		scope->add(_identifier, expr);
	}

	return nullptr;
}
