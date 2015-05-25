#include <string>
#include <cctype>
#include <ios>
#include "lexer.h"

using namespace std;

bool issymbol(char c) {
	static set<char> symbols;

	if (symbols.empty()) {
		for (const auto& op : operators) {
			symbols.insert(op.begin(), op.end());
		}
	}

	return symbols.count(c) > 0;
}

string to_string(char c) {
	return string(1, c);
}

vector<Token> Lexer::tokenize(istream& input, const string& filename) {
	vector<Token> tokens;

	const auto& eof = istream::traits_type::eof();

	int line = 0;
	int starting_column = 0;
	int current_column = 0;

	auto push_token = [&](TokenType type, string text) {
		TokenMetaData meta = { filename, line, starting_column };
		tokens.emplace_back(type, meta, text);
	};

	auto invalid_token = [&](string text, string error) {
		TokenMetaData meta = { filename, line, starting_column };
		throw InvalidTokenError(meta, error, text);
	};

	auto peek = [&] {
		return input.peek();
	};

	auto next_char = [&](char& c) -> bool {
		input >> c;

		if (c == '\n') {
			current_column = 0;
			++line;
		} else {
			++current_column;
		}

		return bool(input);
	};

	input >> noskipws;

	char current_char;
	while (next_char(current_char)) {
		starting_column = current_column - 1;

		if (isspace(current_char)) {
			while (isspace(peek())) {
				next_char(current_char);
			}
		} else if (isalpha(current_char)) {
			string identifier = to_string(current_char);
			while (isalpha(peek())) {
				next_char(current_char);
				identifier += current_char;
			}

			TokenType type = TokenType::Identifier;

			if (keywords.count(identifier) > 0) {
				type = TokenType::Keyword;
			}

			push_token(type, identifier);
		} else if (isdigit(current_char)) {
			string number_literal = to_string(current_char);

			while (isdigit(peek())) {
				next_char(current_char);
				number_literal += current_char;
			}

			push_token(TokenType::NumberLiteral, number_literal);
		} else if (current_char == '\"') {
			string string_literal = to_string(current_char);
			bool closed = false;

			while (next_char(current_char)) {
				string_literal += current_char;

				if (current_char == '\"') {
					closed = true;
					push_token(TokenType::StringLiteral, string_literal);
					break;
				}
			}

			if (!closed) {
				invalid_token(string_literal, "Expected closing double quote");
			}
		} else if (current_char == '#') {
			string comment = "#";

			bool is_block_comment = false;
			if (peek() == '-') {
				is_block_comment = true;
			}

			bool b = false;

			while (next_char(current_char)) {
				if (is_block_comment) {
					if (current_char == '#' && comment.back() == '-') {
						comment += current_char;
						push_token(TokenType::Comment, comment);
						b = true;
						break;
					}

					comment += current_char;

					if (current_char == eof) {
						push_token(TokenType::Comment, comment);
						b = true;
						break;
					}
				} else {
					if (current_char == '\n' || current_char == eof) {
						push_token(TokenType::Comment, comment);
						break;
					}

					comment += current_char;
				}
			}
		} else if (issymbol(current_char)) {
			string op = to_string(current_char);
			auto matches_operator = [] (const string& op_str) {
				return operators.count(op_str) > 0;
			};

			bool operator_was_matched = matches_operator(op);

			while (true) {
				char peeked = peek();

				if (issymbol(peeked)) {
					if (operator_was_matched && !matches_operator(op + peeked)) {
						push_token(TokenType::Operator, op);
						break;
					}

					next_char(current_char);
					op += current_char;
					operator_was_matched = matches_operator(op);
				} else {
					if (operator_was_matched) {
						push_token(TokenType::Operator, op);
					} else {
						invalid_token(op, "Unknown operator");
					}
					break;
				}
			}
		} else {
			string invalid_text = to_string(current_char);

			while (!isspace(peek())) {
				next_char(current_char);
				invalid_text += current_char;
			}

			invalid_token(invalid_text, "Invalid text"); // TODO: better error message
		}
	}

	return tokens;
}

InvalidTokenError::InvalidTokenError(TokenMetaData meta, const string& text, const string& error)
	: runtime_error(error), _text(text), _meta(meta) {
	_error = meta.filename + ":" + to_string(meta.line) + ":" + to_string(meta.column) + ": " + error;
}

const TokenMetaData& InvalidTokenError::meta() const {
	return _meta;
}

const string& InvalidTokenError::text() const {
	return _text;
}

const char* InvalidTokenError::what() const noexcept {
	return _error.c_str();
}
