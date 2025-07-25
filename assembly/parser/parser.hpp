#pragma once

#include <unordered_map>

#include "../common/token.hpp"
#include "../common/ast_node.hpp"
#include "../common/utils.hpp"

class Parser{

public:
    void run(const std::string& source_file_path);
    const std::vector<Ast_Node *>& get_ast_nodes();
private:
    uint32_t _token_index = 0;
    uint32_t _line_index = 0;
    std::unordered_map<std::string,size_t> _labels;
    std::vector<Line> _lines;
    std::vector<Ast_Node *> _heads;// this vector stores ast of each line
    bool exit_code = true;

private:
    void parse_lines();
    void resolve_identifier(Ast_Node* head);
    void set_lines(FILE *source_file);
    Ast_Node *parse_line(Line& line_token);
    const Token *peek(std::vector<Token> &line_tokens);
    const Token *eat(std::vector<Token> &line_tokens);
    void rewind();
    void print_tokens_labels();
};
