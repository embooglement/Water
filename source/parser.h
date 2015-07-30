#ifndef _PARSER_H_
#define _PARSER_H_

#include <memory>
#include <vector>
#include <stack>
#include <utility>

#include "token.h"
#include "token_stream.h"
#include "astnode.h"

struct ParserState {
	bool in_loop;

	static const ParserState Default;
};

class Parser {
public:
	std::pair<std::shared_ptr<ASTNode>, int> parse(TokenStream& tokens);
	void error(const TokenMetaData& meta, const std::string& error);
	ParserState getState() const;
	void pushState(ParserState state);
	void popState();
	bool inLoop() const;
	void pushLoopState(bool in_loop);
private:
	int _error_count;
	std::stack<ParserState> _states;
};

#endif
