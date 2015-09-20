#include <iostream>
#include <functional>
#include <utility>

#include "scope.h"
#include "iohelpers.h"

using namespace std;

shared_ptr<Scope> Scope::global_scope = make_shared<Scope>(nullptr, false);

Scope::Scope(shared_ptr<Scope> parent, bool is_function_scope)
	: _parent(move(parent)), _is_function_scope(is_function_scope) {}

bool Scope::add(string identifier, IdentifierInfo info) {
	function<bool(Scope*)> can_add = [&identifier, &can_add](Scope* scope) -> bool {
		if (!scope || scope->isFunctionScope()) {
			return true;
		}

		if (scope->_vars.find(identifier) != end(scope->_vars)) {
			return false;
		}

		return can_add(scope->parent().get());
	};

	if (can_add(this)) {
		_vars.emplace(move(identifier), info);
		return true;
	}

	return false;
}

shared_ptr<Scope> Scope::parent() {
	return _parent;
}

bool Scope::isFunctionScope() const {
	return _is_function_scope;
}

tuple<IdentifierInfo, shared_ptr<Value>> Scope::get(const string& identifier) const {
	auto it = _vars.find(identifier);
	if (it != end(_vars)) {
		return it->second;
	}

	if (_parent) {
		return _parent->get(identifier);
	}

	return {};
}

boost::optional<IdentifierInfo> Scope::getInfo(const string& identifier) const {
	return std::get<0>(get(identifier));
}

shared_ptr<Value> Scope::getValue(const string& identifier) const {
	return std::get<1>(get(identifier));
}

void Scope::setValue(const string& identifier, shared_ptr<Value> val) {
	auto it = _vars.find(identifier);
	if (it == end(_vars)) {
		if (_parent) {
			_parent->setValue(identifier, move(val));
			return;
		}

		throw UndefinedVariableError(identifier);
	}

	if (val->isReferenceType()) {
		std::get<1>(it->second) = move(val);
	} else {
		std::get<1>(it->second) = val->copy();
	}
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

void Scope::clearValues() {
	for (auto&& var : _vars) {
		std::get<1>(var.second) = nullptr;
	}
}

shared_ptr<Scope>& Scope::getGlobalScope() {
	return global_scope;
}

void Scope::addToGlobalScope(string identifier, IdentifierInfo info, shared_ptr<Value> val) {
	global_scope->add(identifier, info);
	global_scope->setValue(identifier, val);
}
