#include "nodes.hpp"
#include <algorithm>
using namespace f3_compiler::ast_node;
namespace {
	void add_nop_on_label_clash(IR_Gen_Context& ctx) 
	{
		if (ctx.instructions.back().operation == ir_instruction_t::operation_::LABEL) {
			ir_instruction_t nop_instr;
			nop_instr.operation = ir_instruction_t::operation_::NOP;
			ctx.instructions.push_back(nop_instr);
		}
	}
	inline ir_instruction_t::operation_ convert_comparsion_to_branch_operation(ir_instruction_t::operation_ comparison,bool inverse = false){
		switch(comparison){
			case ir_instruction_t::operation_::CMP_EQ:{
				if(inverse)
					return ir_instruction_t::operation_::BNEQ;
				return ir_instruction_t::operation_::BEQ;
			}

			case ir_instruction_t::operation_::CMP_NEQ:{
				if(inverse)
					return ir_instruction_t::operation_::BEQ;
				return ir_instruction_t::operation_::BNEQ;
			}
			case ir_instruction_t::operation_::CMP_LT:{
				if(inverse)
					return ir_instruction_t::operation_::BGT;
				return ir_instruction_t::operation_::BLT;
			}
			case ir_instruction_t::operation_::CMP_LTE:{
				if(inverse)
					return ir_instruction_t::operation_::BGT;
				return ir_instruction_t::operation_::BLE;
			}
			case ir_instruction_t::operation_::CMP_GT:{
				if(inverse)
					return ir_instruction_t::operation_::BLE;
				return ir_instruction_t::operation_::BGT;
			}
			case ir_instruction_t::operation_::CMP_GTE:{
				if(inverse)
					return ir_instruction_t::operation_::BLE;
				return ir_instruction_t::operation_::BGE;
			}

		}
		return ir_instruction_t::operation_::BEQ;
	}
	inline std::string differentiate_var_if_needed(IR_Gen_Context& ctx, const std::string& var_id){

		for (const char c : var_id) {
			if (c == '$')
				return var_id;
		}
		if(var_id[0] == 't')
			return var_id;
		return  "$" + var_id + std::to_string(ctx.symbol_differentiator[var_id]);
	}
}
std::string func_decl_t::generate_ir(IR_Gen_Context& ctx) const {
	ir_instruction_t instr;
	// fucntion entry label
	instr.operation = ir_instruction_t::operation_::FUNC_ENTRY;
	instr.label_id = id;
	ctx.instructions.push_back(std::move(instr));
	for (auto& argument : arguments) {
		ir_instruction_t instr;
		instr.operation = ir_instruction_t::operation_::PARAM;
		instr.dest = differentiate_var_if_needed(ctx,argument.name);
		ctx.instructions.push_back(std::move(instr));
	}
	body->generate_ir(ctx);
	// function return label
	if(!ctx.return_jump_labels.empty()){
		ir_instruction_t instr;
		instr.operation = ir_instruction_t::operation_::LABEL;
		instr.label_id = ctx.generate_label();
		ctx.instructions.push_back(std::move(instr));
	}
	return "";
}
std::string block_statement_t::generate_ir(IR_Gen_Context& ctx) const {
	for(auto& statement : statements){
		statement->generate_ir(ctx);
	}
	return "";
}

std::string var_decl_statement_t::generate_ir(IR_Gen_Context& ctx) const {
	ir_instruction_t instr;
	if(rhs){
		std::string source = rhs->generate_ir(ctx);
		instr.operation = ir_instruction_t::operation_::MOV;
		instr.dest = differentiate_var_if_needed(ctx,name);
		instr.src1 = differentiate_var_if_needed(ctx,source);
		instr.store_dest_in_stack = true;
		ctx.instructions.push_back(std::move(instr));
	}
	return "";
}
std::string if_statement_t::generate_ir(IR_Gen_Context& ctx) const {
	std::string label_1 = ctx.generate_label();
	std::string label_2 = ctx.generate_label();

	ir_instruction_t branch_instr;
	condition.back()->generate_ir(ctx);
	branch_instr.operation = convert_comparsion_to_branch_operation(ctx.comparison_instruction.operation,true);
	branch_instr.label_id = label_1;
	branch_instr.src1 = ctx.comparison_instruction.src1;
	branch_instr.src2 = ctx.comparison_instruction.src2;
	ctx.instructions.push_back(std::move(branch_instr));

	ir_instruction_t label__;
	label__.operation = ir_instruction_t::operation_::LABEL;
	label__.label_id = ctx.generate_label();
	ctx.instructions.push_back(std::move(label__));
	body->generate_ir(ctx);

	if (else_body && !else_body->statements.empty()) {

		ir_instruction_t goto_instr;
		goto_instr.operation = ir_instruction_t::operation_::GOTO;
		goto_instr.label_id = label_2;
		ctx.instructions.push_back(std::move(goto_instr));

		ir_instruction_t else_label;
		else_label.operation = ir_instruction_t::operation_::LABEL;
		else_label.label_id = label_1;
		add_nop_on_label_clash(ctx);
		ctx.instructions.push_back(std::move(else_label));

		else_body->generate_ir(ctx);

		ir_instruction_t label_instr;
		label_instr.operation = ir_instruction_t::operation_::LABEL;
		label_instr.label_id = label_2;
		add_nop_on_label_clash(ctx);
		ctx.instructions.push_back(std::move(label_instr));
		return "";
	}
	ir_instruction_t label_instr;
	label_instr.operation = ir_instruction_t::operation_::LABEL;
	label_instr.label_id = label_1;
	add_nop_on_label_clash(ctx);
	ctx.instructions.push_back(std::move(label_instr));

	return "";
}
std::string while_statement_t::generate_ir(IR_Gen_Context& ctx) const {

	//  entry
	ir_instruction_t goto_instr;
	goto_instr.operation = ir_instruction_t::operation_::GOTO;
	std::string condition_label = ctx.generate_label();
	goto_instr.label_id = condition_label;

	ctx.skip_jump_labels.push_back(condition_label);
	ctx.instructions.push_back(std::move(goto_instr));

	// Body label
	ir_instruction_t label_instr;
	label_instr.operation = ir_instruction_t::operation_::LABEL;
	std::string body_label = ctx.generate_label();
	label_instr.label_id = body_label;
	add_nop_on_label_clash(ctx);
	ctx.instructions.push_back(std::move(label_instr));

	body->generate_ir(ctx);

	ir_instruction_t label_instr_;
	label_instr_.operation = ir_instruction_t::operation_::LABEL;
	label_instr_.label_id = condition_label;
	add_nop_on_label_clash(ctx);
	ctx.instructions.push_back(std::move(label_instr_));

	while(!ctx.skip_jump_labels.empty() && ctx.skip_jump_labels.back() == condition_label) {
		ctx.skip_jump_labels.pop_back();
	}

	// Condition
	condition.back()->generate_ir(ctx);
	ir_instruction_t branch_instruction;
	branch_instruction.operation = convert_comparsion_to_branch_operation(ctx.comparison_instruction.operation);
	branch_instruction.src1 = ctx.comparison_instruction.src1;
	branch_instruction.src2 = ctx.comparison_instruction.src2;
	branch_instruction.label_id = body_label;
	ctx.instructions.push_back(std::move(branch_instruction));


	if(!ctx.break_jump_labels.empty()){
		ir_instruction_t label_instr;
		label_instr.operation = ir_instruction_t::operation_::LABEL;
		std::string break_label = ctx.break_jump_labels.back();
		label_instr.label_id = break_label;
		ctx.instructions.push_back(std::move(label_instr));
		while(!ctx.break_jump_labels.empty() && ctx.break_jump_labels.back() == break_label) {
			ctx.break_jump_labels.pop_back();
		}
	}
	else {
		// For Basic blocks
		ir_instruction_t label__;
		label__.operation = ir_instruction_t::operation_::LABEL;
		label__.label_id = ctx.generate_label();
		ctx.instructions.push_back(std::move(label__));
	}

	return "";
}
std::string for_statement_t::generate_ir(IR_Gen_Context& ctx) const {

	//  entry
	ir_instruction_t goto_instr;
	goto_instr.operation = ir_instruction_t::operation_::GOTO;
	std::string condition_label = ctx.generate_label();
	goto_instr.label_id = condition_label;

	ctx.skip_jump_labels.push_back(condition_label);
	ctx.instructions.push_back(std::move(goto_instr));

	// body label
	ir_instruction_t body_label_instr;
	body_label_instr.operation = ir_instruction_t::operation_::LABEL;
	std::string body_label = ctx.generate_label();
	body_label_instr.label_id = body_label;
	add_nop_on_label_clash(ctx);
	ctx.instructions.push_back(std::move(body_label_instr));
	body->generate_ir(ctx);

	// Increment and stuff
	std::string skip_jump_label;
	std::string break_jump_label;
	if(!ctx.skip_jump_labels.empty()){
		ir_instruction_t label_instr;
		label_instr.operation = ir_instruction_t::operation_::LABEL;
		label_instr.label_id = skip_jump_label = ctx.skip_jump_labels.back();
		add_nop_on_label_clash(ctx);
		ctx.instructions.push_back(std::move(label_instr));
	}
	step->generate_ir(ctx);

	ir_instruction_t label_instr;
	label_instr.operation = ir_instruction_t::operation_::LABEL;

	condition->generate_ir(ctx); // generates the conditional var
	ir_instruction_t branch_instr;
	branch_instr.operation = convert_comparsion_to_branch_operation(ctx.comparison_instruction.operation);
	branch_instr.label_id = body_label;
	branch_instr.src1 = ctx.comparison_instruction.src1;
	branch_instr.src2 = ctx.comparison_instruction.src2;
	ctx.instructions.push_back(std::move(branch_instr));

	if(!ctx.break_jump_labels.empty()){
		label_instr.label_id = break_jump_label = ctx.break_jump_labels.back();
		add_nop_on_label_clash(ctx);
		ctx.instructions.push_back(std::move(label_instr));
	}
	else {
		// For Basic blocks
		ir_instruction_t label__;
		label__.operation = ir_instruction_t::operation_::LABEL;
		label__.label_id = ctx.generate_label();
		ctx.instructions.push_back(std::move(label__));
	}
	while(!ctx.break_jump_labels.empty() &&  ctx.break_jump_labels.back() == break_jump_label){
		ctx.break_jump_labels.pop_back();
	}
	while(!ctx.skip_jump_labels.empty() && ctx.skip_jump_labels.back() == skip_jump_label){
		ctx.skip_jump_labels.pop_back();
	}
	return "";
}
std::string return_statement_t::generate_ir(IR_Gen_Context& ctx) const {

	std::string temp = return_expr->generate_ir(ctx);
	ir_instruction_t return_instruction;
	return_instruction.operation = ir_instruction_t::operation_::RETURN;
	return_instruction.src1 = differentiate_var_if_needed(ctx,temp);
	ctx.instructions.push_back(std::move(return_instruction));
	return "";
}
std::string break_statement_t::generate_ir(IR_Gen_Context& ctx) const {

	std::string break_jump_label = ctx.generate_label();
	ctx.break_jump_labels.push_back(break_jump_label);

	ir_instruction_t go_to_instr;
	go_to_instr.operation = ir_instruction_t::operation_::GOTO;
	go_to_instr.label_id = break_jump_label;
	ctx.instructions.push_back(std::move(go_to_instr));
	return "";
}
std::string skip_statement_t::generate_ir(IR_Gen_Context& ctx) const {

	ir_instruction_t goto_instr;
	goto_instr.operation = ir_instruction_t::operation_::GOTO;
	goto_instr.label_id = ctx.skip_jump_labels.back();
	ctx.instructions.push_back(std::move(goto_instr));
	return "";
}
std::string statement_t::generate_ir(IR_Gen_Context& ctx) const {

	return "";
}
std::string expr_statement_t::generate_ir(IR_Gen_Context& ctx) const {
	if(expr)
		expr->generate_ir(ctx);
	return "";
}
std::string assignment_expression_t::generate_ir(IR_Gen_Context& ctx) const {
	ir_instruction_t instr;
	ctx.left_is_deref = lhs->is_deref();
	std::string left = lhs->generate_ir(ctx);
	std::string right = rhs->generate_ir(ctx);
	if(left[0] != 't')
		instr.store_dest_in_stack = true;
	instr.dest = differentiate_var_if_needed(ctx, left);
	if (!lhs->is_deref()) {
		switch (type)
		{
		default:
			instr.operation = ir_instruction_t::operation_::MOV;
			instr.src1 = differentiate_var_if_needed(ctx,right);
			break;
		case ASSIGNMENT_TYPE::ADD_ASSIGN:
			instr.operation = ir_instruction_t::operation_::ADD;
			instr.src1 = instr.dest;
			instr.src2 = differentiate_var_if_needed(ctx,right);
			break;
		case ASSIGNMENT_TYPE::SUBTRACT_ASSIGN:
			instr.operation = ir_instruction_t::operation_::SUB;
			instr.src1 = instr.dest;
			instr.src2 = differentiate_var_if_needed(ctx,right);
			break;
		case ASSIGNMENT_TYPE::MULTIPLY_ASSIGN:
			instr.operation = ir_instruction_t::operation_::MUL;
			instr.src1 = instr.dest;
			instr.src2 = differentiate_var_if_needed(ctx,right);
			break;
		case ASSIGNMENT_TYPE::DIVIDE_ASSIGN:
			instr.operation = ir_instruction_t::operation_::DIV;
			instr.src1 = instr.dest;
			instr.src2 = differentiate_var_if_needed(ctx,right);
			break;
		case ASSIGNMENT_TYPE::MOD_ASSIGN:
			instr.operation = ir_instruction_t::operation_::REM;
			instr.src1 = instr.dest;
			instr.src2 = differentiate_var_if_needed(ctx,right);
		}
	}
	else {
		instr.operation = ir_instruction_t::operation_::STORE;
		std::string temp;
		std::string _temp;
		std::string destination = ctx.generate_temp();
		ir_instruction_t instr_2;
		ir_instruction_t load_instr;
		switch (type)
		{
		case ASSIGNMENT_TYPE::ADD_ASSIGN:
			temp = ctx.generate_temp();
			load_instr.operation = ir_instruction_t::operation_::LOAD;
			load_instr.dest = temp;
			load_instr.src1 = left;
			ctx.instructions.push_back(std::move(load_instr));

			instr_2.operation = ir_instruction_t::operation_::ADD;
			instr_2.dest = destination;
			instr_2.src1 = temp;
			instr_2.src2 = right;
			ctx.instructions.push_back(std::move(instr_2));

			instr.src1 = destination;
			break;
		case ASSIGNMENT_TYPE::SUBTRACT_ASSIGN:
			temp = ctx.generate_temp();
			load_instr.operation = ir_instruction_t::operation_::LOAD;
			load_instr.dest = temp;
			load_instr.src1 = left;
			ctx.instructions.push_back(std::move(load_instr));

			instr_2.operation = ir_instruction_t::operation_::SUB;
			instr_2.dest = destination;
			instr_2.src1 = temp;
			instr_2.src2 = right;
			ctx.instructions.push_back(std::move(instr_2));

			instr.src1 = destination;
			break;

		case ASSIGNMENT_TYPE::MULTIPLY_ASSIGN:
			temp = ctx.generate_temp();
			load_instr.operation = ir_instruction_t::operation_::LOAD;
			load_instr.dest = temp;
			load_instr.src1 = left;
			ctx.instructions.push_back(std::move(load_instr));

			instr_2.operation = ir_instruction_t::operation_::MUL;
			instr_2.dest = destination;
			instr_2.src1 = temp;
			instr_2.src2 = right;
			ctx.instructions.push_back(std::move(instr_2));

			instr.src1 = destination;
			break;
		case ASSIGNMENT_TYPE::DIVIDE_ASSIGN:
			temp = ctx.generate_temp();
			load_instr.operation = ir_instruction_t::operation_::LOAD;
			load_instr.dest = temp;
			load_instr.src1 = left;
			ctx.instructions.push_back(std::move(load_instr));

			instr_2.operation = ir_instruction_t::operation_::DIV;
			instr_2.dest = destination;
			instr_2.src1 = temp;
			instr_2.src2 = right;
			ctx.instructions.push_back(std::move(instr_2));

			instr.src1 = destination;
			break;
		case ASSIGNMENT_TYPE::MOD_ASSIGN:
			temp = ctx.generate_temp();
			load_instr.operation = ir_instruction_t::operation_::LOAD;
			load_instr.dest = temp;
			load_instr.src1 = left;
			ctx.instructions.push_back(std::move(load_instr));

			instr_2.operation = ir_instruction_t::operation_::REM;
			instr_2.dest = destination;
			instr_2.src1 = temp;
			instr_2.src2 = right;
			ctx.instructions.push_back(std::move(instr_2));

			instr.src1 = destination;
			break;
		default:
			instr.src1 = differentiate_var_if_needed(ctx,right);
			instr.operation = ir_instruction_t::operation_::STORE;
		}

	}
	ctx.instructions.push_back(std::move(instr));
	return "";
}
std::string integer_literal_t::generate_ir(IR_Gen_Context& ctx) const {
	ir_instruction_t instr;
	instr.operation = ir_instruction_t::operation_::LOAD_CONST;
	std::string temp = ctx.generate_temp();
	instr.dest = temp;
	instr.src1 = std::to_string(value);
	ctx.instructions.push_back(std::move(instr));
	return temp;
}
std::string var_expression_t::generate_ir(IR_Gen_Context& ctx) const {
	return name;
}
std::string binary_expression_t::generate_ir(IR_Gen_Context& ctx) const {

	ir_instruction_t instr;
	std::string left = lhs->generate_ir(ctx);
	std::string right = rhs->generate_ir(ctx);
	instr.src1 = differentiate_var_if_needed(ctx,left);
	instr.src2 = differentiate_var_if_needed(ctx,right);
	switch (op) {
	case BIN_OP::ADD:
		instr.operation = ir_instruction_t::operation_::ADD;
		break;
	case BIN_OP::SUB:
		instr.operation = ir_instruction_t::operation_::SUB;
		break;
	case BIN_OP::DIV:
		instr.operation = ir_instruction_t::operation_::DIV;
		break;
	case BIN_OP::MUL:
		instr.operation = ir_instruction_t::operation_::MUL;
		break;
	case BIN_OP::MOD:
		instr.operation = ir_instruction_t::operation_::REM;
		break;
	case BIN_OP::GT:
		instr.operation = ir_instruction_t::operation_::CMP_GT;
		ctx.comparison_instruction = std::move(instr);
		return "";
	case BIN_OP::LT:
		instr.operation = ir_instruction_t::operation_::CMP_LT;
		ctx.comparison_instruction = std::move(instr);
		return "";
	case BIN_OP::GTE:
		instr.operation = ir_instruction_t::operation_::CMP_GTE;
		ctx.comparison_instruction = std::move(instr);
		return "";
	case BIN_OP::LTE:
		instr.operation = ir_instruction_t::operation_::CMP_LTE;
		ctx.comparison_instruction = std::move(instr);
		return "";
	case BIN_OP::EQUALITY:
		instr.operation = ir_instruction_t::operation_::CMP_EQ;
		ctx.comparison_instruction = std::move(instr);
		return "";
	case BIN_OP::NOT_EQUAL:
		instr.operation = ir_instruction_t::operation_::CMP_NEQ;
		ctx.comparison_instruction = std::move(instr);
		return "";
	case BIN_OP::AND:
		instr.operation = ir_instruction_t::operation_::AND;
		break;
	case BIN_OP::OR:
		instr.operation = ir_instruction_t::operation_::OR;
		break;
	default:
		break;
	}
	std::string dest = ctx.generate_temp();
	instr.dest = dest;
	ctx.instructions.push_back(std::move(instr));
	return dest;
}

std::string unary_expression_t::generate_ir(IR_Gen_Context& ctx) const {

	ir_instruction_t instr;
	ir_instruction_t instr_2;
	ir_instruction_t instr_3;
	instr_2.operation = ir_instruction_t::operation_::LOAD_CONST;
	if(op == UNARY_OP::INCR || op == UNARY_OP::DECR){
		ctx.left_is_deref = true;
	}
	std::string left = expr->generate_ir(ctx);

	std::string destination;
	std::string temp = ctx.generate_temp();
	switch (op)
	{
	case UNARY_OP::NEG:
		instr.operation = ir_instruction_t::operation_::NEG;
		break;
	case UNARY_OP::NOT:
		instr.operation = ir_instruction_t::operation_::NOT;
		break;
	case UNARY_OP::ADDR:
		instr.operation = ir_instruction_t::operation_::ADDR;
		break;
	case UNARY_OP::DEREF:{
		if (ctx.left_is_deref )
		{
			if(ptr_depth == 1)
				return left;
			std::string addr_temp = ctx.generate_temp();
			ir_instruction_t load_ptr;
			load_ptr.operation = ir_instruction_t::operation_::LOAD;
			load_ptr.dest = addr_temp;
			load_ptr.src1 = differentiate_var_if_needed(ctx, left);
			ctx.instructions.push_back(std::move(load_ptr));
			return addr_temp;
		}
		if (ptr_depth == 1) {
			std::string addr_temp = ctx.generate_temp();
			ir_instruction_t load_ptr;
			load_ptr.operation = ir_instruction_t::operation_::LOAD;
			load_ptr.dest = addr_temp;
			load_ptr.src1 = differentiate_var_if_needed(ctx, left);
			ctx.instructions.push_back(std::move(load_ptr));
			return addr_temp;
		}
		std::string addr_temp = ctx.generate_temp();
		ir_instruction_t load_ptr;
		load_ptr.operation = ir_instruction_t::operation_::LOAD;
		load_ptr.dest = left;
		load_ptr.src1 = differentiate_var_if_needed(ctx, left);
		ctx.instructions.push_back(std::move(load_ptr));
		return left;
	}
	case UNARY_OP::INCR:
		if (!expr->is_deref()) {
			instr.operation = ir_instruction_t::operation_::ADD;
			// if the non temporary symbol is current function's
			instr.src1 = instr.dest = differentiate_var_if_needed(ctx,left);
			instr.src2 = std::to_string(1);
			if(instr.dest[0] != 't')
				instr.store_dest_in_stack = true;
			ctx.instructions.push_back(std::move(instr));
			return left;
		}
		instr_2.operation = ir_instruction_t::operation_::LOAD;
		instr_2.dest = temp;
		instr_2.src1 = differentiate_var_if_needed(ctx,left);
		ctx.instructions.push_back(std::move(instr_2));

		instr_3.operation = ir_instruction_t::operation_::ADD;
		instr_3.dest = instr_3.src1 = temp;
		instr_3.src2 = std::to_string(1);
		ctx.instructions.push_back(std::move(instr_3));
		instr.operation = ir_instruction_t::operation_::STORE;
		instr.src1 = temp;
		instr.dest = differentiate_var_if_needed(ctx,left);
		ctx.instructions.push_back(std::move(instr));
		ctx.left_is_deref = false;
		return temp;
	case UNARY_OP::DECR:
		if (!expr->is_deref()) {
			instr.operation = ir_instruction_t::operation_::SUB;
			instr.src1 = instr.dest = differentiate_var_if_needed(ctx,left);
			instr.src2 = std::to_string(1);
			if(instr.dest[0] != 't')
				instr.store_dest_in_stack = true;
			ctx.instructions.push_back(std::move(instr));
			return left;
		}
		instr_2.operation = ir_instruction_t::operation_::LOAD;
		instr_2.dest = temp;
		instr_2.src1 = differentiate_var_if_needed(ctx,left);
		ctx.instructions.push_back(std::move(instr_2));

		instr_3.operation = ir_instruction_t::operation_::SUB;
		instr_3.dest = instr_3.src1 = temp;
		instr_3.src2 = std::to_string(1);
		ctx.instructions.push_back(std::move(instr_3));
		instr.operation = ir_instruction_t::operation_::STORE;
		instr.src1 = temp;
		instr.dest = differentiate_var_if_needed(ctx,left);
		ctx.instructions.push_back(std::move(instr));
		ctx.left_is_deref = false;
		return temp;
	default:
		break;
	}
	destination = ctx.generate_temp();
	instr.dest = destination;
	instr.src1 = differentiate_var_if_needed(ctx, left);
	ctx.instructions.push_back(std::move(instr));
	return destination;
}
std::string func_call_expr_t::generate_ir(IR_Gen_Context& ctx) const {
	for(auto& argument : arguments){
		ir_instruction_t instr;
		instr.operation = ir_instruction_t::operation_::ARG;
		instr.src1 = differentiate_var_if_needed(ctx,argument->generate_ir(ctx));
		ctx.instructions.push_back(std::move(instr));
	}
	ir_instruction_t call_instr;
	call_instr.operation = ir_instruction_t::operation_::CALL;
	std::string destination = ctx.generate_temp();
	call_instr.dest = destination;
	call_instr.label_id = id;
	call_instr.func_argument_count = static_cast<uint32_t>(arguments.size());
	ctx.instructions.push_back(std::move(call_instr));
	return destination;
}
// AST printing
auto constexpr COLOR_RESET = "\033[0m";
auto constexpr COLOR_FUNC = "\033[36m"; // cyan
auto constexpr COLOR_VAR = "\033[35m"; // magenta
auto constexpr COLOR_TYPE = "\033[32m"; // green
auto constexpr COLOR_LIT = "\033[33m"; // yellow
auto constexpr COLOR_OP = "\033[31m";  // red;
auto constexpr COLOR_LABEL = "\033[90m"; // gray
static inline std::string bin_op_to_string(f3_compiler::ast_node::BIN_OP op) {
	switch (op) {
	case BIN_OP::ADD: return "+";
	case BIN_OP::SUB: return "-";
	case BIN_OP::DIV: return "/";
	case BIN_OP::MUL: return "*";
	case BIN_OP::MOD: return "%";
	case BIN_OP::GT: return ">";
	case BIN_OP::LT: return "<";
	case BIN_OP::GTE: return ">=";
	case BIN_OP::LTE: return "<=";
	case BIN_OP::EQUALITY: return "==";
	case BIN_OP::NOT_EQUAL: return "!=";
	case BIN_OP::AND: return "&&";
	case BIN_OP::OR: return "||";
	}
	return "????";
}

static inline std::string unary_op_to_string(f3_compiler::ast_node::UNARY_OP op) {
	switch (op)
	{
	case UNARY_OP::NEG:
		return "-";
	case UNARY_OP::INCR:
		return "++";
	case UNARY_OP::DECR:
		return "--";
	case UNARY_OP::NOT:
		return "!";
	case UNARY_OP::ADDR:
		return "&";
	case UNARY_OP::DEREF:
		return "*";
	default:
		return "????";
	}

}
static void draw_branch(std::ostream& os, uint32_t indent_level, bool is_last) {
	for (uint32_t i = 0; i < indent_level; i++)
		os << "    ";
	os << (is_last ? "'-- " : "|--");
}



// AST PRINTING
void while_statement_t::print_ast(std::ostream& os, uint32_t indent_level /*= 0*/, bool is_last /*= true*/) const
{
	draw_branch(os, indent_level, is_last);
	os << COLOR_LABEL << "while_statement" << COLOR_RESET << "\n";

	// Condition
	draw_branch(os, indent_level + 1, false);
	os << COLOR_LABEL << "condition" << COLOR_RESET << "\n";
	for (size_t i = 0; i < condition.size(); i++) {
		condition[i]->print_ast(os, indent_level + 2, i == condition.size() - 1);
	}

	// Body
	draw_branch(os, indent_level + 1, true);
	os << COLOR_LABEL << "body" << COLOR_RESET << "\n";
	if (body) body->print_ast(os, indent_level + 2);
}

void return_statement_t::print_ast(std::ostream& os, uint32_t indent_level /*= 0*/, bool is_last /*= true*/) const
{
	draw_branch(os, indent_level, is_last);
	os << COLOR_LABEL << "return_statement" << COLOR_RESET << "\n";

	// Print the expression being returned (if any)
	if (return_expr) {
		draw_branch(os, indent_level + 1, true);
		os << COLOR_LABEL << "value" << COLOR_RESET << "\n";
		return_expr->print_ast(os, indent_level + 2, true);
	}
}

void func_decl_t::print_ast(std::ostream& os, uint32_t indent_level /*= 0*/, bool is_last /*= true*/) const
{
	draw_branch(os, indent_level, is_last);
	os << COLOR_FUNC << "func_decl" << COLOR_RESET << "\n";
	draw_branch(os, indent_level + 1, false);
	os << COLOR_LABEL << "name : " << COLOR_FUNC << id << COLOR_RESET << "\n";
	draw_branch(os, indent_level + 1, false);
	os << COLOR_LABEL << "return_type : " << COLOR_TYPE << return_type.str() << COLOR_RESET << "\n";
	draw_branch(os, indent_level + 1, arguments.empty() && !body);
	os << COLOR_LABEL << "arguments" << COLOR_RESET << "\n";
	for (size_t i = 0; i < arguments.size(); i++) {
		arguments[i].print_ast(os, indent_level + 2, i == arguments.size() - 1 && !body);
	}
	if (body) {
		draw_branch(os, indent_level + 1, true);
		os << COLOR_LABEL << "body" << COLOR_RESET << "\n";
		body->print_ast(os, indent_level + 2);
	}
}

void expr_statement_t::print_ast(std::ostream& os, uint32_t indent_level /*= 0*/, bool is_last /*= true*/) const
{
	draw_branch(os, indent_level, is_last);
	os << COLOR_LABEL << "expr_statement" << COLOR_RESET << "\n";
	if (expr) {
		expr->print_ast(os, indent_level + 1, true);
	}
}
void skip_statement_t::print_ast(std::ostream& os, uint32_t indent_level /*= 0*/, bool is_last /*= true*/) const
{
	draw_branch(os, indent_level, is_last);
	os << COLOR_LABEL << "skip_statement" << COLOR_RESET << "\n";
}
void integer_literal_t::print_ast(std::ostream& os, uint32_t indent_level, bool is_last) const
{
	draw_branch(os, indent_level, is_last);
	os << COLOR_LABEL << "integer : " << COLOR_LIT << value << COLOR_RESET << "\n";
}
void var_expression_t::print_ast(std::ostream& os, uint32_t indent_level, bool is_last) const
{
	draw_branch(os, indent_level, is_last);
	os << COLOR_LABEL << "variable : " << COLOR_VAR << name << COLOR_RESET << "\n";
}
void func_call_expr_t::print_ast(std::ostream& os, uint32_t indent_level /*= 0*/, bool is_last /*= true*/) const
{
	draw_branch(os, indent_level, is_last);
	os << COLOR_LABEL << "func_call" << COLOR_RESET << "\n";

	// Function name
	draw_branch(os, indent_level + 1, false);
	os << COLOR_LABEL << "name : " << COLOR_FUNC << id << COLOR_RESET << "\n";

	// Arguments
	draw_branch(os, indent_level + 1, arguments.empty());
	os << COLOR_LABEL << "arguments" << COLOR_RESET << "\n";

	for (size_t i = 0; i < arguments.size(); i++) {
		arguments[i]->print_ast(os, indent_level + 2, i == arguments.size() - 1);
	}
}
void binary_expression_t::print_ast(std::ostream& os, uint32_t indent_level, bool is_last) const
{
	draw_branch(os, indent_level, is_last);
	os << COLOR_LABEL << "binary_op : " << COLOR_OP << bin_op_to_string(op) << COLOR_RESET << "\n";
	if (lhs) lhs->print_ast(os, indent_level + 1, false);
	if (rhs) rhs->print_ast(os, indent_level + 1, true);
}

void unary_expression_t::print_ast(std::ostream& os, uint32_t indent_level, bool is_last) const
{
	draw_branch(os, indent_level, is_last);
	os << COLOR_LABEL << "unary_op(" << ptr_depth << "): " << COLOR_OP << unary_op_to_string(op) << COLOR_RESET << "\n";
	if (expr) expr->print_ast(os, indent_level + 1, false);
}
void assignment_expression_t::print_ast(std::ostream& os, uint32_t indent_level, bool is_last) const
{
	draw_branch(os, indent_level, is_last);
	switch (type) {

	case ASSIGNMENT_TYPE::ADD_ASSIGN:
		os << COLOR_LABEL << "p_assignment(+=)" << COLOR_RESET << "\n";
		break;
	case ASSIGNMENT_TYPE::SUBTRACT_ASSIGN:
		os << COLOR_LABEL << "m_assignment(-=)" << COLOR_RESET << "\n";
		break;
	default:
		os << COLOR_LABEL << "assignment(=)" << COLOR_RESET << "\n";
	}
	if (lhs) lhs->print_ast(os, indent_level + 1, false);
	if (rhs) rhs->print_ast(os, indent_level + 1, true);
}
void var_decl_statement_t::print_ast(std::ostream& os, uint32_t indent_level, bool is_last) const
{
	draw_branch(os, indent_level, is_last);
	os << COLOR_LABEL << "variable_decl : " << COLOR_RESET
		<< COLOR_TYPE << type.str() << COLOR_RESET
		<< " " << COLOR_VAR << name << COLOR_RESET << "\n";
	if (rhs) {
		rhs->print_ast(os, indent_level + 1, true);
	}
}
void block_statement_t::print_ast(std::ostream& os, uint32_t indent_level, bool is_last /*= true*/) const
{
	for (size_t i = 0; i < statements.size(); i++) {
		// For each child statement, indent one level deeper
		// Pass 'true' for is_last only if this is the last child
		statements[i]->print_ast(os, indent_level + 1, i == statements.size() - 1);
	}
}
void if_statement_t::print_ast(std::ostream& os, uint32_t indent_level /*= 0*/, bool is_last /*= true*/) const
{
	draw_branch(os, indent_level, is_last);
	os << COLOR_LABEL << "if_statement" << COLOR_RESET << "\n";

	draw_branch(os, indent_level + 1, false);
	os << COLOR_LABEL << "condition" << COLOR_RESET << "\n";
	for (size_t i = 0; i < condition.size(); i++) {
		condition[i]->print_ast(os, indent_level + 2, i == condition.size() - 1);
	}

	draw_branch(os, indent_level + 1, !else_body);
	os << COLOR_LABEL << "then" << COLOR_RESET << "\n";
	if (body) body->print_ast(os, indent_level + 2);

	if (else_body) {
		draw_branch(os, indent_level + 1, true);
		os << COLOR_LABEL << "else" << COLOR_RESET << "\n";
		else_body->print_ast(os, indent_level + 2);
	}
}
void break_statement_t::print_ast(std::ostream& os, uint32_t indent_level /*= 0*/, bool is_last /*= true*/) const
{
	draw_branch(os, indent_level, is_last);
	os << COLOR_LABEL << "break_statement" << COLOR_RESET << "\n";
}
void for_statement_t::print_ast(std::ostream& os, uint32_t indent_level /*= 0*/, bool is_last /*= true*/) const
{
	draw_branch(os, indent_level, is_last);
	os << COLOR_LABEL << "for_statement" << COLOR_RESET << "\n";

	// Range expression
	draw_branch(os, indent_level + 1, false);
	os << COLOR_LABEL << "range" << COLOR_RESET << "\n";
	if (condition)
		condition->print_ast(os, indent_level + 2, true);

	// Step expression
	draw_branch(os, indent_level + 1, false);
	os << COLOR_LABEL << "step" << COLOR_RESET << "\n";
	if (step)
		step->print_ast(os, indent_level + 2, true);

	// Body block
	draw_branch(os, indent_level + 1, true);
	os << COLOR_LABEL << "body" << COLOR_RESET << "\n";
	if (body)
		body->print_ast(os, indent_level + 2, true);
}

void f3_compiler::ast_node::func_decl_param_t::print_ast(std::ostream& os, uint32_t indent_level /*= 0*/, bool is_last /*= true*/) const
{
	draw_branch(os, indent_level, is_last);
	os << COLOR_LABEL << "param : " << COLOR_RESET
		<< COLOR_TYPE << type.str() << COLOR_RESET
		<< " " << COLOR_VAR << name << COLOR_RESET << "\n";
}
