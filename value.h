#ifndef _VALUE_H_
#define _VALUE_H_

#include <iostream>
#include <string>

enum class ValueType {
	Number,
	String,
	Boolean
};

class Value {
public:
	ValueType type() const;
	virtual void output(std::ostream& out) const = 0;
protected:
	Value(ValueType type);
private:
	ValueType _type;
};

class NumberValue : public Value {
public:
	NumberValue(double number);
	virtual void output(std::ostream& out) const override;
	double valueOf() const;
private:
	double _number;
};

class StringValue : public Value {
public:
	StringValue(std::string str);
	virtual void output(std::ostream& out) const override;
	std::string valueOf() const;
private:
	std::string _str;
};

class BooleanValue : public Value {
public:
	BooleanValue(bool boolean);
	virtual void output(std::ostream& out) const override;
	bool valueOf() const;
private:
	bool _value;
};

#endif
