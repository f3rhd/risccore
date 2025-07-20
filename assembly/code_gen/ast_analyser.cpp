#include "ast_analyser.hpp"
namespace ast_analyser{

    // assumes that it head is not nullptr
    static const std::string* val;
    static void ast_has_valid_operation(AST_Node *node, bool &success) {
        if(node->node_type != AST_NODE_TYPE::OPERATION){

            utils::throw_error_message({"Instruction should start with an operation.", node->str_value,node->line_info});
            success = 0;
        }
    };
    static void ast_must_have_register_first(AST_Node* node,bool& success){

        if(node->opr_type != instruction_look_up::OPERATION_TYPE::PSEUDO_TYPE_6 
            && node->opr_type != instruction_look_up::OPERATION_TYPE::PSEUDO_TYPE_4
           && (node->left == nullptr || node->left->node_type != AST_NODE_TYPE::REGISTER)){
            if(node->left == nullptr)
                val = nullptr;
            else
                val = node->left->str_value;
            utils::throw_error_message({"Instruction should have a register as first operand.", val , node->line_info});
            success = 0;
        }
    }
    static void r_type_analysis(AST_Node *head, bool &success) {

        if(head->middle == nullptr || head->middle->node_type != AST_NODE_TYPE::REGISTER){

            if(head->middle == nullptr)
                val = nullptr;
            else
                val = head->middle->str_value;
            utils::throw_error_message({"R-Type operation should have a register as second operand.", val, head->line_info});
            success = 0;
        }
        if(head->right == nullptr || head->right->node_type != AST_NODE_TYPE::REGISTER){
            if(head->right == nullptr)
                val = nullptr;
            else
                val = head->right->str_value;

            utils::throw_error_message({"R-Type operation should have a register as  third operand.", val, head->line_info});
            success = 0;
        }
    };
    static void i_type_analysis(AST_Node* head, bool& success){

        if((*(head->str_value))[0] == 'l' ) // below logic is for lw,lh,lbu,lhu and jalr
        {

            if(head->middle == nullptr || head->middle->node_type != AST_NODE_TYPE::IMMEDIATE){

                if(head->middle == nullptr)
                    val = nullptr;
                else
                    val = head->middle->str_value;
                utils::throw_error_message({"Operation should have an immediate value as second operand.", val, head->line_info});
                success = 0;
            }
            if(head->right == nullptr || head->right->node_type != AST_NODE_TYPE::REGISTER){
                if(head->right == nullptr)
                    val = nullptr;
                else
                    val = head->right->str_value;
                utils::throw_error_message({"Operation should have a register as third operand.", val, head->line_info});
                success = 0;
            }
        } 
        else if((*(head->str_value))[0] == 'j'){

            if(head->middle == nullptr || head->middle->node_type != AST_NODE_TYPE::REGISTER){

                if(head->middle == nullptr)
                    val = nullptr;
                else
                    val = head->middle->str_value;
                utils::throw_error_message({"Operation should have an register value as second operand.", val, head->line_info});
                success = 0;
            }
            if(head->right == nullptr || head->right->node_type != AST_NODE_TYPE::IMMEDIATE){
                if(head->right == nullptr)
                    val = nullptr;
                else
                    val = head->right->str_value;
                utils::throw_error_message({"Operation should have a immediate as third operand.", val, head->line_info});
                success = 0;
            }
        }
        else { //opposite applies for addi,xori and etc.

            if(head->right == nullptr || head->right->node_type != AST_NODE_TYPE::IMMEDIATE){
                if(head->right == nullptr)
                    val = nullptr;
                else
                    val = head->right->str_value;
                utils::throw_error_message({"Operation should have an immediate value as third operand.", head->right->str_value, head->line_info});
                success = 0;
            }
            if(head->middle == nullptr || head->middle->node_type != AST_NODE_TYPE::REGISTER){
                if(head->middle == nullptr)
                    val = nullptr;
                else
                    val = head->middle->str_value;
                utils::throw_error_message({"Operation should have a register as second operand.", val, head->line_info});
                success = 0;
            }
        }
    }
    static void b_type_analysis(AST_Node* head, bool& success){

        if(head->right == nullptr || head->middle->node_type != AST_NODE_TYPE::REGISTER){
            if(head->right == nullptr)
                val = nullptr;
            else
                val = head->right->str_value;
            utils::throw_error_message({"Operation should have a register as second operand.", val, head->line_info});
            success = 0;
        }
        if(head->middle == nullptr || head->right->node_type != AST_NODE_TYPE::IDENTIFIER){
            if(head->middle == nullptr)
                val = nullptr;
            else
                val = head->middle->str_value;
            utils::throw_error_message({"Operation should have a label identifier as third operand.", val, head->line_info});
            success = 0;
        }
        // Parser will make the identifier node's imm value -1 if it points to nowhere
        if(head->right->identifier_immediate == -1){
            utils::throw_error_message({"Identifier points to non-valid label.", head->right->str_value, head->line_info});
            success = 0;
        } 
    }
    static void j_type_analysis(AST_Node *head, bool &success) {

        if(head->middle == nullptr || head->middle->node_type != AST_NODE_TYPE::IDENTIFIER){
            if(head->middle == nullptr)
                val = nullptr;
            else
                val = head->middle->str_value;
            utils::throw_error_message({"Operation should have an label identifier as second operand.", val, head->line_info});
            success = 0;
        }
        if(head->middle->identifier_immediate == -1){
            utils::throw_error_message({"Identifier points to non-valid label.", head->right->str_value, head->line_info});
            success = 0;
        } 
        if(head->right != nullptr){
            utils::throw_error_message({"Operation cannot have a third operand.", head->right->str_value, head->line_info});
            success = 0;
        }
    };
    static void u_type_analysis(AST_Node* head, bool& success){
        if(head->middle == nullptr || head->middle->node_type != AST_NODE_TYPE::IMMEDIATE){
            if(head->middle == nullptr)
                val = nullptr;
            else
                val = head->middle->str_value;
            utils::throw_error_message({"Operation should have an immediate value as second operand.", val, head->line_info});
            success = 0;
        }
        if(head->right != nullptr){
            utils::throw_error_message({"Operation cannot have a third operand.", head->right->str_value, head->line_info});
            success = 0;
        }
    }
    static void pseudo_type_1_analysis(AST_Node *head, bool &success) {

        if(head->middle == nullptr || head->middle->node_type != AST_NODE_TYPE::REGISTER){
            if(head->middle == nullptr)
                val = nullptr;
            else
                val = head->middle->str_value;
            utils::throw_error_message({"Operation should have a register as second operand", val, head->line_info});
            success = 0;
        }

        if(head->right != nullptr){
            val = head->right->str_value;
            utils::throw_error_message({"Operation can not have a third operand.", val, head->line_info});
            success = 0;
        }
    };
    static void pseudo_type_4_analysis(AST_Node *head, bool &success) {
        if(head->left == nullptr || head->left->node_type != AST_NODE_TYPE::IDENTIFIER){
            if(head->left == nullptr)
                val = nullptr;
            else
                val = head->left->str_value;
            utils::throw_error_message({"Operation should have an label identifier as first operand", val, head->line_info});
            success = 0;
        }

        if(head->middle != nullptr){
            utils::throw_error_message({"Operation can not have a second operand", head->middle->str_value, head->line_info});
            success = 0;
        }
        if(head->right != nullptr){

            utils::throw_error_message({"Operation can not have a third operand", head->right->str_value, head->line_info});
            success = 0;
        }
    }
    static void pseudo_type_5_analysis(AST_Node *head, bool &success) {
        if(head->middle != nullptr){
            utils::throw_error_message({"Operation can not have a second operand", head->middle->str_value, head->line_info});
            success = 0;
        }
        if(head->right != nullptr){

            utils::throw_error_message({"Operation can not have a third operand", head->right->str_value, head->line_info});
            success = 0;
        }
    }
    static void pseudo_type_6_analysis(AST_Node *head, bool &success) {
        if(head->left != nullptr || head->middle != nullptr || head->right != nullptr){
            utils::throw_error_message({"Operation can not have any operands", nullptr, head->line_info });
            success = 0;
        }
    }
    static int analyse_line_ast(AST_Node* head) {

        bool success = 1;

        ast_has_valid_operation(head, success);
        ast_must_have_register_first(head, success);
        switch(head->opr_type){

            case instruction_look_up::OPERATION_TYPE::R_TYPE : {
                r_type_analysis(head, success);
                break;
            }
            case instruction_look_up::OPERATION_TYPE::I_TYPE : {
                i_type_analysis(head, success);
                break;
            }

            case instruction_look_up::OPERATION_TYPE::PSEUDO_TYPE_3:
            case instruction_look_up::OPERATION_TYPE::B_TYPE : {
                b_type_analysis(head, success);
                break;
            }
            case instruction_look_up::OPERATION_TYPE::PSEUDO_TYPE_2:
            case instruction_look_up::OPERATION_TYPE::J_TYPE : {
                j_type_analysis(head, success);
                break;
            }
            case instruction_look_up::OPERATION_TYPE::U_TYPE : {
                u_type_analysis(head, success);
                break;
            }
            case instruction_look_up::OPERATION_TYPE::PSEUDO_TYPE_1: {
                pseudo_type_1_analysis(head, success);
                break;
            }
            case instruction_look_up::OPERATION_TYPE::PSEUDO_TYPE_4: {
                pseudo_type_4_analysis(head, success);
                break;
            }
            case instruction_look_up::OPERATION_TYPE::PSEUDO_TYPE_5 : {
                pseudo_type_5_analysis(head, success);
                break;
            }
            case instruction_look_up::OPERATION_TYPE::PSEUDO_TYPE_6: {
                pseudo_type_6_analysis(head,success);
                break;
            }
            case instruction_look_up::OPERATION_TYPE::UNKNOWN: {
                utils::throw_error_message({"Unknown operation was found.", head->right->str_value, head->line_info});
                success = 0;
                break;
            }
        }
        return success;
    }

    int analyse_ast_lines(const std::vector<AST_Node*>& heads){

        int had_error = 0;
        for(AST_Node* head : heads){

            if(analyse_line_ast(head) == 0 && !had_error)
                had_error = true;
        }

        return had_error;
    }
};