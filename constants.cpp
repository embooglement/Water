#include "constants.h"

using namespace std;

const set<string> keywords = {
	"print",
	"if", "else", "while", "for",
	"and", "or", "not"
};

const map<Builtin, string> operators = {
	{ Builtin::Assignment, "=" },
	{ Builtin::Dereference, "." },
	{ Builtin::StatementDelimiter, ";" },
	{ Builtin::ArgumentDelimiter, "," },

	{ Builtin::OpenParen, "(" },
	{ Builtin::CloseParen, ")" },

	{ Builtin::OpenFunctionCall, "(" },
	{ Builtin::CloseFunctionCall, ")" },

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
	{ Builtin::NotEqualTo, "!=" }
};

const map<Builtin, OperatorInfo> operator_info = {
	// { Operator, OperatorInfo { precedence, is_binary, left_assosciative } }
	{ Builtin::Addition, { 1, true, true } },
	{ Builtin::Subtraction, { 1, true, true } },

	{ Builtin::Multiplication, { 2, true, true } },
	{ Builtin::Division, { 2, true, true } },
	{ Builtin::Modulus, { 2, true, true } },

	{ Builtin::Negation, { 3, false, false } },

	{ Builtin::Exponent, { 4, true, false } },

	{ Builtin::LessThan, { 0, true, true } },
	{ Builtin::LessThanOrEqual, { 0, true, true } },

	{ Builtin::GreaterThan, { 0, true, true } },
	{ Builtin::GreaterThanOrEqual, { 0, true, true } },

	{ Builtin::EqualTo, { 0, true, true } },
	{ Builtin::NotEqualTo, { 0, true, true } }
};

bool isBuiltin(const string& op) {
	return getBuiltin(op) != Builtin::Invalid;
}

bool isBuiltin(const string& op, Builtin builtin) {
	auto it = operators.find(builtin);
	if (it != end(operators)) {
		return it->second == op;
	}

	return false;
}

Builtin getBuiltin(const string& op) {
	for (auto&& op_pair : operators) {
		if (op_pair.second == op) {
			return op_pair.first;
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

OperatorInfo getOperatorInfo(Builtin builtin) {
	auto it = operator_info.find(builtin);

	if (it != end(operator_info)) {
		return it->second;
	}

	return { -1, false, false };
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
