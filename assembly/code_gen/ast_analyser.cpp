#include "ast_analyser.hpp"
namespace ast_analyser{

    // assumes that it head is not nullptr
    static int analyse_line_ast(AST_Node* head) {

        std::string val;
        bool success = 1;
        if(head->node_type != AST_NODE_TYPE::OPERATION){

            utils::throw_error_message({"Instruction should start with an operation.", head->str_value,head->line_info});
            success = 0;
        }
        
        if(head->left == nullptr || head->left->node_type != AST_NODE_TYPE::REGISTER){
            if(head->left == nullptr)
                val = "";
            else
                val = head->left->str_value;
            utils::throw_error_message({"Instruction should have a register as first operand.", val , head->line_info});
            success = 0;
        }
        switch(head->opr_type){

            case instruction_look_up::OPERATION_TYPE::R_TYPE : {
                if(head->middle == nullptr || head->middle->node_type != AST_NODE_TYPE::REGISTER){

                    if(head->middle == nullptr)
                        val = "";
                    else
                        val = head->middle->str_value;
                    utils::throw_error_message({"R-Type operation should have a register as second operand.", val, head->line_info});
                    success = 0;
                }
                if(head->right == nullptr || head->right->node_type != AST_NODE_TYPE::REGISTER){
                    if(head->right == nullptr)
                        val = "";
                    else
                        val = head->right->str_value;

                    utils::throw_error_message({"R-Type operation should have a register as  third operand.", val, head->line_info});
                    success = 0;
                }
                break;
            }
            case instruction_look_up::OPERATION_TYPE::I_TYPE : {

                if(head->str_value[0] == 'l' || head->str_value[0] == 'j') // below logic is for lw,lh,lbu,lhu
                {

                    if(head->middle == nullptr || head->middle->node_type != AST_NODE_TYPE::IMMEDIATE){

                        if(head->middle == nullptr)
                            val = "";
                        else
                            val = head->middle->str_value;
                        utils::throw_error_message({"I-Type load operation should have an immediate value as second operand.", val, head->line_info});
                        success = 0;
                    }
                    if(head->right == nullptr || head->right->node_type != AST_NODE_TYPE::REGISTER){
                        if(head->right == nullptr)
                            val = "";
                        else
                            val = head->right->str_value;
                        utils::throw_error_message({"I-Type load operation should have a register as third operand.", val, head->line_info});
                        success = 0;
                    }
                    break;
                } 
                else { //opposite applies for addi,xori and etc.

                    if(head->right == nullptr || head->right->node_type != AST_NODE_TYPE::IMMEDIATE){
                        if(head->right == nullptr)
                            val = "";
                        else
                            val = head->right->str_value;
                        utils::throw_error_message({"I-Type non-load operation should have an immediate value as third operand.", head->right->str_value, head->line_info});
                        success = 0;
                    }
                    if(head->middle == nullptr || head->middle->node_type != AST_NODE_TYPE::REGISTER){
                        if(head->middle == nullptr)
                            val = "";
                        else
                            val = head->middle->str_value;
                        utils::throw_error_message({"I-Type non-load operation should have a register as second operand.", val, head->line_info});
                        success = 0;
                    }
                    break;
                }

            }
            case instruction_look_up::OPERATION_TYPE::B_TYPE : {
                    if(head->right == nullptr || head->middle->node_type != AST_NODE_TYPE::REGISTER){
                        if(head->right == nullptr)
                            val = "";
                        else
                            val = head->right->str_value;
                        utils::throw_error_message({"B-Type operation should have a register as second operand.", val, head->line_info});
                        success = 0;
                    }
                    if(head->middle == nullptr || head->right->node_type != AST_NODE_TYPE::IDENTIFIER){
                        if(head->middle == nullptr)
                            val = "";
                        else
                            val = head->middle->str_value;
                        utils::throw_error_message({"B-Type operation should have a label identifier as third operand.", val, head->line_info});
                        success = 0;
                    }
                    // Parser will make the identifier node's imm value -1 if it points to nowhere
                    if(head->right->identifier_immediate == -1){
                        utils::throw_error_message({"Identifier points to non-valid label.", head->right->str_value, head->line_info});
                        success = 0;
                    } 
                    break;
            }
            case instruction_look_up::OPERATION_TYPE::J_TYPE : {
                if(head->middle == nullptr || head->middle->node_type != AST_NODE_TYPE::IDENTIFIER){
                    if(head->middle == nullptr)
                        val = "";
                    else
                        val = head->middle->str_value;
                    utils::throw_error_message({"J-Type operation should have an label identifier as second operand.", val, head->line_info});
                    success = 0;
                }
                if(head->middle->identifier_immediate == -1){
                    utils::throw_error_message({"Identifier points to non-valid label.", head->right->str_value, head->line_info});
                    success = 0;
                } 
                if(head->right != nullptr){
                    utils::throw_error_message({"J-Type operation cannot have a third operand.", head->right->str_value, head->line_info});
                    success = 0;
                }
                break;
            }
            case instruction_look_up::OPERATION_TYPE::U_TYPE : {
                if(head->middle == nullptr || head->middle->node_type != AST_NODE_TYPE::IMMEDIATE){
                    if(head->middle == nullptr)
                        val = "";
                    else
                        val = head->middle->str_value;
                    utils::throw_error_message({"U-Type operation should have an immediate value as second operand.", val, head->line_info});
                    success = 0;
                }
                if(head->right != nullptr){
                    utils::throw_error_message({"J-Type operation cannot have a third operand.", head->right->str_value, head->line_info});
                    success = 0;
                }
                break;
            }
            case instruction_look_up::OPERATION_TYPE::PSEUDO : { //@Incomplete 

            }
            
            case instruction_look_up::OPERATION_TYPE::UNKNOWN: {
                utils::throw_error_message({"Unknown operation was found.", head->right->str_value, head->line_info});
                success = 0;
                break;
            }
        }
        return success;
    }

    int analyse_ast_lines(std::vector<AST_Node*> heads){

        int had_error = 0;
        for(AST_Node* head : heads){

            if(analyse_line_ast(head) == 0 && !had_error)
                had_error = true;
        }

        return had_error;
    }
};