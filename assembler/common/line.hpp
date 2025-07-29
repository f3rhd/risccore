#pragma once
#include <string>
#include <vector>

#include "token.hpp"
struct Line{

    std::vector<Token>          tokens;
    uint32_t                    memory_row_number;
    uint32_t                    true_row_number;
    const std::string*          identifier_str_ptr;
    const std::string*          label_str_ptr;
};