#include <vector>
#include <iostream>

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

void Parser::set_labels(const std::vector<Line>& lines){
    for(const Line& line: lines){

        if(line.ctx.has_label){

            /* 
                Labels could hold the memory addresses of the instruction in the next or the same row in two scenarios.
                start:
                    do something  <- we need to point here that is why line.row_number + 1

                start: do something <- it is ok to just point by line.row_number because they are in the same row
            */
            if(line.ctx.is_label_only){

                labels.insert({line.label, line.row_number+1});
            }else{
                labels.insert({line.label, line.row_number});
            }

        }
    }
}
//void Parser::set_identifiers(const std::vector<Line>& lines){
//    for(const Line& line : lines){
//
//        //since identifier and the line are in the same row
//        if(line.ctx.has_identifier){
//
//            identifiers.insert({line.identifier, line.row_number});
//        }
//        
//    }
//}
void Parser::rewind(){
    token_index = 0;
}
AST_Node* Parser::parse_line(Line& line){

    const Token  *active_token = eat(line.tokens);

    if(active_token == nullptr){
        return nullptr;
    }
    if(active_token->type == TOKEN_TYPE::LABEL)
        eat(line.tokens);
    /*
        lb  ra , 1023(x22)
        beq rs1, rs2 , loop
        addi rs1, rs2 , 1000
        jal  rd, label
    */
    switch(active_token->type){

        case TOKEN_TYPE::OPERATION: {
        

            AST_Node* opr_node  = utils::make_operation_node(active_token->word, instruction_look_up::get_instr_type(active_token->word),&line);
            opr_node->left = parse_line(line);
            const Token *next_token = peek(line.tokens);
            if(next_token != nullptr && next_token->type == TOKEN_TYPE::COMMA){
                eat(line.tokens);
            } else{
                utils::throw_error_message({"Expected ','",opr_node->left->str_value, &line});
                exit(1);
            }
            opr_node->middle = parse_line(line);
            next_token = peek(line.tokens);
            if(next_token != nullptr && (next_token->type == TOKEN_TYPE::COMMA || next_token->type == TOKEN_TYPE::LPAREN)){
                eat(line.tokens);
            }else{
                utils::throw_error_message({"Expected ',' or '('",opr_node->middle->str_value, &line});
                exit(1);
            }
            opr_node->right = parse_line(line);
            // No need eating the RPAREN
            return opr_node;
        }

        /* xor
            / | \
            rd rs1 rs2
        */
        case TOKEN_TYPE::REGISTER : {
            AST_Node *reg_node = utils::make_reg_node(active_token->word,&line);
            return reg_node;
        }
        case TOKEN_TYPE::IMMEDIATE : {
            //lb  ra , 1023(x22)
            /*
                    lb - x22
                    /  \ 
                    ra  1023
            */
            AST_Node *imm_node = utils::make_imm_node(active_token->word,&line);
            return imm_node;
        }
        case TOKEN_TYPE::IDENTIFIER: {
            return utils::make_identifier_node(active_token->word,line.row_number,&line);
        }
        default:
            return nullptr;
    }

}

//void Parser::print_identifiers(){
//
//    std::cout << "Identifiers:\n";
//    for(auto& it : identifiers){
//        std::cout << '\t' << it.second << ": " << it.first << '\n';
//    }
//}

//void Parser::print_labels(){
//
//    std::cout << "Labels:\n";
//    for(auto& it : labels){
//        std::cout << '\t' << it.second << ": " << it.first << '\n';
//    }
//}
void Parser::resolve_identifiers(std::vector<AST_Node*> heads){


    for(AST_Node* head :  heads){
        AST_Node *candidate_identifier_node;
        
        switch(head->opr_type){
            using namespace instruction_look_up;
            case OPERATION_TYPE::B_TYPE: {
                // right leaf should be an identifier
                candidate_identifier_node = head->right;
                break;
            }
            case OPERATION_TYPE::J_TYPE : {
                candidate_identifier_node = head->middle;
                break;
            }

            case OPERATION_TYPE::PSEUDO : { // @Incomplete : Some Pseudo instructions also jump and do shit

            }
        }
        if(head->right->node_type ==  AST_NODE_TYPE::IDENTIFIER){
                    
            // look for label table
            auto it = labels.find(candidate_identifier_node->str_value);

            if(it == labels.end()){ // @Incomplete :  Means that the identifier points to nonexistent label, handle this error too.

            }

            size_t label_row_number = it->second;

            head->right->identifier_immediate = utils::calculate_offset(label_row_number, head->right->identifier_row_number);
        }

    }
}
std::vector<AST_Node*> Parser::parse_lines(std::vector<Line>& lines){

    std::vector<AST_Node*> heads;
    for(Line& line : lines){
        rewind();
        heads.push_back(parse_line(line));
    }
    return heads;
}