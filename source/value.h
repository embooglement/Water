#ifndef _VALUE_H_
#define _VALUE_H_

#include <iostream>
#include <string>
#include <memory>
#include <utility>
#include <vector>
#include <unordered_map>
#include "runtime_errors.h"

enum class ValueType {
	Null,
	Sentinel,
	Number,
	String,
	Boolean,
	Function
};

class ASTNode;
class Scope;

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

class NullValue : public Value {
public:
	static const ValueType value_type = ValueType::Null;
	NullValue();
	virtual void output(std::ostream& out) const override;
	std::nullptr_t valueOf() const;
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

class SentinelValue : public Value {
public:
	static const ValueType value_type = ValueType::Sentinel;
	SentinelValue();
	virtual void output(std::ostream& out) const override;
	bool isReturn() const;
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

class FunctionValue : public Value {
public:
	static const ValueType value_type = ValueType::Function;
	FunctionValue(std::string identifier, std::vector<std::string> argument_names, std::shared_ptr<ASTNode> body);
	virtual void output(std::ostream& out) const override;
	virtual std::shared_ptr<Value> call(std::shared_ptr<Scope> scope, std::vector<std::shared_ptr<Value>> arguments) const;
	std::string id() const;
private:
	std::string _identifier;
	std::vector<std::string> _argument_names;
	std::shared_ptr<ASTNode> _body;
};

class PrintFunctionValue : public FunctionValue {
public:
	PrintFunctionValue();
	virtual std::shared_ptr<Value> call(std::shared_ptr<Scope> scope, std::vector<std::shared_ptr<Value>> arguments) const override;
};

double toNumber(const std::shared_ptr<Value>& var);
std::string toString(const std::shared_ptr<Value>& var);
bool toBoolean(const std::shared_ptr<Value>& var);

void setupGlobalScope();

#endif
