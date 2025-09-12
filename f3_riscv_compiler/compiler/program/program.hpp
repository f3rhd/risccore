#pragma once
#include <vector>
#include "../common/ir_gen_context.hpp"
#include "../common/nodes.hpp"
namespace f3_compiler {
	struct Program {
		void generate_IR();
		std::vector<ir_instruction_t> instructions;
		void print_IR(std::ostream& os);
		void print_ast();
		bool has_error() const;
		Program(std::vector<std::unique_ptr<ast_node::func_decl_t>>&& functions) : _functions(std::move(functions)) {}
	private:
		std::vector<std::unique_ptr<ast_node::func_decl_t>>  _functions;
		bool had_error = false;
	};
}