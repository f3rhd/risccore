#include "utils.hpp"

namespace utils{

    AST_Node* make_reg_node(const std::string& reg_identifier){

        AST_Node *reg_node = new AST_Node;

        if(reg_node == nullptr){ // @Incomplete : Handle this
        }
        reg_node->identifier = reg_identifier;
        reg_node->node_type = AST_NODE_TYPE::REGISTER;
        return reg_node;
    }

    AST_Node* make_instr_node(const std::string& instr_identifier,instruction_look_up::INSTRUCTION_TYPE instr_type) {
        AST_Node *instr_node = new AST_Node;
        if(instr_node == nullptr){ //@Incomplete : Handle this

        }
        instr_node->identifier = instr_identifier;
        instr_node->node_type = AST_NODE_TYPE::INSTRUCTION;
        instr_node->instr_type = instr_type;

        return instr_node;
    }
    AST_Node *make_imm_node(const std::string& imm_val){
        AST_Node *imm_node = new AST_Node;

        if(imm_node == nullptr){ //@Incomplete : Handle this

        }
        imm_node->identifier = imm_val;
        imm_node->node_type = AST_NODE_TYPE::IMMEDIATE;
        return imm_node;
    }
    AST_Node* make_identifier_node(const std::string& _id){
        AST_Node *label_node = new AST_Node;

        if(label_node == nullptr){ //@Incomplete : handle this

        }
        label_node->identifier = _id;
        label_node->node_type = AST_NODE_TYPE::IDENTIFIER;

        return label_node;
    }
    AST_Node *make_label_node(const std::string& label_id){
        AST_Node *label_node = new AST_Node;

        if(label_node == nullptr){ //@Incomplete : handle this

        }
        label_node->identifier = label_id;
        label_node->node_type = AST_NODE_TYPE::LABEL;

        return label_node;

    }
        std::string ast_node_type_to_string(AST_NODE_TYPE type)
    {

        switch (type)
        {
        case AST_NODE_TYPE::INSTRUCTION:
            return "INSTRUCTION";
        case AST_NODE_TYPE::REGISTER:
            return "REGISTER";
        case AST_NODE_TYPE::IMMEDIATE:
            return "IMMEDIATE";
        case AST_NODE_TYPE::LABEL:
            return "LABEL";
        case AST_NODE_TYPE::DIRECTIVE:
            return "DIRECTIVE";
        //case AST_NODE_TYPE::IDENTIFIER:
        //    return "IDENTIFIER";
        default:
            return "UNKNOWN";
        }
    }
    std::string token_type_to_string(TOKEN_TYPE type) {
        switch (type) {
        case TOKEN_TYPE::INSTRUCTION:
            return "INSTRUCTION";
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

    std::string instr_type_to_string(instruction_look_up::INSTRUCTION_TYPE type) {
        using instruction_look_up::INSTRUCTION_TYPE;
        switch (type) {
        case INSTRUCTION_TYPE::R_TYPE:
            return "R_TYPE";
        case INSTRUCTION_TYPE::S_TYPE:
            return "S_TYPE";
        case INSTRUCTION_TYPE::I_TYPE:
            return "I_TYPE";
        case INSTRUCTION_TYPE::B_TYPE:
            return "B_TYPE";
        case INSTRUCTION_TYPE::J_TYPE:
            return "J_TYPE";
        case INSTRUCTION_TYPE::U_TYPE:
            return "U_TYPE";
        case INSTRUCTION_TYPE::PSEUDO:
            return "PSEUDO";
        case INSTRUCTION_TYPE::UNKNOWN:
            return "UNKNOWN";
        default:
            return "UNKNOWN";
        }
    }
    std::vector<Line> get_lines(FILE *source_file)
    {
        char line_text[500];
        std::vector<Line> lines;

        while(fgets(line_text,sizeof(line_text),source_file)){
            std::vector<Token> line_tokens = tokenizer::tokenize_line_text(line_text);
            std::string _line_text(line_text);
            lines.push_back({_line_text, line_tokens});
        }
        return lines;
    }

    std::string get_label_from_line(const Line& line){

        for(const Token& token : line.tokens){
            if(token.type == TOKEN_TYPE::LABEL){
                return token.word;
            }
        }

        return "";
    }
    bool line_is_label(const Line& line){

        if(line.tokens.size() == 1 && line.tokens[0].type == TOKEN_TYPE::LABEL)
            return true;
        return false;
    }
}