#include "look_up.hpp"
namespace instruction_look_up{

    const std::unordered_set<std::string> registers = {
        "zero","ra","sp","gp",
        "tp","t0","t1","t2",
        "s0","fp","s1","a0",
        "a1", "a2", "a3", "a4",
        "a5", "a6", "a7", "s2",
        "s3", "s4", "s5", "s6",
        "s7", "s8", "s9", "s10",
        "s11","t3", "t4", "t5","t6"
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
    OPERATION_TYPE get_instr_type(const std::string& instr){

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
        else if(pseudo_ops.count(instr))
            return OPERATION_TYPE::PSEUDO;
        else
            return OPERATION_TYPE::UNKNOWN;
    }
};