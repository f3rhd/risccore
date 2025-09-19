#pragma once
#include <string>

#include "look_up.hpp"
#include "line.hpp"

namespace f3_riscv_assembler {
	enum class AST_NODE_TYPE
	{
		OPERATION,
		REGISTER,
		IMMEDIATE,
		LABEL,
		DIRECTIVE,
		IDENTIFIER,
		UNKNOWN
	};
	struct Ast_Node{

		instruction_look_up::OPERATION_TYPE opr_type = instruction_look_up::OPERATION_TYPE::UNKNOWN;
		const std::string* str_ptr_value = nullptr;
		AST_NODE_TYPE node_type = AST_NODE_TYPE::UNKNOWN;
		Ast_Node *left = nullptr;
		Ast_Node *right = nullptr;
		Ast_Node *middle = nullptr;
		Line      *line_info = nullptr;
		int32_t identifier_immediate = 0;  // imm value of that identifier
	};

}
