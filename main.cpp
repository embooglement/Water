#include <iostream>
#include <fstream>
#include <memory>
#include <vector>
#include <utility>

#include "token.h"
#include "lexer.h"
#include "parser.h"
#include "astnode.h"

using namespace std;

void print_tokens(const vector<Token>& tokens);
void test_ast();

int main(int argc, const char** argv) {
	--argc;
	++argv;

	if (argc == 0) {
		cout << "Missing file argument" << endl;
		return -1;
	}

	auto filename = argv[0];
	ifstream file {filename};

	if (!file) {
		cout << filename << " is not a valid filename" << endl;
		return -1;
	}

	Lexer lexer;
	vector<Token> tokens;

	try {
		// TODO: return error count
		tokens = lexer.tokenize(file, filename);
	} catch (InvalidTokenError error) {
		cerr << "ERROR " << error.what() << endl;
	}

	print_tokens(tokens);
	cout << endl;

	Parser parser;
	shared_ptr<ASTNode> tree;
	int error_count;
	tie(tree, error_count) = parser.parse(begin(tokens), end(tokens));

	if (tree) {
		cout << "\nOutput: " << endl;
		tree->output(cout, 0);
		cout << endl;
	} else {
		cerr << "No parse tree produced" << endl;
	}

	if (error_count > 0) {
		cerr << error_count
		     << (error_count == 1 ? " error" : " errors")
			 << " generated"
			 << endl;
		return -1;
	}

	return 0;
}

void print_tokens(const vector<Token>& tokens) {
	if (tokens.empty()) {
		cout << "no tokens" << endl;
		return;
	}

	for (auto token : tokens) {
		cout << token.type()
			 << ":\t\"" << token.text() << "\""
			 << "\t\t" << token.meta()
			 << endl;
	}
}
/*
void test_ast() {
	typedef shared_ptr<ASTNode> ASTPtr;
	TokenMetaData meta;

	auto make_operator = [&](string op, ASTPtr left, ASTPtr right) {
		return make_shared<BinaryOperatorNode>(
			meta,
			getBuiltin(op),
			left,
			right
		);
	};

	auto make_number = [&](int num) {
		return make_shared<NumberLiteralNode>(
			meta,
			to_string(num)
		);
	};

	auto make_string = [&](string s) {
		return make_shared<StringLiteralNode>(
			meta,
			s
		);
	};

	auto make_id = [&](string name) {
		return make_shared<IdentifierNode>(
			meta,
			name
		);
	};

	auto make_func = [&](string name, vector<ASTPtr> args) {
		return make_shared<FunctionCallNode>(
			meta,
			make_id(name),
			args
		);
	};

	auto make_block = [&](vector<ASTPtr> statements) {
		return make_shared<BlockNode>(
			meta,
			statements
		);
	};

	auto root = make_block({
		make_func(
			"print",
			{
				make_operator(
					"+",
					make_operator(
						"*",
						make_number(2),
						make_number(3)
					),
					make_operator(
						"-",
						make_number(5),
						make_id("x")
					)
				),
				make_operator(
					"%",
					make_id("x"),
					make_id("y")
				)
			}
		),
		make_operator(
			"=",
			make_id("x"),
			make_number(47)
		),
		make_func(
			"foo",
			{
				make_string("hello"),
				make_string("world")
			}
		)
	});

	root->output(cout, 0);
}
*/
