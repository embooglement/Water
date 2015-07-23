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
	Sentinel,
	Null,
	Number,
	String,
	Boolean,
	Function,
	Array
};

class ASTNode;
class Scope;
class Value;

double toNumber(const std::shared_ptr<Value>& var);
std::string toString(const std::shared_ptr<Value>& var);
bool toBoolean(const std::shared_ptr<Value>& var);

class Value {
public:
	ValueType type() const;
	virtual void output(std::ostream& out) const = 0;

	template <typename Type>
	auto valueAs() -> decltype(std::declval<Type>().valueOf()) {
		if (type() != Type::value_type)	{
			throw TypeError();
		}

		return static_cast<Type*>(this)->valueOf();
	}

protected:
	Value(ValueType type);
private:
	ValueType _type;
};

class SentinelValue : public Value {
public:
	static const ValueType value_type = ValueType::Sentinel;
	SentinelValue();
	virtual void output(std::ostream& out) const override;
	bool isReturn() const;
};

class NullValue : public Value {
private:
	static const std::shared_ptr<NullValue> _null_value;
	NullValue();
public:
	static const ValueType value_type = ValueType::Null;
	static const std::shared_ptr<NullValue>& get();
	virtual void output(std::ostream& out) const override;
	std::nullptr_t valueOf() const;
};

class NumberValue : public Value {
public:
	static const ValueType value_type = ValueType::Number;
	NumberValue(double number);
	virtual void output(std::ostream& out) const override;
	double valueOf() const;
	void update(double new_value);

	static std::shared_ptr<NumberValue> create(double number);

	template <typename Op>
	static std::shared_ptr<NumberValue> applyOperator(const std::shared_ptr<Value>& lhs, const std::shared_ptr<Value>& rhs, Op op) {
		return std::make_shared<NumberValue>(op(toNumber(lhs), toNumber(rhs)));
	}
private:
	double _number;
};

class StringValue : public Value {
public:
	static const ValueType value_type = ValueType::String;
	StringValue(std::string str);
	virtual void output(std::ostream& out) const override;
	std::string valueOf() const;
	void update(std::string new_value);

	static std::shared_ptr<StringValue> create(std::string str);
private:
	std::string _str;
};

class BooleanValue : public Value {
public:
	static const ValueType value_type = ValueType::Boolean;
	BooleanValue(bool boolean);
	virtual void output(std::ostream& out) const override;
	bool valueOf() const;
	void update(bool new_value);

	static std::shared_ptr<BooleanValue> create(bool boolean);
private:
	bool _value;
};

class ArrayValue : public Value {
public:
	static const ValueType value_type = ValueType::Array;
	ArrayValue(std::vector<std::shared_ptr<Value>> elements);
	virtual void output(std::ostream& out) const override;
	const std::vector<std::shared_ptr<Value>>& valueOf() const;
private:
	std::vector<std::shared_ptr<Value>> _elements;
};

// TODO: maybe add a EmptyArrayValue class as optimization?

class FunctionValue : public Value {
public:
	static const ValueType value_type = ValueType::Function;
	FunctionValue(std::string identifier);
	virtual void output(std::ostream& out) const override;
	virtual std::shared_ptr<Value> call(std::shared_ptr<Scope>& scope, const std::vector<std::shared_ptr<Value>>& arguments) const = 0;
	const std::string& id() const;
protected:
	std::string _identifier;
};

class UserDefinedFunctionValue : public FunctionValue {
public:
	UserDefinedFunctionValue(std::string identifier, std::vector<std::string> argument_names, std::shared_ptr<ASTNode> body);
	virtual std::shared_ptr<Value> call(std::shared_ptr<Scope>& scope, const std::vector<std::shared_ptr<Value>>& arguments) const;

	static std::shared_ptr<UserDefinedFunctionValue> create(std::string identifier, std::vector<std::string> argument_names, std::shared_ptr<ASTNode> body);
private:
	std::vector<std::string> _argument_names;
	std::shared_ptr<ASTNode> _body;
};

class BuiltinFunctionValue : public FunctionValue {
public:
	typedef std::function<std::shared_ptr<Value>(std::shared_ptr<Scope>&, const std::vector<std::shared_ptr<Value>>&)> _FuncType;
	BuiltinFunctionValue(std::string identifier, const _FuncType& func);
	virtual std::shared_ptr<Value> call(std::shared_ptr<Scope>& scope, const std::vector<std::shared_ptr<Value>>& arguments) const override;

	static std::shared_ptr<BuiltinFunctionValue> create(std::string identifier, const _FuncType& func);
private:
	const _FuncType _func;
};

#endif
