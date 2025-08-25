#pragma once
#include <vector>
#include "../common/token.hpp"
#include "../common/look_up.hpp"
#include "../common/utils.hpp"
namespace f3_riscv_assembler {

	namespace tokenizer {

		std::vector<Token> tokenize_line_text(const std::string& line);
	};
}