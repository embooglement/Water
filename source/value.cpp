#include <sstream>
#include <cmath>

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

shared_ptr<Value> Value::get(const shared_ptr<Value>& index) const {
	throw InterpretorError("(get not implemented)");
}

void Value::set(const shared_ptr<Value>& index, shared_ptr<Value> new_value) {
	throw InterpretorError("(set not implemented)");
}

shared_ptr<Value> Value::copy() const {
	throw InterpretorError("(copy not implemented)");
}

/* ===== SentinelValue ===== */

const shared_ptr<SentinelValue> SentinelValue::Return { new SentinelValue(_SentinelType::Return) };
const shared_ptr<SentinelValue> SentinelValue::Break { new SentinelValue(_SentinelType::Break) };
const shared_ptr<SentinelValue> SentinelValue::Continue { new SentinelValue(_SentinelType::Continue) };

SentinelValue::SentinelValue(_SentinelType type)
	: Value(value_type), _type(type) {}

void SentinelValue::output(ostream& out) const {
	out << "(sentinel)";
}

bool SentinelValue::isReturn() const {
	return _type == _SentinelType::Return;
}

bool SentinelValue::isBreak() const {
	return _type == _SentinelType::Break;
}

bool SentinelValue::isContinue() const {
	return _type == _SentinelType::Continue;
}

bool SentinelValue::isReferenceType() const {
	return false;
}

/* ===== NullValue ===== */

const shared_ptr<NullValue> NullValue::_null_value {new NullValue()};

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

bool NullValue::isReferenceType() const {
	return true;
}

/* ===== NumberValue ===== */

NumberValue::NumberValue(double number)
	: Value(value_type), _number(number) {}

void NumberValue::output(ostream& out) const {
	out << valueOf();
}

bool NumberValue::isReferenceType() const {
	return false;
}

shared_ptr<Value> NumberValue::copy() const {
	return NumberValue::create(valueOf());
}

double NumberValue::valueOf() const {
	return _number;
}

void NumberValue::update(double new_value) {
	_number = new_value;
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

bool StringValue::isReferenceType() const {
	return true;
}

string StringValue::valueOf() const {
	return _str;
}

void StringValue::update(string new_value) {
	_str = move(new_value);
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

bool BooleanValue::isReferenceType() const {
	return false;
}

shared_ptr<Value> BooleanValue::copy() const {
	return BooleanValue::create(valueOf());
}

bool BooleanValue::valueOf() const {
	return _value;
}

void BooleanValue::update(bool new_value) {
	_value = new_value;
}

shared_ptr<BooleanValue> BooleanValue::create(bool boolean) {
	return make_shared<BooleanValue>(boolean);
}

/* ===== ArrayValue ===== */

ArrayValue::ArrayValue(std::vector<std::shared_ptr<Value>> elements)
	: Value(value_type), _elements(move(elements)) {}

void ArrayValue::output(std::ostream& out) const {
	out << "[";

	for (int i = 0; i < _elements.size(); ++i) {
		_elements[i]->output(out);

		if (i + 1 < _elements.size()) {
			out << ", ";
		}
	}

	out << "]";
}

shared_ptr<Value> ArrayValue::get(const shared_ptr<Value>& index) const {
	switch (index->type()) {
		case ValueType::Number:
			return getIndex(toNumber(index));
		case ValueType::String:
			return getMember(toString(index));
		default:
			throw InvalidPropertyType();
	}
}

shared_ptr<Value> ArrayValue::get(unsigned int index) const {
	return _elements[index];
}

void ArrayValue::set(const shared_ptr<Value>& index, shared_ptr<Value> new_value) {
	if (index->type() != ValueType::Number) {
		throw TypeError("Expression is not of type Number");
	}

	int i = toNumber(index);
	if (i >= _elements.size()) {
		throw OutOfBoundsError(i, _elements.size());
	}

	_elements[i] = move(new_value);
}

bool ArrayValue::isReferenceType() const {
	return true;
}

unsigned int ArrayValue::length() const {
	return _elements.size();
}

unsigned int ArrayValue::convertIndex(double index) const {
	// TODO: this behavior is probably bad
	return static_cast<unsigned int>(floor(index));
}

shared_ptr<Value> ArrayValue::getIndex(double index) const {
	auto i = convertIndex(index);
	if (i >= _elements.size()) {
		throw OutOfBoundsError(i, _elements.size());
	}

	return _elements[i];
}

shared_ptr<Value> ArrayValue::getMember(const string& member) const {
	if (member == "length") {
		return NumberValue::create(length());
	} else if (member == "push") {
		return BuiltinFunctionValue::create("push", [this](const vector<shared_ptr<Value>>& arguments) mutable -> shared_ptr<Value> {
			for (auto argument : arguments) {
				// TODO: this is an awful hack that should be removed when values have actual prototypes
				const_cast<ArrayValue*>(this)->_elements.push_back(argument);
			}

			return NullValue::get();
		});
	}

	return NullValue::get();
}

void ArrayValue::setIndex(double index, shared_ptr<Value> new_value) {
	auto i = convertIndex(index);
	if (i >= _elements.size()) {
		throw OutOfBoundsError(i, _elements.size());
	}

	_elements[i] = move(new_value);
}

void ArrayValue::setMember(const string& member, shared_ptr<Value> new_value) {
	throw ImmutableError(member);
}

/* ===== ObjectValue ===== */

ObjectValue::ObjectValue(unordered_map<string, shared_ptr<Value>> members)
	: Value(value_type), _members(move(members)) {}

void ObjectValue::output(ostream& out) const {
	out << "{";

	// TODO: this isn't really deterministic
	auto end_it = end(_members);
	for (auto it = begin(_members); it != end_it; ++it) {
		out << it->first << ": ";
		it->second->output(out);

		if (next(it) != end_it) {
			out << ", ";
		}
	}

	out << "}";
}

shared_ptr<Value> ObjectValue::get(const shared_ptr<Value>& index) const {
	switch (index->type()) {
		case ValueType::Number:
			return getIndex(toNumber(index));
		case ValueType::String:
			return getMember(toString(index));
		default:
			throw InvalidPropertyType();
	}
}

void ObjectValue::set(const shared_ptr<Value>& index, shared_ptr<Value> new_value) {
	switch (index->type()) {
		case ValueType::Number:
			setIndex(toNumber(index), move(new_value));
			break;
		case ValueType::String:
			setMember(toString(index), move(new_value));
			break;
		default:
			throw InvalidPropertyType();
	}
}

bool ObjectValue::isReferenceType() const {
	return true;
}

string ObjectValue::convertIndex(double index) const {
	// TODO: this should probably be smarter about formatting the index
	return to_string(index);
}

shared_ptr<Value> ObjectValue::getIndex(double index) const {
	return getMember(convertIndex(index));
}

shared_ptr<Value> ObjectValue::getMember(const string& member) const {
	auto it = _members.find(member);
	if (it == end(_members)) {
		return NullValue::get();
	}

	return it->second;
}

void ObjectValue::setIndex(double index, shared_ptr<Value> new_value) {
	setMember(convertIndex(index), move(new_value));
}

void ObjectValue::setMember(const string& member, shared_ptr<Value> new_value) {
	_members[member] = move(new_value);
}

vector<string> ObjectValue::keys() const {
	vector<string> keys;
	for (auto&& member : _members) {
		keys.push_back(member.first);
	}
	return keys;
}

/* ===== FunctionValue ===== */

FunctionValue::FunctionValue(string identifier)
	: Value(value_type), _identifier(move(identifier)) {}

void FunctionValue::output(ostream& out) const {
	out << _identifier;
}

bool FunctionValue::isReferenceType() const {
	return false;
}

shared_ptr<Value> FunctionValue::copy() const {
	// TODO: this is hacky, not sure how to void a const_cast in this case though
	return const_cast<FunctionValue*>(this)->shared_from_this();
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

shared_ptr<Value> UserDefinedFunctionValue::call(const vector<shared_ptr<Value>>& arguments) const {
	int arguments_passed_size = arguments.size();
	int arguments_expected_size = _argument_names.size();

	if (arguments_passed_size != arguments_expected_size) {
		throw InvalidArgumentsCountError(id(), arguments_expected_size, arguments_passed_size);
	}

	auto scope = _body->scope();

	for (int i = 0; i < arguments_passed_size; ++i) {
		scope->setValue(_argument_names[i], arguments[i]);
	}

	scope->setValue(return_value_alias, NullValue::get());
	_body->evaluate();

	return scope->getValue(return_value_alias);
}

shared_ptr<UserDefinedFunctionValue> UserDefinedFunctionValue::create(string identifier, vector<string> argument_names, shared_ptr<ASTNode> body) {
	return make_shared<UserDefinedFunctionValue>(move(identifier), move(argument_names), move(body));
}

/* ===== BuiltinFunctionValue ===== */

BuiltinFunctionValue::BuiltinFunctionValue(string identifier, const BuiltinFunctionValue::_FuncType& func)
	: FunctionValue(move(identifier)), _func(move(func)) {}

shared_ptr<Value> BuiltinFunctionValue::call(const vector<shared_ptr<Value>>& arguments) const {
	return _func(arguments);
}

shared_ptr<BuiltinFunctionValue> BuiltinFunctionValue::create(string identifier, const BuiltinFunctionValue::_FuncType& func) {
	return make_shared<BuiltinFunctionValue>(move(identifier), func);
}
