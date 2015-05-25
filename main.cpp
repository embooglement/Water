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
	vector<Token> tokens;

	try {
		tokens = lexer.tokenize(file, filename);
	} catch (InvalidTokenError error) {
		cout << error.what() << endl;
	}

	for (auto token : tokens) {
		cout << token.type()
			 << ":\t\"" << token.text() << "\""
			 << "\t\t" << token.meta()
			 << endl;
	}

//	Parser parser;
//	shared_ptr<ASTNode> tree = parser.parse(tokens);

	// TODO: print out abstract syntax tree

	return 0;
}
