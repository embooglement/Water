#include <sstream>

#include "value.h"
#include "scope.h"
#include "astnode.h"

using namespace std;

/* ===== Conversions ===== */

double toNumber(const shared_ptr<Value>& var) {
	return var->valueAs<NumberValue>();
}

string toString(const shared_ptr<Value>& var) {
	return var->valueAs<StringValue>();
}

bool toBoolean(const shared_ptr<Value>& var) {
	return var->valueAs<BooleanValue>();
}

/* ===== Value ===== */

Value::Value(ValueType type)
	: _type(type) {}

ValueType Value::type() const {
	return _type;
}

/* ===== SentinelValue ===== */

SentinelValue::SentinelValue()
	: Value(value_type) {}

void SentinelValue::output(ostream& out) const {
	out << "(sentinel)";
}

bool SentinelValue::isReturn() const {
	return true;
}

/* ===== NullValue ===== */

const shared_ptr<NullValue> NullValue::_null_value;

NullValue::NullValue()
	: Value(value_type) {}

const shared_ptr<NullValue>& NullValue::get() {
	return _null_value;
}

void NullValue::output(ostream& out) const {
	out << "(null)";
}

nullptr_t NullValue::valueOf() const {
	return nullptr;
}

/* ===== NumberValue ===== */

NumberValue::NumberValue(double number)
	: Value(value_type), _number(number) {}

void NumberValue::output(ostream& out) const {
	out << valueOf();
}

double NumberValue::valueOf() const {
	return _number;
}

shared_ptr<NumberValue> NumberValue::create(double number) {
	return make_shared<NumberValue>(number);
}

/* ===== StringValue ===== */

StringValue::StringValue(string str)
	: Value(value_type), _str(move(str)) {}

void StringValue::output(ostream& out) const {
	out << valueOf();
}

string StringValue::valueOf() const {
	return _str;
}

shared_ptr<StringValue> StringValue::create(string str) {
	return make_shared<StringValue>(move(str));
}

/* ===== BooleanValue ===== */

BooleanValue::BooleanValue(bool boolean)
	: Value(value_type), _value(boolean) {}

void BooleanValue::output(ostream& out) const {
	out << (valueOf() ? "true" : "false");
}

bool BooleanValue::valueOf() const {
	return _value;
}

shared_ptr<BooleanValue> BooleanValue::create(bool boolean) {
	return make_shared<BooleanValue>(boolean);
}

/* ===== FunctionValue ===== */

FunctionValue::FunctionValue(string identifier)
	: Value(value_type), _identifier(move(identifier)) {}

void FunctionValue::output(ostream& out) const {
	out << _identifier;
}

const string& FunctionValue::id() const {
	return _identifier;
}

/* ===== UserDefinedFunctionValue ===== */

UserDefinedFunctionValue::UserDefinedFunctionValue(string identifier, vector<string> argument_names, shared_ptr<ASTNode> body)
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

shared_ptr<UserDefinedFunctionValue> UserDefinedFunctionValue::create(string identifier, vector<string> argument_names, shared_ptr<ASTNode> body) {
	return make_shared<UserDefinedFunctionValue>(move(identifier), move(argument_names), move(body));
}

/* ===== BuiltinFunctionValue ===== */

BuiltinFunctionValue::BuiltinFunctionValue(string identifier, const BuiltinFunctionValue::_FuncType& func)
	: FunctionValue(move(identifier)), _func(move(func)) {}

shared_ptr<Value> BuiltinFunctionValue::call(shared_ptr<Scope>& scope, const vector<shared_ptr<Value>>& arguments) const {
	return _func(scope, arguments);
}

shared_ptr<BuiltinFunctionValue> BuiltinFunctionValue::create(string identifier, const BuiltinFunctionValue::_FuncType& func) {
	return make_shared<BuiltinFunctionValue>(move(identifier), func);
}
