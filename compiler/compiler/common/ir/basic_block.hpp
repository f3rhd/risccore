#pragma  once
#include "ir_gen_context.hpp"
#include <set>
namespace f3_compiler {
	struct basic_block_t {
		const ir_instruction_t* label_instr = nullptr;
		std::vector<ir_instruction_t*> instructions;
		std::vector<const basic_block_t*> successors;
		//std::set<std::string> use;
		//std::set<std::string> def;
		//std::set<std::string> live_in;
		//std::set<std::string> live_out;
	};
}