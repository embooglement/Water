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
	Array,
	Object,
	Function
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
	virtual std::shared_ptr<Value> get(const std::shared_ptr<Value>& index) const;
	virtual void set(const std::shared_ptr<Value>& index, std::shared_ptr<Value> new_value);
	virtual bool isReferenceType() const = 0;
	virtual std::shared_ptr<Value> copy() const;

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
	static const std::shared_ptr<SentinelValue> Return;
	static const std::shared_ptr<SentinelValue> Break;
	static const std::shared_ptr<SentinelValue> Continue;

	virtual void output(std::ostream& out) const override;
	virtual bool isReferenceType() const override;
	bool isReturn() const;
	bool isBreak() const;
	bool isContinue() const;

protected:
	enum class _SentinelType {
		Return,
		Break,
		Continue
	} _type;

	SentinelValue(_SentinelType);
};

class NullValue : public Value {
private:
	static const std::shared_ptr<NullValue> _null_value;
	NullValue();
public:
	static const ValueType value_type = ValueType::Null;
	static const std::shared_ptr<NullValue>& get();
	virtual void output(std::ostream& out) const override;
	virtual bool isReferenceType() const override;
	std::nullptr_t valueOf() const;
};

class NumberValue : public Value {
public:
	static const ValueType value_type = ValueType::Number;
	NumberValue(double number);
	virtual void output(std::ostream& out) const override;
	virtual bool isReferenceType() const override;
	virtual std::shared_ptr<Value> copy() const override;
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
	virtual bool isReferenceType() const override;
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
	virtual bool isReferenceType() const override;
	virtual std::shared_ptr<Value> copy() const override;
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
	virtual std::shared_ptr<Value> get(const std::shared_ptr<Value>& index) const override;
	virtual void set(const std::shared_ptr<Value>& index, std::shared_ptr<Value> new_value);
	virtual bool isReferenceType() const override;
	unsigned int length() const;
protected:
	unsigned int convertIndex(double index) const;
	std::shared_ptr<Value> getIndex(double index) const;
	std::shared_ptr<Value> getMember(const std::string& member) const;
	void setIndex(double index, std::shared_ptr<Value> new_value);
	void setMember(const std::string& member, std::shared_ptr<Value> new_value);
private:
	std::vector<std::shared_ptr<Value>> _elements;
};

class ObjectValue : public Value {
public:
	static const ValueType value_type = ValueType::Object;
	ObjectValue(std::unordered_map<std::string, std::shared_ptr<Value>> members);
	virtual void output(std::ostream& out) const override;
	virtual std::shared_ptr<Value> get(const std::shared_ptr<Value>& index) const override;
	virtual void set(const std::shared_ptr<Value>& index, std::shared_ptr<Value> new_value);
	virtual bool isReferenceType() const override;
protected:
	std::string convertIndex(double index) const;
	std::shared_ptr<Value> getIndex(double index) const;
	std::shared_ptr<Value> getMember(const std::string& member) const;
	void setIndex(double index, std::shared_ptr<Value> new_value);
	void setMember(const std::string& member, std::shared_ptr<Value> new_value);
private:
	std::unordered_map<std::string, std::shared_ptr<Value>> _members;
};

class FunctionValue : public Value, public std::enable_shared_from_this<FunctionValue> {
public:
	static const ValueType value_type = ValueType::Function;
	FunctionValue(std::string identifier);
	virtual void output(std::ostream& out) const override;
	virtual bool isReferenceType() const override;
	virtual std::shared_ptr<Value> copy() const override;
	virtual std::shared_ptr<Value> call(const std::vector<std::shared_ptr<Value>>& arguments) const = 0;
	const std::string& id() const;
protected:
	std::string _identifier;
};

class UserDefinedFunctionValue : public FunctionValue {
public:
	UserDefinedFunctionValue(std::string identifier, std::vector<std::string> argument_names, std::shared_ptr<ASTNode> body);
	virtual std::shared_ptr<Value> call(const std::vector<std::shared_ptr<Value>>& arguments) const;

	static std::shared_ptr<UserDefinedFunctionValue> create(std::string identifier, std::vector<std::string> argument_names, std::shared_ptr<ASTNode> body);
private:
	std::vector<std::string> _argument_names;
	std::shared_ptr<ASTNode> _body;
};

class BuiltinFunctionValue : public FunctionValue {
public:
	typedef std::function<std::shared_ptr<Value>(const std::vector<std::shared_ptr<Value>>&)> _FuncType;
	BuiltinFunctionValue(std::string identifier, const _FuncType& func);
	virtual std::shared_ptr<Value> call(const std::vector<std::shared_ptr<Value>>& arguments) const override;

	static std::shared_ptr<BuiltinFunctionValue> create(std::string identifier, const _FuncType& func);
private:
	const _FuncType _func;
};

#endif
