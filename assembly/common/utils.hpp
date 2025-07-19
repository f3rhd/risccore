#pragma once
#include "ast_node.hpp"
#include "look_up.hpp"
#include "tokenizer.hpp"
#include "line.hpp"
#include "error.hpp"
#include <regex>
namespace utils {

    AST_Node            *make_reg_node(const std::string &reg_identfier,Line* line_info_);
    AST_Node            *make_operation_node(const std::string &instr_identifier, instruction_look_up::OPERATION_TYPE instr_type, Line* line_info_);
    AST_Node            *make_imm_node(const std::string& imm_val, Line* line_info);
    AST_Node            *make_identifier_node(const std::string &_id,Line* line_info_);
    std::string         ast_node_type_to_string(AST_NODE_TYPE type);
    std::string         token_type_to_string(TOKEN_TYPE type);
    std::string         opr_type_to_string(instruction_look_up::OPERATION_TYPE type);
    std::string         get_label_in_line(const Line &line);
    std::string         get_identifier_in_line(const Line &line);
    bool                line_has_identifier(const Line &line);
    bool                line_has_label(const Line &line);
    bool                line_is_label_only(const Line &line);
    int32_t             calculate_offset(uint32_t label_row_number, uint32_t identifier_row_number);
    int32_t             str_to_int32(const std::string &s);
    void                throw_error_message(const Error_Message &msg);
};
