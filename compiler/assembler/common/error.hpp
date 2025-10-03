#pragma once

#include "line.hpp"

#include <string>
#include <iostream>
namespace riscv_assembler {
	struct Error_Message{

		const std::string& message;
		const std::string* error_causing_str_ptr = nullptr;
		Line *error_causing_line = nullptr;
	};
}
