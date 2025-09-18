#pragma once
#include <vector>
#include <unordered_map>
#include "../common/ir/ir_gen_context.hpp"
#include "../common/ir/basic_block.hpp"
#include "../common/ir/interference_node.hpp"
#include "../common/ir/function_block.hpp"
#include "../common/ast/nodes.hpp"

namespace f3_compiler {
	struct Program {
		void analyse();
		void generate_IR();
		void generate_asm(std::ostream& os);
		void print_IR(std::ostream& os);
		void print_ast();
		bool has_error();
		Program(std::vector<std::unique_ptr<ast_node::func_decl_t>>&& functions) : _functions(std::move(functions)) {}
	private:
		void set_control_flow_graph();
		void compute_instruction_live_in_out();
		void compute_instruction_use_def();
		void create_interference_graph_nodes();
		void compute_interference_graph();
		void generate_basic_blocks();
		void generate_function_blocks();
		void convert_function_blocks_to_asm(std::ostream& os);
		std::string get_allocated_reg_for_var(const std::string &id);

	private:
		std::unordered_map<std::string,int32_t> _coloring;
		std::vector<std::string> _spilled_vars;
		const basic_block_t* get_basic_block_by_label(const std::string& label);
		std::vector<std::unique_ptr<ast_node::func_decl_t>>  _functions;
		std::vector<ir_instruction_t> _instructions;
		std::vector<basic_block_t> _blocks;
		std::unordered_map<std::string,interference_node_t> _interference_nodes;
		std::vector<function_block_t> _function_blocks;
		std::vector<error_t> _errors;
	};
}