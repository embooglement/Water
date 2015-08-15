#ifndef _PARSER_H_
#define _PARSER_H_

#include <memory>
#include <vector>
#include <stack>
#include <utility>

#include "token.h"
#include "token_stream.h"
#include "astnode.h"

struct IdentifierInfo {
	bool is_const;
};

// TODO: figure out if there's a way to merge this with the runtime Scope class
class ParserScope {
public:
	ParserScope() = default;
	ParserScope(std::shared_ptr<ParserScope> parent, bool can_overshadow = false);
	std::pair<IdentifierInfo, bool> get(std::string identifier) const;
	bool contains(std::string identifier) const;
	bool add(std::string identifier, IdentifierInfo info);
	std::shared_ptr<ParserScope> parent();
	void output(std::ostream& out, int indent = 0);

	static std::shared_ptr<ParserScope>& getGlobalScope();
	static void addToGlobalScope(std::string identifier, IdentifierInfo info);
private:
	bool _can_overshadow = false;
	std::shared_ptr<ParserScope> _parent = nullptr;
	std::unordered_map<std::string, IdentifierInfo> _vars;
	static std::shared_ptr<ParserScope> global_scope;
};

class Parser {
public:
	std::pair<std::shared_ptr<ASTNode>, int> parse(TokenStream& tokens);
	void error(const TokenMetaData& meta, const std::string& error);

	ParserScope& scope();
	void pushScope(bool can_overshadow = false);
	void popScope();

	bool inLoop() const;
	void pushLoopState(bool in_loop);
	void popLoopState();
private:
	int _error_count;
	std::shared_ptr<ParserScope> _scope;
	std::stack<bool> _in_loop;
};

#endif
