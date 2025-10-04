#pragma once
#include <vector>
#include <unordered_map>
#include "../common/ir/ir_gen_context.hpp"
#include "../common/ir/basic_block.hpp"
#include "../common/ir/interference_node.hpp"
#include "../common/ir/function_block.hpp"
#include "../common/ast/nodes.hpp"

namespace fs_compiler {
	struct Program {
		void analyse();
		void generate_IR();
		void generate_asm(std::ostream& os);
		void print_IR(std::ostream& os);
		void print_ast();
		bool has_error();
		Program(std::vector<std::unique_ptr<ast_node::func_decl_t>>&& functions) : _functions(std::move(functions)) {}
	private:
		void convert_function_blocks_to_asm(std::ostream& os);
		void generate_function_blocks();
	private:
		std::vector<std::unique_ptr<ast_node::func_decl_t>>  _functions;
		std::vector<ir_instruction_t> _instructions;
		std::vector<function_block_t> _function_blocks;
		std::vector<error_t> _errors;
	};
}