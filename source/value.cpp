#include <sstream>
#include <functional>
#include <cmath>

#include "value.h"
#include "scope.h"
#include "astnode.h"

using namespace std;

/* ===== Conversions ===== */

double toNumber(const std::shared_ptr<Value>& var) {
	return var->valueAs<NumberValue>();
}

std::string toString(const std::shared_ptr<Value>& var) {
	return var->valueAs<StringValue>();
}

bool toBoolean(const std::shared_ptr<Value>& var) {
	return var->valueAs<BooleanValue>();
}

/* ===== Value ===== */

Value::Value(ValueType type, bool is_const)
	: _type(type), _is_const(is_const) {}

ValueType Value::type() const {
	return _type;
}

bool Value::isConst() const {
	return _is_const;
}

/* ===== NullValue ===== */

const std::shared_ptr<NullValue> NullValue::_null_value;

NullValue::NullValue()
	: Value(value_type, true) {}

const shared_ptr<NullValue>& NullValue::get() {
	return _null_value;
}

void NullValue::output(ostream& out) const {
	out << "(null)";
}

nullptr_t NullValue::valueOf() const {
	return nullptr;
}

/* ===== SentinelValue ===== */

SentinelValue::SentinelValue()
	: Value(value_type, true) {}

void SentinelValue::output(ostream& out) const {
	out << "(sentinel)";
}

bool SentinelValue::isReturn() const {
	return true;
}

/* ===== NumberValue ===== */

NumberValue::NumberValue(bool is_const, double number)
	: Value(value_type, is_const), _number(number) {}

void NumberValue::output(std::ostream& out) const {
	out << valueOf();
}

double NumberValue::valueOf() const {
	return _number;
}

/* ===== StringValue ===== */

StringValue::StringValue(bool is_const, string str)
	: Value(value_type, is_const), _str(move(str)) {}

void StringValue::output(std::ostream& out) const {
	out << valueOf();
}

string StringValue::valueOf() const {
	return _str;
}

/* ===== BooleanValue ===== */

BooleanValue::BooleanValue(bool is_const, bool boolean)
	: Value(value_type, is_const), _value(boolean) {}

void BooleanValue::output(ostream& out) const {
	out << (valueOf() ? "true" : "false");
}

bool BooleanValue::valueOf() const {
	return _value;
}

/* ===== FunctionValue ===== */

FunctionValue::FunctionValue(string identifier)
	: Value(value_type, true), _identifier(move(identifier)) {}

void FunctionValue::output(ostream& out) const {
	out << _identifier;
}

const string& FunctionValue::id() const {
	return _identifier;
}

/* ===== UserDefinedFunctionValue ===== */

UserDefinedFunctionValue::UserDefinedFunctionValue(std::string identifier, std::vector<std::string> argument_names, std::shared_ptr<ASTNode> body)
	: FunctionValue(move(identifier)), _argument_names(move(argument_names)), _body(move(body)) {
	if (_identifier.empty()) {
		// TODO: print as hex value with proper formatting
		_identifier = (ostringstream() << (void*)_body.get()).str();
	}
}

shared_ptr<Value> UserDefinedFunctionValue::call(shared_ptr<Scope>& scope, const vector<shared_ptr<Value>>& arguments) const {
	int arguments_passed_size = arguments.size();
	int arguments_expected_size = _argument_names.size();

	if (arguments.size() != _argument_names.size()) {
		throw InvalidArgumentsCountError(id(), _argument_names.size(), arguments.size());
	}

	auto argument_scope = scope->push();
	for (int i = 0; i < arguments_passed_size; ++i) {
		argument_scope->overshadow(_argument_names[i], arguments[i]);
	}

	argument_scope->overshadow(return_value_alias, NullValue::get());
	_body->evaluate(argument_scope);

	return argument_scope->get(return_value_alias);
}

/* ===== BuiltinFunctionValue ===== */

BuiltinFunctionValue::BuiltinFunctionValue(string identifier, const BuiltinFunctionValue::_FuncType& func)
	: FunctionValue(move(identifier)), _func(move(func)) {}

shared_ptr<Value> BuiltinFunctionValue::call(shared_ptr<Scope>& scope, const vector<shared_ptr<Value>>& arguments) const {
	return _func(scope, arguments);
}

/* ==== GlobalVars ====*/

typedef shared_ptr<Value> ValuePtr;
typedef shared_ptr<Scope> ScopePtr;
typedef vector<shared_ptr<Value>> Arguments;

template <typename Func>
void addFunctionToGlobalScope(const string& identifier, Func&& func) {
	auto func_value = make_shared<BuiltinFunctionValue>(identifier, BuiltinFunctionValue::_FuncType(forward<Func>(func)));
	Scope::addToGlobalScope(identifier, move(func_value));
}

template <typename Func>
void addUnaryMathFunctionToGlobalScope(const string& identifier, Func&& func) {
	addFunctionToGlobalScope(identifier, [&func](ScopePtr& scope, const Arguments& arguments) -> ValuePtr {
		 auto number = toNumber(arguments[0]);
		 return make_shared<NumberValue>(true, func(number));
	});
}

template <typename Func>
void addBinaryMathFunctionToGlobalScope(const string& identifier, Func&& func) {
	addFunctionToGlobalScope(identifier, [&func](ScopePtr& scope, const Arguments& arguments) -> ValuePtr {
		 auto number1 = toNumber(arguments[0]);
		 auto number2 = toNumber(arguments[1]);
		 return make_shared<NumberValue>(true, func(number1, number2));
	});
}

void setupGlobalScope() {
	/* ===== IO ===== */

	addFunctionToGlobalScope("print", [](ScopePtr& scope, const Arguments& arguments) -> ValuePtr {
		auto arguments_count = arguments.size();

		for (Arguments::size_type i = 0; i < arguments_count; ++i) {
			auto&& argument = arguments[i];

			if (argument) {
				argument->output(cout);
			} else {
				cout << "(undefined)";
			}

			if (i + 1 < arguments_count) {
				cout << " ";
			}
		}

		cout << flush;
		return nullptr;
	});

	addFunctionToGlobalScope("println", [](ScopePtr& scope, const Arguments& arguments) -> ValuePtr {
		auto print = static_pointer_cast<FunctionValue>(scope->get("print"));
		print->call(scope, arguments);
		cout << endl;
		return nullptr;
	});

	addFunctionToGlobalScope("read", [](ScopePtr& scope, const Arguments& arguments) -> ValuePtr {
		string str;
		cin >> str;
		return make_shared<StringValue>(true, move(str));
	});

	addFunctionToGlobalScope("readln", [](ScopePtr& scope, const Arguments& arguments) -> ValuePtr {
		string str;
		getline(cin, str);
		return make_shared<StringValue>(true, move(str));
	});

	/* ===== Math ===== */

	// (note: these are using the long double versions of functions to avoid needing to cast)
	Scope::addToGlobalScope("PI", make_shared<NumberValue>(true, M_PI));
	Scope::addToGlobalScope("E", make_shared<NumberValue>(true, M_E));

	// general
	addUnaryMathFunctionToGlobalScope("abs", fabsl);
	addUnaryMathFunctionToGlobalScope("sqrt", sqrtl);
	addUnaryMathFunctionToGlobalScope("cbrt", cbrtl);
	addUnaryMathFunctionToGlobalScope("floor", floorl);
	addUnaryMathFunctionToGlobalScope("ceil", ceill);
	addUnaryMathFunctionToGlobalScope("gamma", tgammal);
	addBinaryMathFunctionToGlobalScope("max", fmaxl);
	addBinaryMathFunctionToGlobalScope("min", fminl);
	addUnaryMathFunctionToGlobalScope("sign", [](double x) {
		return x == 0.0 ? 0.0 : (x < 0.0 ? -1.0 : 1.0);
	});
	addUnaryMathFunctionToGlobalScope("factorial", [](double x) {
		return tgammal(x + 1.0);
	});

	// expontentials
	addUnaryMathFunctionToGlobalScope("exp", expl);
	addUnaryMathFunctionToGlobalScope("exp2", exp2l);
	addUnaryMathFunctionToGlobalScope("log", logl);
	addUnaryMathFunctionToGlobalScope("log10", log10l);
	addUnaryMathFunctionToGlobalScope("log2", log2l);

	// trig
	addUnaryMathFunctionToGlobalScope("sin", sinl);
	addUnaryMathFunctionToGlobalScope("cos", cosl);
	addUnaryMathFunctionToGlobalScope("tan", tanl);
	addUnaryMathFunctionToGlobalScope("asin", asinl);
	addUnaryMathFunctionToGlobalScope("acos", acosl);
	addUnaryMathFunctionToGlobalScope("atan", atanl);
	addBinaryMathFunctionToGlobalScope("atan2", atan2l);

	// hyperbolic trig
	addUnaryMathFunctionToGlobalScope("sinh", sinhl);
	addUnaryMathFunctionToGlobalScope("cosh", coshl);
	addUnaryMathFunctionToGlobalScope("tanh", tanhl);
	addUnaryMathFunctionToGlobalScope("asinh", asinhl);
	addUnaryMathFunctionToGlobalScope("acosh", acoshl);
	addUnaryMathFunctionToGlobalScope("atanh", atanhl);


	/* ===== Functional ===== */

	addFunctionToGlobalScope("bind", [](ScopePtr& scope, const Arguments& arguments) -> ValuePtr {
		if (arguments.empty()) {
			throw InvalidArgumentsCountError("bind", 1, 0);
		}

		const auto& arg0 = arguments[0];
		if (arg0->type() != ValueType::Function) {
			throw TypeError();
		}

		if (arguments.size() < 2) {
			return arg0;
		}

		auto func = static_pointer_cast<FunctionValue>(arguments[0]);
		// TODO: this needs a better name
		return make_shared<BuiltinFunctionValue>(func->id() + "_bind", [arguments, func](ScopePtr& scope, const Arguments& following_arguments) -> ValuePtr {
			Arguments new_arguments (next(begin(arguments)), end(arguments));
			new_arguments.insert(end(new_arguments), begin(following_arguments), end(following_arguments));
			return func->call(scope, new_arguments);
		});
	});
}
