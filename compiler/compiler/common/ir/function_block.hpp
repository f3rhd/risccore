#pragma once
#include "ir_gen_context.hpp"
#include <unordered_set>
struct function_block_t {
    int32_t frame_size = 0;
    ir_instruction_t *label_instr = nullptr;
    std::vector<ir_instruction_t *> instructions;
    std::unordered_map<std::string,int32_t> local_vars;
};