#ifndef _PARSER_H_
#define _PARSER_H_

#include <memory>
#include <vector>
#include <utility>

#include "token.h"
#include "astnode.h"

typedef std::vector<Token>::iterator TokenIter;

class Parser {
public:
	std::pair<std::shared_ptr<ASTNode>, int> parse(TokenIter tokens_begin, TokenIter tokens_end);
	void error(const TokenMetaData& meta, const std::string& error);
private:
	int _error_count;
};

#endif
