#include "value.h"

using namespace std;

/* ===== Value ===== */

Value::Value(ValueType type, bool is_const)
	: _type(type), _is_const(is_const) {}

ValueType Value::type() const {
	return _type;
}

bool Value::isConst() const {
	return _is_const;
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

double toNumber(const std::shared_ptr<Value>& var) {
	return var->valueAs<NumberValue>();
}

std::string toString(const std::shared_ptr<Value>& var) {
	return var->valueAs<StringValue>();
}

bool toBoolean(const std::shared_ptr<Value>& var) {
	return var->valueAs<BooleanValue>();
}

/* ===== Variables ===== */

std::unordered_map<std::string, std::shared_ptr<Value>> global_variables;

void addGlobalVariable(const string& identifier, const shared_ptr<Value>& var) {
	auto it = global_variables.find(identifier);
	if (it != end(global_variables)) {
		throw DeclarationError(identifier);
	}

	global_variables[identifier] = var;
}

shared_ptr<Value> getGlobalVariable(const string& identifier) {
	auto it = global_variables.find(identifier);
	if (it == end(global_variables)) {
		return nullptr;
	} else {
		return it->second;
	}
}
