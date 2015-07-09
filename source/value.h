#ifndef _VALUE_H_
#define _VALUE_H_

#include <iostream>
#include <string>
#include <memory>
#include <utility>
#include <unordered_map>

enum class ValueType {
	Null,
	Number,
	String,
	Boolean
};

class TypeError : public std::runtime_error {
public:
	TypeError()
		: std::runtime_error("invalid type conversion") {}
};

class DeclarationError : public std::runtime_error {
public:
	DeclarationError(const std::string& identifier)
		: std::runtime_error("invalid declaration: " + identifier + " is already declared") {}
};

class UndefinedVariableError : public std::runtime_error {
public:
	UndefinedVariableError(const std::string& identifier)
		: std::runtime_error("undefined variable name: " + identifier) {}
};

class Value {
public:
	ValueType type() const;
	bool isConst() const;
	virtual void output(std::ostream& out) const = 0;

	template <typename Type>
	auto valueAs() -> decltype(std::declval<Type>().valueOf()) {
		if (type() != Type::value_type)	{
			throw TypeError();
		}

		return static_cast<Type*>(this)->valueOf();
	}

protected:
	Value(ValueType type, bool is_const);
private:
	ValueType _type;
	bool _is_const;
};

class NumberValue : public Value {
public:
	static const ValueType value_type = ValueType::Number;
	NumberValue(bool is_const, double number);
	virtual void output(std::ostream& out) const override;
	double valueOf() const;
private:
	double _number;
};

class StringValue : public Value {
public:
	static const ValueType value_type = ValueType::String;
	StringValue(bool is_const, std::string str);
	virtual void output(std::ostream& out) const override;
	std::string valueOf() const;
private:
	std::string _str;
};

class BooleanValue : public Value {
public:
	static const ValueType value_type = ValueType::Boolean;
	BooleanValue(bool is_const, bool boolean);
	virtual void output(std::ostream& out) const override;
	bool valueOf() const;
private:
	bool _value;
};

double toNumber(const std::shared_ptr<Value>& var);
std::string toString(const std::shared_ptr<Value>& var);
bool toBoolean(const std::shared_ptr<Value>& var);

#endif
