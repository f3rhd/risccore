#include "utils.hpp"
#include <iostream>
#include <format>

namespace utils{

    Ast_Node* make_reg_node(const std::string* reg_identifier,Line* line_info_){

        Ast_Node *reg_node = new Ast_Node;

        if(reg_node == nullptr){ 
            printf("Error! -> Malloc Failed");
            exit(1);
        }
        reg_node->str_ptr_value = reg_identifier;
        reg_node->node_type = AST_NODE_TYPE::REGISTER;
        reg_node->line_info = line_info_;
        return reg_node;
    }

    Ast_Node* make_operation_node(const std::string* opr_identifier,instruction_look_up::OPERATION_TYPE instr_type,Line* line_info_) {
        Ast_Node *opr_node = new Ast_Node;
        if(opr_node == nullptr){ 
            printf("Error! -> Malloc Failed");
            exit(1);
        }
        opr_node->str_ptr_value = opr_identifier;
        opr_node->node_type = AST_NODE_TYPE::OPERATION;
        opr_node->opr_type = instr_type;
        opr_node->line_info = line_info_;

        return opr_node;
    }
    Ast_Node *make_imm_node(const std::string* imm_val,Line* line_info_){
        Ast_Node *imm_node = new Ast_Node;

        if(imm_node == nullptr){ 
            printf("Error! -> Malloc Failed");
            exit(1);
        }
        imm_node->str_ptr_value = imm_val;
        imm_node->node_type = AST_NODE_TYPE::IMMEDIATE;
        imm_node->line_info = line_info_;
        return imm_node;
    }
    Ast_Node* make_identifier_node(const std::string* _id,Line* line_info_){
        Ast_Node *id_node = new Ast_Node;

        if(id_node == nullptr){ 

            printf("Error! -> Malloc Failed");
            exit(1);
        }
        id_node->str_ptr_value = _id;
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

    void replace_in_string(std::string &_str, const std::string &from, const std::string &to){
        size_t start_pos = 0;
        while((start_pos = _str.find(from, start_pos)) != std::string::npos) {
            _str.replace(start_pos, from.length(), to);
            start_pos += to.length(); // Move past the replaced part
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
        case OPERATION_TYPE::PSEUDO_TYPE_0:
        case OPERATION_TYPE::PSEUDO_TYPE_1:
        case OPERATION_TYPE::PSEUDO_TYPE_2:
        case OPERATION_TYPE::PSEUDO_TYPE_3:
        case OPERATION_TYPE::PSEUDO_TYPE_4:
        case OPERATION_TYPE::PSEUDO_TYPE_5:
        case OPERATION_TYPE::PSEUDO_TYPE_6:
            return "PSEUDO";
        case OPERATION_TYPE::UNKNOWN:
            return "UNKNOWN";
        default:
            return "UNKNOWN";
        }
    }

    void free_ast(Ast_Node* head) {
        if (head == nullptr) return;

        free_ast(head->left);
        free_ast(head->right);
        free_ast(head->middle);

        // Free the current node
        delete head;
    }
    bool line_is_label_only(const Line& line){

        if(line.tokens.size() == 1 && line.tokens[0].type == TOKEN_TYPE::LABEL)
            return true;
        return false;
    } 
    const std::string* get_identifier_in_line(const Line& line){
        for(const Token& token : line.tokens){
            if(token.type == TOKEN_TYPE::IDENTIFIER){
                return &token.word;
            }
       }
       return nullptr;
    }
    const std::string* get_label_in_line(const Line& line){
        for(const Token& token : line.tokens){
            if(token.type == TOKEN_TYPE::LABEL){
                return &token.word;
            }
        }
        return nullptr;
    }
    int32_t calculate_offset(uint32_t label_row_number, uint32_t identifier_row_number){

        return (label_row_number - identifier_row_number) * 4;
    }
    void throw_error_message(const Error_Message& msg){

        std::cout << "\n==================== ERROR ====================\n";
        std::cout << "Line " << msg.error_causing_line->true_row_number << ": " ;
        for(const Token& token : msg.error_causing_line->tokens){
            std::cout << token.word << ' ';
        }
        std::cout << '\n';
        if(msg.error_causing_str_ptr == nullptr)
        std::cout << "Cause:   '" << "" << "'\n";
        else
        std::cout << "Cause:   '" << *msg.error_causing_str_ptr << "'\n";
        std::cout << "Message: " << msg.message << "\n";
        std::cout << "==============================================\n\n";
    }

    int32_t str_to_int32(const std::string &s){ 
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
    void print_loading_bar(double fraction, int width) {
        int filled = static_cast<int>(fraction * width + 0.5);
        if (filled > width) filled = width;
        std::string bar(filled, '#');
        bar += std::string(width - filled, '-');
        int percent = static_cast<int>(fraction * 100 + 0.5);
        std::printf("[%-*s] %3d%%\n", width, bar.c_str(), percent);
    }
    std::string format_duration(std::chrono::nanoseconds ns) {
        using namespace std::chrono;
        if (ns >= seconds(1)) {
            double secs = duration<double>(ns).count();
            return std::format("{:.3f} s", secs);
        } else if (ns >= milliseconds(1)) {
            double ms = duration<double, std::milli>(ns).count();
            return std::format("{:.3f} ms", ms);
        } else if (ns >= microseconds(1)) {
            double us = duration<double, std::micro>(ns).count();
            return std::format("{:.3f} µs", us);
        } else {
            double nsd = static_cast<double>(ns.count());
            return std::format("{:.0f} ns", nsd);
        }
    }

    static uintmax_t get_file_size(FILE* file_pointer){
        fseek(file_pointer, 0ull, SEEK_END);
        return ftell(file_pointer);
    }
    uintmax_t approx_num_of_lines(FILE* fp){
        uintmax_t num_of_chars = get_file_size(fp);
        // avg instruction takes 20 chars
        return num_of_chars / 20ull;
    }
}