#include "lexer.hpp"
#include <string_view> 
using namespace f3_compiler;
Lexer::Lexer(const std::string& source_file_path) {

	std::ifstream subject_file = std::ifstream(source_file_path);
	if (!subject_file) {

		token_t file_path_token;
		file_path_token.value = source_file_path;
		make_error(ERROR_CODE::FILE_DOES_NOT_EXIST, file_path_token,"File does not exist.");
		exit(EXIT_FAILURE);
	}

	std::string line;
	while (std::getline(subject_file, line)) {
		_source_string_stream << line << '\n';
	}
	_source_string_size = _source_string_stream.str().size();
	tokenize();
	if (!_errors.empty()) {
		exit(EXIT_FAILURE);
	}
	subject_file.close();
}

const std::vector<token_t>& f3_compiler::Lexer::get_tokens()
{
	return tokens;
}

//std::string Lexer::token_name(TOKEN_TYPE type) {
//		switch (type) {
//		case TOKEN_TYPE::PLUS: return "Token_Type::PLUS";
//		case TOKEN_TYPE::MINUS: return "Token_Type::MINUS";
//		case TOKEN_TYPE::SLASH: return "Token_Type::DIVIDE";
//		case TOKEN_TYPE::STAR: return "Token_Type::STAR";
//		case TOKEN_TYPE::PERCENTAGE: return "Token_Type::PERCENTAGE";
//		case TOKEN_TYPE::LPAREN: return "Token_Type::LPAREN";
//		case TOKEN_TYPE::RPAREN: return "Token_Type::RPAREN";
//		case TOKEN_TYPE::LCURLY: return "Token_Type::LCURLY";
//		case TOKEN_TYPE::RCURLY: return "Token_Type::RCURLY";
//		case TOKEN_TYPE::ASSIGNMENT: return "Token_Type::ASSIGNMENT";
//		case TOKEN_TYPE::ARROW: return "Token_Type::ARROW";
//		case TOKEN_TYPE::GREATER: return "Token_Type::Greater";
//		case TOKEN_TYPE::GRATER_EQUAL: return "Token_Type::GREATER_EQUAL";
//		case TOKEN_TYPE::LESS: return "Token_Type::LESS";
//		case TOKEN_TYPE::LESS_EQUAL: return "Token_Type::LESS_EQUAL";
//		case TOKEN_TYPE::EQUAL: return "Token_Type::EQUAL";
//		case TOKEN_TYPE::SEMICOLON: return "Token_Type::SEMICOLON";
//		case TOKEN_TYPE::INT_LITERAL: return "Token_Type::INT_LITERAL";
//		case TOKEN_TYPE::EXCLAMATION: return "Token_Type::EXCLAMATION";
//		case TOKEN_TYPE::NOT_EQUAL: return "Token_Type::NOT_EQUAL";
//		case TOKEN_TYPE::AMPERSAND: return "Token_Type::AMPERSAND";
//		case TOKEN_TYPE::DOUBLE_AMPERSAND: return "Token_Type::DOUBLE_AMPERSAND";
//		//case TOKEN_TYPE::COLUMN: return "Token_Type::COLUMN";
//		case TOKEN_TYPE::DOUBLE_COLUMN: return "Token_Type::DOUBLE_COLUMN";
//		case TOKEN_TYPE::KW_INT: return "Token_Type::KW_INT";
//		case TOKEN_TYPE::KW_UINT: return "Token_Type::KW_UINT";
//		case TOKEN_TYPE::KW_VOID: return "Token_Type::KW_VOID";
//		case TOKEN_TYPE::KW_OR: return "Token_Type::KW_OR";
//		case TOKEN_TYPE::KW_AND: return "Token_Type::KW_AND";
//		case TOKEN_TYPE::KW_RETURN: return "Token_Type::KW_RETURN";
//		case TOKEN_TYPE::KW_FUNC: return "Token_Type::KW_FUNC";
//		case TOKEN_TYPE::KW_IF: return "Token_Type::KW_IF";
//		case TOKEN_TYPE::KW_ELSE: return "Token_Type::KW_ELSE";
//		case TOKEN_TYPE::KW_WHILE: return "Token_Type::KW_WHILE";
//		case TOKEN_TYPE::KW_FOR: return "Token_Type::KW_FOR";
//		case TOKEN_TYPE::KW_SKIP: return "Token_Type::KW_SKIP";
//		case TOKEN_TYPE::IDENTIFIER: return "Token_Type::KW_IDENTIFIER";
//		case TOKEN_TYPE::DOT:  return "Token_Type::DOT";
//		case TOKEN_TYPE::COMMA: return "Token_Type::COMMA";
//		case TOKEN_TYPE::END_OF_FILE: return "Token_Type::EOF";
//		default: return "UNKNOWN";
//	} 
//}
void Lexer::tokenize() {

	const std::string& source_string = _source_string_stream.str();
	char ch = 0;
	bool comment_start = false;
	auto make_token = [&](TOKEN_TYPE type, const std::string& value, size_t row, size_t column) {
		tokens.emplace_back(type, value, source_location_t{ row, column });
	};
	while (!eof()) {


		ch = source_string[_i];
		// Comments
		if (!comment_start && ch == '/' && source_string[_i + 1] == '*') {
			comment_start = true;
			continue;
		}
		if (comment_start && ch == '*' && source_string[_i + 1] == '/') {
			// Skip '*' and '/'
			_i += 2;
			comment_start = false;
			continue;
		}
		if (std::isspace(static_cast<unsigned char>(ch))) {
			_i++;
			if (ch == '\n') {
				_col = 1;
				_row++;
				continue;
			}
			_col++;
			continue;
		}

		if (comment_start) {
			_col++;
			_i++;
			continue;
		}

		/*
		@Incomplete : 
		Will later add : ^,>>, <<, |,~
		*/
		//Check for single char tokens
		if (ch == ')') {

			make_token(TOKEN_TYPE::RPAREN, std::string(1, ch), _row, _col);
			_i++;
			_col++;
			continue;
		}
		if (ch == '(') {
			make_token(TOKEN_TYPE::LPAREN, std::string(1, ch), _row, _col);
			_i++;
			_col++;
			continue;
		}
		if(ch == '^'){
			make_token(TOKEN_TYPE::CARET, std::string(1, ch), _row, _col);
			_i++;
			_col++;
			continue;
		}
		if(ch == '|'){
			make_token(TOKEN_TYPE::SINGLE_COLUMN, std::string(1, ch), _row, _col);
			_i++;
			_col++;
			continue;
		}
		if(ch == '~'){
			make_token(TOKEN_TYPE::TILDA, std::string(1, ch), _row, _col);
			_i++;
			_col++;
			continue;
		}
		if (ch == '+') {

			if (source_string[_i+1] == '+') {
				make_token(TOKEN_TYPE::DOUBLE_PLUS, std::string("++"), _row, _col);
				_i += 2;
			}
			else if (source_string[_i + 1] == '=') {
				make_token(TOKEN_TYPE::MINUS_EQUAL, std::string("+="), _row, _col);
				_i += 2;
			}
			else {
				make_token(TOKEN_TYPE::PLUS, std::string(1, ch), _row, _col);
				_i++;
			}
			_col++;
			continue;
		}
		if (ch == '-') {

			if (source_string[_i + 1] == '>') {
				make_token(TOKEN_TYPE::ARROW, std::string("->"), _row, _col);
				_i += 2; // Skip the <
			}
			else if (source_string[_i + 1] == '-') {
				make_token(TOKEN_TYPE::DOUBLE_MINUS, std::string("--"), _row, _col);
				_i += 2;
			}
			else if (source_string[_i + 1] == '=') {
				make_token(TOKEN_TYPE::PLUS_EQUAL, std::string("-="), _row, _col);
				_i += 2;
			}
			else {
				make_token(TOKEN_TYPE::MINUS, std::string(1, ch), _row, _col);
				_i++;
			}
			_col++;
			continue;
		}
		if (ch == '!') {
			if (source_string[_i + 1] == '=') {
				make_token(TOKEN_TYPE::NOT_EQUAL, std::string("!="), _row, _col);
				_i += 2;
			}
			else {
				make_token(TOKEN_TYPE::EXCLAMATION, std::string(1, ch), _row, _col);
				_i++;
			}
			_col++;
			continue;
		}
		if (ch == '*') {
			if (source_string[_i + 1] == '=') {
				make_token(TOKEN_TYPE::STAR_EQUAL, std::string("*="), _row, _col);
				_i += 2;
			}
			else {
				make_token(TOKEN_TYPE::STAR, std::string("*"), _row, _col);
				_i++;
			}
			_col++;
			continue;
		}
		if (ch == '/') {
			if (source_string[_i + 1] == '=') {
				make_token(TOKEN_TYPE::SLASH_EQUAL, std::string("/="), _row, _col);
				_i += 2;
			}
			else {
				make_token(TOKEN_TYPE::SLASH, std::string("/"), _row, _col);
				_i++;
			}
			_col++;
			continue;
		}
		if (ch == '%') {

			if (source_string[_i + 1] == '=') {
				make_token(TOKEN_TYPE::PERCENTAGE_EQUAL, std::string("%="), _row, _col);
				_i += 2;
			}
			else {
				make_token(TOKEN_TYPE::PERCENTAGE, std::string("%"), _row, _col);
				_i++;
			}
			_col++;
	
			continue;
		}
		if (ch == '=') {
			if (source_string[_i + 1] == '=') {
				make_token(TOKEN_TYPE::EQUAL, std::string("=="), _row, _col);
				_i += 2;
			}
			else {
				make_token(TOKEN_TYPE::ASSIGNMENT, std::string(1, ch), _row, _col);
				_i++;
			}
			_col++;
			continue;
		}
		if (ch == '{') {

			make_token(TOKEN_TYPE::LCURLY, std::string(1, ch), _row, _col);
			_i++;
			_col++;
			continue;
		}
		if (ch == '}') {

			make_token(TOKEN_TYPE::RCURLY, std::string(1, ch), _row, _col);
			_i++;
			_col++;
			continue;
		}
		if (ch == ';') {
			make_token(TOKEN_TYPE::SEMICOLON, std::string(1, ch), _row, _col);
			_i++;
			_col++;
			continue;
		}
		if (ch == '>') {
			if (source_string[_i + 1] == '=') {
				make_token(TOKEN_TYPE::GRATER_EQUAL, std::string(">="), _row, _col);
				_i += 2;
			}
			else if(source_string[_i+1] == '>' ){
				make_token(TOKEN_TYPE::GREATER_GREATER, std::string(">>"), _row, _col);
				_i += 2;
			}
			else {
				make_token(TOKEN_TYPE::GREATER, std::string(">"), _row, _col);
				_i++;
			}
			_col++;
			continue;
		}
		if (ch == '<') {
			if (source_string[_i + 1] == '=') {
				make_token(TOKEN_TYPE::LESS_EQUAL, std::string("<="), _row, _col);
				_i += 2;
			}
			else if(source_string[_i+1] == '<' ){
				make_token(TOKEN_TYPE::LESS_LESS, std::string("<<"), _row, _col);
				_i += 2;
			}
			else {
				make_token(TOKEN_TYPE::LESS, std::string(1, ch), _row, _col);
				_i++;
			}
			_col++;
			continue;
		}
		if (ch == '!') {
			if (source_string[_i + 1] == '=') {
				make_token(TOKEN_TYPE::NOT_EQUAL, std::string("!="), _row, _col);
				_i += 2;
			}
			else {
				make_token(TOKEN_TYPE::EXCLAMATION, std::string(1, ch), _row, _col);
				_i++;
			}
			_col++;
			continue;
		}
		if (ch == '&') {

			if (source_string[_i + 1] == '&') {
				make_token(TOKEN_TYPE::DOUBLE_AMPERSAND, std::string("&&"), _row, _col);
				_i += 2;
			}
			else {
				make_token(TOKEN_TYPE::AMPERSAND, std::string(1, ch), _row, _col);
				_i++;
			}
			_col++;
			continue;
		}
		if (ch == '|') {
			if (source_string[_i + 1] == '|') {
				make_token(TOKEN_TYPE::DOUBLE_COLUMN, std::string("||"), _row, _col);
				_i += 2;
			}
			else {
				make_token(TOKEN_TYPE::DOUBLE_COLUMN, std::string(1, ch), _row, _col);
				_i++;
			}
			_col++;
			continue;

		}
		if (ch == '.') {
			if (source_string[_i + 1] == '.' && source_string[_i + 2] == '.') {
				make_token(TOKEN_TYPE::TRIPLE_DOT, std::string("..."), _row, _col);
				_i += 3;
			}
			else {
				make_token(TOKEN_TYPE::DOT, std::string(1, ch), _row,_col);
				_i += 1;
			}
			_col++;
			continue;
		}
		if (ch == ',') {
			make_token(TOKEN_TYPE::COMMA, std::string(1, ch), _row, _col);
			_col++;
			_i++;
			continue;
		}
		if (ch == '@' || ch == '#' || ch == '$') { // @Temporary : Might remove some conditions
			token_t invalid_token;
			invalid_token.value = ch;
			invalid_token.location.row_number = _row;
			invalid_token.location.column_number = _col;
			make_error(ERROR_CODE::INVALID_CHAR, invalid_token, { "Unrecognized token." });
			_col++;
			_i++;
			continue;
		}

		// Integers (decimal)
		if (std::isdigit(static_cast<unsigned char>(ch))) {
			std::string  num;
			while (!eof() && std::isdigit(static_cast<unsigned char>(source_string[_i]))) {
				num.push_back(source_string[_i]);
				_i++;
			}
			make_token(TOKEN_TYPE::INT_LITERAL, num, _row, _col);
			continue;
		}

		size_t start = _i;
		while (_i < source_string.size() && std::isalnum(static_cast<unsigned int>(source_string[_i])) || source_string[_i] == '_') {
			_i++;
		}
		if (start == _i)
			continue;

		auto length = _i - start;
		std::string token_str = source_string.substr(start, length);
		if (token_str == "int") {
			make_token(TOKEN_TYPE::KW_INT, token_str, _row, _col);
			_col++;
			continue;
		}
		//if (token_str == "uint") {
		//	make_token(TOKEN_TYPE::KW_UINT, token_str, _row, _col);
		//	_col++;
		//	continue;
		//}
		if (token_str == "void") {
			make_token(TOKEN_TYPE::KW_VOID, token_str, _row, _col);
			_col++;
			continue;
		}
		if (token_str == "or") {
			make_token(TOKEN_TYPE::KW_OR, token_str, _row, _col);
			_col++;
			continue;
		}
		if (token_str == "and") {

			make_token(TOKEN_TYPE::KW_AND, token_str, _row, _col);
			_col++;
			continue;
		}
		if (token_str == "return") {

			make_token(TOKEN_TYPE::KW_RETURN, token_str, _row, _col);
			_col++;
			continue;
		}
		if (token_str == "func") {

			make_token(TOKEN_TYPE::KW_FUNC, token_str, _row, _col);
			_col++;
			continue;
		}
		if (token_str == "if") {

			make_token(TOKEN_TYPE::KW_IF, token_str, _row, _col);
			_col++;
			continue;
		}
		if (token_str == "else") {

			make_token(TOKEN_TYPE::KW_ELSE, token_str, _row, _col);
			_col++;
			continue;
		}
		if (token_str == "while") {
			make_token(TOKEN_TYPE::KW_WHILE, token_str, _row, _col);
			_col++;
			continue;
		}
		if (token_str == "for") {
			make_token(TOKEN_TYPE::KW_FOR, token_str, _row, _col);
			_col++;
			continue;
		}
		if (token_str == "skip") {
			make_token(TOKEN_TYPE::KW_SKIP, token_str, _row, _col);
			_col++;
			continue;
		}
		if (token_str == "break") {
			make_token(TOKEN_TYPE::KW_BREAK, token_str, _row, _col);
			_col++;
			continue;
		}
		make_token(TOKEN_TYPE::IDENTIFIER, token_str, _row, _col);

		_col++;
	}
	if (comment_start) {
		token_t tkn;
		tkn.value = "";
		tkn.location.row_number = _row;
		tkn.location.column_number = _col;
		make_error(ERROR_CODE::UNTERMINATED_COMMENT, tkn,{"Comment is unclosed at EOF."});
	}
	make_token(TOKEN_TYPE::END_OF_FILE, "EOF", _row, _col);
}
bool Lexer::eof() const {
	return _i >= _source_string_size;
}
void Lexer::make_error(ERROR_CODE err_code, const token_t& cause,const std::string& message){
	_errors.emplace_back(err_code, cause,message);
}


