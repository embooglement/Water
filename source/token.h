#ifndef _TOKEN_H_
#define _TOKEN_H_

#include <string>
#include <iostream>

enum class TokenType {
	UnknownToken = -2,
	InvalidToken = -1,
	Builtin,
	NumberLiteral,
	StringLiteral,
	Identifier,
	Comment
};

std::ostream& operator<<(std::ostream& out, TokenType type);

struct TokenMetaData {
	std::string filename;
	int line;
	int column;
};

std::ostream& operator<<(std::ostream& out, const TokenMetaData& meta);

class Token {
public:
	Token(TokenType token_type, TokenMetaData meta, std::string text);
	TokenType type() const;
	const std::string& text() const;
	const TokenMetaData& meta() const;
private:
	TokenMetaData _meta;
	TokenType _type;
	std::string _text;
};

#endif
