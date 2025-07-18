#pragma once

#include <unordered_map>

#include "../common/token.hpp"
#include "../common/ast_node.hpp"
#include "../common/utils.hpp"

class Parser{

public:
    std::vector<AST_Node*> parse_lines(std::vector<Line> &lines);
    void set_labels(const std::vector<Line>& lines);
    //void set_identifiers(const std::vector<Line> &lines);
    void print_labels();
    //void print_identifiers();
    void resolve_identifiers(std::vector<AST_Node*> heads); // this vector stores ast of each line
private:
    size_t token_index = 0;
    size_t line_index = 0;
    std::unordered_map<std::string,size_t> labels;
    //std::unordered_map<std::string, size_t> identifiers;

private:
    AST_Node *parse_line(Line& line_token);
    const Token *peek(std::vector<Token> &line_tokens);
    const Token *eat(std::vector<Token> &line_tokens);
    void rewind();
};
