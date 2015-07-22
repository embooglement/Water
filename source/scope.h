#ifndef _SCOPE_H_
#define _SCOPE_H_

#include <unordered_map>
#include "value.h"

class Scope : public std::enable_shared_from_this<Scope> {
public:
	Scope(std::shared_ptr<Scope> parent);
	void add(std::string identifier, std::shared_ptr<Value> val);
	void overshadow(std::string identifier, std::shared_ptr<Value> val);
	void remove(const std::string& identifier);
	void update(const std::string& identifier, std::shared_ptr<Value> new_value);
	void update(const std::shared_ptr<Value>& old_value, std::shared_ptr<Value> new_value);
	std::shared_ptr<Value> get(const std::string& identifier) const;
	bool contains(const std::string& identifier) const;
	std::shared_ptr<Scope> createNestedScope();

	static std::shared_ptr<Scope>& getGlobalScope();
	static void addToGlobalScope(std::string identifier, std::shared_ptr<Value> val);
private:
	std::unordered_map<std::string, std::shared_ptr<Value>> _vars;
	std::shared_ptr<Scope> _parent;
	static std::shared_ptr<Scope> global_scope;
};

#endif
