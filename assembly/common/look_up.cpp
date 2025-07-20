#include "look_up.hpp"
#include <regex>
#include <algorithm>
namespace instruction_look_up{

    const std::vector<std::string> registers = {
        "zero","ra","sp","gp",
        "tp","t0","t1","t2",
        "s0","s1","a0",
        "a1", "a2", "a3", "a4",
        "a5", "a6", "a7", "s2",
        "s3", "s4", "s5", "s6",
        "s7", "s8", "s9", "s10",
        "s11","t3", "t4", "t5","t6","fp"
    };
    const std::unordered_set<std::string_view> registers_view = {
        "zero","ra","sp","gp",
        "tp","t0","t1","t2",
        "s0","s1","a0",
        "a1", "a2", "a3", "a4",
        "a5", "a6", "a7", "s2",
        "s3", "s4", "s5", "s6",
        "s7", "s8", "s9", "s10",
        "s11","t3", "t4", "t5","t6","fp"
    };
    const std::unordered_set<std::string_view> operations_view = {

        "lb", "lh", "lw", "lbu", "lhu",
        "addi", "slli", "slti", "sltiu",
        "xori", "srli", "srai", "ori", "andi", "jalr"
        "add","sub","sll","sltu","xor","srl","sra",
        "or","and","sll","slt","sltu","xor","srl","sra",
        "or","and"
        "sb","sh","sw"
        "beq","bne","blt","bge","bltu","bgeu"
        "jal"
        "auipc", "lui"
        "nop","li","mv","not","neg",
        "seqz","snez","sltz","sgtz","beqz",
        "bnez","blez","bgez","bltz","bgtz",
        "ble","bgt","bleu","bgtu","j","jal",
        "jr","jalr","ret","call","la"     // @Incomplete  : We have 3 or 4 more pseudo operations.
    };
    const std::unordered_set<std::string> i_type_ops = {
        "lb", "lh", "lw", "lbu", "lhu",
        "addi", "slli", "slti", "sltiu",
        "xori", "srli", "srai", "ori", "andi", "jalr"
    };

    const std::unordered_set<std::string> r_type_ops = {
        "add","sub","sll","sltu","xor","srl","sra",
        "or","and","sll","slt","sltu","xor","srl","sra",
        "or","and"
    };
    const std::unordered_set<std::string> s_type_ops = {
        "sb","sh","sw"
    };
    const std::unordered_set<std::string> b_type_ops = {

        "beq","bne","blt","bge","bltu","bgeu"
    };
    const std::unordered_set<std::string> j_type_ops = {
        "jal"
    };
    const std::unordered_set<std::string> u_type_ops = {
        "auipc", "lui"
    };

    const std::unordered_set<std::string> pseudo_ops = {
        "nop","li","mv","not","neg",
        "seqz","snez","sltz","sgtz","beqz",
        "bnez","blez","bgez","bltz","bgtz",
        "ble","bgt","bleu","bgtu","j","jal",
        "jr","jalr","ret","call","la"     // @Incomplete  : We have 3 or 4 more pseudo operations.
    };

    const std::unordered_set<std::string> pseudo_ops_type_0 = { //@Incomplete
    };
    const std::unordered_set<std::string> pseudo_ops_type_1 = { // These bitches have two operands and both of them are registers
        "mv","not","neg","seqz","snez","sltz","sgtz"
    };
    const std::unordered_set<std::string> pseudo_ops_type_2 = { // These bithces have two operands and left is register while middle is label identifier
        "beqz", "bnez", "blez", "bgez", "bltz", "bgtz"
    };
    const std::unordered_set<std::string> pseudo_ops_type_3 = { // These baddies have three operands left and middle are registers while right is label identifier
        "ble","bgt","bleu","bgtu"
    };
    const std::unordered_set<std::string> pseudo_ops_type_4 = { // These have one operand only and it is just a label identifier
        "j","jal","call"
    };
    const std::unordered_set<std::string> pseudo_ops_type_5 = { // These mommas have also one operand only and they are just registers
        "jr","jalr"
    };
    const std::unordered_set<std::string> pseudo_ops_type_6 = { // These mad bitches dont got any operands
        "ret","nop"
    };
    OPERATION_TYPE get_opr_type(const std::string& instr){

        if(r_type_ops.count(instr))
            return OPERATION_TYPE::R_TYPE;
        else if(i_type_ops.count(instr))
            return OPERATION_TYPE::I_TYPE;
        else if(s_type_ops.count(instr))
            return OPERATION_TYPE::S_TYPE;
        else if(b_type_ops.count(instr))
            return OPERATION_TYPE::B_TYPE;
        else if(j_type_ops.count(instr))
            return OPERATION_TYPE::J_TYPE;
        else if(u_type_ops.count(instr))
            return OPERATION_TYPE::U_TYPE;
        else if(pseudo_ops_type_0.count(instr))
            return OPERATION_TYPE::PSEUDO_TYPE_0;
        else if(pseudo_ops_type_1.count(instr))
            return OPERATION_TYPE::PSEUDO_TYPE_1;
        else if(pseudo_ops_type_2.count(instr))
            return OPERATION_TYPE::PSEUDO_TYPE_2;
        else if(pseudo_ops_type_3.count(instr))
            return OPERATION_TYPE::PSEUDO_TYPE_3;
        else if(pseudo_ops_type_4.count(instr))
            return OPERATION_TYPE::PSEUDO_TYPE_4;
        else if(pseudo_ops_type_5.count(instr))
            return OPERATION_TYPE::PSEUDO_TYPE_5;
        else if(pseudo_ops_type_6.count(instr))
            return OPERATION_TYPE::PSEUDO_TYPE_6;
        else
            return OPERATION_TYPE::UNKNOWN;
    }    
    bool is_register(const std::string& s){
        return std::find(registers.begin(),registers.end(),s) != registers.end();
    }

    bool is_register(const std::string_view &s){
        return std::find(registers_view.begin(),registers_view.end(),s) != registers_view.end();
    }
    bool is_immediate(const std::string_view& s){
        static const std::regex hex_pattern(R"(0[xX][0-9a-fA-F]+)");
        static const std::regex dec_pattern(R"(-?[0-9]+)");

        if (s.size() >= 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
            return std::regex_match(s.begin(), s.end(), hex_pattern);
        }
        return std::regex_match(s.begin(), s.end(), dec_pattern);
    }
    bool is_immediate(const std::string& s){
        if(s.size() >= 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X')){
            return std::regex_match(s, std::regex("0[xX][0-9a-fA-F]+"));
        }
        return std::regex_match(s, std::regex("-?[0-9]+"));
    }
    uint8_t get_register_index(const std::string &reg_id){
        uint8_t index = std::find(registers.begin(), registers.end(), reg_id) - registers.begin();

        //frame pointer and s0 normally have the same index
        if(index == registers.size()- 1){ // frame pointer is located at the end of the registers vector
            return std::find(registers.begin(), registers.end(), "s0") - registers.begin();
        }
        return std::find(registers.begin(), registers.end(), reg_id) - registers.begin();
    }
    uint8_t get_op_code(OPERATION_TYPE type,const std::string& op_id){

        switch(type){
            case OPERATION_TYPE::R_TYPE:
                return 0x33; // R-type opcode
            case OPERATION_TYPE::I_TYPE:
                if(op_id[0] == 'l') // meaning that it is load 
                    return 0x3;
                if(op_id[0] == 'j') // jalr
                    return 0x67;
                return 0x13; // I-type opcode (arithmetic)
            case OPERATION_TYPE::S_TYPE:
                return 0x23; // S-type opcode
            case OPERATION_TYPE::B_TYPE:
                return 0x63; // B-type opcode
            case OPERATION_TYPE::J_TYPE:
                return 0x6F; // J-type opcode
            case OPERATION_TYPE::U_TYPE:
                if(op_id[0] == 'a') //auipc
                    return 0x17;
                return 0x37; // U-type opcode (LUI)
            case OPERATION_TYPE::PSEUDO_TYPE_0:
            case OPERATION_TYPE::PSEUDO_TYPE_1:
            case OPERATION_TYPE::PSEUDO_TYPE_2:
            case OPERATION_TYPE::PSEUDO_TYPE_3:
            case OPERATION_TYPE::PSEUDO_TYPE_4:
            case OPERATION_TYPE::PSEUDO_TYPE_5:
            case OPERATION_TYPE::PSEUDO_TYPE_6:
                return 0x00; // Pseudo instructions don't have a real opcode
            case OPERATION_TYPE::UNKNOWN:
            default:
                return 0xFF; // Unknown opcode
        }
    }

    uint8_t get_func7(const std::string& op_str){
        // RISC-V funct7 field for R-type instructions
        if(op_str == "add" || op_str == "sll" || op_str == "slt" || op_str == "sltu" || op_str == "xor" || op_str == "or" || op_str == "and")
            return 0x00;
        if(op_str == "sub" || op_str == "sra")
            return 0x20;
        if(op_str == "srl")
            return 0x00;
        // For other instructions, return 0x00 by default (most I-type, S-type, etc. don't use funct7)
        return 0x00;
    }
    uint8_t get_func3(const std::string& op_str){
        // R-type and I-type funct3 values
        if(op_str == "add" || op_str == "sub" || op_str == "addi")
            return 0x0;
        if(op_str == "sll" || op_str == "slli")
            return 0x1;
        if(op_str == "slt" || op_str == "slti")
            return 0x2;
        if(op_str == "sltu" || op_str == "sltiu")
            return 0x3;
        if(op_str == "xor" || op_str == "xori")
            return 0x4;
        if(op_str == "srl" || op_str == "srli" || op_str == "sra" || op_str == "srai")
            return 0x5;
        if(op_str == "or" || op_str == "ori")
            return 0x6;
        if(op_str == "and" || op_str == "andi")
            return 0x7;
        // Loads
        if(op_str == "lb") return 0x0;
        if(op_str == "lh") return 0x1;
        if(op_str == "lw") return 0x2;
        if(op_str == "lbu") return 0x4;
        if(op_str == "lhu") return 0x5;
        // Stores
        if(op_str == "sb") return 0x0;
        if(op_str == "sh") return 0x1;
        if(op_str == "sw") return 0x2;
        // Branches
        if(op_str == "beq") return 0x0;
        if(op_str == "bne") return 0x1;
        if(op_str == "blt") return 0x4;
        if(op_str == "bge") return 0x5;
        if(op_str == "bltu") return 0x6;
        if(op_str == "bgeu") return 0x7;
        // JALR
        if(op_str == "jalr") return 0x0;
        // Default for unknown or pseudo instructions
        return 0x0;
    }

};