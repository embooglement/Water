#include <cmath>
#include "astnode.h"

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

shared_ptr<Value> ASTNode::evaluate() const {
	throw EvaluationError("evaluate not implemented");
}

/* ===== IdentifierNode ===== */

IdentifierNode::IdentifierNode(const TokenMetaData& meta, const string& identifier)
	: ASTNode(meta), _identifier(identifier) {}

void IdentifierNode::output(ostream& out, int indent) const {
	indentOutput(out, indent);
	out << _identifier;
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

shared_ptr<Value> NumberLiteralNode::evaluate() const {
	return make_shared<NumberValue>(_number);
}

/* ===== StringLiteralNode ===== */

StringLiteralNode::StringLiteralNode(const TokenMetaData& meta, const string& str)
	: ASTNode(meta), _str(str) {}

void StringLiteralNode::output(ostream& out, int indent) const {
	indentOutput(out, indent);
	out << "\"" << _str << "\"";
}

shared_ptr<Value> StringLiteralNode::evaluate() const {
	return make_shared<StringValue>(_str);
}

/* ===== BooleanLiteralNode ===== */
BooleanLiteralNode::BooleanLiteralNode(const TokenMetaData& meta, bool boolean)
	: ASTNode(meta), _boolean(boolean) {}

void BooleanLiteralNode::output(ostream& out, int indent) const {
	indentOutput(out, indent);
	out << (_boolean ? "true" : "false");
}

shared_ptr<Value> BooleanLiteralNode::evaluate() const {
	return make_shared<BooleanValue>(_boolean);
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

shared_ptr<Value> BinaryOperatorNode::evaluate() const {
	auto as_number = [](const shared_ptr<Value>& var) {
		return static_pointer_cast<NumberValue>(var)->valueOf();
	};

	auto lhs = _left->evaluate();
	auto rhs = _right->evaluate();

	switch (_op) {
		// Arithmetic
		case Builtin::Addition:
			return make_shared<NumberValue>(as_number(lhs) + as_number(rhs));
		case Builtin::Subtraction:
			return make_shared<NumberValue>(as_number(lhs) - as_number(rhs));
		case Builtin::Multiplication:
			return make_shared<NumberValue>(as_number(lhs) * as_number(rhs));
		case Builtin::Division:
			return make_shared<NumberValue>(as_number(lhs) / as_number(rhs));
			break;
		case Builtin::Modulus:
			return make_shared<NumberValue>(fmod(as_number(lhs), as_number(rhs)));
		case Builtin::Exponent:
			return make_shared<NumberValue>(pow(as_number(lhs), as_number(rhs)));

		// Comparisons
		case Builtin::LessThan:
			return make_shared<BooleanValue>(as_number(lhs) < as_number(rhs));
		case Builtin::LessThanOrEqual:
			return make_shared<BooleanValue>(as_number(lhs) <= as_number(rhs));
		case Builtin::GreaterThan:
			return make_shared<BooleanValue>(as_number(lhs) > as_number(rhs));
		case Builtin::GreaterThanOrEqual:
			return make_shared<BooleanValue>(as_number(lhs) >= as_number(rhs));
		case Builtin::EqualTo:
			return make_shared<BooleanValue>(as_number(lhs) == as_number(rhs));
		case Builtin::NotEqualTo:
			return make_shared<BooleanValue>(as_number(lhs) != as_number(rhs));

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

shared_ptr<Value> UnaryOperatorNode::evaluate() const {
	double result;
	auto expr = _expr->evaluate();

	if (expr->type() != ValueType::Number) {
		throw EvaluationError("type of operand of unary operator invalid");
	}

	double expr_value = static_pointer_cast<NumberValue>(expr)->valueOf();

	switch (_op) {
		case Builtin::Negation:
			result = -expr_value;
			break;
		default:
			throw EvaluationError("operator not implemented");
	}

	return make_shared<NumberValue>(result);
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

shared_ptr<Value> FunctionCallNode::evaluate() const {
	auto id = static_pointer_cast<IdentifierNode>(_caller);
	if (id->str() == "print") {
		for (auto&& argument : _arguments) {
			auto expr = argument->evaluate();
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

BlockNode::BlockNode(const TokenMetaData& meta, vector<shared_ptr<ASTNode>> statements)
	: ASTNode(meta), _statements(statements) {}

void BlockNode::output(ostream& out, int indent) const {
	indentOutput(out, indent);
	out << "(block" << endl;

	for (auto&& statement : _statements) {
		statement->output(out, indent + 1);
		out << endl;
	}

	indentOutput(out, indent);
	out << ")";
}

shared_ptr<Value> BlockNode::evaluate() const {
	for (auto&& statement : _statements) {
		statement->evaluate();
	}

	return nullptr;
}

/* ===== IfStatementNode ===== */

IfStatementNode::IfStatementNode(const TokenMetaData& meta, std::shared_ptr<ASTNode> condition, std::shared_ptr<ASTNode> then_statement, std::shared_ptr<ASTNode> else_statement)
	: ASTNode(meta), _condition(condition), _then(then_statement), _else(else_statement) {}

void IfStatementNode::output(std::ostream& out, int indent) const {
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

shared_ptr<Value> IfStatementNode::evaluate() const {
	auto condition = _condition->evaluate();

	if (!condition) {
		cout << "condition is null" << endl;
	}

	if (condition->type() != ValueType::Boolean) {
		throw EvaluationError("type of condition is not Boolean");
	}

	bool condition_value = static_pointer_cast<BooleanValue>(condition)->valueOf();
	if (condition_value) {
		_then->evaluate();
	} else {
		_else->evaluate();
	}

	return nullptr;
}
