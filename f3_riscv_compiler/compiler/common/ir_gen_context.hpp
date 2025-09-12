#pragma  once
#include <vector>
#include <cstdint>
#include <string>
#include <iostream>
#include <sstream>

struct ir_instruction_t {
    enum class operation { 
        UNKNOWN,
        LOAD_CONST, STORE,LOAD,GOTO,NOP,
        ADD, SUB, MUL, DIV, REM,
        CMP_LT, CMP_GT, CMP_GTE, CMP_LTE, CMP_EQ, CMP_NEQ,
        AND, OR, NOT, NEG,
        ADDR, DEREF,
        PARAM,ARG,
        CALL, RETURN, 
        BRANCH_IF, LABEL, 
        BRANCH_IF_NOT,
        MOV 
    } operation = operation::UNKNOWN;

    std::string dest;
    std::string src1, src2;
    std::string label;
    uint32_t func_argument_count = 0;

    std::string to_string() const {
        std::ostringstream out;
        if (operation != operation::LABEL) {
            out << "\t";
        }
        switch (operation) {
        case operation::LOAD_CONST:
            out << dest << " = " << src1; // e.g. t1 = 123
            break;
        case operation::ADD: case operation::SUB: case operation::MUL:
        case operation::DIV: case operation::REM:
        case operation::CMP_LT: case operation::CMP_GT: case operation::CMP_GTE:
        case operation::CMP_LTE: case operation::CMP_EQ: case operation::CMP_NEQ:
        case operation::AND: case operation::OR:
            out << dest << " = " << src1 << " " << op_to_string() << " " << src2;
            break;
        case operation::NOT:
        case operation::NEG:
            out << dest << " = " << op_to_string() << " " << src1;
            break;
        case operation::ADDR:
            out << dest << " = &" << src1;
            break;
        case operation::DEREF:
            out << dest << " = *" << src1;
            break;
        case operation::PARAM:
            out << "PARAM " << src1;
            break;
        case operation::ARG:
            out << "ARG " <<src1;
            break;
        case operation::CALL:
            if (!dest.empty())
                out << dest << " = ";
            out << "CALL " << src1 << ", " << func_argument_count;
            break;
        case operation::RETURN:
            out << "RETURN " << src1;
            break;
        case operation::BRANCH_IF_NOT:
            out << "BRANCH_IF_NOT " << src1 << ", " << label;
            break;
        case operation::BRANCH_IF:
            out << "BRANCH_IF " << src1 << ", " << label;
            break;
        case operation::LABEL:
            out << label << ":";
            break;
        case operation::MOV:
            out << dest << " = " << src1;
            break;
        case operation::UNKNOWN:
            out << "UNKNOWN";
            break;
        case operation::STORE : 
            out << "STORE" << " " << src1 <<  "," << dest;
            break;
        case operation::LOAD : 
            out << "LOAD" << " " <<dest << "," << src1;
            break;
        case operation::GOTO:
            out << "GOTO" << " " << label;
            break;
        case operation::NOP:
            out << "NOP";
            break;
        }
        return out.str();
    }
    private:
    std::string op_to_string() const {
        switch (operation) {
        case operation::ADD: return "+";
        case operation::SUB: return "-";
        case operation::MUL: return "*";
        case operation::DIV: return "/";
        case operation::REM: return "%";
        case operation::CMP_LT: return "<";
        case operation::CMP_GT: return ">";
        case operation::CMP_GTE: return ">=";
        case operation::CMP_LTE: return "<=";
        case operation::CMP_EQ: return "==";
        case operation::CMP_NEQ: return "!=";
        case operation::AND: return "&&";
        case operation::OR: return "||";
        case operation::NOT: return "!";
        case operation::NEG: return "-";
        default: return "?";
        }
    }


};
struct IR_Gen_Context {

	std::vector<ir_instruction_t> instructions;
	std::vector<std::string> skip_jump_labels;
    std::vector<std::string> break_jump_labels;
	std::vector<std::string> return_jump_labels;
    bool left_is_deref = false;

	std::string generate_label() { return ".L" + std::to_string(label_id++); }
	std::string generate_temp() { return "t" + std::to_string(temp_id++); }
	void push_scope() { scopes.emplace_back(); }
	void pop_scope() { scopes.pop_back(); }

private:
	uint32_t label_id = 0;
	uint32_t temp_id = 0;

private:
	std::vector<std::vector<std::string>> scopes;
};
