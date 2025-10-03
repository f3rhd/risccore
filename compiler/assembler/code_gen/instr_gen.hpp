#pragma once

#include <vector>

#include "../common/ast_node.hpp"
#include "../common/instruction.hpp"

namespace riscv_assembler {
	namespace instr_gen {

		class generator {
			public:
				void generate_instructions(std::vector<Ast_Node *> &heads);
				const std::vector<Instruction> &get_instructions();

			private:
				std::vector<Instruction> convert_to_instr(Ast_Node *head);

			private:
				std::vector<Instruction> _instructions;
		};

	}
}