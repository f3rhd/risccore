#pragma once

#include <unordered_map>

#include "../common/token.hpp"
#include "../common/ast_node.hpp"
#include "../common/utils.hpp"

class Parser{

public:
    std::vector<Ast_Node *>& get_ast_nodes();
    void parse_lines(std::vector<Line>& lines,std::unordered_map<std::string,uint64_t>& labels);

private:
    void resolve_label_identifier(Ast_Node* head,std::unordered_map<std::string,uint64_t>& labels);
    Ast_Node *parse_line(Line& line_token);
    const Token *peek(std::vector<Token> &line_tokens);
    const Token *eat(std::vector<Token> &line_tokens);
    void rewind();
private:
    uint32_t _token_index = 0;
    std::vector<Ast_Node *> _heads;// this vector stores ast of each line
    bool successful_exit = true;
};
