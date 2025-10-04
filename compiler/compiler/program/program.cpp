#include "program.hpp"
#include <unordered_set>
#include <iomanip>
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
					//logical_offset += 4;
					logical_offset_s0 += std::stoi(instr->src1); 
					if (block.stack.find(instr->dest) == block.stack.end()) {
						block.stack.emplace(instr->dest,-logical_offset_s0);
					}
				}
				else if(!instr->dest.empty() && instr->dest[0] != '@' && !is_immediate(instr->dest)){
					instr->store_dest_in_stack = true;
					if (block.stack.find(instr->dest) == block.stack.end()) {
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
							block.stack.emplace(instr->dest,-logical_offset_s0);
						}
					}
				}
				else if(!instr->src1.empty() && instr->src1[0] != '@' && !is_immediate(instr->src1) && !instr->src1_is_stack_offset){
					instr->load_var_from_memory = true;
					if (block.stack.find(instr->src1) == block.stack.end()) 
					{
						logical_offset_s0 += 4;
						block.stack.emplace(instr->src1,-logical_offset_s0);
					}
				}
				else if(!instr->src2.empty() && instr->src2[0] != '@' &&  !is_immediate(instr->src2)){
					instr->load_var_from_memory = true;
					if (block.stack.find(instr->src2) == block.stack.end()) {
						logical_offset_s0 += 4;
						block.stack.emplace(instr->src2,-logical_offset_s0);
					}
				}
			}
			block.frame_size = logical_offset_s0 + 8;
		}
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

			size_t scratch_reg_index = 0;
			std::vector<std::string> scratch_registers = { "t0","t1","t2","t3","t4","t5","t6" };
			std::unordered_map <std::string, std::string> var_to_reg;
			std::unordered_map<std::string, std::string> reg_to_var;

		
			auto allocate_reg = [&](const std::string& id) {
				if(id.empty()) return std::string("zero");

				auto it = var_to_reg.find(id);
				if(it != var_to_reg.end()){
					return it->second;
				}

				// allocate from general-purpose scratch registers (exclude last, used as ephemeral scratch)
				const size_t general_count = scratch_registers.size() - 1; // exclude last (t6)
				for(size_t i = 0; i < general_count; ++i){
					const auto &reg = scratch_registers[i];
					if(reg_to_var.find(reg) == reg_to_var.end()){
						var_to_reg.emplace(id, reg);
						reg_to_var.emplace(reg, id);
						return reg;
					}
				}

				std::string chosen = scratch_registers[scratch_reg_index % general_count];
				++scratch_reg_index;
				auto oldIt = reg_to_var.find(chosen);
				if(oldIt != reg_to_var.end()){
					var_to_reg.erase(oldIt->second);
					reg_to_var.erase(oldIt);
				}
				var_to_reg.emplace(id, chosen);
				reg_to_var.emplace(chosen, id);
				return chosen;
			};
			auto resolve_reg = [&](const std::string& id, bool load_from_mem = true) -> std::string {
				if(id.empty()) return std::string("zero");
				if(is_immediate(id)) {
					return scratch_registers.back();
				}
				auto sit = function_block.stack.find(id);
				if(sit != function_block.stack.end()){
					std::string r;
					auto it = var_to_reg.find(id);
					if(it != var_to_reg.end()){
						r = it->second;
					}
					else {
						r = allocate_reg(id);
					}
					if (load_from_mem) {
						auto off = function_block.stack[id];
						std::string off_str = off > 0 ? std::to_string(off) : actual_offset(off);
						emit("lw", r + "," + off_str + "(s0)");
					}
					return r;
				}
				return allocate_reg(id);
			};

			// temporary scratch register used when we need to materialize immediates or spilled loads
			const std::string scratch = "t6";

			int32_t argument_counter = 0;
			int32_t call_argument_counter = 0;
			for (size_t instr_idx = 0; instr_idx < function_block.instructions.size(); ++instr_idx)
			{
				ir_instruction_t* instruction = function_block.instructions[instr_idx];
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
						std::string destReg = resolve_reg(instruction->dest,false);
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
						argument_counter = 0;
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