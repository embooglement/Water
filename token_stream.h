#ifndef _TOKEN_STREAM_H_
#define _TOKEN_STREAM_H_

#include <vector>
#include "token.h"

typedef std::vector<Token>::const_iterator TokenIter;

class TokenStream {
public:
	TokenStream(TokenIter begin, TokenIter end);
	bool hasNext() const;
	// TODO: add a function that does !hasNext()
	Token get() const;
	void eat();
private:
	TokenIter _current;
	const TokenIter _end;
};

#endif
