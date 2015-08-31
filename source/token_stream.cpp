#include "token_stream.h"
#include <algorithm>

using namespace std;

TokenStream::TokenStream(TokenIter begin, TokenIter end, bool ignore_comments)
	: _current(begin), _end(end), _ignore_comments(ignore_comments) {
		if (ignore_comments && _current->type() == TokenType::Comment) {
			eat();
		}

		if (_current != _end) {
			_meta = _current->meta();
		}
	}

bool TokenStream::hasNext() const {
	return !empty();
}

bool TokenStream::empty() const {
	if (_current == _end) {
		return true;
	}

	if (!_ignore_comments) {
		return false;
	}

	return all_of(_current, _end, [](const Token& token) {
		return token.type() == TokenType::Comment;
	});

	return true;
}

Token TokenStream::get() const {
	return *_current;
}

void TokenStream::eat() {
	++_current;

	while (_ignore_comments && _current != _end) {
		if (_current->type() == TokenType::Comment) {
			++_current;
		} else {
			break;
		}
	}

	// if (hasNext()) {
	// 	_meta = _current->meta();
	// }
}

TokenMetaData TokenStream::meta() const {
	return _meta;
}
