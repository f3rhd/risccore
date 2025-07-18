#include "../common/ast_node.hpp"
#include "../common/instruction.hpp"
#include "../common/utils.hpp"

// generates instruction based on the ast
namespace instr_gen{

    static Instruction convert_to_instr(AST_Node* head){

        // Extract fields from AST_Node
        Instruction instr;
        instr.type = head->opr_type;
        instr.opcode = instruction_look_up::get_op_code(instr.type);
        instr.func3 = instruction_look_up::get_func3(head->str_value);
        instr.func7 = instruction_look_up::get_func7(head->str_value);
        using instruction_look_up::OPERATION_TYPE;
        switch(instr.type){
            case OPERATION_TYPE::R_TYPE:
                // R-type: opcode, rd, rs1, rs2, funct3, funct7
                // add rd,rs1,rs2
                instr.rd = instruction_look_up::get_register_index(head->left->str_value);
                instr.rs1 = instruction_look_up::get_register_index(head->middle->str_value);
                instr.rs2 = instruction_look_up::get_register_index(head->right->str_value);
                break;
            case OPERATION_TYPE::I_TYPE:

                instr.rd = instruction_look_up::get_register_index(head->left->str_value);
                if(head->str_value[0] == 'l'){
                    instr.imm = utils::str_to_int32(head->middle->str_value);
                    instr.rs1 = instruction_look_up::get_register_index(head->right->str_value);
                }else if(head->str_value[0] == 'j') {
                    instr.imm = head->middle->identifier_immediate;
                }
                else{
                    instr.imm = utils::str_to_int32(head->right->str_value);
                    instr.rs1 = instruction_look_up::get_register_index(head->middle->str_value);
                }
                break;
            case OPERATION_TYPE::S_TYPE:
                // S-type: opcode, rs1, rs2, imm, funct3
                // rd and funct7 not used
                // sb rs2 imm(rs1)
                instr.rs2 = instruction_look_up::get_register_index(head->left->str_value);
                instr.imm = utils::str_to_int32(head->middle->str_value);
                instr.rs1 = instruction_look_up::get_register_index(head->right->str_value);
                break;
            case OPERATION_TYPE::B_TYPE:
                // B-type: opcode, rs1, rs2, imm, funct3
                // rd and funct7 not used
                instr.rs1 = instruction_look_up::get_register_index(head->left->str_value);
                instr.rs2 = instruction_look_up::get_register_index(head->middle->str_value);
                instr.imm = head->right->identifier_immediate;
                break;
            case OPERATION_TYPE::J_TYPE:
                // J-type: opcode, rd, imm
                // rs1, rs2, funct3, funct7 not used
                instr.rd = instruction_look_up::get_register_index(head->left->str_value);
                instr.imm = head->middle->identifier_immediate;
                break;
            case OPERATION_TYPE::U_TYPE:
                // U-type: opcode, rd, imm
                // rs1, rs2, funct3, funct7 not used
                instr.rd = instruction_look_up::get_register_index(head->left->str_value);
                instr.imm = utils::str_to_int32(head->middle->str_value);
                break;

            //@Incomplete: Below cases
            case OPERATION_TYPE::PSEUDO:
                // Pseudo: depends on expansion
                break;
            case OPERATION_TYPE::UNKNOWN:
            default:
                // Unknown: do nothing or error
                break;
        }
        return instr;
    }
    std::vector<Instruction> generate_instructions(const std::vector<AST_Node *> &heads){
        std::vector<Instruction> instructions;
        for(AST_Node* head : heads){

            instructions.push_back(convert_to_instr(head));
        }
        return instructions;
    }
}