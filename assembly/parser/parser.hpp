#pragma once

#include <unordered_map>

#include "../common/token.hpp"
#include "../common/ast_node.hpp"
#include "../common/utils.hpp"

class Parser{

public:
    void run(FILE *source_file);
    const std::vector<AST_Node *> get_ast_nodes();
private:
    uint32_t _token_index = 0;
    uint32_t _line_index = 0;
    std::unordered_map<const std::string*,size_t> _labels;
    std::vector<Line> _lines;
    std::vector<AST_Node *> _heads;

private:
    std::vector<AST_Node*> parse_lines();
    void set_labels();
    void resolve_identifiers(); // this vector stores ast of each line
    void print_labels();
    void set_lines(FILE *source_file); // returns the number of memory rows
    AST_Node *parse_line(Line& line_token);
    const Token *peek(std::vector<Token> &line_tokens);
    const Token *eat(std::vector<Token> &line_tokens);
    void rewind();
    void print_tokens();
};
