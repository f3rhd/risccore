#pragma once

#include <unordered_map>

#include "../common/token.hpp"
#include "../common/ast_node.hpp"
#include "../common/utils.hpp"

class Parser{

public:
    std::vector<AST_Node*> parse_lines(std::vector<Line> &lines);
    void set_labels(const std::vector<Line>& lines);
    void resolve_identifiers(std::vector<AST_Node*> heads); // this vector stores ast of each line
    void print_labels();
    std::vector<Line>   get_lines(FILE *source_file);
private:
    uint32_t token_index = 0;
    uint32_t line_index = 0;
    std::unordered_map<std::string,size_t> labels;

private:
    AST_Node *parse_line(Line& line_token);
    const Token *peek(std::vector<Token> &line_tokens);
    const Token *eat(std::vector<Token> &line_tokens);
    void rewind();
};
