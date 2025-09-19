#pragma  once
#include <vector>
#include <sstream>
#include <fstream>

#include "../common/other/error.hpp"
#include "../common/other/token.hpp"

namespace f3_compiler {
	class Lexer {

	public:
		Lexer(const std::string& source_file_path);
		const std::vector<token_t>& get_tokens();
	private:
		std::vector<token_t> tokens;
		void tokenize();
		void make_error(ERROR_CODE err_code, const token_t& cause, const std::string& message);
		std::string token_name(TOKEN_TYPE type);
		bool eof() const;
	private:
		size_t _i = 0;
		size_t _row = 1;
		size_t _col = 1;
		size_t _source_string_size = 0;
		std::vector<error_t> _errors;
		std::ostringstream _source_string_stream;
	};
};
