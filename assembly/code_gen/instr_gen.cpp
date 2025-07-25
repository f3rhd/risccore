#include "../common/ast_node.hpp"
#include "../common/instruction.hpp"
#include "../common/utils.hpp"
#include "instr_gen.hpp"

// generates instruction based on the ast
namespace instr_gen{

    Instruction generator::convert_to_instr(Ast_Node* head){

        // Extract fields from AST_Node
        Instruction instr;
        instr.type = head->opr_type;
        instr.opcode = instruction_look_up::get_op_code(instr.type,*head->str_ptr_value);
        instr.func3 = instruction_look_up::get_func3(*head->str_ptr_value);
        instr.func7 = instruction_look_up::get_func7(*head->str_ptr_value);
        using instruction_look_up::OPERATION_TYPE;
        switch(instr.type){
            case OPERATION_TYPE::R_TYPE:{
                // R-type: opcode, rd, rs1, rs2, funct3, funct7
                // add rd,rs1,rs2
                instr.rd = instruction_look_up::get_register_index(*head->left->str_ptr_value);
                instr.rs1 = instruction_look_up::get_register_index(*head->middle->str_ptr_value);
                instr.rs2 = instruction_look_up::get_register_index(*head->right->str_ptr_value);
                break;
            }
            case OPERATION_TYPE::I_TYPE:{

                instr.rd = instruction_look_up::get_register_index(*head->left->str_ptr_value);
                if((*(head->str_ptr_value))[0] == 'l'){
                    instr.imm = utils::str_to_int32(*(head->middle->str_ptr_value));
                    instr.rs1 = instruction_look_up::get_register_index(*head->right->str_ptr_value);
                }else if((*(head->str_ptr_value))[0] == 'j') {
                    instr.imm = head->middle->identifier_immediate;
                }
                else{
                    instr.imm = utils::str_to_int32(*head->right->str_ptr_value);
                    instr.rs1 = instruction_look_up::get_register_index(*head->middle->str_ptr_value);
                }
                break;
            }
            case OPERATION_TYPE::S_TYPE:{
                // S-type: opcode, rs1, rs2, imm, funct3
                // rd and funct7 not used
                // sb rs2 imm(rs1)
                instr.rs2 = instruction_look_up::get_register_index(*head->left->str_ptr_value);
                instr.imm = utils::str_to_int32(*head->middle->str_ptr_value);
                instr.rs1 = instruction_look_up::get_register_index(*head->right->str_ptr_value);
                break;
            }
            case OPERATION_TYPE::B_TYPE:{
                // B-type: opcode, rs1, rs2, imm, funct3
                // rd and funct7 not used
                instr.rs1 = instruction_look_up::get_register_index(*head->left->str_ptr_value);
                instr.rs2 = instruction_look_up::get_register_index(*head->middle->str_ptr_value);
                instr.imm = head->right->identifier_immediate;
                break;
            }
            case OPERATION_TYPE::J_TYPE: {
                // J-type: opcode, rd, imm
                // rs1, rs2, funct3, funct7 not used
                instr.rd = instruction_look_up::get_register_index(*head->left->str_ptr_value);
                instr.imm = head->middle->identifier_immediate;
                break;
            }
            case OPERATION_TYPE::U_TYPE: {
                // U-type: opcode, rd, imm
                // rs1, rs2, funct3, funct7 not used
                instr.rd = instruction_look_up::get_register_index(*head->left->str_ptr_value);
                instr.imm = utils::str_to_int32(*head->middle->str_ptr_value);
                break;
            }
            case OPERATION_TYPE::PSEUDO_TYPE_1:
            case OPERATION_TYPE::PSEUDO_TYPE_2:
            case OPERATION_TYPE::PSEUDO_TYPE_3:
            case OPERATION_TYPE::PSEUDO_TYPE_4:
            case OPERATION_TYPE::PSEUDO_TYPE_5:
            case OPERATION_TYPE::PSEUDO_TYPE_6:{ // I hate this...
                if(*head->str_ptr_value == "nop"){
                    instr.opcode = instruction_look_up::get_op_code(instruction_look_up::OPERATION_TYPE::I_TYPE,*head->str_ptr_value);
                    instr.type = instruction_look_up::OPERATION_TYPE::I_TYPE;
                    instr.rd = instruction_look_up::get_register_index("zero");
                    instr.rs1 = instruction_look_up::get_register_index("zero");
                    instr.imm = 0;
                }
                else if(*head->str_ptr_value == "mv"){
                    instr.opcode = instruction_look_up::get_op_code(instruction_look_up::OPERATION_TYPE::I_TYPE,*head->str_ptr_value);
                    instr.type = instruction_look_up::OPERATION_TYPE::I_TYPE;
                    instr.rd = instruction_look_up::get_register_index(*head->left->str_ptr_value);
                    instr.rs1 = instruction_look_up::get_register_index(*head->middle->str_ptr_value);
                    instr.imm = 0;
                }
                else if(*head->str_ptr_value == "not"){
                    instr.opcode = instruction_look_up::get_op_code(instruction_look_up::OPERATION_TYPE::I_TYPE,*head->str_ptr_value);
                    instr.type = instruction_look_up::OPERATION_TYPE::I_TYPE;
                    instr.func3 = instruction_look_up::get_func3("xori");
                    instr.rd = instruction_look_up::get_register_index(*head->left->str_ptr_value);
                    instr.rs1 = instruction_look_up::get_register_index(*head->middle->str_ptr_value);
                    instr.imm = -1;
                }
                else if(*head->str_ptr_value == "neg"){
                    instr.opcode = instruction_look_up::get_op_code(instruction_look_up::OPERATION_TYPE::R_TYPE);
                    instr.type = instruction_look_up::OPERATION_TYPE::R_TYPE;
                    instr.func7 = instruction_look_up::get_func7("sub");
                    instr.rd = instruction_look_up::get_register_index(*head->left->str_ptr_value);
                    instr.rs1 = instruction_look_up::get_register_index("zero");
                    instr.rs2 = instruction_look_up::get_register_index(*head->middle->str_ptr_value);

                }
                else if(*head->str_ptr_value  ==  "seqz"){

                    instr.opcode = instruction_look_up::get_op_code(instruction_look_up::OPERATION_TYPE::I_TYPE,*head->str_ptr_value);
                    instr.type = instruction_look_up::OPERATION_TYPE::I_TYPE;
                    instr.func3 = instruction_look_up::get_func3("sltiu");
                    instr.rd = instruction_look_up::get_register_index(*head->left->str_ptr_value);
                    instr.rs1 = instruction_look_up::get_register_index(*head->middle->str_ptr_value);
                    instr.imm = 1;
                }
                else if(*head->str_ptr_value == "snez"){

                    instr.opcode = instruction_look_up::get_op_code(instruction_look_up::OPERATION_TYPE::R_TYPE);
                    instr.type = instruction_look_up::OPERATION_TYPE::R_TYPE;
                    instr.func3 = instruction_look_up::get_func3("sltu");
                    instr.rd = instruction_look_up::get_register_index(*head->left->str_ptr_value);
                    instr.rs1 = instruction_look_up::get_register_index("zero");
                    instr.rs2 = instruction_look_up::get_register_index(*head->middle->str_ptr_value);
                }
                else if(*head->str_ptr_value == "sltz"){
                    instr.opcode = instruction_look_up::get_op_code(instruction_look_up::OPERATION_TYPE::R_TYPE);
                    instr.type = instruction_look_up::OPERATION_TYPE::R_TYPE;
                    instr.func3 = instruction_look_up::get_func3("slt");
                    instr.rd = instruction_look_up::get_register_index(*head->left->str_ptr_value);
                    instr.rs1 = instruction_look_up::get_register_index(*head->middle->str_ptr_value);
                    instr.rs2 = instruction_look_up::get_register_index("zero");
                }
                else if(*head->str_ptr_value == "sgtz"){
                    instr.opcode = instruction_look_up::get_op_code(instruction_look_up::OPERATION_TYPE::R_TYPE);
                    instr.type = instruction_look_up::OPERATION_TYPE::R_TYPE;
                    instr.func3 = instruction_look_up::get_func3("slt");
                    instr.rd = instruction_look_up::get_register_index(*head->left->str_ptr_value);
                    instr.rs1 = instruction_look_up::get_register_index("zero");
                    instr.rs2 = instruction_look_up::get_register_index(*head->middle->str_ptr_value);
                }
                else if(*head->str_ptr_value == "beqz"){
                    instr.opcode = instruction_look_up::get_op_code(instruction_look_up::OPERATION_TYPE::B_TYPE);
                    instr.type = instruction_look_up::OPERATION_TYPE::B_TYPE;
                    instr.rs1 = instruction_look_up::get_register_index(*head->left->str_ptr_value);
                    instr.rs2 = instruction_look_up::get_register_index("zero");
                    instr.imm = head->middle->identifier_immediate;
                }
                else if(*head->str_ptr_value == "bnez"){
                    instr.opcode = instruction_look_up::get_op_code(instruction_look_up::OPERATION_TYPE::B_TYPE);
                    instr.type = instruction_look_up::OPERATION_TYPE::B_TYPE;
                    instr.func3 = instruction_look_up::get_func3("bne");
                    instr.rs1 = instruction_look_up::get_register_index(*head->left->str_ptr_value);
                    instr.rs2 = instruction_look_up::get_register_index("zero");
                    instr.imm = head->middle->identifier_immediate;
                }
                else if(*head->str_ptr_value == "blez"){

                    instr.opcode = instruction_look_up::get_op_code(instruction_look_up::OPERATION_TYPE::B_TYPE);
                    instr.type = instruction_look_up::OPERATION_TYPE::B_TYPE;
                    instr.func3 = instruction_look_up::get_func3("bge");
                    instr.rs2 = instruction_look_up::get_register_index(*head->left->str_ptr_value);
                    instr.rs1 = instruction_look_up::get_register_index("zero");
                    instr.imm = head->middle->identifier_immediate;
                }
                else if(*head->str_ptr_value == "bgez") {
                    instr.opcode = instruction_look_up::get_op_code(instruction_look_up::OPERATION_TYPE::B_TYPE);
                    instr.type = instruction_look_up::OPERATION_TYPE::B_TYPE;
                    instr.func3 = instruction_look_up::get_func3("bge");
                    instr.rs1 = instruction_look_up::get_register_index(*head->left->str_ptr_value);
                    instr.rs2 = instruction_look_up::get_register_index("zero");
                    instr.imm = head->middle->identifier_immediate;

                }
                else if(*head->str_ptr_value == "bltz"){

                    instr.opcode = instruction_look_up::get_op_code(instruction_look_up::OPERATION_TYPE::B_TYPE);
                    instr.type = instruction_look_up::OPERATION_TYPE::B_TYPE;
                    instr.func3 = instruction_look_up::get_func3("blt");
                    instr.rs1 = instruction_look_up::get_register_index(*head->left->str_ptr_value);
                    instr.rs2 = instruction_look_up::get_register_index("zero");
                    instr.imm = head->middle->identifier_immediate;

                }
                else if(*head->str_ptr_value == "bgtz"){
                    instr.opcode = instruction_look_up::get_op_code(instruction_look_up::OPERATION_TYPE::B_TYPE);
                    instr.type = instruction_look_up::OPERATION_TYPE::B_TYPE;
                    instr.func3 = instruction_look_up::get_func3("blt");
                    instr.rs2 = instruction_look_up::get_register_index(*head->left->str_ptr_value);
                    instr.rs1 = instruction_look_up::get_register_index("zero");
                    instr.imm = head->middle->identifier_immediate;

                }
                else if(*head->str_ptr_value == "ble"){

                    instr.opcode = instruction_look_up::get_op_code(instruction_look_up::OPERATION_TYPE::B_TYPE);
                    instr.type = instruction_look_up::OPERATION_TYPE::B_TYPE;
                    instr.func3 = instruction_look_up::get_func3("bge");
                    instr.rs1 = instruction_look_up::get_register_index(*head->middle->str_ptr_value);
                    instr.rs2 = instruction_look_up::get_register_index(*head->left->str_ptr_value);
                    instr.imm = head->right->identifier_immediate;
                }
                else if(*head->str_ptr_value == "bgt"){
                    instr.opcode = instruction_look_up::get_op_code(instruction_look_up::OPERATION_TYPE::B_TYPE);
                    instr.type = instruction_look_up::OPERATION_TYPE::B_TYPE;
                    instr.func3 = instruction_look_up::get_func3("blt");
                    instr.rs1 = instruction_look_up::get_register_index(*head->middle->str_ptr_value);
                    instr.rs2 = instruction_look_up::get_register_index(*head->left->str_ptr_value);
                    instr.imm = head->right->identifier_immediate;

                }
                else if(*head->str_ptr_value == "bleu") {
                    instr.opcode = instruction_look_up::get_op_code(instruction_look_up::OPERATION_TYPE::B_TYPE);
                    instr.type = instruction_look_up::OPERATION_TYPE::B_TYPE;
                    instr.func3 = instruction_look_up::get_func3("bgeu");
                    instr.rs1 = instruction_look_up::get_register_index(*head->middle->str_ptr_value);
                    instr.rs2 = instruction_look_up::get_register_index(*head->left->str_ptr_value);
                    instr.imm = head->right->identifier_immediate;

                }
                else if(*head->str_ptr_value == "bgtu") {
                    instr.opcode = instruction_look_up::get_op_code(instruction_look_up::OPERATION_TYPE::B_TYPE);
                    instr.type = instruction_look_up::OPERATION_TYPE::B_TYPE;
                    instr.func3 = instruction_look_up::get_func3("bltu");
                    instr.rs1 = instruction_look_up::get_register_index(*head->middle->str_ptr_value);
                    instr.rs2 = instruction_look_up::get_register_index(*head->left->str_ptr_value);
                    instr.imm = head->right->identifier_immediate;
                }
                else if(*head->str_ptr_value == "j") {

                    instr.opcode = instruction_look_up::get_op_code(instruction_look_up::OPERATION_TYPE::J_TYPE);
                    instr.type = instruction_look_up::OPERATION_TYPE::J_TYPE;
                    instr.rd = instruction_look_up::get_register_index("zero");
                    instr.imm = head->left->identifier_immediate;
                }
                else if(*(head->str_ptr_value) == "jal"){

                    instr.opcode = instruction_look_up::get_op_code(instruction_look_up::OPERATION_TYPE::J_TYPE);
                    instr.type = instruction_look_up::OPERATION_TYPE::J_TYPE;
                    instr.rd = instruction_look_up::get_register_index("ra");
                    instr.imm = head->left->identifier_immediate;
                }
                else if(*head->str_ptr_value == "jr") {

                    instr.opcode = instruction_look_up::get_op_code(instruction_look_up::OPERATION_TYPE::I_TYPE,*head->str_ptr_value);
                    instr.type = instruction_look_up::OPERATION_TYPE::I_TYPE;
                    instr.rd = instruction_look_up::get_register_index("zero");
                    instr.rs1 = instruction_look_up::get_register_index(*head->left->str_ptr_value);
                    instr.imm = 0;
                }
                else if(*head->str_ptr_value == "jalr") {
                    instr.opcode = instruction_look_up::get_op_code(instruction_look_up::OPERATION_TYPE::I_TYPE,*head->str_ptr_value);
                    instr.type = instruction_look_up::OPERATION_TYPE::I_TYPE;
                    instr.rd = instruction_look_up::get_register_index("ra");
                    instr.rs1 = instruction_look_up::get_register_index(*head->left->str_ptr_value);
                    instr.imm = 0;

                }
                else if(*head->str_ptr_value == "ret") {

                    instr.opcode = instruction_look_up::get_op_code(instruction_look_up::OPERATION_TYPE::I_TYPE,*head->str_ptr_value);
                    instr.type = instruction_look_up::OPERATION_TYPE::I_TYPE;
                    instr.rd = instruction_look_up::get_register_index("zero");
                    instr.rs1 = instruction_look_up::get_register_index("ra");
                    instr.imm = 0;
                }
                else if(*head->str_ptr_value == "call"){
                    instr.opcode = instruction_look_up::get_op_code(instruction_look_up::OPERATION_TYPE::J_TYPE);
                    instr.type = instruction_look_up::OPERATION_TYPE::J_TYPE;
                    instr.rd = instruction_look_up::get_register_index("ra");
                    instr.imm = head->left->identifier_immediate;

                }
                break;
            }
                
            case OPERATION_TYPE::UNKNOWN:
            default:
                // Unknown: do nothing or error
                break;
        }
        
        return instr;
    }
    void generator::generate_instructions(const std::vector<Ast_Node *> &heads){

        _instructions.reserve(heads.size());
        for(Ast_Node* head : heads){

            _instructions.push_back(convert_to_instr(head));
        }
    }
    const std::vector<Instruction> &generator::get_instructions()
    {
        return _instructions;
    }
}