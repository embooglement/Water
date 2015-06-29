#include <string>
#include <set>
#include <cctype>
#include <ios>

#include "lexer.h"
#include "constants.h"
#include "errors.h"

using namespace std;

static bool issymbol(char c) {
	return symbol_chars.count(c) > 0;
}

static string to_string(char c) {
	return string(1, c);
}

pair<vector<Token>, int> Lexer::tokenize(istream& input, const string& filename) {
	vector<Token> tokens;
	int error_count = 0;

	const auto& eof = istream::traits_type::eof();

	int line = 0;
	int starting_column = 0;
	int current_column = 0;

	auto push_token = [&](TokenType type, string text) {
		TokenMetaData meta = { filename, line, starting_column };
		tokens.emplace_back(type, meta, text);
	};

	auto invalid_token = [&](string error) {
		++error_count;
		TokenMetaData meta = { filename, line, starting_column };
		this->error(meta, error);
	};

	auto peek = [&] {
		return input.peek();
	};

	auto get_next_char = [&](char& c) -> bool {
		input >> c;

		if (c == '\n') {
			current_column = 0;
			++line;
		} else {
			++current_column;
		}

		return bool(input);
	};

	auto has_input = [&] {
		return bool(input);
	};

	input >> noskipws;

	char current_char;
	while (get_next_char(current_char)) {
		starting_column = current_column - 1;

		if (isspace(current_char)) {
			while (has_input() && isspace(peek())) {
				get_next_char(current_char);
			}
		} else if (isalpha(current_char)) {
			string identifier = to_string(current_char);
			while (has_input() && isalpha(peek())) {
				get_next_char(current_char);
				identifier += current_char;
			}

			TokenType type = TokenType::Identifier;

			if (keywords.count(identifier) > 0) {
				type = TokenType::Keyword;
			}

			push_token(type, identifier);
		} else if (isdigit(current_char)) {
			string number_literal = to_string(current_char);

			while (has_input() && isdigit(peek())) {
				get_next_char(current_char);
				number_literal += current_char;
			}

			push_token(TokenType::NumberLiteral, number_literal);
		} else if (current_char == '\"') {
			string string_literal = "";
			bool closed = false;

			while (has_input() && get_next_char(current_char)) {
				if (current_char == '\"') {
					closed = true;
					push_token(TokenType::StringLiteral, string_literal);
					break;
				} else if (current_char == '\n') {
					break;
				}

				string_literal += current_char;
			}

			if (!closed) {
				invalid_token("expected closing double quote");
			}
		} else if (current_char == '#') {
			string comment = "#";

			bool is_block_comment = false;
			if (peek() == '-') {
				is_block_comment = true;
			}

			bool b = false;

			while (has_input() && get_next_char(current_char)) {
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
			bool operator_was_matched = isBuiltin(op);

			while (has_input()) {
				char peeked = peek();

				if (issymbol(peeked)) {
					if (operator_was_matched && !isBuiltin(op + peeked)) {
						push_token(TokenType::Operator, op);
						break;
					}

					get_next_char(current_char);
					op += current_char;
					operator_was_matched = isBuiltin(op);
				} else {
					if (operator_was_matched) {
						push_token(TokenType::Operator, op);
					} else {
						invalid_token("unknown operator: " + op);
					}
					break;
				}
			}
		} else {
			string invalid_text = to_string(current_char);

			while (has_input() && !isspace(peek())) {
				get_next_char(current_char);
				invalid_text += current_char;
			}

			invalid_token("invalid text: " + invalid_text);
		}
	}

	return { tokens, error_count };
}

void Lexer::error(const TokenMetaData& meta, string error) {
	printError(meta, error);
}
