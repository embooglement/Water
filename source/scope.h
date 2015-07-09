#ifndef _SCOPE_H_
#define _SCOPE_H_

#include <unordered_map>
#include "value.h"

class Scope : public std::enable_shared_from_this<Scope> {
public:
	Scope(std::shared_ptr<Scope> parent);
	void add(const std::string& identifier, std::shared_ptr<Value> val);
	void remove(const std::string& identifier);
	std::shared_ptr<Value> get(const std::string& identifier) const;
	bool contains(const std::string& identifier) const;
	std::shared_ptr<Scope> push(); // TODO: rename this to something more appropriate, make const?

	static std::shared_ptr<Scope> getGlobalScope();
private:
	std::unordered_map<std::string, std::shared_ptr<Value>> _vars;
	std::shared_ptr<Scope> _parent;
	static std::shared_ptr<Scope> global_scope;
};

#endif
