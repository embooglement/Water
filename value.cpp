#include <utility>
#include "value.h"

using namespace std;

/* ===== Value ===== */

Value::Value(ValueType type) : _type(type) {}

ValueType Value::type() const {
	return _type;
}

/* ===== NumberValue ===== */

NumberValue::NumberValue(double number)
	: Value(ValueType::Number), _number(number) {}

void NumberValue::output(std::ostream& out) const {
	out << valueOf();
}

double NumberValue::valueOf() const {
	return _number;
}

/* ===== StringValue ===== */

StringValue::StringValue(string str)
	: Value(ValueType::String), _str(move(str)) {}

void StringValue::output(std::ostream& out) const {
	out << valueOf();
}

string StringValue::valueOf() const {
	return _str;
}

/* ===== BooleanValue ===== */

BooleanValue::BooleanValue(bool boolean)
	: Value(ValueType::Boolean), _value(boolean) {}

void BooleanValue::output(ostream& out) const {
	out << (valueOf() ? "true" : "false");
}

bool BooleanValue::valueOf() const {
	return _value;
}
