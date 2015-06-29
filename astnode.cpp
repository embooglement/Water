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
	double result;
	auto lhs = _left->evaluate();
	auto rhs = _right->evaluate();

	if (lhs->type() != ValueType::Number) {
		throw EvaluationError("type of left hand side of binary operator invalid");
	}

	if (rhs->type() != ValueType::Number) {
		throw EvaluationError("type of right hand side of binary operator invalid");
	}

	double lhs_value = static_pointer_cast<NumberValue>(lhs)->valueOf();
	double rhs_value = static_pointer_cast<NumberValue>(rhs)->valueOf();

	switch (_op) {
		case Builtin::Addition:
			result = lhs_value + rhs_value;
			break;
		case Builtin::Subtraction:
			result = lhs_value - rhs_value;
			break;
		case Builtin::Multiplication:
			result = lhs_value * rhs_value;
			break;
		case Builtin::Division:
			result = lhs_value / rhs_value;
			break;
		case Builtin::Modulus:
			result = fmod(lhs_value, rhs_value);
			break;
		case Builtin::Exponent:
			result = pow(lhs_value, rhs_value);
			break;
		default:
			throw EvaluationError("operator not implemented");
	}

	return make_shared<NumberValue>(result);
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

/* ===== BlockNode ===== */

BlockNode::BlockNode(const TokenMetaData& meta, vector<shared_ptr<ASTNode>> statements)
	: ASTNode(meta), _statements(statements) {}

void BlockNode::output(ostream& out, int indent) const {
	indentOutput(out, indent);
	out << "(block\n";

	for (auto&& statement : _statements) {
		statement->output(out, indent + 1);
		out << "\n";
	}

	indentOutput(out, indent);
	out << ")" << endl;
}

/* ===== StatementkNode ===== */

StatementNode::StatementNode(const TokenMetaData& meta, shared_ptr<ASTNode> statement)
	: ASTNode(meta), _statement(statement) {}

void StatementNode::output(ostream& out, int indent) const {
	indentOutput(out, indent);
	out << "(statement\n";
	_statement->output(out, indent + 1);

	indentOutput(out, indent);
	out << ")";
}
