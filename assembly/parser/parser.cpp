#include <vector>
#include <iostream>

#include "parser.hpp"
#include "../common/utils.hpp"
#include "../code_gen/ast_analyser.hpp"

const Token* Parser::peek(std::vector<Token>& line_tokens){
    if(_token_index < line_tokens.size())
        return &line_tokens[_token_index];
    return nullptr;
}
const Token* Parser::eat(std::vector<Token>& line_tokens){
    if(_token_index < line_tokens.size())
        return &line_tokens[_token_index++];
    return nullptr;

}

void Parser::rewind(){
    _token_index = 0;
}
Ast_Node* Parser::parse_line(Line& line){

    const Token  *active_token = eat(line.tokens);

    if(active_token == nullptr){
        return nullptr;
    }
    if(active_token->type == TOKEN_TYPE::LABEL){
        active_token = eat(line.tokens);

        _labels.emplace(*line.label_str_ptr, line.memory_row_number);
        // This happens in label only lines
        /* main:
            ...
        */
        if(active_token == nullptr){
            return nullptr;
        }
    }

    switch(active_token->type){

        case TOKEN_TYPE::OPERATION: {
        
            Ast_Node* opr_node  = utils::make_operation_node(&(active_token->word), instruction_look_up::get_opr_type(active_token->word),&line);
            opr_node->left = parse_line(line);
            const Token *comma_lp_token = peek(line.tokens);

            // jalr and jal instructions can act as pseudo even tho they are truly not
            if((*(opr_node->str_ptr_value))[0] == 'j'){
              if(opr_node->opr_type == instruction_look_up::OPERATION_TYPE::I_TYPE && comma_lp_token == nullptr)
                    opr_node->opr_type = instruction_look_up::OPERATION_TYPE::PSEUDO_TYPE_5;
              else if(opr_node->opr_type == instruction_look_up::OPERATION_TYPE::J_TYPE && comma_lp_token == nullptr)
                  opr_node->opr_type = instruction_look_up::OPERATION_TYPE::PSEUDO_TYPE_4;
            }
            if(comma_lp_token != nullptr && comma_lp_token->type == TOKEN_TYPE::COMMA){
                eat(line.tokens);
            } else{
                if(opr_node->opr_type != instruction_look_up::OPERATION_TYPE::PSEUDO_TYPE_4 &&  
                    opr_node->opr_type != instruction_look_up::OPERATION_TYPE::PSEUDO_TYPE_5 &&
                    opr_node->opr_type != instruction_look_up::OPERATION_TYPE::PSEUDO_TYPE_6){ // these opreations do not need comma
                    const std::string* val = comma_lp_token ? opr_node->left->str_ptr_value : nullptr;
                    utils::throw_error_message({"Expected ',' ",val, &line});
                    exit(1);
                }
            }
            opr_node->middle = parse_line(line);
            comma_lp_token = peek(line.tokens);

            // J,U,PSEUDO_1 and PSEUDO_2 type operations have two operands, so we do not need to check for second comma
            if(opr_node->opr_type != instruction_look_up::OPERATION_TYPE::PSEUDO_TYPE_4 &&  
                opr_node->opr_type != instruction_look_up::OPERATION_TYPE::PSEUDO_TYPE_5 &&
                opr_node->opr_type != instruction_look_up::OPERATION_TYPE::PSEUDO_TYPE_1 &&
                opr_node->opr_type != instruction_look_up::OPERATION_TYPE::PSEUDO_TYPE_2 &&
                opr_node->opr_type != instruction_look_up::OPERATION_TYPE::PSEUDO_TYPE_6 &&
                opr_node->opr_type != instruction_look_up::OPERATION_TYPE::J_TYPE && 
                opr_node->opr_type != instruction_look_up::OPERATION_TYPE::U_TYPE){
                if(comma_lp_token != nullptr && (comma_lp_token->type == TOKEN_TYPE::COMMA || comma_lp_token->type == TOKEN_TYPE::LPAREN)){
                    eat(line.tokens);
                }else{
                    const std::string* val = comma_lp_token ? opr_node->left->str_ptr_value : nullptr;
                    utils::throw_error_message({"Expected ',' or '('  ",val, &line});
                    exit(1);
                }
            }
            opr_node->right = parse_line(line);
            return opr_node;
        }

        case TOKEN_TYPE::REGISTER : {
            Ast_Node *reg_node = utils::make_reg_node(&(active_token->word),&line);
            return reg_node;
        }
        case TOKEN_TYPE::IMMEDIATE : {

            Ast_Node *imm_node = utils::make_imm_node(&(active_token->word),&line);
            return imm_node;
        }
        case TOKEN_TYPE::IDENTIFIER: {
            return utils::make_identifier_node(&(active_token->word),&line);
        }
        default:
            return nullptr;
    }

}

void Parser::set_lines(FILE *source_file)
{
    char line_text[500];

    uint32_t counter = 1; // true line number
    uint32_t memory_row_number = 1; // instruction address
    uint32_t label_amount = 0;

    bool macro_start = false;
    bool macro_finish = false;
    while(fgets(line_text,sizeof(line_text),source_file)){
        Line _line;
        _line.tokens = tokenizer::tokenize_line_text(line_text);

        if (line_text[0] == '\n' || _line.tokens.size() == 0) {
            counter++;
            continue;
        }
        if(!macro_start && _line.tokens[0].word == ".macro"){
            macro_start = true;
            macro_finish = false;
            counter++;
            continue;
        }
        if(macro_start){
            if(_line.tokens[0].word == ".endm"){
                macro_start = false;
                macro_finish = true;
                continue;
            }
            if(macro_finish){
                utils::throw_error_message({"Macro definition ended before .endm directive", nullptr, &_line});
                exit(1);
            }
            counter++;
            continue;
        }
        if(_line.tokens[0].type == TOKEN_TYPE::LABEL){
            label_amount++;
        }
        _lines.push_back(_line);
        Line &line = _lines[_lines.size() - 1];
        line.label_str_ptr = utils::get_label_in_line(line);
        line.identifier_str_ptr = utils::get_identifier_in_line(line);
        line.true_row_number = counter;

        // If the line is label-only (first token is LABEL and only one token), do not increment memory_row_number
        if (line.tokens.size() == 1 && line.tokens[0].type == TOKEN_TYPE::LABEL) {
            line.memory_row_number = memory_row_number;
        } else {
            line.memory_row_number = memory_row_number;
            memory_row_number++;
        }
        counter++;
    }

    _heads.reserve(counter);
    _labels.reserve(label_amount);
}
void Parser::resolve_identifier(Ast_Node* head){

        Ast_Node *candidate_label_identifier_node;
        
        switch(head->opr_type){
            using namespace instruction_look_up;
            case OPERATION_TYPE::PSEUDO_TYPE_3:
            case OPERATION_TYPE::B_TYPE: {
                // right leaf should be an identifier
                candidate_label_identifier_node = head->right;
                break;
            }
            case OPERATION_TYPE::J_TYPE : 
            case OPERATION_TYPE::PSEUDO_TYPE_2:{
                candidate_label_identifier_node = head->middle;
                break;
            }

            case OPERATION_TYPE::PSEUDO_TYPE_4:  { // @Incomplete : Some Pseudo instructions also jump and do shit
                candidate_label_identifier_node = head->left;
                break;
            }
            default: // Means that the instruction is not branching type
                return;
        }

        if( candidate_label_identifier_node && candidate_label_identifier_node->node_type ==  AST_NODE_TYPE::IDENTIFIER){
                    
            // look for label table
            auto it = _labels.find(*candidate_label_identifier_node->str_ptr_value);

            if(it == _labels.end()){ 

                // If we are in this block means that the identifier points to non existent label
                // Make the identifier immediate value -1 to be able to tell the ast analyser that this is nullptr type thing
                candidate_label_identifier_node->identifier_immediate = -1;
                return;
            }

            size_t label_row_number = it->second;

            candidate_label_identifier_node->identifier_immediate = utils::calculate_offset(label_row_number, candidate_label_identifier_node->line_info->memory_row_number);
        }

}
void Parser::parse_lines(){

    for(Line& line : _lines){
        rewind();
        Ast_Node *head = parse_line(line);
        if(head != nullptr)
            _heads.push_back(head);
    }
    for(Ast_Node* head : _heads){
        resolve_identifier(head);
        if(ast_analyser::analyse_line_ast(head) != 1)
            exit_code = false;

    }
}
void Parser::print_tokens_labels(){
    //Tokens
    for(Line& line : _lines){
        std::cout << "Line " << line.true_row_number << '(' << line.memory_row_number << ')' << ':' << '\n';
        for (Token &token : line.tokens)
        {
            std::cout << '\t' << token.word << "@" << &token.word << "=" << utils::token_type_to_string(token.type) << '\n';
        }
    }
}
void Parser::run(const std::string& processed_file_path){

    FILE* processed_source_file = fopen(processed_file_path.c_str(), "rb");
    if(processed_source_file == nullptr){
        std::cerr << "Error opening file: " << processed_file_path << std::endl;
        exit(1);
    }
    set_lines(processed_source_file);
#ifdef PRINT_TOKENS_LABELS
    print_tokens_labels();
#endif
    parse_lines();
    if(exit_code == false){
        printf("Assembling failed.\n");
        exit(1);
    }
    fclose(processed_source_file);
    remove(processed_file_path.c_str());
}
const std::vector<Ast_Node *>& Parser::get_ast_nodes(){
    return _heads;
}