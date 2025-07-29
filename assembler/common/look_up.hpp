#pragma once
#include <unordered_set>
#include <string>

#include <cstdint>
#include <vector>

namespace instruction_look_up{

    enum class OPERATION_TYPE{
        R_TYPE,
        S_TYPE,
        I_TYPE,
        B_TYPE,
        J_TYPE,
        U_TYPE,
        PSEUDO_TYPE_0,
        PSEUDO_TYPE_1,
        PSEUDO_TYPE_2,
        PSEUDO_TYPE_3,
        PSEUDO_TYPE_4,
        PSEUDO_TYPE_5,
        PSEUDO_TYPE_6,
        UNKNOWN
    };
    extern const std::vector<std::string> registers;
    extern const std::unordered_set<std::string> i_type_ops;
    extern const std::unordered_set<std::string> s_type_ops;
    extern const std::unordered_set<std::string> r_type_ops;
    extern const std::unordered_set<std::string> s_type_ops;
    extern const std::unordered_set<std::string> b_type_ops;
    extern const std::unordered_set<std::string> j_type_ops;
    extern const std::unordered_set<std::string> u_type_ops;
    extern const std::unordered_set<std::string> pseudo_ops;
    extern const std::unordered_set<std::string> pseudo_ops_type_0;
    extern const std::unordered_set<std::string> pseudo_ops_type_1;
    extern const std::unordered_set<std::string> pseudo_ops_type_2;
    extern const std::unordered_set<std::string> pseudo_ops_type_3;
    extern const std::unordered_set<std::string> pseudo_ops_type_4;
    extern const std::unordered_set<std::string> pseudo_ops_type_5;
    extern const std::unordered_set<std::string> pseudo_ops_type_6;

    OPERATION_TYPE get_opr_type(const std::string &instr);
    bool is_register(const std::string &s);
    bool is_immediate(const std::string &s);
    uint8_t get_register_index(const std::string &reg_id);
    /*
        in some operations even the operation type are the same they have different op codes 
        we will use op_id to distinguish between them
    */
    uint8_t get_op_code(OPERATION_TYPE type,const std::string& op_str = ""); 
    uint8_t get_func7(const std::string& op_str);
    uint8_t get_func3(const std::string &op_str);
}