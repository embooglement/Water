#ifndef _SCOPE_H_
#define _SCOPE_H_

#include <memory>
#include <unordered_map>
#include <boost/optional.hpp>

#include "table.h"
#include "value.h"

struct IdentifierInfo {
	bool is_const;
};

class Scope {
public:
	Scope(std::shared_ptr<Scope> parent, bool can_overshadow = false);
	std::tuple<IdentifierInfo, std::shared_ptr<Value>> get(const std::string& identifier) const;
	boost::optional<IdentifierInfo> getInfo(const std::string& identifier) const;
	std::shared_ptr<Value> getValue(const std::string& identifier) const;
	void setValue(const std::string& identifier, std::shared_ptr<Value> val);
	bool contains(const std::string& identifier) const;
	bool add(std::string identifier, IdentifierInfo info);
	std::shared_ptr<Scope> parent();
	void output(std::ostream& out, int indent = 0);

	static std::shared_ptr<Scope>& getGlobalScope();
	static void addToGlobalScope(std::string identifier, IdentifierInfo info, std::shared_ptr<Value> val);
private:
	bool _can_overshadow = false;
	std::shared_ptr<Scope> _parent = nullptr;
	table<std::string, IdentifierInfo, std::shared_ptr<Value>> _vars;
	static std::shared_ptr<Scope> global_scope;
};

#endif
