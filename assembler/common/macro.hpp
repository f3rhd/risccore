#pragma once

#include <string>
#include <vector>
namespace f3_riscv_assembler {
	struct Macro{
		std::vector<std::string> arguments;
		std::vector<std::string> definition;
	};
}