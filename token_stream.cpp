#include "token_stream.h"

TokenStream::TokenStream(TokenIter begin, TokenIter end)
	: _current(begin), _end(end) {}

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
}
