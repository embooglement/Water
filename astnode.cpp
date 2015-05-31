#include "astnode.h"

using namespace std;

/* ===== Helpers ===== */

void indentOutput(ostream& out, int indent) {
	for (int i = 0; i < indent; ++i) {
		out << "    ";
	}
}

/* ===== ASTNode ===== */

ASTNode::ASTNode(const TokenMetaData& meta)
	: _meta(meta) {}

const TokenMetaData& ASTNode::meta() const {
	return _meta;
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

/* ===== BinaryOperatorNode ===== */

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
