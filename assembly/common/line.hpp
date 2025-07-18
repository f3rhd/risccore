#pragma once
#include <string>
#include <vector>

#include "token.hpp"
struct Line_Context{

    bool has_label;
    bool is_label_only;
    bool has_identifier;
};
struct Line{

    std::string text;
    std::vector<Token> tokens;
    // by extras we mean comments and label names
    uint32_t           memory_row_number;
    uint32_t           true_row_number;
    std::string        identifier;
    std::string        label;
    Line_Context       ctx;
};