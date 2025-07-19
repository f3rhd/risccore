#include <vector>
#include <iostream>

#include "parser.hpp"
#include "../common/utils.hpp"

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

                labels.insert({line.label, line.memory_row_number+1});
            }else{
                labels.insert({line.label, line.memory_row_number});
            }

        }
    }
}

void Parser::rewind(){
    token_index = 0;
}
AST_Node* Parser::parse_line(Line& line){

    const Token  *active_token = eat(line.tokens);

    if(active_token == nullptr){
        return nullptr;
    }
    if(active_token->type == TOKEN_TYPE::LABEL){
        active_token = eat(line.tokens);
        // This happens in single line labels only
        /* main:
            ...
        */
        if(active_token == nullptr){
            return nullptr;
        }
    }

    switch(active_token->type){

        case TOKEN_TYPE::OPERATION: {
        
            AST_Node* opr_node  = utils::make_operation_node(active_token->word, instruction_look_up::get_opr_type(active_token->word),&line);
            opr_node->left = parse_line(line);
            const Token *next_token = peek(line.tokens);

            if(next_token != nullptr && next_token->type == TOKEN_TYPE::COMMA){
                eat(line.tokens);
            } else{
                std::string val = next_token ? opr_node->left->str_value : "(null_str)";
                utils::throw_error_message({"Expected ',' ",val, &line});
                exit(1);
            }
            opr_node->middle = parse_line(line);
            next_token = peek(line.tokens);

            // J and U type operations have two operand so we do not need to check for third operand 
            if(opr_node->opr_type != instruction_look_up::OPERATION_TYPE::J_TYPE && opr_node->opr_type != instruction_look_up::OPERATION_TYPE::U_TYPE){
                if(next_token != nullptr && (next_token->type == TOKEN_TYPE::COMMA || next_token->type == TOKEN_TYPE::LPAREN)){
                    eat(line.tokens);
                }else{
                    std::string val = next_token ? opr_node->left->str_value : "(null_str)";
                    utils::throw_error_message({"Expected ',' or '('  ",val, &line});
                    exit(1);
                }
            }
            opr_node->right = parse_line(line);
            // No need eating the RPAREN
            return opr_node;
        }

        case TOKEN_TYPE::REGISTER : {
            AST_Node *reg_node = utils::make_reg_node(active_token->word,&line);
            return reg_node;
        }
        case TOKEN_TYPE::IMMEDIATE : {

            AST_Node *imm_node = utils::make_imm_node(active_token->word,&line);
            return imm_node;
        }
        case TOKEN_TYPE::IDENTIFIER: {
            return utils::make_identifier_node(active_token->word,&line);
        }
        default:
            return nullptr;
    }

}


std::vector<Line> Parser::get_lines(FILE *source_file)
    {
        char line_text[500];
        std::vector<Line> lines;

        int32_t counter = 1;
        int32_t counter2 = 1;
        while(fgets(line_text,sizeof(line_text),source_file)){
            std::vector<Token> line_tokens = tokenizer::tokenize_line_text(line_text);
            std::string _line_text(line_text);
            if (_line_text[0] == '\n' || line_tokens.size() == 0){
                counter2++;
                continue;
            }
            Line line;
            line.text = _line_text;
            line.tokens = line_tokens;
            line.ctx.has_label = utils::line_has_label(line);
            if(line.ctx.has_label)
                line.label = utils::get_label_in_line(line);
            line.ctx.is_label_only = utils::line_is_label_only(line);
            line.ctx.has_identifier = utils::line_has_identifier(line);
            if(line.ctx.has_identifier)
                line.identifier = utils::get_identifier_in_line(line);
            line.memory_row_number = counter;
            line.true_row_number = counter2;
            lines.push_back(line);
            counter++;
            counter2++;
        }
        return lines;
    }
void Parser::resolve_identifiers(std::vector<AST_Node*> heads){


    for(AST_Node* head :  heads){
        AST_Node *candidate_label_identifier_node;
        
        switch(head->opr_type){
            using namespace instruction_look_up;
            case OPERATION_TYPE::PSEUDO_TYPE_3:
            case OPERATION_TYPE::B_TYPE: {
                // right leaf should be an identifier
                candidate_label_identifier_node = head->right;
                break;
            }
            case OPERATION_TYPE::J_TYPE : {
            case OPERATION_TYPE::PSEUDO_TYPE_2:
                candidate_label_identifier_node = head->middle;
                break;
            }

            case OPERATION_TYPE::PSEUDO_TYPE_4:  { // @Incomplete : Some Pseudo instructions also jump and do shit
                candidate_label_identifier_node = head->left;
                break;
            }
            default: // Means that the instruction is not branching type
                continue;
        }

        if( candidate_label_identifier_node && candidate_label_identifier_node->node_type ==  AST_NODE_TYPE::IDENTIFIER){
                    
            // look for label table
            auto it = labels.find(candidate_label_identifier_node->str_value);

            if(it == labels.end()){ 

                // If we are in this block means that the identifier points to non existent label
                // Make the identifier immediate value -1 to be able to tell the ast analyser that this is nullptr type thing
                candidate_label_identifier_node->identifier_immediate = -1;
                continue;
            }

            size_t label_row_number = it->second;

            candidate_label_identifier_node->identifier_immediate = utils::calculate_offset(label_row_number, candidate_label_identifier_node->line_info->memory_row_number);
        }

    }
}
void Parser::print_labels(){
    for(auto& it : labels){
        std::cout << it.first << std::endl;
    }
}
std::vector<AST_Node*> Parser::parse_lines(std::vector<Line>& lines){

    std::vector<AST_Node*> heads;
    for(Line& line : lines){
        rewind();
        AST_Node *head = parse_line(line);
        if(head != nullptr)
            heads.push_back(head);
    }
    return heads;
}