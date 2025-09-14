#include "program.hpp"
#include <unordered_set>
#include <iomanip>
namespace f3_compiler {
	namespace {

		inline int32_t align_up(int32_t offset, int32_t alignment) {
			return (offset + alignment - 1) / alignment * alignment;
		}
		inline std::string actual_offset(int32_t logical_offset, int32_t saved_regs_bytes = 8) {
			return std::to_string(-(saved_regs_bytes + logical_offset));
		}
	}
	void Program::generate_asm(std::ostream& os){
		generate_function_blocks();
		generate_basic_blocks();
		set_control_flow_graph();
		compute_instruction_use_def();
		compute_instruction_live_in_out();
		compute_interference_graph();
		convert_function_blocks_to_asm(os);
	}
	std::string Program::get_allocated_reg_for_var(const std::string& id){
		return "t" + std::to_string(_coloring[id]);
	}
	void Program::generate_IR() {

		IR_Gen_Context ctx;
		for (auto& func : _functions) {
			func->generate_ir(ctx);
			ctx.reset();
		}
		_instructions = std::move(ctx.instructions);
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

			int32_t logical_offset = 0;
			for(auto& instr : block.instructions){
				if(!instr->dest.empty() && instr->dest[0] != 't' && !std::isdigit(instr->dest[0])){
					instr->store_dest_in_stack = true;
					if(_function_blocks.back().local_vars.find(instr->dest) == _function_blocks.back().local_vars.end())
						logical_offset += 4;
					_function_blocks.back().local_vars.emplace(instr->dest,-logical_offset);
				}
				if(!instr->src1.empty() && instr->src1[0] != 't' && !std::isdigit(instr->src1[0])){
					instr->load_var_from_memory = true;
					if(_function_blocks.back().local_vars.find(instr->src1) == _function_blocks.back().local_vars.end())
						logical_offset += 4;
					_function_blocks.back().local_vars.emplace(instr->src1,-logical_offset);
				}
				if(!instr->src2.empty() && instr->src2[0] != 't' &&  !std::isdigit(instr->src2[0])){
					instr->load_var_from_memory = true;
					if(_function_blocks.back().local_vars.find(instr->src2) == _function_blocks.back().local_vars.end())
						logical_offset += 4;
					_function_blocks.back().local_vars.emplace(instr->src2,-logical_offset);
				}
			}
			block.frame_size = logical_offset;
		}
	} 
	void Program::generate_basic_blocks() {
		uint64_t i = 0;
		uint64_t size = _instructions.size();

		while (i < size) {
		// Only start a block if we see a LABEL
		if (_instructions[i].operation == ir_instruction_t::operation_::LABEL) {
			basic_block_t block;
			block.label_instr = &_instructions[i];
			i++;

			// Collect instructions until we hit a terminator or a new LABEL
			while (i < size &&
				   _instructions[i].operation != ir_instruction_t::operation_::LABEL
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
				const_cast<basic_block_t*>(get_basic_block_by_label(block.instructions.back()->label_id))->predecessors.push_back(&block);
				break;
			case ir_instruction_t::operation_::BEQ: 
			case ir_instruction_t::operation_::BNEQ: 
			case ir_instruction_t::operation_::BLT:
			case ir_instruction_t::operation_::BLE:
			case ir_instruction_t::operation_::BGT: 
			case ir_instruction_t::operation_::BGE:
				block.successors.push_back(get_basic_block_by_label(block.instructions.back()->label_id));
				const_cast<basic_block_t*>(get_basic_block_by_label(block.instructions.back()->label_id))->predecessors.push_back(&block);
				if (i + 1 < _blocks.size()) {
					block.successors.push_back(get_basic_block_by_label(_blocks[i + 1].label_instr->label_id));
					const_cast<basic_block_t*>(get_basic_block_by_label(_blocks[i+1].label_instr->label_id))->predecessors.push_back(&block);
				}
				break;
			}
		}
	}

	void Program::compute_instruction_use_def(){
		for(auto& instr : _instructions){
			if(!instr.src1.empty() && !std::isdigit(instr.src1[0])) {
				instr.use.insert(instr.src1);
			}
			if (!instr.src2.empty() && !std::isdigit(instr.src2[0])) {
				instr.use.insert(instr.src2);
			}
			if(!instr.dest.empty() && !std::isdigit(instr.dest[0])) {
				// in store instruction both variables are used rather one of them being defined
				if(instr.operation == ir_instruction_t::operation_::STORE){
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

		// Iterate until live_in/live_out stabilize (fixed point)
		while (changed) {
			changed = false;

			for (auto& block : _blocks) {
				// Iterate instructions backward in the block
				for (auto it = block.instructions.rbegin(); it != block.instructions.rend(); ++it) {
					ir_instruction_t* instr = (*it);
					// Save old sets to detect changes
					std::set<std::string> old_in = instr->live_in;
					std::set<std::string> old_out = instr->live_out;

					// Compute live_out = union of live_in of successors
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

					// Compute live_in = use union (live_out - def)
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
			_interference_nodes.emplace(id, id);
		};
		for (auto& instr : _instructions) {
			if (!instr.src1.empty() && !std::isdigit(instr.src1[0]))
				make_node(instr.src1);
			if (!instr.src2.empty() && !std::isdigit(instr.src2[0]))
				make_node(instr.src2);
			if (!instr.dest.empty() && !std::isdigit(instr.dest[0]))
				make_node(instr.dest);
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

				if(instr->dest.empty())
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
		const int K = 7;

		// Build adjacency sets (unique neighbors)
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

		// degree map
		std::unordered_map<std::string,int> degree;
		for (const auto& kv : adj) degree[kv.first] = static_cast<int>(kv.second.size());

		// Work on a modifiable set of nodes
		std::unordered_set<std::string> nodes;
		for (const auto& kv : adj) nodes.insert(kv.first);

		// simplify stack + spilled candidates
		std::vector<std::string> simplify_stack;
		std::unordered_set<std::string> spilled_candidates;

		// copy adjacency for destructive removal
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

			// No low-degree node found -> pick a spill candidate (heuristic: max degree)
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
				// mark as candidate and remove from graph
				spilled_candidates.insert(spill);
				simplify_stack.push_back(spill);
				for (const auto& nb : adj_copy[spill]) {
					adj_copy[nb].erase(spill);
				}
				adj_copy.erase(spill);
				nodes.erase(spill);
			} else {
				// break to avoid infinite loop
				break;
			}
		}

		// Assign colors by popping stack (reverse order)
		std::unordered_map<std::string,int> coloring;
		std::vector<std::string> actually_spilled;

		while (!simplify_stack.empty()) {
			std::string node = simplify_stack.back();
			simplify_stack.pop_back();

			// collect used colors by already-colored neighbors (original adjacency)
			std::vector<bool> used(K, false);
			for (const std::string& nb : adj[node]) {
				auto itc = coloring.find(nb);
				if (itc != coloring.end()) {
					int c = itc->second;
					if (c >= 0 && c < K) used[c] = true;
				}
			}

			// find first available color
			int chosen = -1;
			for (int c = 0; c < K; ++c) {
				if (!used[c]) {
					chosen = c;
					break;
				}
			}
			if (chosen == -1) {
				// cannot color -> mark spill
				actually_spilled.push_back(node);
			} else {
				coloring[node] = chosen;
			}
		}
		// Save results into Program members
		_coloring = std::move(coloring);
		_spilled_vars = std::move(actually_spilled);
	}

	void Program::convert_function_blocks_to_asm(std::ostream& os){
		for(function_block_t& function_block : _function_blocks){

			// emit the function label and function prologue
			os << function_block.label_instr->label_id << ":\n";
			int32_t frame_size = align_up(function_block.frame_size,16);
			// prologue lines are labels and directives — keep as-is but align instruction columns via helper
			os << '\t' << std::left << std::setw(8) << "addi" << "sp,sp," << -frame_size << '\n';
			os << '\t' << std::left << std::setw(8) << "sw" << "ra," << frame_size - 4 << "(sp)\n";
			os << '\t' << std::left << std::setw(8) << "sw" << "s0," << frame_size - 8 << "(sp)\n";
			os << '\t' << std::left << std::setw(8) << "addi" << "s0,sp," << frame_size << '\n'; // frame pointer

			// printing helper: mnemonic column width
			const int MNEMONIC_WIDTH = 8;
			auto emit = [&](const std::string& mnemonic, const std::string& operands){
				// if mnemonic is empty, still indent but don't pad
				if(mnemonic.empty()){
					os << '\t' << operands << '\n';
				} else {
					os << '\t' << std::left << std::setw(MNEMONIC_WIDTH) << mnemonic << operands << '\n';
				}
			};

			// helper lambdas
			auto is_immediate = [](const std::string& s)->bool{
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
			auto is_spilled = [this](const std::string& name)->bool{
				for(const auto &s : _spilled_vars) if(s==name) return true;
				return false;
			};
			// temporary scratch register used when we need to materialize immediates or spilled loads
			const std::string scratch = "t6";

			int32_t argument_counter = 0;
			int32_t call_argument_counter = 0;
			for (size_t instr_idx = 0; instr_idx < function_block.instructions.size(); ++instr_idx)
			{
				ir_instruction_t* instruction = function_block.instructions[instr_idx];

				if( instruction->operation != ir_instruction_t::operation_::ADDR
					&& instruction->operation != ir_instruction_t::operation_::RETURN 
					&& function_block.local_vars.find(instruction->src1) != function_block.local_vars.end()){
					std::string op = get_allocated_reg_for_var(instruction->src1) + "," + actual_offset(function_block.local_vars[instruction->src1]) + "(s0)";
					emit("lw", op);
				}
				if(instruction->operation != ir_instruction_t::operation_::ADDR
					&& instruction->operation != ir_instruction_t::operation_::RETURN 
					&& function_block.local_vars.find(instruction->src2) != function_block.local_vars.end()){
					std::string op = get_allocated_reg_for_var(instruction->src2) + "," + actual_offset(function_block.local_vars[instruction->src2]) + "(s0)";
					emit("lw", op);
				}
				// emit instruction-specific assembly
				switch(instruction->operation){
					case ir_instruction_t::operation_::PARAM:{
						// PARAM handled as storing argument register onto stack slot for the named parameter
						std::string ops = "a" + std::to_string(argument_counter++) + "," + actual_offset(function_block.local_vars[instruction->dest]) + "(s0)";
						emit("sw", ops);
						break;
					}
					case ir_instruction_t::operation_::LOAD_CONST: {
						// li dest, imm
						std::string destReg = get_allocated_reg_for_var(instruction->dest);
						emit("li", destReg + "," + instruction->src1);
						break;
					}
					case ir_instruction_t::operation_::MOV: {
						// mv dest, src1  (or li if immediate)
						std::string destReg = get_allocated_reg_for_var(instruction->dest);
						if(is_immediate(instruction->src1)){
							emit("li", destReg + "," + instruction->src1);
						} else {
							std::string srcReg = get_allocated_reg_for_var(instruction->src1);
							if (destReg != srcReg)
								emit("mv", destReg + "," + srcReg);
						}
						if(instruction->store_dest_in_stack){
							emit("sw", destReg + "," + actual_offset(function_block.local_vars[instruction->dest]) + "(s0)");
						}
						break;
					}
					case ir_instruction_t::operation_::ADD:
					case ir_instruction_t::operation_::SUB:
					case ir_instruction_t::operation_::MUL:
					case ir_instruction_t::operation_::DIV:
					case ir_instruction_t::operation_::REM: {
						// binary ops: dest = src1 op src2
						std::string destReg = get_allocated_reg_for_var(instruction->dest);
						std::string op1Reg;
						// src1 may be immediate (rare) or var/temp
						if(is_immediate(instruction->src1)){
							// materialize into scratch then use
							emit("li", scratch + "," + instruction->src1);
							op1Reg = scratch;
						} else {
							op1Reg = get_allocated_reg_for_var(instruction->src1);
						}
						std::string op2Reg;
						bool op2Immediate = is_immediate(instruction->src2);
						if(op2Immediate){
							// for ADD we can use addi, for SUB there is no subi pseudo so materialize into scratch
							if(instruction->operation == ir_instruction_t::operation_::ADD){
								emit("addi", destReg + "," + op1Reg + "," + instruction->src2);
								if(instruction->store_dest_in_stack){
									emit("sw", destReg + "," + actual_offset(function_block.local_vars[instruction->dest]) + "(s0)");
								}
								break;
							}
							// for other ops materialize into scratch
							emit("li", scratch + "," + instruction->src2);
							op2Reg = scratch;
						} else {
							op2Reg = get_allocated_reg_for_var(instruction->src2);
						}

						// map op to mnemonic
						switch(instruction->operation){
							case ir_instruction_t::operation_::ADD:
								emit("add", destReg + "," + op1Reg + "," + op2Reg);
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
							default:
								break;
						}
						if(instruction->store_dest_in_stack){
							emit("sw", destReg + "," + actual_offset(function_block.local_vars[instruction->dest]) + "(s0)");
						}
						break;
					}
					case ir_instruction_t::operation_::LOAD: {
						// lw destReg, offset(s0)  where src1 names the variable on stack
						std::string destReg = get_allocated_reg_for_var(instruction->dest);
						// src1 should be a local variable (stack slot)
						auto it = function_block.local_vars.find(instruction->src1);
						if(it == function_block.local_vars.end()){
							auto off = actual_offset(it->second);
							emit("lw", destReg + "," + off + "(s0)");
						} else {
							// fallback: load from address held in register src1: lw dest,0(srcReg)
							std::string addrReg = get_allocated_reg_for_var(instruction->src1);
							emit("lw", destReg + ",0(" + addrReg + ")");
						}
						break;
					}
					case ir_instruction_t::operation_::STORE: {
						// sw src1Reg, offset(s0) where dest names variable slot
						std::string srcReg;
						if(is_immediate(instruction->src1)){
							// materialize immediate into scratch then store
							emit("li", scratch + "," + instruction->src1);
							srcReg = scratch;
						} else {
							srcReg = get_allocated_reg_for_var(instruction->src1);
						}
						auto it = function_block.local_vars.find(instruction->dest);
						if(it == function_block.local_vars.end()){
							auto off = actual_offset(it->second);
							emit("sw", srcReg + "," + (off) + "(s0)");
						} else {
							// fallback: dest is an address in register, store to 0(destReg)
							std::string addrReg = get_allocated_reg_for_var(instruction->dest);
							emit("sw", srcReg + ",0(" + addrReg + ")");
						}
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
							emit("li", scratch + "," + instruction->src1);
							r1 = scratch;
						} else {
							r1 = get_allocated_reg_for_var(instruction->src1);
						}
						if(is_immediate(instruction->src2)){
							emit("li", scratch + "," + instruction->src2);
							r2 = scratch;
						} else {
							r2 = get_allocated_reg_for_var(instruction->src2);
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
						// labels inside functions: emit label followed by newline (no tab)
						os << instruction->label_id << ":\n";
						break;
					}
					case ir_instruction_t::operation_::ARG: {
						// move an argument into aN
						std::string targetReg = "a" + std::to_string(call_argument_counter++);
						if(is_immediate(instruction->src1)){
							emit("li", targetReg + "," + instruction->src1);
						} else {
							// if src1 refers to a local var on stack, load it
							auto it = function_block.local_vars.find(instruction->src1);
							if(it != function_block.local_vars.end()){
								auto off = actual_offset(it->second);
								emit("lw", targetReg + "," + off + "(s0)");
							} else {
								std::string srcReg = get_allocated_reg_for_var(instruction->src1);
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
							std::string destReg = get_allocated_reg_for_var(instruction->dest);
							emit("mv", destReg + ",a0");
						}
						// reset argument counter after call (arguments for next call start from a0)
						argument_counter = 0;
						break;
					}
					case ir_instruction_t::operation_::RETURN: {
						// Move return value to a0 if present, then emit epilogue and ret
						if(!instruction->src1.empty()){
							if(is_immediate(instruction->src1)){
								emit("li", "a0," + instruction->src1);
							} else {
								auto it = function_block.local_vars.find(instruction->src1);
								if(it != function_block.local_vars.end()){
									auto off = actual_offset(it->second);
									emit("lw", "a0," + (off) + "(s0)");
								} else {
									std::string srcReg = get_allocated_reg_for_var(instruction->src1);
									emit("mv", "a0," + srcReg);
								}
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
					case ir_instruction_t::operation_::ADDR :  {
						std::string op = get_allocated_reg_for_var(instruction->dest) + ",s0," + actual_offset(function_block.local_vars[instruction->src1]);
						emit("addi ",op);
						break;
					}
					default: {
						// Unknown / unhandled op: print comment with to_string() to help debugging
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
	bool Program::has_error() const
	{
		return had_error;
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
}