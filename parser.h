#ifndef _PARSER_H_
#define _PARSER_H_

#include <memory>
#include <vector>

#include "token.h"
#include "astnode.h"

class Parser {
public:
	std::shared_ptr<ASTNode> parse(std::vector<Token> tokens);
};

#endif
