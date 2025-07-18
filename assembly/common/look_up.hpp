#pragma once
#include <unordered_set>
#include <string>



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
    extern const std::unordered_set<std::string> registers;
    extern const std::unordered_set<std::string> i_type_ops;
    extern const std::unordered_set<std::string> s_type_ops;
    extern const std::unordered_set<std::string> r_type_ops;
    extern const std::unordered_set<std::string> s_type_ops;
    extern const std::unordered_set<std::string> b_type_ops;
    extern const std::unordered_set<std::string> j_type_ops;
    extern const std::unordered_set<std::string> u_type_ops;
    extern const std::unordered_set<std::string> pseudo_ops;
    OPERATION_TYPE get_instr_type(const std::string &instr);
}