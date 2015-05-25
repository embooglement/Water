#include <iostream>
#include <fstream>
#include <memory>
#include <vector>

#include "token.h"
#include "lexer.h"
#include "parser.h"
#include "astnode.h"

using namespace std;

int main(int argc, const char** argv) {
	--argc;
	++argv;

	auto filename = argv[0];
	ifstream file {filename};

	Lexer lexer;
	vector<Token> tokens = lexer.tokenize(file);

	for (auto token : tokens) {
		cout << token.type() << ": "
			 << "\"" << token.text() << "\""
			 << endl;
	}

//	Parser parser;
//	shared_ptr<ASTNode> tree = parser.parse(tokens);

	// TODO: print out abstract syntax tree

	return 0;
}
