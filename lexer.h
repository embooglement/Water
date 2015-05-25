#ifndef _LEXER_H_
#define _LEXER_H_

#include <vector>
#include <set>
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
	std::vector<Token> tokenize(std::istream& input);
};

#endif
