#pragma  once
#include <vector>
#include <cstdint>
#include <string>
#include <iostream>
#include <sstream>
#include <set>
#include <unordered_map>
#include <utility>
struct ir_instruction_t;
struct IR_Gen_Context {
    std::string generate_label() { return ".L" + std::to_string(_label_id++); }
	std::string generate_temp() { return "t" + std::to_string(_temp_id++); }
    // these two functions are used for name resolution and stuff to prevent the global name clashing
    void push_body(const std::string& id) { 
        _scopes.emplace_back(); 
        _scopes.back().first = id;
    }
    void pop_body() { 
        _scopes.pop_back(); 
    }
    void add_var_id(const std::string & id) { 
        _scopes.back().second.push_back(id); 
    }
    const std::vector<std::pair<std::string,std::vector<std::string>>> &get_scopes() {
        return _scopes; 
    }
    void reset() {
        skip_jump_labels.clear();
        break_jump_labels.clear();
        _scopes.clear();
    }
    std::string generate_body_id() {
        return "b" + std::to_string(_body_id++);
    }
public:
    std::vector<ir_instruction_t> instructions;
	std::vector<std::string> skip_jump_labels;
    std::vector<std::string> break_jump_labels;
    bool left_is_deref = false;
private:
	std::vector<std::pair<std::string,std::vector<std::string>>> _scopes; // will keep track of defined vars in the scopes
	uint32_t _label_id = 0;
	uint32_t _temp_id = 0;
    uint32_t _body_id = 0;
};
struct ir_instruction_t {
    enum class operation_ { 
        UNKNOWN,
        FUNC_ENTRY,
        LOAD_CONST, STORE,LOAD,GOTO,NOP,
        ADD, SUB, MUL, DIV, REM,
        BGT,BLE,BNEQ,BEQ,BGE,BLT,
        LOGICAL_AND, LOGICAL_OR, NEG,
        BIT_AND,BIT_OR,BIT_XOR,BIT_NOT,
        SHIFT_LEFT,SHIFT_RIGHT,
        ADDR, DEREF,
        PARAM,ARG,
        CALL, RETURN, 
        LABEL, 
        MOV 
    } operation = operation_::UNKNOWN;

    bool store_dest_in_stack = false;
    bool load_var_from_memory = false;
    bool load_src_is_ptr = false;
    std::string dest;
    std::string src1, src2;
    std::string label_id;
    std::set<std::string> use;
    std::set<std::string> live_in;
    std::set<std::string> live_out;
    std::set<std::string> def;

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
        case operation_::BIT_AND: 
            out << "AND " << dest << "," << src1 << ',' << src2;
            break;
        case operation_::BIT_OR:
            out << "OR " << dest << "," << src1 << ',' << src2;
            break;
        case operation_::BIT_XOR:
            out << "XOR " << dest << "," << src1 << ',' << src2;
            break;
        case operation_::BIT_NOT:
            out << "NOT " << dest << "," << src1 << ',' << src2;
            break;
        case operation_::SHIFT_LEFT:
            out << "SLL " << dest << "," << src1 << ',' << src2;
            break;
        case operation_::SHIFT_RIGHT:
            out << "SRL " << dest << "," << src1 << ',' << src2;
            break;
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
            out << "CALL " << label_id;
            ;
            break;
        case operation_::RETURN:
            out << "RETURN " << src1;
            break;
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
        case operation_::BLT: return "<";
        case operation_::BGT: return ">";
        case operation_::BGE: return ">=";
        case operation_::BLE: return "<=";
        case operation_::BEQ: return "==";
        case operation_::BNEQ: return "!=";
        case operation_::LOGICAL_AND: return "&&";
        case operation_::LOGICAL_OR: return "||";
        case operation_::BIT_NOT: return "!";
        case operation_::NEG: return "-";
        default: return "?";
        }
    }


};

