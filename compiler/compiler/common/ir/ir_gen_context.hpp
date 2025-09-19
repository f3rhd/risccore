#pragma  once
#include <vector>
#include <cstdint>
#include <string>
#include <iostream>
#include <sstream>
#include <set>
#include <unordered_map>
struct ir_instruction_t {
    enum class operation_ { 
        UNKNOWN,
        FUNC_ENTRY,
        LOAD_CONST, STORE,LOAD,GOTO,NOP,
        ADD, SUB, MUL, DIV, REM,
        BGT,BLE,BNEQ,BEQ,BGE,BLT,
        CMP_LT, CMP_GT, CMP_GTE, CMP_LTE, CMP_EQ, CMP_NEQ,
        AND, OR, NOT, NEG,
        ADDR, DEREF,
        PARAM,ARG,
        CALL, RETURN, 
        LABEL, 
        MOV 
    } operation = operation_::UNKNOWN;

    bool store_dest_in_stack = false;
    bool load_var_from_memory = false;
    std::string dest;
    std::string src1, src2;
    std::string label_id;
    std::set<std::string> use;
    std::set<std::string> live_in;
    std::set<std::string> live_out;
    std::set<std::string> def;
    uint32_t func_argument_count = 0;

    std::string to_string() const {
        std::ostringstream out;
        if (operation != operation_::LABEL && operation != operation_::FUNC_ENTRY) {
            out << "\t";
        }
        switch (operation) {
        case operation_::LOAD_CONST:
            //out << dest << " = " << src1; // e.g. t1 = 123
            out << "LI " << dest << "," << src1;
            break;
        case operation_::ADD: 
            out << "ADD " << dest << "," << src1 << ',' << src2;
            break;
        case operation_::SUB: 
            out << "SUB " << dest << "," << src1 << ',' << src2;
            break;
        case operation_::MUL:
            out << "MUL " << dest << "," << src1 << ',' << src2;
            break;
        case operation_::DIV: 
            out << "DIV " << dest << "," << src1 << ',' << src2;
            break;
        case operation_::REM:
            out << "REM " << dest << "," << src1 << ',' << src2;
            break;
        case operation_::AND: 
            out << "AND " << dest << "," << src1 << ',' << src2;
            break;
        case operation_::OR:
            out << "OR " << dest << "," << src1 << ',' << src2;
            break;
        case operation_::NOT:
        case operation_::NEG:
            out << dest << " = " << op_to_string() << " " << src1;
            break;
        case operation_::ADDR:
            out << dest << " = &" << src1;
            break;
        case operation_::DEREF:
            out << dest << " = *" << src1;
            break;
        case operation_::PARAM:
            out << "PARAM " << dest;
            break;
        case operation_::ARG:
            out << "ARG " <<src1;
            break;
        case operation_::CALL:
            if (!dest.empty())
                out << dest << " = ";
            out << "CALL " << label_id << ", " << func_argument_count;
            break;
        case operation_::RETURN:
            out << "RETURN " << src1;
            break;
        //case operation_::BRANCH_IF_NOT:
        //    out << "BRANCH_IF_NOT " << src1 << ", " << label_id;
        //    break;
        //case operation_::BRANCH_IF:
        //    out << "BRANCH_IF " << src1 << ", " << label_id;
        //    break;
        case operation_::BEQ:
            out << "BEQ " << src1 << ", " << src2 << ", " << label_id;
            break;
        case operation_::BNEQ:
            out << "BNEQ " << src1 << "," << src2 <<  ", " << label_id;
            break;
        case operation_::BGT:
            out << "BGT " << src1 << "," << src2 <<  ", " << label_id;
            break;
        case operation_::BGE:
            out << "BGE " << src1 << "," << src2 <<  ", " << label_id;
            break;
        case operation_::BLE:
            out << "BLE " << src1 << "," << src2 <<  ", " << label_id;
            break;
        case operation_::BLT:
            out << "BLT " << src1 << "," << src2 <<  ", " << label_id;
            break;
        case operation_::LABEL:
        case operation_::FUNC_ENTRY:
            out << label_id << ":";
            break;
        case operation_::MOV:
            out << dest << " = " << src1;
            break;
        case operation_::UNKNOWN:
            out << "UNKNOWN";
            break;
        case operation_::STORE : 
            out << "STORE" << " " << src1 <<  "," << dest;
            break;
        case operation_::LOAD : 
            out << "LOAD" << " " <<dest << "," << src1;
            break;
        case operation_::GOTO:
            out << "GOTO" << " " << label_id;
            break;
        case operation_::NOP:
            out << "NOP";
            break;
        }
        return out.str();
    }
    private:
    std::string op_to_string() const {
        switch (operation) {
        case operation_::ADD: return "+";
        case operation_::SUB: return "-";
        case operation_::MUL: return "*";
        case operation_::DIV: return "/";
        case operation_::REM: return "%";
        case operation_::CMP_LT: return "<";
        case operation_::CMP_GT: return ">";
        case operation_::CMP_GTE: return ">=";
        case operation_::CMP_LTE: return "<=";
        case operation_::CMP_EQ: return "==";
        case operation_::CMP_NEQ: return "!=";
        case operation_::AND: return "&&";
        case operation_::OR: return "||";
        case operation_::NOT: return "!";
        case operation_::NEG: return "-";
        default: return "?";
        }
    }


};
// flags
auto constexpr is_if_statement = 1 << 0;
auto constexpr is_comparsion = 1 << 1;
auto constexpr is_while_statement = 1 << 2;
struct IR_Gen_Context {
    std::string generate_label() { return ".L" + std::to_string(label_id++); }
	std::string generate_temp() { return "t" + std::to_string(temp_id++); }
    void push_scope() { scopes.emplace_back(); }
	void pop_scope() { scopes.pop_back(); }
    void reset() {
        skip_jump_labels.clear();
        break_jump_labels.clear();
        for(auto& [key,value] : symbol_mangles){
            value++;
        }
    }
    uint32_t flag = 0;
    bool left_is_deref = false;
public:
    std::vector<ir_instruction_t> instructions;
	std::vector<std::string> skip_jump_labels;
    std::vector<std::string> break_jump_labels;
    std::unordered_map<std::string, uint32_t> symbol_mangles;
    ir_instruction_t comparison_instruction;
private:
	uint32_t label_id = 0;
	uint32_t temp_id = 0;

private:
	std::vector<std::vector<std::string>> scopes;
};
