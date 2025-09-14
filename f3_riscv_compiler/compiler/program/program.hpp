#pragma once
#include <vector>
#include <unordered_map>
#include "../common/ir/ir_gen_context.hpp"
#include "../common/ir/basic_block.hpp"
#include "../common/ir/interference_node.hpp"
#include "../common/ast/nodes.hpp"

namespace f3_compiler {
	struct Program {
		void generate_IR();
		void generate_basic_blocks();
		void print_IR(std::ostream& os);
		void print_ast();
		bool has_error() const;
		Program(std::vector<std::unique_ptr<ast_node::func_decl_t>>&& functions) : _functions(std::move(functions)) {}
	private:
		void set_control_flow_graph();
		void compute_block_use_def();
		void compute_block_live_in_out();
		void compute_instruction_live_in_out();
		void create_interference_graph_nodes();
		void compute_interference_graph();
		void compute_instruction_use_def();
	private:
		const basic_block_t* get_basic_block_by_label(const std::string& label);
		std::vector<std::unique_ptr<ast_node::func_decl_t>>  _functions;
		bool had_error = false;
		std::vector<ir_instruction_t> _instructions;
		std::vector<basic_block_t> _blocks;
		std::unordered_map<std::string,interference_node_t> _interference_nodes;
	};
}