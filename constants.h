#ifndef _CONSTANTS_H_
#define _CONSTANTS_H_

#include <set>
#include <map>
#include <string>

enum class Builtin {
	Invalid = -1,

	Assignment,
	Dereference,
	StatementDelimiter,
	ArgumentDelimiter,

	OpenParen,
	CloseParen,

	OpenFunctionCall,
	CloseFunctionCall,

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

	GreaterThan,
	GreaterThanOrEqual,

	LessThan,
	LessThanOrEqual,

	EqualTo,
	NotEqualTo
};

//std::ostream& operator<<(std::ostream& out, Builtin builtin);

extern const std::set<std::string> keywords;
extern const std::map<Builtin, std::string> operators;

struct OperatorInfo {
	int precedence;
	bool is_binary;
	bool left_associative;
};

extern const std::map<Builtin, OperatorInfo> operator_info;

bool isBuiltin(const std::string& op);
bool isBuiltin(const std::string& op, Builtin builtin);

Builtin getBuiltin(const std::string& op);
std::string getBuiltinString(Builtin builtin);

OperatorInfo getOperatorInfo(Builtin builtin);

extern const std::set<char> symbol_chars;

#endif
