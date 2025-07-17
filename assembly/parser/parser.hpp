#pragma once

#include <unordered_map>

#include "../common/token.hpp"
#include "../common/ast_node.hpp"
#include "../common/utils.hpp"

class Parser{

private:
    size_t token_index = 0;
    size_t line_index = 0;
    std::unordered_map<std::string,int> labels;

private:
    const Token *peek(std::vector<Token> &line_tokens);
    const Token *eat(std::vector<Token> &line_tokens);
public:
    AST_Node *parse_line(std::vector<Token> &line);
    void      set_labels(std::vector<Line>& line);
};
