#ifndef _PARSER_H_
#define _PARSER_H_

#include <memory>
#include <stack>
#include <utility>

#include "token.h"
#include "token_stream.h"
#include "astnode.h"

class Parser {
public:
	std::pair<std::shared_ptr<ASTNode>, int> parse(TokenStream& tokens);
	void error(const TokenMetaData& meta, const std::string& error);

	std::shared_ptr<Scope> scope();
	void pushScope(bool is_function_scope = false);
	void popScope();

	bool inLoop() const;
	void pushLoopState(bool in_loop);
	void popLoopState();
private:
	int _error_count;
	std::shared_ptr<Scope> _scope;
	std::stack<bool> _in_loop;
};

#endif
