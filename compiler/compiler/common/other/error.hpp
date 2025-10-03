#pragma  once
#include <iostream>
#include "token.hpp"
namespace fs_compiler {

	enum class ERROR_CODE : uint32_t {

		FILE_DOES_NOT_EXIST = 0xFF,
		SYNTAX_ERROR,
		// Lexical Errors
		INVALID_CHAR,
		INVALID_NUMBER_FORMAT,
		UNTERMINATED_COMMENT,
		UNDEFINED_REFERENCE,
		RETURN_DOES_NOT_MATCH,
		LOOP_KEYWORD_IS_OUTSIDE_OF_LOOP,
		FUNCTION_SHOULD_RETURN,
		VAR_IS_NOT_DEFINED_IN_SCOPE,
		CALL_TO_UNDEFINED_FUCNTION,
		FUNCTION_REDEFINITION,
		VARIABLE_REDEFINITION,
		TYPES_DO_NOT_MATCH,
		ARRAYS_ELEMENTS_SHOULD_HAVE_EQUIVALENT_TYPES,
		MAIN_ENTRY_DOES_NOT_EXIST
	};
	struct error_t {

		ERROR_CODE error_code;
		token_t cause_token;
		std::string message;
		error_t(ERROR_CODE err_code, token_t cause,const std::string& msg) : error_code(err_code), cause_token(cause),message(msg) {
			if (cause_token.value.empty())
				std::cout << "error f" << static_cast<uint32_t>(error_code)
				<< "(" << cause.location.row_number
				<< "," << cause.location.column_number
				<< ") : " << message << '\n';
			else
				std::cout << "error f" << static_cast<uint32_t>(error_code) 
					<< "(" << cause.location.row_number 
					<< "," << cause.location.column_number 
					<< ") : " << cause_token.value << " : " << message << '\n';
		};
		error_t(ERROR_CODE err_code,const std::string &cause,const std::string& msg) : error_code(err_code),message(msg) {
			if (cause_token.value.empty())
				std::cout << "error f" << static_cast<uint32_t>(error_code)
				<< "(" << ") : " << message << '\n';
			else
				std::cout << "error f" << static_cast<uint32_t>(error_code) 
					<< "(" << "???"
					<< "," << 	"???"<<
					") : " << cause << " : " << message << '\n';
		};
	};

}
