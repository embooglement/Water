#ifndef _ASTNODE_H_
#define _ASTNODE_H_

#include <iostream>
#include <vector>
#include <unordered_map>
#include <memory>

#include "constants.h"
#include "token.h"
#include "value.h"
#include "scope.h"

class ASTNode {
public:
	ASTNode(const TokenMetaData& meta, std::shared_ptr<Scope> scope);
	virtual ~ASTNode() {}
	const TokenMetaData& meta() const;
	std::shared_ptr<Scope> scope() const;
	virtual bool isLValue() const;
	virtual bool isConst(const std::shared_ptr<Scope>& scope) const;
	virtual void output(std::ostream& out, int indent = 0) const = 0;
	virtual std::shared_ptr<Value> evaluate() const;
	virtual void assign(std::shared_ptr<Value> rhs) const;
protected:
	TokenMetaData _meta;
	std::shared_ptr<Scope> _scope;
};

class IdentifierNode : public ASTNode {
public:
	IdentifierNode(const TokenMetaData& meta, std::shared_ptr<Scope> scope, std::string identifier);
	virtual bool isLValue() const override;
	virtual bool isConst(const std::shared_ptr<Scope>& scope) const override;
	virtual void output(std::ostream& out, int indent = 0) const override;
	virtual std::shared_ptr<Value> evaluate() const override;
	virtual void assign(std::shared_ptr<Value> rhs) const override;
	const std::string& str() const;
private:
	std::string _identifier;
};

class NumberLiteralNode : public ASTNode {
public:
	NumberLiteralNode(const TokenMetaData& meta, std::shared_ptr<Scope> scope, std::string number);
	virtual void output(std::ostream& out, int indent = 0) const override;
	virtual std::shared_ptr<Value> evaluate() const override;
private:
	double _number;
};

class StringLiteralNode : public ASTNode {
public:
	StringLiteralNode(const TokenMetaData& meta, std::shared_ptr<Scope> scope, std::string str);
	virtual void output(std::ostream& out, int indent = 0) const override;
	virtual std::shared_ptr<Value> evaluate() const override;
private:
	std::string _str;
};

class BooleanLiteralNode : public ASTNode {
public:
	BooleanLiteralNode(const TokenMetaData& meta, std::shared_ptr<Scope> scope, bool boolean);
	virtual void output(std::ostream& out, int indent = 0) const override;
	virtual std::shared_ptr<Value> evaluate() const override;
private:
	bool _boolean;
};

class NullLiteralNode : public ASTNode {
public:
	NullLiteralNode(const TokenMetaData& meta, std::shared_ptr<Scope> scope);
	virtual void output(std::ostream& out, int indent = 0) const override;
	virtual std::shared_ptr<Value> evaluate() const override;
};

class ArrayLiteralNode : public ASTNode {
public:
	ArrayLiteralNode(const TokenMetaData& meta, std::shared_ptr<Scope> scope, std::vector<std::shared_ptr<ASTNode>> elements);
	virtual void output(std::ostream& out, int indent = 0) const override;
	virtual std::shared_ptr<Value> evaluate() const override;
private:
	std::vector<std::shared_ptr<ASTNode>> _elements;
};

class ObjectLiteralNode : public ASTNode {
public:
	ObjectLiteralNode(const TokenMetaData& meta, std::shared_ptr<Scope> scope, std::unordered_map<std::string, std::shared_ptr<ASTNode>> members);
	virtual void output(std::ostream& out, int indent = 0) const override;
	virtual std::shared_ptr<Value> evaluate() const override;
private:
	std::unordered_map<std::string, std::shared_ptr<ASTNode>> _members;
};

class SubscriptNode : public ASTNode {
public:
	SubscriptNode(const TokenMetaData& meta, std::shared_ptr<Scope> scope, std::shared_ptr<ASTNode> lhs, std::shared_ptr<ASTNode> index);
	virtual bool isLValue() const override;
	virtual bool isConst(const std::shared_ptr<Scope>& scope) const override;
	virtual void output(std::ostream& out, int indent = 0) const override;
	virtual std::shared_ptr<Value> evaluate() const override;
	virtual void assign(std::shared_ptr<Value> rhs) const override;
private:
	std::shared_ptr<ASTNode> _lhs;
	std::shared_ptr<ASTNode> _index;
};

class AccessMemberNode : public ASTNode {
public:
	AccessMemberNode(const TokenMetaData& meta, std::shared_ptr<Scope> scope, std::shared_ptr<ASTNode> lhs, std::string member);
	virtual bool isLValue() const override;
	virtual bool isConst(const std::shared_ptr<Scope>& scope) const override;
	virtual void output(std::ostream& out, int indent = 0) const override;
	virtual std::shared_ptr<Value> evaluate() const override;
	virtual void assign(std::shared_ptr<Value> rhs) const override;
private:
	std::shared_ptr<ASTNode> _lhs;
	std::shared_ptr<Value> _member;
};

class BinaryOperatorNode : public ASTNode {
public:
	BinaryOperatorNode(const TokenMetaData& meta, std::shared_ptr<Scope> scope, Builtin op, std::shared_ptr<ASTNode> left, std::shared_ptr<ASTNode> right);
	virtual void output(std::ostream& out, int indent = 0) const override;
	virtual std::shared_ptr<Value> evaluate() const override;
private:
	Builtin _op;
	std::shared_ptr<ASTNode> _left;
	std::shared_ptr<ASTNode> _right;
};

class UnaryOperatorNode : public ASTNode {
public:
	UnaryOperatorNode(const TokenMetaData& meta, std::shared_ptr<Scope> scope, Builtin op, std::shared_ptr<ASTNode> expr);
	virtual void output(std::ostream& out, int indent = 0) const override;
	virtual std::shared_ptr<Value> evaluate() const override;
private:
	Builtin _op;
	std::shared_ptr<ASTNode> _expr;
};

class FunctionCallNode : public ASTNode {
public:
	FunctionCallNode(const TokenMetaData& meta, std::shared_ptr<Scope> scope, std::shared_ptr<ASTNode> caller, std::vector<std::shared_ptr<ASTNode>> arguments);
	virtual void output(std::ostream& out, int indent = 0) const override;
	virtual std::shared_ptr<Value> evaluate() const override;
private:
	std::shared_ptr<ASTNode> _caller;
	std::vector<std::shared_ptr<ASTNode>> _arguments;
};

class BlockNode : public ASTNode {
public:
	BlockNode(const TokenMetaData& meta, std::shared_ptr<Scope> scope, bool is_new_scope, std::vector<std::shared_ptr<ASTNode>> statements);
	bool isNewScope() const;
	virtual void output(std::ostream& out, int indent = 0) const override;
	virtual std::shared_ptr<Value> evaluate() const override;
private:
	bool _is_new_scope;
	std::vector<std::shared_ptr<ASTNode>> _statements;
};

class IfStatementNode : public ASTNode {
public:
	IfStatementNode(const TokenMetaData& meta, std::shared_ptr<Scope> scope, std::shared_ptr<ASTNode> condition, std::shared_ptr<ASTNode> if_block, std::shared_ptr<ASTNode> else_block);
	virtual void output(std::ostream& out, int indent = 0) const override;
	virtual std::shared_ptr<Value> evaluate() const override;
private:
	std::shared_ptr<ASTNode> _condition;
	std::shared_ptr<ASTNode> _then;
	std::shared_ptr<ASTNode> _else;
};

class WhileStatementNode : public ASTNode {
public:
	WhileStatementNode(const TokenMetaData& meta, std::shared_ptr<Scope> scope, std::shared_ptr<ASTNode> condition, std::shared_ptr<ASTNode> loop_block);
	virtual void output(std::ostream& out, int indent = 0) const override;
	virtual std::shared_ptr<Value> evaluate() const override;
private:
	std::shared_ptr<ASTNode> _condition;
	std::shared_ptr<ASTNode> _loop;
};

class ForStatementNode : public ASTNode {
public:
	ForStatementNode(const TokenMetaData& meta, std::shared_ptr<Scope> scope, bool is_const, std::string iterator_name, std::shared_ptr<ASTNode> array_expr, std::shared_ptr<ASTNode> loop_block);
	virtual void output(std::ostream& out, int indent = 0) const override;
	virtual std::shared_ptr<Value> evaluate() const override;
private:
	bool _is_const;
	std::string _iterator_name;
	std::shared_ptr<ASTNode> _array;
	std::shared_ptr<ASTNode> _loop;
};

class DeclarationNode : public ASTNode {
public:
	DeclarationNode(const TokenMetaData& meta, std::shared_ptr<Scope> scope, bool is_const, std::string identifier, std::shared_ptr<ASTNode> expr);
	virtual void output(std::ostream& out, int indent = 0) const override;
	virtual std::shared_ptr<Value> evaluate() const override;
private:
	bool _is_const;
	std::string _identifier;
	std::shared_ptr<ASTNode> _expr;
};

class FunctionDeclarationNode : public ASTNode {
public:
	FunctionDeclarationNode(const TokenMetaData& meta, std::shared_ptr<Scope> scope, std::string identifier, std::vector<std::string> argument_names, std::shared_ptr<ASTNode> body);
	virtual void output(std::ostream& out, int indent = 0) const override;
	virtual std::shared_ptr<Value> evaluate() const override;
private:
	std::string _identifier;
	std::vector<std::string> _argument_names;
	std::shared_ptr<ASTNode> _body;
};

class ReturnNode : public ASTNode {
public:
	ReturnNode(const TokenMetaData& meta, std::shared_ptr<Scope> scope, std::shared_ptr<ASTNode> expr);
	virtual void output(std::ostream& out, int indent = 0) const override;
	virtual std::shared_ptr<Value> evaluate() const override;
private:
	std::shared_ptr<ASTNode> _expr;
};

class BreakNode : public ASTNode {
public:
	BreakNode(const TokenMetaData& meta, std::shared_ptr<Scope> scope);
	virtual void output(std::ostream& out, int indent = 0) const override;
	virtual std::shared_ptr<Value> evaluate() const override;
};

class ContinueNode : public ASTNode {
public:
	ContinueNode(const TokenMetaData& meta, std::shared_ptr<Scope> scope);
	virtual void output(std::ostream& out, int indent = 0) const override;
	virtual std::shared_ptr<Value> evaluate() const override;
};

#endif
