#ifndef _RUNTIME_ERROR_H_
#define _RUNTIME_ERROR_H_

#include <string>
#include <stdexcept>

// TODO: get meta data for where the error occurs

class TypeError : public std::runtime_error {
public:
	TypeError()
		: std::runtime_error("Invalid type conversion") {}
};

class DeclarationError : public std::runtime_error {
public:
	DeclarationError(const std::string& identifier)
		: std::runtime_error("Invalid declaration: " + identifier + " is already declared") {}
};

class UndefinedVariableError : public std::runtime_error {
public:
	UndefinedVariableError(const std::string& identifier)
		: std::runtime_error("Undefined variable name: " + identifier) {}
};

class InvalidArgumentsCountError : public std::runtime_error {
public:
	InvalidArgumentsCountError(const std::string& identifier, int expected, int passed)
		: std::runtime_error("Incorrect number of arguments passed to " + identifier + ": expected " + std::to_string(expected) + " but recieved " + std::to_string(passed)) {}
};

#endif
