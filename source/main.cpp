#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>
#include <map>
#include <vector>
#include <utility>

#include "token.h"
#include "lexer.h"
#include "parser.h"
#include "astnode.h"

using namespace std;

void print_tokens(const vector<Token>& tokens);

map<string, vector<string>> getParams(int argc, const char** argv) {
	--argc;
	++argv;

	map<string, vector<string>> params;

	for (int i = 0; i < argc; ++i) {
		string param = argv[i];

		if (param == "-pt" || param == "--print-tokens") {
			params["print-tokens"].push_back("true");
		} else if (param == "-pa" || param == "--print-ast") {
			params["print-ast"].push_back("true");
		} else if (param == "-r") {
			++i;
			if (i >= argc) {
				params["errors"].push_back("missing string to evaluate for -r option");
				break;
			}

			params["evaluate"].push_back(argv[i]);
		} else {
			params["files"].push_back(argv[i]);
		}
	}

	return params;
}

bool paramIsSet(const map<string, vector<string>>& params, string param) {
	auto it = params.find(param);
	return it != end(params) && !it->second.empty();
}

void printParams(const map<string, vector<string>>& params) {
	for (auto&& p : params) {
		cout << "(" << p.first << ", [";
		for (int i = 0; i < p.second.size(); ++i) {
			cout << p.second[i];
			if (i + 1 < p.second.size()) {
				cout << ",";
			}
		}
		cout << "])" << endl;
	}
}

int main(int argc, const char** argv) {
	auto params = getParams(argc, argv);

	auto exit_with_errors = [](int error_count) {
		cout << "Exiting with " << error_count << (error_count == 1 ? " error" : " errors") << endl;
	};

	Lexer lexer;
	vector<Token> tokens;
	int error_count = 0;

	if (paramIsSet(params, "evaluate")) {
		auto str = params["evaluate"][0];
		istringstream stream {str};
		tie(tokens, error_count) = lexer.tokenize(stream, "(command line)");
	} else if (paramIsSet(params, "files")) {
		auto filename = params["files"][0];
		ifstream file {filename};

		if (file.is_open()) {
			tie(tokens, error_count) = lexer.tokenize(file, filename);
		} else {
			++error_count;
			cerr << "ERROR: " << filename << " not found" << endl;
		}
	} else {
		tie(tokens, error_count) = lexer.tokenize(cin, "(stdin)");
	}

	if (error_count > 0) {
		exit_with_errors(error_count);
		return -1;
	}

	if (paramIsSet(params, "print-tokens")) {
		print_tokens(tokens);
	}

	TokenStream token_stream {begin(tokens), end(tokens)};
	Parser parser;
	shared_ptr<ASTNode> tree;

	tie(tree, error_count) = parser.parse(token_stream);

	bool print_ast = paramIsSet(params, "print-ast");
	if (tree) {
		if (print_ast) {
			cout << "\nOutput: " << endl;
			tree->output(cout, 0);
			cout << endl;
			cout << "\nEvaluate: " << endl;
		}

		try {
			setupGlobalScope();
			auto eval = tree->evaluate(Scope::getGlobalScope());
			if (eval) {
				eval->output(cout);
			}
		} catch (const exception& ex) {
			++error_count;
			cerr << ex.what();
		}

		cout << endl;
	} else if (print_ast) {
		cout << "No parse tree produced" << endl;
	}

	if (error_count > 0) {
		exit_with_errors(error_count);
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
