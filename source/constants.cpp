#include "constants.h"

using namespace std;

const map<Builtin, string> builtins = {
	{ Builtin::Assignment, "=" },
	{ Builtin::AccessMember, "." },
	{ Builtin::StatementDelimiter, ";" },
	{ Builtin::ArgumentDelimiter, "," },

	{ Builtin::OpenParen, "(" },
	{ Builtin::CloseParen, ")" },

	{ Builtin::OpenFunctionCall, "(" },
	{ Builtin::CloseFunctionCall, ")" },

	{ Builtin::OpenControlFlowCondition, "(" },
	{ Builtin::CloseControlFlowCondition, ")" },

	{ Builtin::OpenIndex, "[" },
	{ Builtin::CloseIndex, "]" },

	{ Builtin::OpenBlock, "{" },
	{ Builtin::CloseBlock, "}" },

	{ Builtin::Addition, "+" },
	{ Builtin::AdditionAssignment, "+=" },
	{ Builtin::Increment, "++" },

	{ Builtin::Subtraction, "-" },
	{ Builtin::SubtractionAssignment, "-=" },
	{ Builtin::Decrement, "--" },
	{ Builtin::Negation, "-" },

	{ Builtin::Multiplication, "*" },
	{ Builtin::MultiplicationAssignment, "*=" },

	{ Builtin::Division, "/" },
	{ Builtin::DivisionAssignment, "/=" },

	{ Builtin::Modulus, "%" },
	{ Builtin::ModulusAssignment, "%=" },

	{ Builtin::Exponent, "^" },
	{ Builtin::ExponentAssignment, "^=" },

	{ Builtin::LessThan, "<" },
	{ Builtin::LessThanOrEqual, "<=" },

	{ Builtin::GreaterThan, ">" },
	{ Builtin::GreaterThanOrEqual, ">=" },

	{ Builtin::EqualTo, "==" },
	{ Builtin::NotEqualTo, "!=" },

	{ Builtin::LogicalAnd, "and" },
	{ Builtin::LogicalOr, "or" },
	{ Builtin::LogicalNot, "not" },

	{ Builtin::Exists, "exists" },

	{ Builtin::VariableDeclarator, "var" },
	{ Builtin::ConstantDeclarator, "let" },
	{ Builtin::VariableDeclarationOperator, "=" },

	{ Builtin::IfStatement, "if" },
	{ Builtin::ElseStatement, "else" },

	{ Builtin::TrueLiteral, "true" },
	{ Builtin::FalseLiteral, "false" },

	{ Builtin::FunctionDeclaration, "func" },
	{ Builtin::FunctionOpenArgumentList, "(" },
	{ Builtin::FunctionCloseArgumentList, ")" },
	{ Builtin::Return, "return" }
};

const int assignment_level = 0;
const int logical_or_level = assignment_level + 1;
const int logical_and_level = logical_or_level + 1;
const int equality_level = logical_and_level + 1;
const int ordering_level = equality_level + 1;
const int additive_level = ordering_level + 1;
const int multiplicative_level = additive_level + 1;
const int negation_level = multiplicative_level + 1;
const int exponential_level = negation_level + 1;
const int logical_not_level = exponential_level + 1;
const int existential_level = logical_not_level + 1;
const int incremental_level = existential_level + 1;
const int member_access_level = incremental_level + 1;

const map<Builtin, BuiltinInfo> builtin_info = {
	// { Builtin, BuiltinInfo { is_operator, is_binary, precedence, binding_direction } }
	{ Builtin::Assignment, { true, true, assignment_level, BindingDirection::RightAssociative } },
	{ Builtin::AdditionAssignment, { true, true, assignment_level, BindingDirection::RightAssociative } },
	{ Builtin::SubtractionAssignment, { true, true, assignment_level, BindingDirection::RightAssociative } },
	{ Builtin::MultiplicationAssignment, { true, true, assignment_level, BindingDirection::RightAssociative } },
	{ Builtin::DivisionAssignment, { true, true, assignment_level, BindingDirection::RightAssociative } },
	{ Builtin::ModulusAssignment, { true, true, assignment_level, BindingDirection::RightAssociative } },

	{ Builtin::LogicalOr, { true, true, logical_or_level, BindingDirection::LeftAssociative } },

	{ Builtin::LogicalAnd, { true, true, logical_and_level, BindingDirection::LeftAssociative } },

	{ Builtin::EqualTo, { true, true, equality_level, BindingDirection::LeftAssociative } },
	{ Builtin::NotEqualTo, { true, true, equality_level, BindingDirection::LeftAssociative } },

	{ Builtin::LessThan, { true, true, ordering_level, BindingDirection::LeftAssociative } },
	{ Builtin::LessThanOrEqual, { true, true, ordering_level, BindingDirection::LeftAssociative } },
	{ Builtin::GreaterThan, { true, true, ordering_level, BindingDirection::LeftAssociative } },
	{ Builtin::GreaterThanOrEqual, { true, true, ordering_level, BindingDirection::LeftAssociative } },

	{ Builtin::Addition, { true, true, additive_level, BindingDirection::LeftAssociative } },
	{ Builtin::Subtraction, { true, true, additive_level, BindingDirection::LeftAssociative } },

	{ Builtin::Multiplication, { true, true, multiplicative_level, BindingDirection::LeftAssociative } },
	{ Builtin::Division, { true, true, multiplicative_level, BindingDirection::LeftAssociative } },
	{ Builtin::Modulus, { true, true, multiplicative_level, BindingDirection::LeftAssociative } },

	{ Builtin::Negation, { true, false, negation_level, BindingDirection::Prefix } },

	{ Builtin::Exponent, { true, true, exponential_level, BindingDirection::RightAssociative } },

	{ Builtin::LogicalNot, { true, false, logical_not_level, BindingDirection::Prefix } },

	{ Builtin::Exists, { true, false, existential_level, BindingDirection::Postfix } },

	{ Builtin::Increment, { true, false, incremental_level, BindingDirection::Prefix } },
	{ Builtin::Decrement, { true, false, incremental_level, BindingDirection::Prefix } },

	{ Builtin::AccessMember, { true, true, member_access_level, BindingDirection::LeftAssociative } },
};

bool isBuiltin(const string& builtin_text) {
	for (auto&& builtin_pair : builtins) {
		if (builtin_pair.second == builtin_text) {
			return true;
		}
	}

	return false;
}

bool isBuiltin(const string& op, Builtin builtin) {
	auto it = builtins.find(builtin);
	if (it != end(builtins)) {
		return it->second == op;
	}

	return false;
}

Builtin getBinaryBuiltin(const string& builtin_text) {
	for (auto&& builtin_pair : builtins) {
		if (builtin_pair.second == builtin_text) {
			if (isBinaryOperator(builtin_pair.first)) {
				return builtin_pair.first;
			}
		}
	}

	return Builtin::Invalid;
}

Builtin getUnaryBuiltin(const string& builtin_text) {
	for (auto&& builtin_pair : builtins) {
		if (builtin_pair.second == builtin_text) {
			if (!isBinaryOperator(builtin_pair.first)) {
				return builtin_pair.first;
			}
		}
	}

	return Builtin::Invalid;
}

string getBuiltinString(Builtin builtin) {
	auto it = builtins.find(builtin);

	if (it != end(builtins)) {
		return it->second;
	}

	return "(unknown operator)";
}

BuiltinInfo getBuiltinInfo(Builtin builtin) {
	auto it = builtin_info.find(builtin);

	if (it != end(builtin_info)) {
		return it->second;
	}

	return { false, false, -1, BindingDirection::None };
}

bool isBinaryOperator(Builtin builtin) {
	return getBuiltinInfo(builtin).is_binary;
}

bool isAssignmentOperator(BuiltinInfo builtin_info) {
	return builtin_info.precedence == assignment_level;
}

const auto symbol_chars = ([]() -> set<char> {
	set<char> symbols;

	for (const auto& builtin_pair : builtins) {
		const auto& builtin_text = builtin_pair.second;
		if (!builtin_text.empty() && !isalpha(builtin_text[0])) {
			symbols.insert(begin(builtin_text), end(builtin_text));
		}
	}

	return symbols;
})();

bool isSymbol(char c) {
	return symbol_chars.count(c) > 0;
}

const auto keywords = ([]() -> set<string> {
	set<string> keywords;

	for (const auto& builtin_pair : builtins) {
		const auto& builtin_text = builtin_pair.second;
		if (!builtin_text.empty() && isalpha(builtin_text[0])) {
			keywords.insert(builtin_text);
		}
	}

	return keywords;
})();

bool isKeyword(const string& text) {
	return keywords.count(text) > 0;
}

