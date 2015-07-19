#include <iostream>
#include "errors.h"

using namespace std;

void printError(const TokenMetaData& meta, string error) {
	cerr << "ERROR "
		 << meta.filename << ":"
		 << meta.line << ":"
		 << meta.column << ": "
		 << move(error) << endl;
}
