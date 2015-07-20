#include <sstream>

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
		_identifier = (ostringstream() << (void*)_body.get()).str();
	}
}

shared_ptr<Value> UserDefinedFunctionValue::call(shared_ptr<Scope>& scope, const vector<shared_ptr<Value>>& arguments) const {
	int arguments_passed_size = arguments.size();
	int arguments_expected_size = _argument_names.size();

	if (arguments.size() != _argument_names.size()) {
		throw InvalidArgumentsCountError(id(), _argument_names.size(), arguments.size());
	}

	auto argument_scope = scope->createNestedScope();
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
