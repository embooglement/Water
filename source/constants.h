#ifndef _CONSTANTS_H_
#define _CONSTANTS_H_

#include <set>
#include <map>
#include <string>
#include "token.h"

const static std::string return_value_alias = "<return-value>";

enum class Builtin {
	Invalid = -1,

	Assignment,
	AccessMember,
	StatementDelimiter,
	ArgumentDelimiter,

	OpenParen,
	CloseParen,

	OpenFunctionCall,
	CloseFunctionCall,

	OpenControlFlowCondition,
	CloseControlFlowCondition,

	OpenBlock,
	CloseBlock,

	Addition,
	AdditionAssignment,
	Increment,

	Subtraction,
	SubtractionAssignment,
	Decrement,
	Negation,

	Multiplication,
	MultiplicationAssignment,

	Division,
	DivisionAssignment,

	Modulus,
	ModulusAssignment,

	Exponent,
	ExponentAssignment,

	LessThan,
	LessThanOrEqual,

	GreaterThan,
	GreaterThanOrEqual,

	EqualTo,
	NotEqualTo,

	LogicalAnd,
	LogicalOr,
	LogicalNot,

	Exists,

	VariableDeclarator,
	ConstantDeclarator,
	VariableDeclarationOperator,

	IfStatement,
	ElseStatement,
	WhileStatement,
	ForStatement,

	TrueLiteral,
	FalseLiteral,
	NullLiteral,

	FunctionDeclaration,
	FunctionOpenArgumentList,
	FunctionCloseArgumentList,
	Return,

	OpenArrayLiteral,
	CloseArrayLiteral,
	OpenSubscript,
	CloseSubscript,
	ElementDelimiter
};

enum class BindingDirection {
	None,
	LeftAssociative,
	RightAssociative,
	Prefix,
	Postfix
};

struct BuiltinInfo {
	bool is_operator;
	bool is_binary;
	int precedence;
	BindingDirection binding_direction;
};

extern const std::map<Builtin, BuiltinInfo> builtin_info;

bool isBuiltin(const std::string& op);
bool isBuiltin(const std::string& op, Builtin builtin);

Builtin getBinaryBuiltin(const std::string& op);
Builtin getUnaryBuiltin(const std::string& op);
std::string getBuiltinString(Builtin builtin);

BuiltinInfo getBuiltinInfo(Builtin builtin);

bool isBinaryOperator(Builtin builtin);
bool isAssignmentOperator(BuiltinInfo builtin_info);

bool isSymbol(char c);
bool isKeyword(const std::string& text);

#endif
