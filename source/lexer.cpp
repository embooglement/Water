#include <string>
#include <set>
#include <cctype>
#include <ios>

#include "lexer.h"
#include "constants.h"
#include "errors.h"

using namespace std;

static bool isSymbol(char c) {
	return symbol_chars.count(c) > 0;
}

static bool isIdentifier(char c, bool allow_digits = false) {
	return (isalpha(c) || c == '_') || (allow_digits && isdigit(c));
}

static string to_string(char c) {
	return string(1, c);
}

pair<vector<Token>, int> Lexer::tokenize(istream& input, const string& filename) {
	vector<Token> tokens;
	int error_count = 0;

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
		this->error(meta, move(error));
	};

	auto peek = [&] {
		return input.peek();
	};

	auto eat_char = [&]() -> char {
		char c;
		input >> c;

		if (!input) {
			return 0;
		}

		if (c == '\n') {
			current_column = 0;
			++line;
		} else {
			++current_column;
		}

		return c;
	};

	auto has_input = [&] {
		return bool(input) && !input.eof();
	};

	input >> noskipws;

	char current_char;
	while (has_input()) {
		starting_column = current_column;
		current_char = eat_char();

		// End of file
		if (current_char == 0) {
			break;

		// Whitespace
		} else if (isspace(current_char)) {
			while (has_input() && isspace(peek())) {
				eat_char();
			}

		// Identifiers
		} else if (isIdentifier(current_char)) {
			string identifier = to_string(current_char);
			while (has_input() && isIdentifier(peek(), true)) {
				current_char = eat_char();
				identifier += current_char;
			}

			TokenType type = TokenType::Identifier;

			if (keywords.count(identifier) > 0) {
				type = TokenType::Keyword;
			}

			push_token(type, identifier);

		// Number literals
		} else if (isdigit(current_char)) {
			string number_literal = to_string(current_char);
			bool missing_fractional_part = false;

			while (has_input() && isdigit(peek())) {
				current_char = eat_char();
				number_literal += current_char;
			}

			if (has_input() && peek() == '.') {
				eat_char();
				number_literal += '.';
				missing_fractional_part = true;

				while (has_input() && isdigit(peek())) {
					missing_fractional_part = false;
					current_char = eat_char();
					number_literal += current_char;
				}
			}

			if (missing_fractional_part) {
				invalid_token("missing fractional part of number literal");
			} else {
				push_token(TokenType::NumberLiteral, number_literal);
			}

		// String Literals
		} else if (current_char == '\"' || current_char == '\'') {
			string string_literal = "";
			bool is_double_quoted = current_char == '\"';
			bool last_char_is_slash = false;

			while (has_input()) {
				current_char = eat_char();

				if (current_char == '\\') {
					last_char_is_slash = true;
					continue;
				}

				if (last_char_is_slash) {
					switch (current_char) {
						case '\\':
							string_literal += '\\';
							break;
						case 'n':
							string_literal += '\n';
							break;
						case 't':
							string_literal += '\t';
							break;
						case '\'':
							string_literal += '\'';
							break;
						case '\"':
							string_literal += '\"';
							break;
					}

					last_char_is_slash = false;
				} else {
					if (is_double_quoted && current_char == '\"') {
						push_token(TokenType::StringLiteral, string_literal);
						break;
					} else if (!is_double_quoted && current_char == '\'') {
						push_token(TokenType::StringLiteral, string_literal);
						break;
					} else if (current_char == '\n' || !has_input()) {
						if (is_double_quoted) {
							invalid_token(errors::expected_close_double_quote);
						} else {
							invalid_token(errors::expected_close_single_quote);
						}
					}

					string_literal += current_char;
				}
			}

		// Comments
		} else if (current_char == '#') {
			string comment = "#";

			bool is_block_comment = false;
			if (peek() == '-') {
				is_block_comment = true;
			}

			while (has_input()) {
				current_char = eat_char();

				if (is_block_comment) {
					bool last_char_was_hyphen = comment.back() == '-';
					comment += current_char;

					if ((last_char_was_hyphen && current_char == '#') || !has_input()) {
						push_token(TokenType::Comment, "BLOCK COMMENT " + comment);
						break;
					}
				} else {
					if (current_char == '\n' || !has_input()) {
						push_token(TokenType::Comment, "LINE COMMENT " + comment);
						break;
					}

					comment += current_char;
				}
			}

		// Operators
		} else if (isSymbol(current_char)) {
			string op = to_string(current_char);
			bool operator_was_matched = isBuiltin(op);

			while (has_input()) {
				char peeked = peek();

				if (isSymbol(peeked)) {
					if (operator_was_matched && !isBuiltin(op + peeked)) {
						push_token(TokenType::Operator, op);
						break;
					}

					current_char = eat_char();
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

		// Invalid text
		} else {
			string invalid_text = to_string(current_char);

			while (has_input() && !isspace(peek())) {
				current_char = eat_char();
				invalid_text += current_char;
			}

			invalid_token("invalid text: " + invalid_text);
		}
	}

	return { tokens, error_count };
}

void Lexer::error(const TokenMetaData& meta, string error) {
	printError(meta, move(error));
}
