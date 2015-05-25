#ifndef _LEXER_H_
#define _LEXER_H_

#include <vector>
#include <string>
#include <set>
#include <stdexcept>
#include "token.h"

const static std::set<std::string> keywords = {
	"print",
	"if", "else", "while", "for",
	"and", "or", "not"
};

const static std::set<std::string> operators = {
	"=", ".", ";",
	"(", ")",
	"[", "]",
	"{", "}",
	"+", "+=", "++",
	"-", "-=", "--",
	"*", "*=",
	"/", "/=",
	"%", "%=",
	"^", "^=",
	"<", ">", "==", "!="
};

class Lexer {
public:
	// pass stream and filename seperately so main can also use std::cin as input stream
	std::vector<Token> tokenize(std::istream& input, const std::string& filename = "(unknown input)");
};

class InvalidTokenError : public std::runtime_error {
public:
	InvalidTokenError(TokenMetaData meta_data, const std::string& error, const std::string& text);
	const TokenMetaData& meta() const;
	const std::string& text() const;
	virtual const char* what() const noexcept override;
private:
	TokenMetaData _meta;
	std::string _text;
	std::string _error;
};

#endif
