#ifndef _LEXER_H_
#define _LEXER_H_

#include <vector>
#include <string>
#include <stdexcept>
#include <utility>
#include "token.h"

class Lexer {
public:
	// pass stream and filename seperately so main can also use std::cin as input stream
	std::pair<std::vector<Token>, int> tokenize(std::istream& input, const std::string& filename = "(unknown input)");
	void error(const TokenMetaData& meta, std::string error);
};

#endif
