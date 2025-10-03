#pragma once
#include <cstdint>
#include "look_up.hpp"
namespace riscv_assembler {
	struct Instruction {
		instruction_look_up::OPERATION_TYPE type = instruction_look_up::OPERATION_TYPE::UNKNOWN;
		uint8_t rs1 = 0;
		uint8_t rs2 = 0;
		uint8_t rd = 0;
		int32_t imm = 0;      
		uint8_t opcode = 0;
		uint8_t func3 = 0;
		uint8_t func7 = 0;
	};
}