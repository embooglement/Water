#include <string>
#include <cctype>
#include <ios>
#include "lexer.h"

using namespace std;

bool issymbol(char c) {
	switch (c) {
		case '(':
		case ')':
		case '<':
		case '>':
		case '[':
		case ']':
		case '{':
		case '}':
		case '=':
		case '!':
		case '+':
		case '-':
		case '*':
		case '/':
		case '%':
		case '^':
			return true;
		default:
			return false;
	}
}

string to_string(char c) {
	return string(1, c);
}

vector<Token> Lexer::tokenize(istream& input) {
	vector<Token> tokens;

	const auto& eof = istream::traits_type::eof();

	auto push_token = [&tokens](TokenType type, string text) {
		tokens.emplace_back(type, text);
	};

	input >> noskipws;

	char current_char = ' ';
	while (input >> current_char) {
		if (isspace(current_char)) {
			while (isspace(input.peek())) {
				input >> current_char;
			}
		} else if (isalpha(current_char)) {
			string identifier = to_string(current_char);
			while (isalpha(input.peek())) {
				input >> current_char;
				identifier += current_char;
			}

			TokenType type = TokenType::Identifier;

			if (keywords.count(identifier) > 0) {
				type = TokenType::Keyword;
			}

			push_token(type, identifier);
		} else if (isdigit(current_char)) {
			string number_literal = to_string(current_char);

			while (isdigit(input.peek())) {
				input >> current_char;
				number_literal += current_char;
			}

			push_token(TokenType::NumberLiteral, number_literal);
		} else if (current_char == '\"') {
			string string_literal = to_string(current_char);
			bool closed = false;

			while (input >> current_char) {
				string_literal += current_char;

				if (current_char == '\"') {
					closed = true;
					push_token(TokenType::StringLiteral, string_literal);
					break;
				}
			}

			if (!closed) {
				push_token(TokenType::InvalidToken, string_literal);
			}
		} else if (current_char == '#') {
			string comment = "#";

			bool is_block_comment = false;
			if (input.peek() == '-') {
				is_block_comment = true;
			}

			bool b = false;

			while (input >> current_char) {
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
				char peeked = input.peek();

				if (issymbol(peeked)) {
					if (operator_was_matched && !matches_operator(op + peeked)) {
						push_token(TokenType::Operator, op);
						break;
					}

					input >> current_char;
					op += current_char;
					operator_was_matched = matches_operator(op);
				} else {
					if (operator_was_matched) {
						push_token(TokenType::Operator, op);
					} else {
						push_token(TokenType::InvalidToken, op);
					}
					break;
				}
			}
		} else {
			string invalid_text = to_string(current_char);

			while (!isspace(input.peek())) {
				input >> current_char;
				invalid_text += current_char;
			}

			push_token(TokenType::InvalidToken, invalid_text);
		}
	}

	return tokens;
}
