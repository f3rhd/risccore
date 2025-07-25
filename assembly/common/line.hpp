#pragma once
#include <string>
#include <vector>

#include "token.hpp"
struct Line{

    std::vector<Token>          tokens;
    // by extras we mean comments and label names
    uint32_t                    memory_row_number;
    uint32_t                    true_row_number;
    const std::string*          identifier_str_ptr;
    const std::string*          label_str_ptr;
    bool                        macro_related = false;
};