#ifndef _ASTNODE_H_
#define _ASTNODE_H_

#include <iostream>
#include <vector>
#include <memory>

#include "constants.h"
#include "token.h"
#include "value.h"

void indentOutput(std::ostream& out, int indent);

class ASTNode {
public:
	ASTNode(const TokenMetaData& meta);
	virtual void output(std::ostream& out, int indent = 0) const = 0;
	virtual ~ASTNode() {}
	const TokenMetaData& meta() const;
	virtual std::shared_ptr<Value> evaluate() const;
protected:
	TokenMetaData _meta;
};

class IdentifierNode : public ASTNode {
public:
	IdentifierNode(const TokenMetaData& meta, const std::string& identifier);
	virtual void output(std::ostream& out, int indent = 0) const;
private:
	std::string _identifier;
};

class NumberLiteralNode : public ASTNode {
public:
	NumberLiteralNode(const TokenMetaData& meta, const std::string& number);
	virtual void output(std::ostream& out, int indent = 0) const;
	virtual std::shared_ptr<Value> evaluate() const override;
private:
	double _number;
};

class StringLiteralNode : public ASTNode {
public:
	StringLiteralNode(const TokenMetaData& meta, const std::string& str);
	virtual void output(std::ostream& out, int indent = 0) const;
private:
	std::string _str;
};

class BinaryOperatorNode : public ASTNode {
public:
	BinaryOperatorNode(const TokenMetaData& meta, Builtin op, std::shared_ptr<ASTNode> left, std::shared_ptr<ASTNode> right);
	virtual void output(std::ostream& out, int indent = 0) const;
	virtual std::shared_ptr<Value> evaluate() const override;
private:
	Builtin _op;
	std::shared_ptr<ASTNode> _left;
	std::shared_ptr<ASTNode> _right;
};

class UnaryOperatorNode : public ASTNode {
public:
	UnaryOperatorNode(const TokenMetaData& meta, Builtin op, std::shared_ptr<ASTNode> expr);
	virtual void output(std::ostream& out, int indent = 0) const;
	virtual std::shared_ptr<Value> evaluate() const override;
private:
	Builtin _op;
	std::shared_ptr<ASTNode> _expr;
};

class FunctionCallNode : public ASTNode {
public:
	FunctionCallNode(const TokenMetaData& meta, std::shared_ptr<ASTNode> caller, std::vector<std::shared_ptr<ASTNode>> arguments);
	virtual void output(std::ostream& out, int indent = 0) const;
private:
	std::shared_ptr<ASTNode> _caller;
	std::vector<std::shared_ptr<ASTNode>> _arguments;
};

class BlockNode : public ASTNode {
public:
	BlockNode(const TokenMetaData& meta, std::vector<std::shared_ptr<ASTNode>> statements);
	virtual void output(std::ostream& out, int indent = 0) const;
private:
	 std::vector<std::shared_ptr<ASTNode>> _statements;
};

class StatementNode : public ASTNode {
public:
	StatementNode(const TokenMetaData& meta, std::shared_ptr<ASTNode> statement);
	virtual void output(std::ostream& out, int indent = 0) const;
private:
	std::shared_ptr<ASTNode> _statement;
};

class ControlFlowNode : public ASTNode {
	// TODO: implement
};

class DeclarationNode : public ASTNode {
	// TODO: implement
};

class AssignmentNode : public ASTNode {
	// TODO: implement
};

class AssignmentBinaryOpNode : public AssignmentNode {
	// TODO: implement
};

class AssignmentUnaryOpNode : public AssignmentNode {
	// TODO: implement
};

#endif
