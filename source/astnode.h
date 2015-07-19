#ifndef _ASTNODE_H_
#define _ASTNODE_H_

#include <iostream>
#include <vector>
#include <memory>

#include "constants.h"
#include "token.h"
#include "value.h"
#include "scope.h"

void indentOutput(std::ostream& out, int indent);

class ASTNode {
public:
	ASTNode(const TokenMetaData& meta);
	virtual ~ASTNode() {}
	const TokenMetaData& meta() const;
	virtual void output(std::ostream& out, int indent = 0) const = 0;
	virtual std::shared_ptr<Value> evaluate(std::shared_ptr<Scope>& scope) const;
protected:
	TokenMetaData _meta;
};

class IdentifierNode : public ASTNode {
public:
	IdentifierNode(const TokenMetaData& meta, std::string identifier);
	virtual void output(std::ostream& out, int indent = 0) const override;
	virtual std::shared_ptr<Value> evaluate(std::shared_ptr<Scope>& scope) const override;
	const std::string& str() const;
private:
	std::string _identifier;
};

class NumberLiteralNode : public ASTNode {
public:
	NumberLiteralNode(const TokenMetaData& meta, std::string number);
	virtual void output(std::ostream& out, int indent = 0) const override;
	virtual std::shared_ptr<Value> evaluate(std::shared_ptr<Scope>& scope) const override;
private:
	double _number;
};

class StringLiteralNode : public ASTNode {
public:
	StringLiteralNode(const TokenMetaData& meta, std::string str);
	virtual void output(std::ostream& out, int indent = 0) const override;
	virtual std::shared_ptr<Value> evaluate(std::shared_ptr<Scope>& scope) const override;
private:
	std::string _str;
};

class BooleanLiteralNode : public ASTNode {
public:
	BooleanLiteralNode(const TokenMetaData& meta, bool boolean);
	virtual void output(std::ostream& out, int indent = 0) const override;
	virtual std::shared_ptr<Value> evaluate(std::shared_ptr<Scope>& scope) const override;
private:
	bool _boolean;
};

class BinaryOperatorNode : public ASTNode {
public:
	BinaryOperatorNode(const TokenMetaData& meta, Builtin op, std::shared_ptr<ASTNode> left, std::shared_ptr<ASTNode> right);
	virtual void output(std::ostream& out, int indent = 0) const override;
	virtual std::shared_ptr<Value> evaluate(std::shared_ptr<Scope>& scope) const override;
private:
	Builtin _op;
	std::shared_ptr<ASTNode> _left;
	std::shared_ptr<ASTNode> _right;
};

class UnaryOperatorNode : public ASTNode {
public:
	UnaryOperatorNode(const TokenMetaData& meta, Builtin op, std::shared_ptr<ASTNode> expr);
	virtual void output(std::ostream& out, int indent = 0) const override;
	virtual std::shared_ptr<Value> evaluate(std::shared_ptr<Scope>& scope) const override;
private:
	Builtin _op;
	std::shared_ptr<ASTNode> _expr;
};

class FunctionCallNode : public ASTNode {
public:
	FunctionCallNode(const TokenMetaData& meta, std::shared_ptr<ASTNode> caller, std::vector<std::shared_ptr<ASTNode>> arguments);
	virtual void output(std::ostream& out, int indent = 0) const override;
	virtual std::shared_ptr<Value> evaluate(std::shared_ptr<Scope>& scope) const override;
private:
	std::shared_ptr<ASTNode> _caller;
	std::vector<std::shared_ptr<ASTNode>> _arguments;
};

class BlockNode : public ASTNode {
public:
	BlockNode(const TokenMetaData& meta, bool is_new_scope, std::vector<std::shared_ptr<ASTNode>> statements);
	bool isNewScope() const;
	virtual void output(std::ostream& out, int indent = 0) const override;
	virtual std::shared_ptr<Value> evaluate(std::shared_ptr<Scope>& scope) const override;
private:
	bool _is_new_scope;
	std::vector<std::shared_ptr<ASTNode>> _statements;
};

class IfStatementNode : public ASTNode {
public:
	IfStatementNode(const TokenMetaData& meta, std::shared_ptr<ASTNode> condition, std::shared_ptr<ASTNode> if_block, std::shared_ptr<ASTNode> else_block);
	virtual void output(std::ostream& out, int indent = 0) const override;
	virtual std::shared_ptr<Value> evaluate(std::shared_ptr<Scope>& scope) const override;
private:
	std::shared_ptr<ASTNode> _condition;
	std::shared_ptr<ASTNode> _then;
	std::shared_ptr<ASTNode> _else;
};

class DeclarationNode : public ASTNode {
public:
	DeclarationNode(const TokenMetaData& meta, bool is_const, std::string identifier, std::shared_ptr<ASTNode> expr);
	virtual void output(std::ostream& out, int indent = 0) const override;
	virtual std::shared_ptr<Value> evaluate(std::shared_ptr<Scope>& scope) const override;
private:
	bool _is_const;
	std::string _identifier;
	std::shared_ptr<ASTNode> _expr;
};

class FunctionDeclarationNode : public ASTNode {
public:
	FunctionDeclarationNode(const TokenMetaData& meta, std::string identifier, std::vector<std::string> argument_names, std::shared_ptr<ASTNode> body);
	virtual void output(std::ostream& out, int indent = 0) const override;
	virtual std::shared_ptr<Value> evaluate(std::shared_ptr<Scope>& scope) const override;
private:
	std::string _identifier;
	std::vector<std::string> _argument_names;
	std::shared_ptr<ASTNode> _body;
};

class ReturnNode : public ASTNode {
public:
	ReturnNode(const TokenMetaData& meta, std::shared_ptr<ASTNode> expr);
	virtual void output(std::ostream& out, int indent = 0) const override;
	virtual std::shared_ptr<Value> evaluate(std::shared_ptr<Scope>& scope) const override;
private:
	std::shared_ptr<ASTNode> _expr;
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
