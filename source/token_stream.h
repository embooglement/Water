#ifndef _TOKEN_STREAM_H_
#define _TOKEN_STREAM_H_

#include <vector>
#include "token.h"

typedef std::vector<Token>::const_iterator TokenIter;

class TokenStream {
public:
	TokenStream(TokenIter begin, TokenIter end, bool ignore_comments = true);
	bool hasNext() const;
	bool empty() const;
	Token get() const;
	void eat();
	TokenMetaData meta() const;
private:
	TokenMetaData _meta;
	TokenIter _current;
	const TokenIter _end;
	bool _ignore_comments;
};

#endif
