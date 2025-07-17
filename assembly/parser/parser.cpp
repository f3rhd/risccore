#include <vector>

#include "parser.hpp"


const Token* Parser::peek(std::vector<Token>& line_tokens){
    if(token_index < line_tokens.size())
        return &line_tokens[token_index];
    return nullptr;
}
const Token* Parser::eat(std::vector<Token>& line_tokens){
    if(token_index < line_tokens.size())
        return &line_tokens[token_index++];
    return nullptr;

}
void Parser::set_labels(std::vector<Line>& lines){

    for(Line& line: lines){

        if(utils::line_is_label(line)){

        } else{

        }
        line_index++;
    }
}
AST_Node* Parser::parse_line(std::vector<Token>& line_tokens){

    const Token  *active_token = eat(line_tokens);

    if(active_token == nullptr){
        return nullptr;
    }
    /*
        lb  ra , 1023(x22)
        beq rs1, rs2 , loop
        addi rs1, rs2 , 1000
        jal  rd, label
    */
    switch(active_token->type){

        case TOKEN_TYPE::INSTRUCTION: {

            AST_Node* instr_node  = utils::make_instr_node(active_token->word, instruction_look_up::get_instr_type(active_token->word));
            instr_node->left = parse_line(line_tokens);
            if(peek(line_tokens)->type == TOKEN_TYPE::COMMA){
                eat(line_tokens);
            }
            instr_node->middle = parse_line(line_tokens);
            if(peek(line_tokens)->type == TOKEN_TYPE::COMMA || peek(line_tokens)->type == TOKEN_TYPE::LPAREN){
                eat(line_tokens);
            }
            instr_node->right = parse_line(line_tokens);
            return instr_node;
        }

        /* xor
            / | \
            rd rs1 rs2
        */
        case TOKEN_TYPE::REGISTER : {
            AST_Node *reg_node = utils::make_reg_node(active_token->word);
            return reg_node;
        }
        case TOKEN_TYPE::IMMEDIATE : {
            //lb  ra , 1023(x22)
            /*
                    lb - x22
                    /  \ 
                    ra  1023
            */
            AST_Node *imm_node = utils::make_imm_node(active_token->word);
            return imm_node;
        }
        case TOKEN_TYPE::LABEL : {

            AST_Node *label_node = utils::make_label_node(active_token->word);
            label_node->middle = parse_line(line_tokens);
        }
        case TOKEN_TYPE::IDENTIFIER: {
            return utils::make_identifier_node(active_token->word);
        }
        default:
            return nullptr;
    }

}
