#include "token.h"

using namespace std;

ostream& operator<<(ostream& out, TokenType type) {
	#define TOKEN_CASE(type) case TokenType::type: out << #type; break
	switch (type) {
		TOKEN_CASE(InvalidToken);
		TOKEN_CASE(NumberLiteral);
		TOKEN_CASE(StringLiteral);
		TOKEN_CASE(Identifier);
		TOKEN_CASE(Operator);
		TOKEN_CASE(Comment);
		TOKEN_CASE(Keyword);
		default: out << "UnknownToken";
	}
	#undef TOKEN_CASE

	return out;
}

ostream& operator<<(ostream& out, const TokenMetaData& meta) {
	return out << meta.filename << ":" << meta.line << ":" << meta.column;
}

Token::Token(TokenType type, TokenMetaData meta, string text)
	: _meta(meta), _type(type), _text(move(text)) {}

TokenType Token::type() const {
	return _type;
}

const TokenMetaData& Token::meta() const {
	return _meta;
}

const string& Token::text() const {
	return _text;
}
