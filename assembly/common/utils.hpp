#pragma once
#include "ast_node.hpp"
#include "look_up.hpp"
#include "tokenizer.hpp"
#include "line.hpp"
namespace utils {

    AST_Node            *make_reg_node(const std::string &reg_identfier);
    AST_Node            *make_instr_node(const std::string &instr_identifier, instruction_look_up::INSTRUCTION_TYPE instr_type);
    AST_Node            *make_imm_node(const std::string& imm_val);
    AST_Node            *make_identifier_node(const std::string &_id);
    AST_Node            *make_label_node(const std::string &label_id);
    std::string         ast_node_type_to_string(AST_NODE_TYPE type);
    std::string         token_type_to_string(TOKEN_TYPE type);
    std::string         instr_type_to_string(instruction_look_up::INSTRUCTION_TYPE type);
    std::vector<Line>   get_lines(FILE *source_file);
    std::string         get_label(const Line &line);
    bool                line_is_label(const Line &line);
};
