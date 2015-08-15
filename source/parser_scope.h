#ifndef _PARSER_SCOPE_H_
#define _PARSER_SCOPE_H_

#include <utility>
#include <memory>
#include <string>
#include <unordered_map>

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

#endif
