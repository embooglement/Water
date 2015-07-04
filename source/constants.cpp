#include "constants.h"

using namespace std;

const set<string> keywords = {
	"print",
	"if", "else", "while", "for",
	"and", "or", "not",
	"true", "false"
};

const map<Builtin, string> operators = {
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

	{ Builtin::IfStatement, "if" },
	{ Builtin::ElseStatement, "else" }
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

bool isBuiltin(const string& op) {
	for (auto&& op_pair : operators) {
		if (op_pair.second == op) {
			return true;
		}
	}

	return false;
}

bool isBuiltin(const string& op, Builtin builtin) {
	auto it = operators.find(builtin);
	if (it != end(operators)) {
		return it->second == op;
	}

	return false;
}

bool isBuiltin(TokenType token_type) {
	return token_type == TokenType::Keyword || token_type == TokenType::Operator;
}

Builtin getBinaryBuiltin(const string& op) {
	for (auto&& op_pair : operators) {
		if (op_pair.second == op) {
			if (isBinaryOperator(op_pair.first)) {
				return op_pair.first;
			}
		}
	}

	return Builtin::Invalid;
}

Builtin getUnaryBuiltin(const string& op) {
	for (auto&& op_pair : operators) {
		if (op_pair.second == op) {
			if (!isBinaryOperator(op_pair.first)) {
				return op_pair.first;
			}
		}
	}

	return Builtin::Invalid;
}

string getBuiltinString(Builtin builtin) {
	auto it = operators.find(builtin);

	if (it != end(operators)) {
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

const auto symbol_chars = ([]() -> set<char> {
	set<char> symbols;

	if (symbols.empty()) {
		for (const auto& op : operators) {
			symbols.insert(op.second.begin(), op.second.end());
		}
	}

	return symbols;
})();
