#ifndef _TOKEN_H_
#define _TOKEN_H_

#include <string>
#include <iostream>

enum class TokenType {
	UnknownToken = -2,
	InvalidToken = -1,
	NumberLiteral,
	StringLiteral,
	Identifier,
	Operator,
	Comment,
	Keyword
};

std::ostream& operator<<(std::ostream& out, TokenType type);

class Token {
public:
	Token(TokenType token_type, std::string text);
	TokenType type() const;
	const std::string& text() const;
private:
	const TokenType _type;
	const std::string _text;
};

#endif
