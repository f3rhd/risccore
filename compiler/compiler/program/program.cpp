#include "program.hpp"
#include <unordered_set>
#include <iomanip>
#include <algorithm>
namespace fs_compiler {
	namespace {

		auto is_immediate(const std::string& s)->bool{
			if(s.empty()) return false;
			if(s.size()>2 && s[0]=='0' && (s[1]=='x' || s[1]=='X')) return true;
			size_t i = 0;
			if(s[0]=='-') i = 1;
			if(i==s.size()) return false;
			for(; i < s.size(); ++i){
				if(!std::isdigit(static_cast<unsigned char>(s[i]))) return false;
			}
			return true;
		};
		inline int32_t align_up(int32_t offset, int32_t alignment) {
			return (offset + alignment - 1) / alignment * alignment;
		}
		inline std::string actual_offset(int32_t logical_offset, int32_t saved_regs_bytes = 8) {
			return std::to_string((logical_offset - saved_regs_bytes));
		}
	}
	void Program::generate_asm(std::ostream& os){
		generate_function_blocks();
		convert_function_blocks_to_asm(os);
	}
	void Program::generate_IR() {

		IR_Gen_Context ctx;
		for (auto& func : _functions) {
			func->generate_ir(ctx);
			ctx.reset();
		}
		_instructions = std::move(ctx.instructions);
		generate_basic_blocks();
		set_control_flow_graph();
		compute_instruction_use_def();
		compute_instruction_live_in_out();
		compute_interference_graph();
	}

	void Program::print_liveness_json(std::ostream& os) {
		auto escape_json = [](const std::string& s) -> std::string {
			std::string out;
			out.reserve(s.size() + 8);
			for (unsigned char c : s) {
				switch (c) {
				case '\"': out += "\\\""; break;
				case '\\': out += "\\\\"; break;
				case '\b': out += "\\b";  break;
				case '\f': out += "\\f";  break;
				case '\n': out += "\\n";  break;
				case '\r': out += "\\r";  break;
				case '\t': out += "\\t";  break;
				default:
					if (c < 0x20) {
						// control char -> \u00XX
						const char hex[] = "0123456789ABCDEF";
						out += "\\u00";
						out += hex[(c >> 4) & 0xF];
						out += hex[c & 0xF];
					} else {
						out.push_back(static_cast<char>(c));
					}
				}
			}
			return out;
		};

		os << "[\n";
		for (size_t i = 0; i < _instructions.size(); ++i) {
			const ir_instruction_t& instr = _instructions[i];
			if (i) os << ",\n";
			os << "  {\n";
			os << "    \"index\": " << i << ",\n";
			os << "    \"instr\": \"" << escape_json(instr.to_string()) << "\",\n";

			auto emit_set = [&](const std::set<std::string>& s, const char* name) {
				os << "    \"" << name << "\": [";
				bool first = true;
				for (const auto& v : s) {
					if (!first) os << ",";
					os << "\n      \"" << escape_json(v) << "\"";
					first = false;
				}
				if (!s.empty()) os << "\n    ";
				os << "]";
			};

			emit_set(instr.use, "use"); os << ",\n";
			emit_set(instr.def, "def"); os << ",\n";
			emit_set(instr.live_in, "live_in"); os << ",\n";
			emit_set(instr.live_out, "live_out"); os << "\n";

			os << "  }";
		}
		os << "\n]\n";
	}
	const basic_block_t* Program::get_basic_block_by_label(const std::string& label)
	{
		for (auto& block : _blocks) {
			if (block.label_instr->label_id == label) {
				return &block;
			}
		}
		return nullptr;
	}
	void Program::generate_function_blocks(){
		uint64_t i = 0;
		uint64_t size = _instructions.size();

		while (i < size) {
			// Only start a block if we see a func entry
			if (_instructions[i].operation == ir_instruction_t::operation_::FUNC_ENTRY) {
				function_block_t block;
				block.label_instr = &_instructions[i];
				i++;

				// Collect instructions until we hit a  func entry
				while (i < size &&
					_instructions[i].operation != ir_instruction_t::operation_::FUNC_ENTRY
					) {
					
					block.instructions.push_back(&_instructions[i]);

					i++;
				}
				_function_blocks.push_back(std::move(block));
			} else {
					// Skip non-labels until we find the next FUNC_ENTRY
					i++;
			}
		}

		for(auto& block : _function_blocks){

			int32_t logical_offset_s0 = 0;
			int32_t logical_offset_sp = 0;
			size_t param_count = 0;
			for(auto& instr : block.instructions){
				if (instr->operation == ir_instruction_t::operation_::ALLOC) {
					logical_offset_s0 += 4;
					logical_offset_s0 += std::stoi(instr->src1); 
					if (block.stack.find(instr->dest) == block.stack.end()) {
						block.stack.emplace(instr->dest,-logical_offset_s0);
					}
				}
				else if(!instr->def.empty()){
					for(auto& defined : instr->def){
						if(std::find(_spilled_vars.begin(),_spilled_vars.end(),defined) != _spilled_vars.end() && block.stack.find(instr->dest) == block.stack.end()){
							logical_offset_s0 += 4;
							block.stack.emplace(instr->dest,-logical_offset_s0);
							instr->store_dest_in_stack = true;
						}
						else if(defined[0] != '@'){
							if (block.stack.find(defined) == block.stack.end()) {
								if (instr->operation == ir_instruction_t::operation_::PARAM && param_count >= 8) {
									block.stack.emplace(instr->dest,logical_offset_sp);
									logical_offset_sp += 4;
									param_count++;
								}
								else {
									if (instr->operation == ir_instruction_t::operation_::PARAM) {
										param_count++;
									}
									logical_offset_s0 += 4;
									block.stack.emplace(defined,-logical_offset_s0);
									instr->load_var_from_memory = true;
								}
							}
						}
					}
				}
				else if(!instr->use.empty() && !instr->src1_is_stack_offset){
					for(auto& used : instr->use){
						if(std::find(_spilled_vars.begin(),_spilled_vars.end(),used) != _spilled_vars.end() && block.stack.find(instr->dest) == block.stack.end()){
							logical_offset_s0 += 4;
							block.stack.emplace(used,-logical_offset_s0);
							instr->load_var_from_memory = true;
						}
						else if(used[0] != '@'){
							instr->load_var_from_memory = true;
						}
					}
				}
			}
			block.frame_size = logical_offset_s0 + 8;
		}
	} 
	void Program::generate_basic_blocks() {
		uint64_t i = 0;
		uint64_t size = _instructions.size();

		while (i < size) {
		// Only start a block if we see a LABEL or func entry
		if (_instructions[i].operation == ir_instruction_t::operation_::LABEL || _instructions[i].operation == ir_instruction_t::operation_::FUNC_ENTRY) {
			basic_block_t block;
			block.label_instr = &_instructions[i];
			i++;

			// Collect instructions until we hit a terminator or a new LABEL
			while (i < size &&
				_instructions[i].operation != ir_instruction_t::operation_::LABEL
				&& _instructions[i].operation != ir_instruction_t::operation_::FUNC_ENTRY
				) {
				
				block.instructions.push_back(&_instructions[i]);

				// If it is  a  terminator end the block after adding it
				if (_instructions[i].operation == ir_instruction_t::operation_::GOTO ||
					_instructions[i].operation == ir_instruction_t::operation_::BEQ ||
					_instructions[i].operation == ir_instruction_t::operation_::BNEQ ||
					_instructions[i].operation == ir_instruction_t::operation_::BLE ||
					_instructions[i].operation == ir_instruction_t::operation_::BGE ||
					_instructions[i].operation == ir_instruction_t::operation_::BLT ||
					_instructions[i].operation == ir_instruction_t::operation_::BGT ||
					_instructions[i].operation == ir_instruction_t::operation_::RETURN) {
					i++;
					break;
				}

				i++;
			}
				_blocks.push_back(std::move(block));
			} else {
				// Skip non-labels until we find the next LABEL
				i++;
			}
		}
	}
	
	void Program::set_control_flow_graph() {

		for (uint64_t i = 0; i < _blocks.size(); i++) {
			auto& block = _blocks[i];
			if (block.instructions.empty()) {
				continue;
			}
			switch (block.instructions.back()->operation) { case ir_instruction_t::operation_::RETURN:
				// this block shall have no successors
				continue;
			case ir_instruction_t::operation_::GOTO:
				block.successors.push_back(get_basic_block_by_label(block.instructions.back()->label_id));
				break;
			case ir_instruction_t::operation_::BEQ: 
			case ir_instruction_t::operation_::BNEQ: 
			case ir_instruction_t::operation_::BLT:
			case ir_instruction_t::operation_::BLE:
			case ir_instruction_t::operation_::BGT: 
			case ir_instruction_t::operation_::BGE:
				block.successors.push_back(get_basic_block_by_label(block.instructions.back()->label_id));
				if (i + 1 < _blocks.size()) {
					block.successors.push_back(get_basic_block_by_label(_blocks[i + 1].label_instr->label_id));
				}
				break;
			}
		}
	}

	void Program::compute_instruction_use_def(){
		for(auto& instr : _instructions){
			if (instr.operation == ir_instruction_t::operation_::ALLOC)
				continue;
			if(!instr.src1.empty() && !is_immediate(instr.src1) && !instr.src1_is_stack_offset) {
				instr.use.insert(instr.src1);
			}
			if (!instr.src2.empty() && !is_immediate(instr.src2)) {
				instr.use.insert(instr.src2);
			}
			if(!instr.dest.empty() && !is_immediate(instr.dest)) {
				// in store instruction both variables are used rather one of them being defined
				if(instr.operation == ir_instruction_t::operation_::STORE){
					// when it is array we wont be using that
					if(instr.store_dest_is_ptr)
						instr.use.insert(instr.dest);
				}
				else{
					instr.def.insert(instr.dest);
				}
			}
		}
	}
	void Program::compute_instruction_live_in_out() {
		bool changed = true;

		while (changed) {
			changed = false;

			for (auto& block : _blocks) {
				// Iterate instructions backward in the block
				for (auto it = block.instructions.rbegin(); it != block.instructions.rend(); ++it) {
					ir_instruction_t* instr = (*it);
					// Save old sets to detect changes
					std::set<std::string> old_in = instr->live_in;
					std::set<std::string> old_out = instr->live_out;

					// live_out = union of live_in of successors
					instr->live_out.clear();
					if (it == block.instructions.rbegin()) {
						// last instruction in block
						for (auto& succ_block : block.successors) {
							if(succ_block->instructions.empty())
								continue;
							instr->live_out.insert(
								succ_block->instructions.front()->live_in.begin(),
								succ_block->instructions.front()->live_in.end()
							);
						}
					} else {
						// next instruction in the same block
						auto next_it = std::prev(it);
						instr->live_out = (*(next_it))->live_in;
					}

					// live_in = use union (live_out - def)
					instr->live_in = instr->use;
					for (const std::string& var : instr->live_out) {
						if (instr->def.find(var) == instr->def.end()) {
							instr->live_in.insert(var);
						}
					}
					// Check if anything changed
					if (instr->live_in != old_in || instr->live_out != old_out) {
						changed = true;
					}
				}
			}
		}
	}

	void Program::create_interference_graph_nodes()
	{
		auto make_node = [this](const std::string& id) -> void {
			_interference_nodes.emplace(id,interference_node_t{id,{}});
		};
		for (auto& instr : _instructions) {
			if (instr.operation == ir_instruction_t::operation_::ALLOC)
				continue;
			if (!instr.src1.empty() && !is_immediate(instr.src1) && !instr.src1_is_stack_offset)
				make_node(instr.src1);
			if (!instr.src2.empty() && !is_immediate(instr.src2))
				make_node(instr.src2);
			if (!instr.dest.empty() && !is_immediate(instr.dest)) {
				if (instr.operation == ir_instruction_t::operation_::STORE) {
					if(instr.store_dest_is_ptr)
						make_node(instr.dest);
				}
				else {
						make_node(instr.dest);
				}
			}
		}
	}
	void Program::compute_interference_graph()
	{
		create_interference_graph_nodes();
		auto get_node = [this](const std::string& id) -> interference_node_t* {
			auto it = _interference_nodes.find(id);
			if (it == _interference_nodes.end()) {
				auto n = interference_node_t{id, {}};
				_interference_nodes[id] = n;
				return &_interference_nodes[id];
			}
			return &(it->second);
		};

		for(const auto& block : _blocks){
			for (auto it = block.instructions.begin(); it != block.instructions.end();it++){

				const ir_instruction_t* instr = *it;

				if (instr->operation == ir_instruction_t::operation_::ALLOC)
					continue;
				if(instr->dest.empty())
					continue;
				if (instr->operation == ir_instruction_t::operation_::STORE && !instr->store_dest_is_ptr)
					continue;
				interference_node_t* d = get_node(instr->dest);

				for (const std::string& o : instr->live_out) {
					if (o != instr->dest && !o.empty()) {
						interference_node_t* n = get_node(o);
						d->edges.push_back(n);
						n->edges.push_back(d);
					}
				}
			}
		}
		// number of registers
		const int K = 6;

		// build unoredered_map of [id,neighbors] from interference nodes
		std::unordered_map<std::string, std::unordered_set<std::string>> adj;
		adj.reserve(_interference_nodes.size());
		for (const auto& kv : _interference_nodes) {
			const std::string& id = kv.first;
			const interference_node_t& node = kv.second;
			std::unordered_set<std::string>& neighbors = adj[id]; // creates empty if needed
			for (interference_node_t*p : node.edges) {
				if (p == nullptr) continue;
				if (p->id == id) continue;
				neighbors.insert(p->id);
			}
		}

		// Work on a modifiable set of nodes
		std::unordered_set<std::string> nodes;
		for (const auto& kv : adj) nodes.insert(kv.first);

		std::vector<std::string> simplify_stack;
		std::unordered_set<std::string> spilled_candidates;

		std::unordered_map<std::string, std::unordered_set<std::string>> adj_copy = adj;

		while (!nodes.empty()) {
			bool removed_node = false;

			// try to find node with degree < K
			for (const std::string& n : nodes) {
				int deg = static_cast<int>(adj_copy[n].size());
				if (deg < K) {
					// push and remove
					simplify_stack.push_back(n);
					// remove n from neighbors
					for (const std::string& nb : adj_copy[n]) {
						adj_copy[nb].erase(n);
					}
					adj_copy.erase(n);
					nodes.erase(n);
					removed_node = true;
					break;
				}
			}

			if (removed_node) continue;

			std::string spill;
			int maxdeg = -1;
			for (const std::string& n : nodes) {
				int deg = static_cast<int>(adj_copy[n].size());
				if (deg > maxdeg) {
					maxdeg = deg;
					spill = n;
				}
			}
			if (!spill.empty()) {
				spilled_candidates.insert(spill);
				simplify_stack.push_back(spill);
				for (const auto& nb : adj_copy[spill]) {
					adj_copy[nb].erase(spill);
				}
				adj_copy.erase(spill);
				nodes.erase(spill);
			} else {
				break;
			}
		}

		std::unordered_map<std::string,int> coloring;
		std::vector<std::string> actually_spilled;

		while (!simplify_stack.empty()) {
			std::string node = simplify_stack.back();
			simplify_stack.pop_back();

			std::vector<bool> used(K, false);
			for (const std::string& nb : adj[node]) {
				auto itc = coloring.find(nb);
				if (itc != coloring.end()) {
					int c = itc->second;
					if (c >= 0 && c < K) used[c] = true;
				}
			}

			int chosen = -1;
			for (int c = 0; c < K; ++c) {
				if (!used[c]) {
					chosen = c;
					break;
				}
			}
			if (chosen == -1) {
				actually_spilled.push_back(node);
			} else {
				coloring[node] = chosen;
			}
		}
		_coloring = std::move(coloring);
		_spilled_vars = std::move(actually_spilled);
	}
	
	std::string Program::get_allocated_reg_for_var(const std::string& id){
		if (_coloring.find(id) != _coloring.end()) {
			return "t" + std::to_string(_coloring[id]);
		}
		return "";
	}
	void Program::convert_function_blocks_to_asm(std::ostream& os){
		for(function_block_t& function_block : _function_blocks){

			// emit the function label and function prologue
			os << function_block.label_instr->label_id << ":\n";
			int32_t frame_size = align_up(function_block.frame_size,16);
			os << '\t' << std::left << std::setw(8) << "addi" << "sp,sp," << -frame_size << '\n';
			os << '\t' << std::left << std::setw(8) << "sw" << "ra," << frame_size - 4 << "(sp)\n";
			os << '\t' << std::left << std::setw(8) << "sw" << "s0," << frame_size - 8 << "(sp)\n";
			os << '\t' << std::left << std::setw(8) << "addi" << "s0,sp," << frame_size << '\n'; // frame pointer

			const int MNEMONIC_WIDTH = 8;
			auto emit = [&](const std::string& mnemonic, const std::string& operands){
				if(mnemonic.empty()){
					os << '\t' << operands << '\n';
				} else {
					os << '\t' << std::left << std::setw(MNEMONIC_WIDTH) << mnemonic << operands << '\n';
				}
			};

		
			// temporary scratch register used when we need to materialize immediates or spilled loads
			const std::string scratch = "t6";
			const std::string scratch2 = "t5";

			auto resolve_reg = [&](const std::string id) -> std::string
			{
				std::string allc = get_allocated_reg_for_var(id);
				if(!allc.empty())
					return allc;
				if(std::find(_spilled_vars.begin(),_spilled_vars.end(),id) != _spilled_vars.end()){
					emit("lw", scratch + "," + actual_offset(function_block.stack[id]) + "(s0)");
				}
				return scratch;
			};
	
			int32_t argument_counter = 0;
			int32_t call_argument_counter = 0;
			for (size_t instr_idx = 0; instr_idx < function_block.instructions.size(); ++instr_idx)
			{
				ir_instruction_t* instruction = function_block.instructions[instr_idx];

				if( instruction->operation != ir_instruction_t::operation_::ADDR
					&& instruction->operation != ir_instruction_t::operation_::ARG
					&& function_block.stack.find(instruction->src1) != function_block.stack.end()
					&& !instruction->src1_is_stack_offset
					){
					std::string op = resolve_reg(instruction->src1) + "," + actual_offset(function_block.stack[instruction->src1]) + "(s0)";
					emit("lw", op);
				}
				if(instruction->operation != ir_instruction_t::operation_::ADDR
					&& instruction->operation != ir_instruction_t::operation_::ARG
					&& function_block.stack.find(instruction->src2) != function_block.stack.end()
					){
					std::string op = resolve_reg(instruction->src2) + "," + actual_offset(function_block.stack[instruction->src2]) + "(s0)";
					emit("lw", op);
				}
				// store instructon's destination is a memory address
				if(instruction->operation == ir_instruction_t::operation_::STORE &&
					instruction->store_dest_is_ptr
					&& function_block.stack.find(instruction->dest) != function_block.stack.end()
					){
					std::string op = resolve_reg(instruction->dest) + "," + actual_offset(function_block.stack[instruction->dest]) + "(s0)";
					emit("lw", op);
				}

				switch(instruction->operation){
					case ir_instruction_t::operation_::PARAM:{
						// PARAM handled as storing argument register onto stack slot for the named parameter
						if (argument_counter < 8) {
							std::string ops = "a" + std::to_string(argument_counter++) + "," + actual_offset(function_block.stack[instruction->dest]) + "(s0)";
							emit("sw", ops);
						}
						break;
					}
					case ir_instruction_t::operation_::LOAD_CONST: {
						// li dest, imm
						std::string destReg = resolve_reg(instruction->dest);
						emit("li", destReg + "," + instruction->src1);
						break;
					}
					case ir_instruction_t::operation_::MOV: {
						// mv dest, src1  (or li if immediate)
						std::string destReg = resolve_reg(instruction->dest);
						if(is_immediate(instruction->src1)){
							emit("li", destReg + "," + instruction->src1);
						} else {
							std::string srcReg = resolve_reg(instruction->src1);
							if (destReg != srcReg)
								emit("mv", destReg + "," + srcReg);
						}
						if(instruction->store_dest_in_stack){
							emit("sw", destReg + "," + actual_offset(function_block.stack[instruction->dest]) + "(s0)");
						}
						break;
					}
					case ir_instruction_t::operation_::BIT_AND:
					case ir_instruction_t::operation_::SHIFT_LEFT:
					case ir_instruction_t::operation_::SHIFT_RIGHT:
					case ir_instruction_t::operation_::BIT_OR:
					case ir_instruction_t::operation_::BIT_XOR:
					case ir_instruction_t::operation_::ADD:
					case ir_instruction_t::operation_::SUB:
					case ir_instruction_t::operation_::MUL:
					case ir_instruction_t::operation_::DIV:
					case ir_instruction_t::operation_::REM: {
						// binary ops: dest = src1 op src2
						std::string destReg = resolve_reg(instruction->dest);
						std::string op1Reg;
						// src1 may be immediate 
						if(is_immediate(instruction->src1)){
							// materialize into scratch then use
							emit("li", scratch + "," + instruction->src1);
							op1Reg = scratch;
						} else {
							op1Reg = resolve_reg(instruction->src1);
						}
						std::string op2Reg;
						bool op2Immediate = is_immediate(instruction->src2);
						if(op2Immediate){

							if(instruction->operation == ir_instruction_t::operation_::ADD){
								if(instruction->src1_is_stack_offset){
									emit("addi",scratch + "," + "s0" + "," + std::to_string(std::stoi(instruction->src2) + std::stoi(actual_offset(function_block.stack[instruction->src1]))) );
									emit("mv", destReg + "," + scratch);
								}
								else{
									emit("addi", destReg + "," + op1Reg + "," + instruction->src2);
									if(instruction->store_dest_in_stack){
										emit("sw", destReg + "," + actual_offset(function_block.stack[instruction->dest]) + "(s0)");
									}
								}
								break;
							}
							if(instruction->operation == ir_instruction_t::operation_::SHIFT_LEFT){
								emit("slli", destReg + "," + op1Reg + "," + instruction->src2);
								if(instruction->store_dest_in_stack){
									emit("sw", destReg + "," + actual_offset(function_block.stack[instruction->dest]) + "(s0)");
								}
								break;
							}
							if(instruction->operation == ir_instruction_t::operation_::SHIFT_RIGHT){
								emit("srli", destReg + "," + op1Reg + "," + instruction->src2);
								if(instruction->store_dest_in_stack){
									emit("sw", destReg + "," + actual_offset(function_block.stack[instruction->dest]) + "(s0)");
								}
								break;
							}
							if(instruction->operation == ir_instruction_t::operation_::BIT_OR){
								emit("ori", destReg + "," + op1Reg + "," + instruction->src2);
								if(instruction->store_dest_in_stack){
									emit("sw", destReg + "," + actual_offset(function_block.stack[instruction->dest]) + "(s0)");
								}
								break;
							}
							if(instruction->operation == ir_instruction_t::operation_::BIT_XOR){
								emit("xori", destReg + "," + op1Reg + "," + instruction->src2);
								if(instruction->store_dest_in_stack){
									emit("sw", destReg + "," + actual_offset(function_block.stack[instruction->dest]) + "(s0)");
								}
								break;
							}
							if(instruction->operation == ir_instruction_t::operation_::BIT_AND){
								emit("andi", destReg + "," + op1Reg + "," + instruction->src2);
								if(instruction->store_dest_in_stack){
									emit("sw", destReg + "," + actual_offset(function_block.stack[instruction->dest]) + "(s0)");
								}
								break;
							}
							// for other ops materialize into scratch
							emit("li", scratch + "," + instruction->src2);
							op2Reg = scratch;
							if (instruction->operation == ir_instruction_t::operation_::DIV) {
								emit("div", destReg + "," + op1Reg + "," + op2Reg);
								if(instruction->store_dest_in_stack){
									emit("sw", destReg + "," + actual_offset(function_block.stack[instruction->dest]) + "(s0)");
								}
								break;
							}
							if (instruction->operation == ir_instruction_t::operation_::MUL) {

								emit("mul", destReg + "," + op1Reg + "," + op2Reg);
								if(instruction->store_dest_in_stack){
									emit("sw", destReg + "," + actual_offset(function_block.stack[instruction->dest]) + "(s0)");
								}
								break;
							}
							if (instruction->operation == ir_instruction_t::operation_::REM) {

								emit("rem", destReg + "," + op1Reg + "," + op2Reg);
								if(instruction->store_dest_in_stack){
									emit("sw", destReg + "," + actual_offset(function_block.stack[instruction->dest]) + "(s0)");
								}
								break;
							}
							if (instruction->operation == ir_instruction_t::operation_::SUB) {

								emit("sub", destReg + "," + op1Reg + "," + op2Reg);
								if(instruction->store_dest_in_stack){
									emit("sw", destReg + "," + actual_offset(function_block.stack[instruction->dest]) + "(s0)");
								}
								break;
							}
						} else {
							// map op to mnemonic
							op2Reg = resolve_reg(instruction->src2);
							switch(instruction->operation){
								case ir_instruction_t::operation_::ADD:
									if(instruction->src1_is_stack_offset){
										emit("addi",scratch + "," + "s0," + std::to_string(std::stoi(actual_offset(function_block.stack[instruction->src1]))) );
										emit("add", destReg + "," + op2Reg + "," + scratch);
									}
									else{

										emit("add", destReg + "," + op1Reg + "," + op2Reg);
									}
									break;
								case ir_instruction_t::operation_::SUB:
									emit("sub", destReg + "," + op1Reg + "," + op2Reg);
									break;
								case ir_instruction_t::operation_::MUL:
									emit("mul", destReg + "," + op1Reg + "," + op2Reg);
									break;
								case ir_instruction_t::operation_::DIV:
									emit("div", destReg + "," + op1Reg + "," + op2Reg);
									break;
								case ir_instruction_t::operation_::REM:
									emit("rem", destReg + "," + op1Reg + "," + op2Reg);
									break;
								case ir_instruction_t::operation_::BIT_AND:
									emit("and", destReg + "," + op1Reg + "," + op2Reg);
									break;
								case ir_instruction_t::operation_::BIT_OR:
									emit("or", destReg + "," + op1Reg + "," + op2Reg);
									break;
								case ir_instruction_t::operation_::BIT_XOR:
									emit("xor", destReg + "," + op1Reg + "," + op2Reg);
									break;
								case ir_instruction_t::operation_::SHIFT_LEFT:
									emit("sll", destReg + "," + op1Reg + "," + op2Reg);
									break;
								case ir_instruction_t::operation_::SHIFT_RIGHT:
									emit("slr", destReg + "," + op1Reg + "," + op2Reg);
									break;
								default:
									break;
							}

						}
							if(instruction->store_dest_in_stack){
							emit("sw", destReg + "," + actual_offset(function_block.stack[instruction->dest]) + "(s0)");
						}
						break;
					}
					case ir_instruction_t::operation_::LOAD: {
						std::string destReg = resolve_reg(instruction->dest);
						auto it = function_block.stack.find(instruction->src1);
						if (instruction->load_src_is_ptr) {

							std::string addrReg = resolve_reg(instruction->src1);
							emit("lw", destReg + ",0(" + addrReg + ")");
						}
						else if(it != function_block.stack.end()){
							auto off = actual_offset(it->second);
							emit("lw", destReg + "," + off + "(s0)");
						} 
						break;
					}
					case ir_instruction_t::operation_::STORE: {
						// sw src1Reg, 0(ptr) where dest names variable slot
						std::string srcReg;
						std::string addrReg = resolve_reg(instruction->dest);
						if(is_immediate(instruction->src1)){
							emit("li", scratch + "," + instruction->src1);
							srcReg = scratch;
						}
						else {
							srcReg = resolve_reg(instruction->src1);

						}
						/*
							in store instructions instruction_t::src2 implies the offset from the pointer 
							we use it in arrays and stuff
						*/
						if(instruction->store_dest_is_ptr){
							if(instruction->src2.empty())
								emit("sw", srcReg + ",0(" + addrReg + ")");
							else if(is_immediate(instruction->src2)){ // this branch will never be executed probably because the offsets of the pointers are happening to be always zero
								emit("sw", srcReg + "," + std::to_string(std::stoi(instruction->src2) + std::stoi(actual_offset(function_block.stack[instruction->dest]))) + "(" + addrReg + ")");
							}
						}
						else{
							emit("sw", srcReg + "," + std::to_string(std::stoi(instruction->src2) + std::stoi(actual_offset(function_block.stack[instruction->dest]))) + "(" + "s0" + ")");
						}
						break;
					}
					case ir_instruction_t::operation_::ALLOC: {
						break;
					}
					case ir_instruction_t::operation_::GOTO: {
						emit("j", instruction->label_id);
						break;
					}
					case ir_instruction_t::operation_::BEQ:
					case ir_instruction_t::operation_::BNEQ:
					case ir_instruction_t::operation_::BLT:
					case ir_instruction_t::operation_::BLE:
					case ir_instruction_t::operation_::BGT:
					case ir_instruction_t::operation_::BGE: {
						// branch src1, src2, label
						std::string r1, r2;
						if(is_immediate(instruction->src1)){
							if(instruction->src1 == "0"){
								r1 = "zero";
							}
							else{

								emit("li", scratch + "," + instruction->src1);
								r1 = scratch;
							}
						} else {
							r1 = resolve_reg(instruction->src1);
						}
						if(is_immediate(instruction->src2)){
							if(instruction->src2 == "0"){
								r2 = "zero";
							}else{
								emit("li", scratch + "," + instruction->src2);
								r2 = scratch;
							}
						} else {
							r2 = resolve_reg(instruction->src2);
						}
						std::string mnemonic;
						switch(instruction->operation){
							case ir_instruction_t::operation_::BEQ: mnemonic = "beq"; break;
							case ir_instruction_t::operation_::BNEQ: mnemonic = "bne"; break;
							case ir_instruction_t::operation_::BLT: mnemonic = "blt"; break;
							case ir_instruction_t::operation_::BLE: mnemonic = "ble"; break;
							case ir_instruction_t::operation_::BGT: mnemonic = "bgt"; break;
							case ir_instruction_t::operation_::BGE: mnemonic = "bge"; break;
							default: mnemonic = "beq"; break;
						}
						emit(mnemonic, r1 + "," + r2 + "," + instruction->label_id);
						break;
					}
					case ir_instruction_t::operation_::LABEL: {
						// labels inside functions: emit label followed by newline
						os << instruction->label_id << ":\n";
						break;
					}
					case ir_instruction_t::operation_::ARG: {
						if (call_argument_counter >= 8) {
							std::string srcreg = resolve_reg(instruction->src1);
							emit("sw", srcreg + "," + std::to_string(function_block.stack[instruction->src1]) + ",(sp)");
							argument_counter++;
						}
						// move an argument into aN
						std::string targetReg = "a" + std::to_string(call_argument_counter++);
						if (instruction->src1_is_stack_offset) {
							emit("addi",scratch + "," + "s0," + actual_offset(function_block.stack[instruction->src1]));
							emit("mv", targetReg + "," + scratch);
						}
						else {
							if(is_immediate(instruction->src1)){
								emit("li", targetReg + "," + instruction->src1);
							} else {
								std::string srcReg = resolve_reg(instruction->src1);
								emit("mv", targetReg + "," + srcReg);
							}
						}
						break;
					}
					case ir_instruction_t::operation_::CALL: {
						// emit call and move return value from a0 to dest (if dest exists)
						// The ARG instructions should have prefixed this CALL in IR and populated a0..aN already.
						emit("call", instruction->label_id);
						if(!instruction->dest.empty()){
							std::string destReg = resolve_reg(instruction->dest);
							emit("mv", destReg + ",a0");
						}
						// reset argument counter after call (arguments for next call start from a0)
						call_argument_counter = 0;
						break;
					}
					case ir_instruction_t::operation_::RETURN: {
						// Move return value to a0 if present, then emit epilogue and ret
						if(!instruction->src1.empty()){
							if(is_immediate(instruction->src1)){
								emit("li", "a0," + instruction->src1);
							} else {
								std::string srcReg = resolve_reg(instruction->src1);
								emit("mv", "a0," + srcReg);
							}
						}
						// function epilogue: restore s0, ra and free frame, then ret
						emit("lw", "ra," + std::to_string(frame_size - 4) + "(sp)");
						emit("lw", "s0," + std::to_string(frame_size - 8) + "(sp)");
						emit("addi", "sp,sp," + std::to_string(frame_size));
						emit("jr ", "ra");
						break;
					}
					case ir_instruction_t::operation_::NOP:{
						emit("nop", "");
						break;
					}
					case ir_instruction_t::operation_::NEG:{
						std::string op = resolve_reg(instruction->dest)  + "," + resolve_reg(instruction->src1);
						emit("neg",op);
						break;
					}
					case ir_instruction_t::operation_::BIT_NOT :{
						std::string op = resolve_reg(instruction->dest)  + "," + resolve_reg(instruction->src1);
						emit("not", op);
						break;
					}
					case ir_instruction_t::operation_::ADDR:
					{
						std::string op = resolve_reg(instruction->dest) + ",s0," + actual_offset(function_block.stack[instruction->src1]);
						emit("addi ",op);
						break;
					}
					default: {
						// unknown
						emit("", std::string("# unhandled IR: ") + instruction->to_string());
						argument_counter = 0;
						call_argument_counter = 0;
						break;
					}
				} // end switch
			} // end for instructions

			// If function did not end with an explicit RETURN, emit epilogue + ret
			// find last instruction in function block (if any)
			if(!function_block.instructions.empty()){
				ir_instruction_t* last = function_block.instructions.back();
				if(last->operation != ir_instruction_t::operation_::RETURN){
					// emit epilogue aligned via helper
					emit("lw", "ra," + std::to_string(frame_size - 4) + "(sp)");
					emit("lw", "s0," + std::to_string(frame_size - 8) + "(sp)");
					emit("addi", "sp,sp," + std::to_string(frame_size));
					emit("jr ", "ra");
				}
			} else {
				// empty function body, still emit epilogue
				emit("lw", "ra," + std::to_string(frame_size - 4) + "(sp)");
				emit("lw", "s0," + std::to_string(frame_size - 8) + "(sp)");
				emit("addi", "sp,sp," + std::to_string(frame_size));
				emit("jr", "ra");
			}
		} // end for function_blocks
	}

	void Program::print_ast()
	{
		for (auto& func : _functions) {
			func->print_ast(std::cout);
		}
	}
	void Program::print_IR(std::ostream& os){
		for (auto& instruction : _instructions) {
				os << instruction.to_string() << '\n';
		}
	}
	void Program::analyse(){
		Analysis_Context ctx;
		for(auto& func : _functions){
			func->analyse(ctx);
			ctx.reset();
		}
		if(ctx.get_func_decl_info("main") == nullptr){
			ctx.make_error(ERROR_CODE::MAIN_ENTRY_DOES_NOT_EXIST, "", "Program should have a main entry function");
		}
		_errors = std::move(ctx.get_errors());
	}
	bool Program::has_error() {
		return _errors.size() > 0;
	}
}