#include "Token.h"

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

Token::Token(TokenType type, std::string text)
	: _type(type), _text(text) {}
	
TokenType Token::type() const { 
	return _type;
}
	
const std::string& Token::text() const {
	return _text;
}
