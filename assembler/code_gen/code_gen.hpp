
#pragma once
#include <vector>
#include "../common/instruction.hpp"

#include <cstdint>
namespace code_gen {

    void generate_bin_file(const std::string &output_file_name, const std::vector<Instruction> &instructions);
};