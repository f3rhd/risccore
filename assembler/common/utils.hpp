#pragma once
#include <chrono>
#include <cstdio>
#include <iomanip>

#include "ast_node.hpp"
#include "look_up.hpp"
#include "../tokenizer/tokenizer.hpp"
#include "line.hpp"
#include "error.hpp"
namespace utils {

    Ast_Node                  *make_reg_node(const std::string* reg_identfier,Line* line_info_);
    Ast_Node                  *make_operation_node(const std::string* instr_identifier, instruction_look_up::OPERATION_TYPE instr_type, Line* line_info_);
    Ast_Node                  *make_imm_node(const std::string* imm_val, Line* line_info);
    Ast_Node                  *make_identifier_node(const std::string* _id,Line* line_info_);
    std::string               ast_node_type_to_string(AST_NODE_TYPE type);
    std::string               token_type_to_string(TOKEN_TYPE type);
    std::string               opr_type_to_string(instruction_look_up::OPERATION_TYPE type);
    const std::string*        get_label_in_line(const Line &line);
    const std::string*        get_identifier_in_line(const Line &line);
    bool                      line_is_label_only(const Line &line);
    int32_t                   calculate_offset(uint32_t label_row_number, uint32_t identifier_row_number);
    int32_t                   str_to_int32(const std::string &s);
    void                      throw_error_message(const Error_Message &msg);
    void                      free_ast(Ast_Node* head);
    void                      replace_in_string(std::string &str, const std::string &from, const std::string &to);
    std::string               format_duration(std::chrono::nanoseconds ns);
    void                      print_loading_bar(double fraction, int width = 40);
    uintmax_t                 approx_num_of_lines(FILE *fp);
};
