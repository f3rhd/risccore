#pragma once

#include <unordered_map>

#include "../common/token.hpp"
#include "../common/ast_node.hpp"
#include "../common/utils.hpp"

class Parser{

public:
    std::vector<Ast_Node *>& get_ast_nodes();
    void parse_lines(std::vector<Line>& lines);

private:
    void resolve_identifier(Ast_Node* head);
    Ast_Node *parse_line(Line& line_token);
    const Token *peek(std::vector<Token> &line_tokens);
    const Token *eat(std::vector<Token> &line_tokens);
    void rewind();
private:
    uint32_t _token_index = 0;
    uint32_t _line_index = 0;
    std::unordered_map<std::string,size_t> _labels;
    std::vector<Ast_Node *> _heads;// this vector stores ast of each line
    bool exit_code = true;
};
