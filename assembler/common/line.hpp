#pragma once
#include <string>
#include <vector>

#include "token.hpp"
struct Line { // This struct instances will be used parser

    std::vector<Token>          tokens;
    uint32_t                    memory_row_number;
    uint32_t                    true_row_number;
    const std::string*          identifier_str_ptr = nullptr;
    const std::string*          label_str_ptr = nullptr;
};