#include "utils.hpp"
#include <iostream>

namespace utils{

    AST_Node* make_reg_node(const std::string& reg_identifier,Line* line_info_){

        AST_Node *reg_node = new AST_Node;

        if(reg_node == nullptr){ 
            printf("Error! -> Malloc Failed");
            exit(1);
        }
        reg_node->str_value = reg_identifier;
        reg_node->node_type = AST_NODE_TYPE::REGISTER;
        reg_node->line_info = line_info_;
        return reg_node;
    }

    AST_Node* make_operation_node(const std::string& opr_identifier,instruction_look_up::OPERATION_TYPE instr_type,Line* line_info_) {
        AST_Node *opr_node = new AST_Node;
        if(opr_node == nullptr){ 
            printf("Error! -> Malloc Failed");
            exit(1);
        }
        opr_node->str_value = opr_identifier;
        opr_node->node_type = AST_NODE_TYPE::OPERATION;
        opr_node->opr_type = instr_type;
        opr_node->line_info = line_info_;

        return opr_node;
    }
    AST_Node *make_imm_node(const std::string& imm_val,Line* line_info_){
        AST_Node *imm_node = new AST_Node;

        if(imm_node == nullptr){ 
            printf("Error! -> Malloc Failed");
            exit(1);
        }
        imm_node->str_value = imm_val;
        imm_node->node_type = AST_NODE_TYPE::IMMEDIATE;
        imm_node->line_info = line_info_;
        return imm_node;
    }
    AST_Node* make_identifier_node(const std::string& _id,Line* line_info_){
        AST_Node *id_node = new AST_Node;

        if(id_node == nullptr){ 

            printf("Error! -> Malloc Failed");
            exit(1);
        }
        id_node->str_value = _id;
        id_node->node_type = AST_NODE_TYPE::IDENTIFIER;
        id_node->line_info = line_info_;

        return id_node;
    }
    std::string ast_node_type_to_string(AST_NODE_TYPE type)
    {

        switch (type)
        {
        case AST_NODE_TYPE::OPERATION:
            return "OPERATION";
        case AST_NODE_TYPE::REGISTER:
            return "REGISTER";
        case AST_NODE_TYPE::IMMEDIATE:
            return "IMMEDIATE";
        case AST_NODE_TYPE::LABEL:
            return "LABEL";
        case AST_NODE_TYPE::DIRECTIVE:
            return "DIRECTIVE";
        case AST_NODE_TYPE::IDENTIFIER:
            return "IDENTIFIER";
        default:
            return "UNKNOWN";
        }
    }
    std::string token_type_to_string(TOKEN_TYPE type) {
        switch (type) {
        case TOKEN_TYPE::OPERATION:
            return "OPERATION";
        case TOKEN_TYPE::REGISTER:
            return "REGISTER";
        case TOKEN_TYPE::IMMEDIATE:
            return "IMMEDIATE";
        case TOKEN_TYPE::LABEL:
            return "LABEL";
        case TOKEN_TYPE::DIRECTIVE:
            return "DIRECTIVE";
        case TOKEN_TYPE::COMMA:
            return "COMMA";
        case TOKEN_TYPE::LPAREN:
            return "LPAREN";
        case TOKEN_TYPE::RPAREN:
            return "RPAREN";
        case TOKEN_TYPE::IDENTIFIER:
            return "IDENTIFIER";
        default:
            return "UNKNOWN";
        }
    }

    std::string opr_type_to_string(instruction_look_up::OPERATION_TYPE type) {
        using instruction_look_up::OPERATION_TYPE;
        switch (type) {
        case OPERATION_TYPE::R_TYPE:
            return "R_TYPE";
        case OPERATION_TYPE::S_TYPE:
            return "S_TYPE";
        case OPERATION_TYPE::I_TYPE:
            return "I_TYPE";
        case OPERATION_TYPE::B_TYPE:
            return "B_TYPE";
        case OPERATION_TYPE::J_TYPE:
            return "J_TYPE";
        case OPERATION_TYPE::U_TYPE:
            return "U_TYPE";
        case OPERATION_TYPE::PSEUDO:
            return "PSEUDO";
        case OPERATION_TYPE::UNKNOWN:
            return "UNKNOWN";
        default:
            return "UNKNOWN";
        }
    }

    bool line_is_label_only(const Line& line){

        if(line.tokens.size() == 1 && line.tokens[0].type == TOKEN_TYPE::LABEL)
            return true;
        return false;
    } 
    bool line_has_label(const Line& line){
        for(const Token& token : line.tokens){
            if(token.type == TOKEN_TYPE::LABEL){
                return true;
            }
        }
        return false;
    }
    bool line_has_identifier(const Line& line){
        for(const Token& token : line.tokens){
            if(token.type == TOKEN_TYPE::IDENTIFIER){
                return true;
            }
        }
        return false;
    }   
    std::string get_identifier_in_line(const Line& line){
        std::string id;
        for(const Token& token : line.tokens){
            if(token.type == TOKEN_TYPE::IDENTIFIER){
                return token.word;
            }
        }
        return id;
    }
    std::string get_label_in_line(const Line& line){
        std::string id;
        for(const Token& token : line.tokens){
            if(token.type == TOKEN_TYPE::LABEL){
                return token.word;
            }
        }
        return id;
    }
    int32_t calculate_offset(size_t label_row_number, size_t identifier_row_number){

        return (label_row_number - identifier_row_number) * 4;
    }
    void throw_error_message(const Error_Message& msg){

        std::cout << "\n==================== ERROR ====================\n";
        std::cout << "Line " << msg.error_causing_line->true_row_number << ": " << msg.error_causing_line->text;
        std::cout << "Cause:   '" << msg.error_causing_word << "'\n";
        std::cout << "Message: " << msg.message << "\n";
        std::cout << "==============================================\n\n";
    }

    int32_t str_to_int32(const std::string &s){ //@Incomplete
        // Handles decimal and hexadecimal (0x/0X prefix) strings
        int32_t val = 0;
        try {
            if (s.size() > 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
                val = static_cast<int32_t>(std::stol(s, nullptr, 16));
            } else {
                val = static_cast<int32_t>(std::stol(s, nullptr, 10));
            }
        } catch (...) {
            val = 0; // Could add error handling here if needed
        }
        return val;
    }
}