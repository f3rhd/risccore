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
    AST_Node* make_identifier_node(const std::string& _id,size_t identifier_row_number_,Line* line_info_){
        AST_Node *id_node = new AST_Node;

        if(id_node == nullptr){ 

            printf("Error! -> Malloc Failed");
            exit(1);
        }
        id_node->str_value = _id;
        id_node->node_type = AST_NODE_TYPE::IDENTIFIER;
        id_node->identifier_row_number = identifier_row_number_;
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


    std::string get_label_from_line(const Line& line){

        for(const Token& token : line.tokens){
            if(token.type == TOKEN_TYPE::LABEL){
                return token.word;
            }
        }

        return "";
    }
    static bool line_is_label_only(const Line& line){

        if(line.tokens.size() == 1 && line.tokens[0].type == TOKEN_TYPE::LABEL)
            return true;
        return false;
    } 
    static bool line_has_label(const Line& line){
        for(const Token& token : line.tokens){
            if(token.type == TOKEN_TYPE::LABEL){
                return true;
            }
        }
        return false;
    }
    static bool line_has_identifier(const Line& line){
        for(const Token& token : line.tokens){
            if(token.type == TOKEN_TYPE::IDENTIFIER){
                return true;
            }
        }
        return false;
    }   
    static std::string get_identifier_in_line(const Line& line){
        std::string id;
        for(const Token& token : line.tokens){
            if(token.type == TOKEN_TYPE::IDENTIFIER){
                return token.word;
            }
        }
        return id;
    }
    static std::string get_label_in_line(const Line& line){
        std::string id;
        for(const Token& token : line.tokens){
            if(token.type == TOKEN_TYPE::LABEL){
                return token.word;
            }
        }
        return id;
    }
    std::vector<Line> get_lines(FILE *source_file)
    {
        char line_text[500];
        std::vector<Line> lines;

        int counter = 1;
        while(fgets(line_text,sizeof(line_text),source_file)){
            std::vector<Token> line_tokens = tokenizer::tokenize_line_text(line_text);
            std::string _line_text(line_text);
            if (_line_text[0] == '\n')
                continue;
            Line line;
            line.text = _line_text;
            line.tokens = line_tokens;
            line.ctx.has_label = line_has_label(line);
            if(line.ctx.has_label)
                line.label = get_label_in_line(line);
            line.ctx.is_label_only = line_is_label_only(line);
            line.ctx.has_identifier = line_has_identifier(line);
            if(line.ctx.has_identifier)
                line.identifier = get_identifier_in_line(line);
            line.row_number = counter;
            lines.push_back(line);
            counter++;
        }
        return lines;
    }
    int calculate_offset(size_t label_row_number, size_t identifier_row_number){

        return (label_row_number - identifier_row_number) * 4;
    }
    void throw_error_message(const Error_Message& msg){

        std::cout << "Error! ->" << '"'<< msg.error_causing_word << '"' << " in line " << msg.error_causing_line -> row_number << ": " << msg.error_causing_line->text << '\n';
        std::cout << '\t' << msg.message << '\n';
    }
}