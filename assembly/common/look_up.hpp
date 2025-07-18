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
        PSEUDO,
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