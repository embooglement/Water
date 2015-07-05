#ifndef _CONSTANTS_H_
#define _CONSTANTS_H_

#include <set>
#include <map>
#include <string>
#include "token.h"

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

	OpenIndex,
	CloseIndex,

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

	IfStatement,
	ElseStatement,
	WhileStatement,
	ForStatement
};

extern const std::set<std::string> keywords;
extern const std::map<Builtin, std::string> operators;

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
bool isBuiltin(TokenType token_type);

Builtin getBinaryBuiltin(const std::string& op);
Builtin getUnaryBuiltin(const std::string& op);
std::string getBuiltinString(Builtin builtin);

BuiltinInfo getBuiltinInfo(Builtin builtin);

bool isBinaryOperator(Builtin builtin);

extern const std::set<char> symbol_chars;

#endif