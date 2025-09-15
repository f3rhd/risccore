#pragma once
#include <string>
#include <cstdint>

namespace f3_compiler {
	struct source_location_t {
		size_t row_number = 1;
		size_t column_number = 1;
	};
	enum class TOKEN_TYPE {

		UKNOWN,

		// (2+3)-(5-4)*3%4 + 4>3 
		PLUS, // +
		MINUS, // -
		SLASH, // /
		STAR, // *
		PERCENTAGE, // %
		LPAREN, // (
		RPAREN, // )
		LCURLY, // {
		RCURLY, // }
		ASSIGNMENT, // =
		MINUS_EQUAL, // -=
		PLUS_EQUAL, // +=
		STAR_EQUAL, // *=
		SLASH_EQUAL, //-=
		PERCENTAGE_EQUAL, // %=
		ARROW, // ->
		GREATER, // >
		GRATER_EQUAL, // >=
		LESS, //  <
		LESS_EQUAL, // <=
		EQUAL, // ==
		SEMICOLON, // ;
		INT_LITERAL,
		EXCLAMATION, // !
		DOUBLE_PLUS, // ++
		DOUBLE_MINUS, // --
		NOT_EQUAL, // !=
		AMPERSAND, // &
		DOUBLE_AMPERSAND, // &&
		DOUBLE_COLUMN, // ||
		DOT, // .
		TRIPLE_DOT, 
		COMMA,

		/*
		@Incomplete : 
		Will later add : ^,>>, <<,?, |,~
		*/

		KW_INT, // int
		KW_UINT, // uint
		KW_VOID, // void
		KW_OR, // or
		KW_AND, // and
		KW_RETURN, // return
		KW_FUNC, // func
		KW_IF, // if
		KW_ELSE, // else
		KW_WHILE, // while
		KW_BREAK, // break
		KW_SKIP, // skip 
		KW_FOR, // for
		/*
		func main() -> void {
		}
		*/
		IDENTIFIER,
		END_OF_FILE
	};
	struct token_t {

		TOKEN_TYPE type = TOKEN_TYPE::UKNOWN;
		std::string value;
		source_location_t location;

		token_t() = default;
		token_t(TOKEN_TYPE typ,const std::string& word,source_location_t loc) : type(typ),value(word),location(loc) {}
	};

}
