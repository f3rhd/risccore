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
    size_t             row_number;
    std::string        identifier;
    std::string        label;
    Line_Context       ctx;
};