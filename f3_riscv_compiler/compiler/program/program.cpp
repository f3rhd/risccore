#include "program.hpp"
namespace f3_compiler {
	void Program::generate_IR() {

		IR_Gen_Context ctx;
		for (auto& func : _functions) {
			func->generate_ir(ctx);
		}
		_instructions = std::move(ctx.instructions);
	}

	void Program::generate_basic_blocks() {
		generate_IR();
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
				   _instructions[i].operation != ir_instruction_t::operation_::LABEL) {
				
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
		set_control_flow_graph();
		compute_instruction_use_def();
		compute_instruction_live_in_out();
		compute_interference_graph();
		//compute_block_use_def();
		//compute_block_live_in_out();
	}
	//void Program::compute_block_use_def()
	//{
	//	for (auto& block : _blocks) {
	//		block.use.clear();
	//		block.def.clear();

	//		std::set<std::string> defined;
	//		for (const auto& instr : block.instructions) {

	//			//src1 is used
	//			 if (!instr->src1.empty() && !std::isdigit(instr->src1[0]) && defined.find(instr->src1) == defined.end())
	//				block.use.insert(instr->src1);

	//			// src2 is used
	//			if (!instr->src2.empty() && !std::isdigit(instr->src2[1]) && defined.find(instr->src2) == defined.end())
	//				block.use.insert(instr->src2);

	//			if (!instr->dest.empty()) {
	//				block.def.insert(instr->dest);
	//				defined.insert(instr->dest);
	//			}
	//		}
	//	}

	//}
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

	//void Program::compute_block_live_in_out()
	//{
	//	std::vector<basic_block_t*> worklist;
	//	worklist.reserve(_blocks.size());
	//	for (basic_block_t& block : _blocks) {
	//		worklist.push_back(&block);
	//	}

	//	while (!worklist.empty()) {
	//		basic_block_t* B = worklist.back();
	//		worklist.pop_back();

	//		std::set<std::string> old_in = B->live_in;
	//		std::set<std::string> old_out = B->live_out;

	//		// live-out = union of live-in of successors
	//		B->live_out.clear();
	//		for (auto* succ : B->successors)
	//			B->live_out.insert(succ->live_in.begin(), succ->live_in.end());

	//		// live-in = use union (live-out - def)
	//		B->live_in = B->use;
	//		for (const auto& temp : B->live_out)
	//			if (B->def.find(temp) == B->def.end())
	//				B->live_in.insert(temp);

	//		// if changed, add predecessors back to worklist
	//		if (B->live_in != old_in || B->live_out != old_out) {
	//			for (basic_block_t* pred : B->predecessors)
	//				worklist.push_back(pred);
	//		}
	//	}
	//}


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
							if (!succ_block->instructions.empty()) {
								instr->live_out.insert(
									succ_block->instructions.front()->live_in.begin(),
									succ_block->instructions.front()->live_in.end()
								);
							}
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
			for (auto it = block.instructions.rbegin(); it != block.instructions.rend();it++){

				const ir_instruction_t* instr = *it;

				interference_node_t* d = get_node(instr->dest);

				for (const std::string& o : instr->live_out) {
					if (o != instr->dest) {
						interference_node_t* n = get_node(o);
						d->edges.push_back(n);
						n->edges.push_back(d);
					}
				}
			}
		}
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