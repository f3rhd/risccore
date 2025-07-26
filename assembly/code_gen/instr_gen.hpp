#pragma once

#include <vector>

#include "../common/ast_node.hpp"
#include "../common/instruction.hpp"

namespace instr_gen {

    class generator {
        public:
            void generate_instructions(const std::vector<Ast_Node *> &heads);
            const std::vector<Instruction> &get_instructions();

        private:
            std::vector<Instruction> convert_to_instr(Ast_Node *head);

        private:
            std::vector<Instruction> _instructions;
    };

}