#pragma once
#include <string>
#include <vector>

#include "token.hpp"
struct Line{

    std::string text;
    std::vector<Token> tokens;
};