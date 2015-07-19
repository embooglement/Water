#include "token_stream.h"

TokenStream::TokenStream(TokenIter begin, TokenIter end, bool ignore_comments)
	: _current(begin), _end(end), _ignore_comments(ignore_comments) {
		if (ignore_comments && _current->type() == TokenType::Comment) {
			eat();
		}
	}

bool TokenStream::hasNext() const {
	return !empty();
}

bool TokenStream::empty() const {
	return _current == _end;
}

Token TokenStream::get() const {
	return *_current;
}

void TokenStream::eat() {
	++_current;

	if (!_ignore_comments) {
		return;
	}

	while (_current != _end) {
		if (_current->type() == TokenType::Comment) {
			++_current;
		} else {
			break;
		}
	}
}
