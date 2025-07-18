#pragma once

#include <vector>

#include "../common/ast_node.hpp"
#include "../common/instruction.hpp"

namespace instr_gen {

    std::vector<Instruction> generate_instructions(const std::vector<AST_Node *> &heads);
}