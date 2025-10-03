#pragma once

#include <string>
#include <vector>
namespace riscv_assembler {
	struct Macro{
		std::vector<std::string> arguments;
		std::vector<std::string> definition;
	};
}