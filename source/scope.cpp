#include <utility>
#include "scope.h"

using namespace std;

shared_ptr<Scope> Scope::global_scope = make_shared<Scope>(nullptr);

Scope::Scope(shared_ptr<Scope> parent)
	: _parent(move(parent)) {}

void Scope::add(string identifier, shared_ptr<Value> val) {
	if (contains(identifier)) {
		throw DeclarationError(identifier);
	}

	_vars.emplace(move(identifier), move(val));
}

void Scope::overshadow(string identifier, shared_ptr<Value> val) {
	_vars.emplace(move(identifier), move(val));
}

void Scope::remove(const string& identifier) {
	auto it = _vars.find(identifier);
	if (it == end(_vars)) {
		if (_parent) {
			_parent->remove(identifier);
			return;
		}

		throw UndefinedVariableError(identifier);
	}

	_vars.erase(identifier);
}

void Scope::update(const string& identifier, shared_ptr<Value> val) {
	auto it = _vars.find(identifier);
	if (it == end(_vars)) {
		if (_parent) {
			_parent->update(identifier, move(val));
			return;
		}

		throw UndefinedVariableError(identifier);
	}

	_vars[identifier] = val;
}

shared_ptr<Value> Scope::get(const string& identifier) const {
	auto it = _vars.find(identifier);
	if (it == end(_vars)) {
		if (_parent) {
			return _parent->get(identifier);
		}

		throw UndefinedVariableError(identifier);
	}

	return it->second;
}

bool Scope::contains(const string& identifier) const {
	if (_vars.find(identifier) != end(_vars)) {
		return true;
	}

	if (_parent) {
		return _parent->contains(identifier);
	}

	return false;
}

shared_ptr<Scope> Scope::push() {
	return make_shared<Scope>(shared_from_this());
}

shared_ptr<Scope>& Scope::getGlobalScope() {
	return global_scope;
}

void Scope::addToGlobalScope(string identifier, shared_ptr<Value> val) {
	global_scope->add(move(identifier), move(val));
}
