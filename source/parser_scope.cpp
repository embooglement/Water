#include <iostream>
#include <functional>

#include "parser_scope.h"
#include "iohelpers.h"

using namespace std;

ParserScope::ParserScope(shared_ptr<ParserScope> parent, bool can_overshadow)
	: _parent(parent), _can_overshadow(can_overshadow) {}

pair<IdentifierInfo, bool> ParserScope::get(string identifier) const {
	auto it = _vars.find(identifier);
	if (it != end(_vars)) {
		return { it->second, true };
	}

	if (_parent) {
		return _parent->get(move(identifier));
	}

	return { {}, false };
}

bool ParserScope::contains(string identifier) const {
	return get(move(identifier)).second;
}

bool ParserScope::add(string identifier, IdentifierInfo info) {
	function<bool(ParserScope*)> can_add = [&identifier, &can_add](ParserScope* scope) -> bool {
		if (!scope || scope->_can_overshadow) {
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

shared_ptr<ParserScope> ParserScope::parent() {
	return _parent;
}

void ParserScope::output(ostream& out, int indent) {
	out << io::indent(indent) << "scope: " << boolalpha << _can_overshadow << endl;

	for (auto p : _vars) {
		out << io::indent(indent) << p.first << ": " << boolalpha << p.second.is_const << endl;
	}

	if (_parent) {
		_parent->output(out, indent + 1);
	}

	out << endl;
}

shared_ptr<ParserScope>& ParserScope::getGlobalScope() {
	return global_scope;
}

void ParserScope::addToGlobalScope(string identifier, IdentifierInfo info) {
	global_scope->_vars.emplace(move(identifier), info);
}

shared_ptr<ParserScope> ParserScope::global_scope = make_shared<ParserScope>(nullptr, true);
