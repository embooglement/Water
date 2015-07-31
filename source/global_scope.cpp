#include <sstream>
#include <functional>
#include <cmath>

#include "global_scope.h"
#include "value.h"
#include "scope.h"
#include "astnode.h"

using namespace std;

/* ==== GlobalVars ====*/

typedef shared_ptr<Value> ValuePtr;
typedef shared_ptr<Scope> ScopePtr;
typedef vector<shared_ptr<Value>> Arguments;

template <typename Func>
void addFunctionToGlobalScope(const string& identifier, Func&& func) {
	auto func_value = BuiltinFunctionValue::create(identifier, BuiltinFunctionValue::_FuncType(forward<Func>(func)));
	Scope::addToGlobalScope(identifier, move(func_value));
}

template <typename Func>
void addUnaryMathFunctionToGlobalScope(const string& identifier, Func&& func) {
	addFunctionToGlobalScope(identifier, [&func, identifier](ScopePtr& scope, const Arguments& arguments) -> ValuePtr {
		 if (arguments.size() != 1) {
			 throw InvalidArgumentsCountError(identifier, 1, arguments.size());
		 }

		 if (arguments[0]->type() != ValueType::Number) {
			 throw TypeError("Argument is not of type Number");
		 }

		 auto number = toNumber(arguments[0]);
		 return NumberValue::create(func(number));
	});
}

template <typename Func>
void addBinaryMathFunctionToGlobalScope(const string& identifier, Func&& func) {
	addFunctionToGlobalScope(identifier, [&func, identifier](ScopePtr& scope, const Arguments& arguments) -> ValuePtr {
		 if (arguments.size() != 2) {
			 throw InvalidArgumentsCountError(identifier, 2, arguments.size());
		 }

		 if (arguments[0]->type() != ValueType::Number) {
			 throw TypeError("First argument is not of type Number");
		 }

		 if (arguments[1]->type() != ValueType::Number) {
			 throw TypeError("Second argument is not of type Number");
		 }

		 auto number1 = toNumber(arguments[0]);
		 auto number2 = toNumber(arguments[1]);
		 return NumberValue::create(func(number1, number2));
	});
}

void setupMetaModule() {
	addFunctionToGlobalScope("is_defined", [](ScopePtr& scope, const Arguments& arguments) -> ValuePtr {
		if (arguments.size() != 1) {
			throw InvalidArgumentsCountError("is_defined", 1, arguments.size());
		}

		auto&& argument = arguments[0];
		if (argument->type() != ValueType::String) {
			throw TypeError("Argument is not of type String");
		}

		auto variable_name = toString(argument);
		return BooleanValue::create(scope->contains(variable_name));
	});

	addFunctionToGlobalScope("reference_equals", [](ScopePtr& scope, const Arguments& arguments) -> ValuePtr {
		if (arguments.size() != 2) {
			throw InvalidArgumentsCountError("is_defined", 2, arguments.size());
		}

		return BooleanValue::create(arguments[0] == arguments[1]);
	});
}

void setupDataStructuresModule() {
	addFunctionToGlobalScope("length", [](ScopePtr& scope, const Arguments& arguments) -> ValuePtr {
		if (arguments.size() != 1) {
			throw InvalidArgumentsCountError("length", 1, arguments.size());
		}

		auto&& argument = arguments[0];
		if (argument->type() != ValueType::Array) {
			throw TypeError("Argument is not of type Array");
		}

		auto arr = static_pointer_cast<ArrayValue>(argument);
		return NumberValue::create(arr->length());
	});
}

void setupIOModule() {
	addFunctionToGlobalScope("print", [](ScopePtr& scope, const Arguments& arguments) -> ValuePtr {
		auto arguments_count = arguments.size();

		for (Arguments::size_type i = 0; i < arguments_count; ++i) {
			arguments[i]->output(cout);

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
		return StringValue::create(move(str));
	});

	addFunctionToGlobalScope("readln", [](ScopePtr& scope, const Arguments& arguments) -> ValuePtr {
		string str;
		getline(cin, str);
		return StringValue::create(move(str));
	});
}

void setupMathModule() {
	// (note: these are using the long double versions of functions to avoid needing to cast)

	// constants
	Scope::addToGlobalScope("PI", NumberValue::create(M_PI));
	Scope::addToGlobalScope("E", NumberValue::create(M_E));

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
}

void setupFunctionalModule() {
	addFunctionToGlobalScope("bind", [](ScopePtr& scope, const Arguments& arguments) -> ValuePtr {
		if (arguments.empty()) {
			throw InvalidArgumentsCountError("bind", 1, 0);
		}

		if (arguments[0]->type() != ValueType::Function) {
			throw TypeError("First argument is not of type Function");
		}

		if (arguments.size() < 2) {
			return arguments[0];
		}

		ostringstream name_stream;
		name_stream << "bind_";

		for (auto&& argument : arguments) {
			argument->output(name_stream);
			name_stream << "_";
		}

		auto func = static_pointer_cast<FunctionValue>(arguments[0]);
		return BuiltinFunctionValue::create(name_stream.str(), [arguments, func](ScopePtr& scope, const Arguments& following_arguments) -> ValuePtr {
			Arguments new_arguments (next(begin(arguments)), end(arguments));
			new_arguments.insert(end(new_arguments), begin(following_arguments), end(following_arguments));
			return func->call(scope, new_arguments);
		});
	});

	addFunctionToGlobalScope("constant", [](ScopePtr& scope, const Arguments& arguments) -> ValuePtr {
		if (arguments.empty()) {
			throw InvalidArgumentsCountError("constant", 1, 0);
		}

		const auto& constant_value = arguments[0];
		ostringstream name_stream;
		name_stream << "constant_";
		constant_value->output(name_stream);

		return BuiltinFunctionValue::create(name_stream.str(), [constant_value](ScopePtr& scope, const Arguments& arguments) -> ValuePtr {
			return constant_value;
		});
	});

	addFunctionToGlobalScope("compose", [](ScopePtr& scope, const Arguments& arguments) -> ValuePtr {
		if (arguments.empty()) {
			throw InvalidArgumentsCountError("compose", 1, 0);
		}

		vector<shared_ptr<FunctionValue>> functions;
		ostringstream name_stream;
		name_stream << "compose_";

		for (auto&& argument : arguments) {
			if (argument->type() != ValueType::Function) {
				throw TypeError("Argument is not of type Function");
			}

			auto func = static_pointer_cast<FunctionValue>(argument);
			func->output(name_stream);
			name_stream << "_";

			functions.push_back(move(func));
		}

		return BuiltinFunctionValue::create(name_stream.str(), [functions](ScopePtr& scope, const Arguments& arguments) -> ValuePtr {
			vector<shared_ptr<Value>> new_arguments = arguments;
			shared_ptr<Value> returned_value = NullValue::get();

			auto end_it = functions.rend();
			for (auto it = functions.rbegin(); it != end_it; ++it) {
				auto&& func = *it;
				returned_value = func->call(scope, new_arguments);
				new_arguments.assign({returned_value});
			}

			return returned_value;
		});
	});

	addFunctionToGlobalScope("id", [](ScopePtr& scope, const Arguments& arguments) -> ValuePtr {
		if (arguments.size() != 1) {
			throw InvalidArgumentsCountError("id", 1, arguments.size());
		}

		return arguments[0];
	});
}

void setupGlobalScope() {
	setupMetaModule();
	setupDataStructuresModule();
	setupIOModule();
	setupMathModule();
	setupFunctionalModule();
}
