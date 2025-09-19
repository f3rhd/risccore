#include "nodes.hpp"
#include <algorithm>
using namespace f3_compiler::ast_node;
/*
  /$$$$$$   /$$$$$$  /$$$$$$$$        /$$$$$$  /$$   /$$  /$$$$$$  /$$   /$$     /$$ /$$$$$$  /$$$$$$  /$$$$$$ 
 /$$__  $$ /$$__  $$|__  $$__/       /$$__  $$| $$$ | $$ /$$__  $$| $$  |  $$   /$$//$$__  $$|_  $$_/ /$$__  $$
| $$  \ $$| $$  \__/   | $$         | $$  \ $$| $$$$| $$| $$  \ $$| $$   \  $$ /$$/| $$  \__/  | $$  | $$  \__/
| $$$$$$$$|  $$$$$$    | $$         | $$$$$$$$| $$ $$ $$| $$$$$$$$| $$    \  $$$$/ |  $$$$$$   | $$  |  $$$$$$ 
| $$__  $$ \____  $$   | $$         | $$__  $$| $$  $$$$| $$__  $$| $$     \  $$/   \____  $$  | $$   \____  $$
| $$  | $$ /$$  \ $$   | $$         | $$  | $$| $$\  $$$| $$  | $$| $$      | $$    /$$  \ $$  | $$   /$$  \ $$
| $$  | $$|  $$$$$$/   | $$         | $$  | $$| $$ \  $$| $$  | $$| $$$$$$$$| $$   |  $$$$$$/ /$$$$$$|  $$$$$$/
|__/  |__/ \______/    |__/         |__/  |__/|__/  \__/|__/  |__/|________/|__/    \______/ |______/ \______/ 
*/
void func_decl_t::analyse(Analysis_Context& ctx) {
	analysis_func_decl_info_t decl_info;
	decl_info.arguments = &arguments;
	decl_info.return_type = &return_type;
	ctx.current_func_id = id;
	ctx.push_func_decl(id, decl_info);

	// Create a new scope for the function and register parameters
	ctx.push_scope();
	for (const auto& param : arguments) {
		ctx.add_var(param.name, param.type);
	}

	// analyse body
	if(body)
		body->analyse(ctx);

	// pop function scope
	ctx.pop_scope();

	if(return_type.base != type_t::BASE::VOID && !ctx.has_return){
		ctx.make_error(ERROR_CODE::FUNCTION_SHOULD_RETURN,
					   id, "Missing return statement in a non-void functon");
	}
}
type_t block_statement_t::analyse(Analysis_Context& ctx) const {
	ctx.push_scope();
	for(auto& statement : statements){
		statement->analyse(ctx);
	}
	ctx.pop_scope();
	return {};
}
type_t return_statement_t::analyse(Analysis_Context& ctx) const {
	// Get the return expression result
	ctx.has_return = true;
	type_t return_expression_type;
	if (return_expr)
		return_expression_type = return_expr->analyse(ctx);
	else
		return_expression_type = {type_t::BASE::VOID, 0};

	auto func_decl_info = ctx.get_func_decl_info(ctx.current_func_id);
	// I cannot find a reason why this could happen but we are gonna see as program include no global statements
	if(!func_decl_info) {
		ctx.make_error(ERROR_CODE::RETURN_DOES_NOT_MATCH, "return", "Return statement could not find valid function");
		return {};
	}
	if(func_decl_info->return_type->base == return_expression_type.base &&
		func_decl_info->return_type->pointer_depth == return_expression_type.pointer_depth){
			// No errors
			return {};
	}
	ctx.make_error(ERROR_CODE::RETURN_DOES_NOT_MATCH, "return", "Expected return type " + func_decl_info->return_type->str() + ", got " + return_expression_type.str());
	return {};
}

type_t skip_statement_t::analyse(Analysis_Context& ctx) const {
	if(!ctx.in_loop)
		ctx.make_error(ERROR_CODE::LOOP_KEYWORD_IS_OUTSIDE_OF_LOOP, "skip", "Skip statement can not be used outside of a loop");
	return {};
}
type_t break_statement_t::analyse(Analysis_Context& ctx) const {
	if(!ctx.in_loop)
		ctx.make_error(ERROR_CODE::LOOP_KEYWORD_IS_OUTSIDE_OF_LOOP, "break", "Break statement can not be used outside of a loop");
	return {};
}
type_t expr_statement_t::analyse(Analysis_Context& ctx) const {
	if(expr)
		expr->analyse(ctx);
	return {};
}
type_t func_call_expr_t::analyse(Analysis_Context& ctx) const {
	auto func_decl_info = ctx.get_func_decl_info(id);
	if(!func_decl_info){
		ctx.make_error(ERROR_CODE::CALL_TO_UNDEFINED_FUCNTION, id, "Undefined function call.");
		return {type_t::BASE::UNKNOWN, 0};
	}
	if (func_decl_info->arguments) {
		if (func_decl_info->arguments->size() != arguments.size()) {
			ctx.make_error(ERROR_CODE::CALL_TO_UNDEFINED_FUCNTION, id, "Function call arguments do not match to declaration");
			return {type_t::BASE::UNKNOWN, 0};
		}
	}
	for (size_t i = 0; i < arguments.size();i++) {
		type_t var_argument_type = arguments[i]->analyse(ctx);
		type_t decl_type = (*(func_decl_info->arguments))[i].type;
		if (var_argument_type.base != decl_type.base || var_argument_type.pointer_depth != decl_type.pointer_depth) {
			ctx.make_error(ERROR_CODE::CALL_TO_UNDEFINED_FUCNTION, id, "Function call arguments do not match to declaration.");
			return {type_t::BASE::UNKNOWN, 0};
		}
	}
	return *(func_decl_info->return_type);
}
type_t var_expression_t::analyse(Analysis_Context& ctx) const {
	if(ctx.get_var_type(name).base == type_t::BASE::UNKNOWN){
		ctx.make_error(ERROR_CODE::VAR_IS_NOT_DEFINED_IN_SCOPE, name, "Variable is not defined in this scope.");
	}
	return ctx.get_var_type(name);
}
type_t assignment_expression_t::analyse(Analysis_Context& ctx) const {
 	type_t left_type = lhs->analyse(ctx);
 	type_t right_type = rhs->analyse(ctx);

	if(left_type.base == right_type.base && left_type.pointer_depth == right_type.pointer_depth){
		return left_type;
	}
	ctx.make_error(ERROR_CODE::TYPES_DO_NOT_MATCH, "", "Assignment expression's left and right handside types do not match");
	return {type_t::BASE::UNKNOWN, 0};
}
type_t integer_literal_t::analyse(Analysis_Context& ctx) const {
	return {type_t::BASE::INT, 0};
}
type_t binary_expression_t::analyse(Analysis_Context& ctx) const {

	type_t l = lhs->analyse(ctx);
	type_t r = rhs->analyse(ctx);

	auto make_unknown = [](){ return type_t{type_t::BASE::UNKNOWN, 0}; };
	auto make_int = [](){ return type_t{type_t::BASE::INT, 0}; };

	// arithmetic
	if (op == BIN_OP::ADD || op == BIN_OP::SUB || op == BIN_OP::MUL || op == BIN_OP::DIV || op == BIN_OP::MOD) {
		// require same base (INT/UINT), and not pointers
		if (l.base == r.base && (l.base == type_t::BASE::INT || l.base == type_t::BASE::UINT) && l.pointer_depth == 0 && r.pointer_depth == 0) {
			return l;
		}
		ctx.make_error(ERROR_CODE::TYPES_DO_NOT_MATCH, "", "Arithmetic operands must be integer types and match");
		return make_unknown();
	}

	if (op == BIN_OP::GT || op == BIN_OP::LT || op == BIN_OP::GTE || op == BIN_OP::LTE ||
		op == BIN_OP::EQUALITY || op == BIN_OP::NOT_EQUAL) {
		// allow pointer equality/inequality when pointer_depth matches
		if (l.pointer_depth == r.pointer_depth && (l.base == r.base)) {
			return make_int();
		}
		ctx.make_error(ERROR_CODE::TYPES_DO_NOT_MATCH, "", "Comparison operands must be of the same type");
		return make_unknown();
	}

	if (op == BIN_OP::AND || op == BIN_OP::OR) {
		// require integer (boolean-ish) non-pointer
		if (l.base == type_t::BASE::INT && r.base == type_t::BASE::INT && l.pointer_depth == 0 && r.pointer_depth == 0) {
			return make_int();
		}
		ctx.make_error(ERROR_CODE::TYPES_DO_NOT_MATCH, "", "Logical operators require integer (non-pointer) operands");
		return make_unknown();
	}

	return make_unknown();
}
type_t unary_expression_t::analyse(Analysis_Context& ctx) const {
	type_t t = expr->analyse(ctx);
	auto make_unknown = [](){ return type_t{type_t::BASE::UNKNOWN, 0}; };

	switch (op) {
	case UNARY_OP::NEG:
		if ((t.base == type_t::BASE::INT || t.base == type_t::BASE::UINT) && t.pointer_depth == 0) {
			return t;
		}
		ctx.make_error(ERROR_CODE::TYPES_DO_NOT_MATCH, "", "Unary - requires an integer operand");
		return make_unknown();
	case UNARY_OP::NOT:
		if ((t.base == type_t::BASE::INT || t.base == type_t::BASE::UINT) && t.pointer_depth == 0) {
			return {type_t::BASE::INT, 0};
		}
		ctx.make_error(ERROR_CODE::TYPES_DO_NOT_MATCH, "", "Unary ! requires an integer operand");
		return make_unknown();
	case UNARY_OP::ADDR:
		// must be lvalue
		if (!expr->is_lvalue()) {
			ctx.make_error(ERROR_CODE::TYPES_DO_NOT_MATCH, "", "Address-of requires an lvalue");
			return make_unknown();
		}
		return {t.base, t.pointer_depth + 1};
	case UNARY_OP::DEREF:
		if (t.pointer_depth <= 0) {
			ctx.make_error(ERROR_CODE::TYPES_DO_NOT_MATCH, "", "Dereference requires a pointer type");
			return make_unknown();
		}
		return {t.base, t.pointer_depth - 1};
	case UNARY_OP::INCR:
	case UNARY_OP::DECR:
		if (!expr->is_lvalue() && !expr->is_deref()) {
			ctx.make_error(ERROR_CODE::TYPES_DO_NOT_MATCH, "", "Increment/Decrement requires an lvalue");
			return make_unknown();
		}
		if (!(t.base == type_t::BASE::INT || t.base == type_t::BASE::UINT) || t.pointer_depth != 0) {
			ctx.make_error(ERROR_CODE::TYPES_DO_NOT_MATCH, "", "Increment/Decrement requires a non-pointer integer");
			return make_unknown();
		}
		return t;
	default:
		return make_unknown();
	}
}
type_t var_decl_statement_t::analyse(Analysis_Context& ctx) const {
	// If there's an initializer, check its type
	if (rhs) {
		type_t rhs_t = rhs->analyse(ctx);
		if (rhs_t.base != type.base || rhs_t.pointer_depth != type.pointer_depth) {
			ctx.make_error(ERROR_CODE::TYPES_DO_NOT_MATCH, name, "Initializer type does not match variable declaration");
		}
	}
	// add the variable to the current scope
	ctx.add_var(name, type);
	return {};
}
type_t if_statement_t::analyse(Analysis_Context& ctx) const {
	// conditions must be integer (non-pointer)
	for (const auto& cond : condition) {
		type_t ct = cond->analyse(ctx);
		if (ct.base != type_t::BASE::INT || ct.pointer_depth != 0) {
			ctx.make_error(ERROR_CODE::TYPES_DO_NOT_MATCH, "if", "Condition must be an integer (non-pointer) expression");
		}
	}

	// then block
	if (body) {
		body->analyse(ctx);
	}

	// else block
	if (else_body) {
		else_body->analyse(ctx);
	}

	return {};
}
type_t while_statement_t::analyse(Analysis_Context& ctx) const {
	// conditions must be integer (non-pointer)
	for (const auto& cond : condition) {
		type_t ct = cond->analyse(ctx);
		if (ct.base != type_t::BASE::INT || ct.pointer_depth != 0) {
			ctx.make_error(ERROR_CODE::TYPES_DO_NOT_MATCH, "while", "Condition must be an integer (non-pointer) expression");
		}
	}

	bool prev_in_loop = ctx.in_loop;
	ctx.in_loop = true;
	if (body) {
		body->analyse(ctx);
	}
	ctx.in_loop = prev_in_loop;

	return {};
}
type_t for_statement_t::analyse(Analysis_Context& ctx) const {
	// condition (range) must be integer (non-pointer) if present
	if (condition) {
		type_t ct = condition->analyse(ctx);
		if (ct.base != type_t::BASE::INT || ct.pointer_depth != 0) {
			ctx.make_error(ERROR_CODE::TYPES_DO_NOT_MATCH, "for", "Range/condition must be an integer (non-pointer) expression");
		}
	}

	// step if present
	if (step) {
		step->analyse(ctx);
	}

	// body with loop flags
	bool prev_in_loop = ctx.in_loop;
	ctx.in_loop = true;
	if (body) {
		ctx.push_scope();
		body->analyse(ctx);
		ctx.pop_scope();
	}
	ctx.in_loop = prev_in_loop;

	return {};
}
 /*
  /$$$$$$ /$$$$$$$         /$$$$$$  /$$$$$$$$ /$$   /$$ /$$$$$$$$ /$$$$$$$   /$$$$$$  /$$$$$$$$ /$$$$$$  /$$$$$$  /$$   /$$
|_  $$_/| $$__  $$       /$$__  $$| $$_____/| $$$ | $$| $$_____/| $$__  $$ /$$__  $$|__  $$__/|_  $$_/ /$$__  $$| $$$ | $$
  | $$  | $$  \ $$      | $$  \__/| $$      | $$$$| $$| $$      | $$  \ $$| $$  \ $$   | $$     | $$  | $$  \ $$| $$$$| $$
  | $$  | $$$$$$$/      | $$ /$$$$| $$$$$   | $$ $$ $$| $$$$$   | $$$$$$$/| $$$$$$$$   | $$     | $$  | $$  | $$| $$ $$ $$
  | $$  | $$__  $$      | $$|_  $$| $$__/   | $$  $$$$| $$__/   | $$__  $$| $$__  $$   | $$     | $$  | $$  | $$| $$  $$$$
  | $$  | $$  \ $$      | $$  \ $$| $$      | $$\  $$$| $$      | $$  \ $$| $$  | $$   | $$     | $$  | $$  | $$| $$\  $$$
 /$$$$$$| $$  | $$      |  $$$$$$/| $$$$$$$$| $$ \  $$| $$$$$$$$| $$  | $$| $$  | $$   | $$    /$$$$$$|  $$$$$$/| $$ \  $$
|______/|__/  |__/       \______/ |________/|__/  \__/|________/|__/  |__/|__/  |__/   |__/   |______/ \______/ |__/  \__/
 */
namespace {
	void add_nop_on_label_clash(IR_Gen_Context& ctx) 
	{
		if (ctx.instructions.back().operation == ir_instruction_t::operation_::LABEL) {
			ir_instruction_t nop_instr;
			nop_instr.operation = ir_instruction_t::operation_::NOP;
			ctx.instructions.push_back(nop_instr);
		}
	}
	inline ir_instruction_t::operation_ convert_comparsion_to_branch_operation(BIN_OP comparison,bool inverse = false,expression_t* expr = nullptr){
		if(expr)
			comparison = dynamic_cast<binary_expression_t *>(expr)->op;
		switch (comparison)
		{
		case BIN_OP::EQUALITY:{
				if(inverse)
					return ir_instruction_t::operation_::BNEQ;
				return ir_instruction_t::operation_::BEQ;
		}
		case BIN_OP::NOT_EQUAL:{
				if(inverse)
					return ir_instruction_t::operation_::BEQ;
				return ir_instruction_t::operation_::BNEQ;
		}
		case BIN_OP::LT:{
				if(inverse)
					return ir_instruction_t::operation_::BGT;
				return ir_instruction_t::operation_::BLT;
		}
		case BIN_OP::LTE:{
				if(inverse)
					return ir_instruction_t::operation_::BGT;
				return ir_instruction_t::operation_::BLE;
		}
		case BIN_OP::GT:{
				if(inverse)
					return ir_instruction_t::operation_::BLE;
				return ir_instruction_t::operation_::BGT;
		}
		case BIN_OP::GTE:{
				if(inverse)
					return ir_instruction_t::operation_::BLE;
				return ir_instruction_t::operation_::BGE;
		}
		}
		return ir_instruction_t::operation_::BEQ;
	}
	inline std::string mangle_var_if_needed(IR_Gen_Context& ctx, const std::string& var_id){

		for (const char c : var_id) {
			if (c == '$')
				return var_id;
		}
		if(var_id[0] == 't')
			return var_id;
		return  "$" + var_id + std::to_string(ctx.symbol_mangles[var_id]);
	}
	inline std::string mangle(const std::string& str) {
		return "." + str;
	}
	void generate_jump_if_false(expression_t* expr, IR_Gen_Context& ctx, const std::string& false_label);
	void generate_jump_if_true(expression_t* expr, IR_Gen_Context& ctx, const std::string& true_label);

	void generate_jump_if_false(expression_t* expr, IR_Gen_Context& ctx, const std::string& false_label) {
		if (auto be = dynamic_cast<binary_expression_t*>(expr)) {
			switch (be->op) {
				case BIN_OP::AND:
					generate_jump_if_false(be->lhs.get(), ctx, false_label);
					generate_jump_if_false(be->rhs.get(), ctx, false_label);
					return;
				case BIN_OP::OR: {
					// cont stands for continue
					std::string cont = ctx.generate_label();
					generate_jump_if_true(be->lhs.get(), ctx, cont);
					generate_jump_if_false(be->rhs.get(), ctx, false_label);
					// emit cont label
					add_nop_on_label_clash(ctx);
					ir_instruction_t lbl;
					lbl.operation = ir_instruction_t::operation_::LABEL;
					lbl.label_id = cont;
					ctx.instructions.push_back(std::move(lbl));
					return;
				}
				// comparisons
				case BIN_OP::GT:
				case BIN_OP::LT:
				case BIN_OP::GTE:
				case BIN_OP::LTE:
				case BIN_OP::EQUALITY:
				case BIN_OP::NOT_EQUAL: {
					std::string l = be->lhs->generate_ir(ctx);
					std::string r = be->rhs->generate_ir(ctx);
					ir_instruction_t branch;
					branch.operation = convert_comparsion_to_branch_operation(be->op, true);
					branch.src1 = mangle_var_if_needed(ctx, l);
					branch.src2 = mangle_var_if_needed(ctx, r);
					branch.label_id = false_label;
					ctx.instructions.push_back(std::move(branch));
					return;
				}
				default:
					break;
			}
		}
		if (expr) {
			std::string val = expr->generate_ir(ctx);
			ir_instruction_t branch;
			branch.operation = ir_instruction_t::operation_::BEQ;
			branch.src1 = mangle_var_if_needed(ctx, val);
			branch.src2 = "0";
			branch.label_id = false_label;
			ctx.instructions.push_back(std::move(branch));
		}

	}

	void generate_jump_if_true(expression_t* expr, IR_Gen_Context& ctx, const std::string& true_label) {
		if (auto be = dynamic_cast<binary_expression_t*>(expr)) {
			switch (be->op) {
				case BIN_OP::OR:
					// a || b -> if a true goto true; if b true goto true
					generate_jump_if_true(be->lhs.get(), ctx, true_label);
					generate_jump_if_true(be->rhs.get(), ctx, true_label);
					return;
				case BIN_OP::AND: {
					std::string cont = ctx.generate_label();
					generate_jump_if_false(be->lhs.get(), ctx, cont);
					generate_jump_if_true(be->rhs.get(), ctx, true_label);
					add_nop_on_label_clash(ctx);
					ir_instruction_t lbl;
					lbl.operation = ir_instruction_t::operation_::LABEL;
					lbl.label_id = cont;
					ctx.instructions.push_back(std::move(lbl));
					return;
				}
				case BIN_OP::GT:
				case BIN_OP::LT:
				case BIN_OP::GTE:
				case BIN_OP::LTE:
				case BIN_OP::EQUALITY:
				case BIN_OP::NOT_EQUAL: {
					std::string l = be->lhs->generate_ir(ctx);
					std::string r = be->rhs->generate_ir(ctx);
					ir_instruction_t branch;
					branch.operation = convert_comparsion_to_branch_operation(be->op, false); 
					branch.src1 = mangle_var_if_needed(ctx, l);
					branch.src2 = mangle_var_if_needed(ctx, r);
					branch.label_id = true_label;
					ctx.instructions.push_back(std::move(branch));
					return;
				}
				default:
					break;
			}
		}

		if (expr) {
			std::string val = expr->generate_ir(ctx);
			ir_instruction_t branch;
			branch.operation = ir_instruction_t::operation_::BNEQ;
			branch.src1 = mangle_var_if_needed(ctx, val);
			branch.src2 = "0";
			branch.label_id = true_label;
			ctx.instructions.push_back(std::move(branch));
		}
	}
}
std::string func_decl_t::generate_ir(IR_Gen_Context& ctx) const {
	ir_instruction_t instr;
	// fucntion entry label
	instr.operation = ir_instruction_t::operation_::FUNC_ENTRY;
	instr.label_id = mangle(id);
	ctx.instructions.push_back(std::move(instr));
	for (auto& argument : arguments) {
		ir_instruction_t instr;
		instr.operation = ir_instruction_t::operation_::PARAM;
		instr.dest = mangle_var_if_needed(ctx,argument.name);
		ctx.instructions.push_back(std::move(instr));
	}
	body->generate_ir(ctx);
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
		instr.dest = mangle_var_if_needed(ctx,name);
		instr.src1 = mangle_var_if_needed(ctx,source);
		instr.store_dest_in_stack = true;
		ctx.instructions.push_back(std::move(instr));
	}
	return "";
}
std::string if_statement_t::generate_ir(IR_Gen_Context& ctx) const {
	ctx.flag = is_if_statement;
	std::string label_1 = ctx.generate_label();
	std::string label_2 = ctx.generate_label();
	ir_instruction_t branch_instr;

	generate_jump_if_false(condition.back().get(), ctx, label_1);
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
	generate_jump_if_true(condition.back().get(), ctx, body_label);

	add_nop_on_label_clash(ctx);
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
	if(step)
		step->generate_ir(ctx);

	ir_instruction_t label_instr;
	label_instr.operation = ir_instruction_t::operation_::LABEL;

	generate_jump_if_true(condition.get(), ctx, body_label);
	if (!ctx.break_jump_labels.empty())
	{
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

	ir_instruction_t return_instruction;
	if(!return_expr){
		return_instruction.operation = ir_instruction_t::operation_::RETURN;
		ctx.instructions.push_back(std::move(return_instruction));
		return "";
	}
	std::string temp = return_expr->generate_ir(ctx);
	return_instruction.operation = ir_instruction_t::operation_::RETURN;
	return_instruction.src1 = mangle_var_if_needed(ctx,temp);
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
	instr.dest = mangle_var_if_needed(ctx, left);
	if (!lhs->is_deref()) {
		switch (type)
		{
		default:
			instr.operation = ir_instruction_t::operation_::MOV;
			instr.src1 = mangle_var_if_needed(ctx,right);
			break;
		case ASSIGNMENT_TYPE::ADD_ASSIGN:
			instr.operation = ir_instruction_t::operation_::ADD;
			instr.src1 = instr.dest;
			instr.src2 = mangle_var_if_needed(ctx,right);
			break;
		case ASSIGNMENT_TYPE::SUBTRACT_ASSIGN:
			instr.operation = ir_instruction_t::operation_::SUB;
			instr.src1 = instr.dest;
			instr.src2 = mangle_var_if_needed(ctx,right);
			break;
		case ASSIGNMENT_TYPE::MULTIPLY_ASSIGN:
			instr.operation = ir_instruction_t::operation_::MUL;
			instr.src1 = instr.dest;
			instr.src2 = mangle_var_if_needed(ctx,right);
			break;
		case ASSIGNMENT_TYPE::DIVIDE_ASSIGN:
			instr.operation = ir_instruction_t::operation_::DIV;
			instr.src1 = instr.dest;
			instr.src2 = mangle_var_if_needed(ctx,right);
			break;
		case ASSIGNMENT_TYPE::MOD_ASSIGN:
			instr.operation = ir_instruction_t::operation_::REM;
			instr.src1 = instr.dest;
			instr.src2 = mangle_var_if_needed(ctx,right);
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
			instr.src1 = mangle_var_if_needed(ctx,right);
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
	instr.src1 = mangle_var_if_needed(ctx,left);
	instr.src2 = mangle_var_if_needed(ctx,right);
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
	// @Uncomplete : I actually forgot these exist
	//case BIN_OP::AND:
	//	instr.operation = ir_instruction_t::operation_::AND;
	//	break;
	//case BIN_OP::OR:
	//	instr.operation = ir_instruction_t::operation_::OR;
	//	break;
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
	auto prev = ctx.left_is_deref;
	if(op == UNARY_OP::INCR || op == UNARY_OP::DECR){
		ctx.left_is_deref = true;
	}
	std::string left = expr->generate_ir(ctx);
	ctx.left_is_deref = prev;
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
				return mangle_var_if_needed(ctx,left);
			std::string addr_temp = ctx.generate_temp();
			ir_instruction_t load_ptr;
			load_ptr.operation = ir_instruction_t::operation_::LOAD;
			load_ptr.dest = addr_temp;
			load_ptr.src1 = mangle_var_if_needed(ctx, left);
			load_ptr.load_src_is_ptr = true;
			ctx.instructions.push_back(std::move(load_ptr));
			return addr_temp;
		}
		if (ptr_depth == 1) {
			std::string addr_temp = ctx.generate_temp();
			ir_instruction_t load_ptr;
			load_ptr.operation = ir_instruction_t::operation_::LOAD;
			load_ptr.dest = addr_temp;
			load_ptr.src1 = mangle_var_if_needed(ctx, left);
			load_ptr.load_src_is_ptr = true;
			ctx.instructions.push_back(std::move(load_ptr));
			return addr_temp;
		}
		std::string addr_temp = ctx.generate_temp();
		ir_instruction_t load_ptr;
		load_ptr.operation = ir_instruction_t::operation_::LOAD;
		load_ptr.dest = left;
		load_ptr.src1 = mangle_var_if_needed(ctx, left);
		load_ptr.load_src_is_ptr = true;
		ctx.instructions.push_back(std::move(load_ptr));
		return left;
	}
	case UNARY_OP::INCR:
		if (!expr->is_deref()) {
			instr.operation = ir_instruction_t::operation_::ADD;
			// if the non temporary symbol is current function's
			instr.src1 = instr.dest = mangle_var_if_needed(ctx,left);
			instr.src2 = std::to_string(1);
			if(instr.dest[0] != 't')
				instr.store_dest_in_stack = true;
			ctx.instructions.push_back(std::move(instr));
			return left;
		}
		instr_2.operation = ir_instruction_t::operation_::LOAD;
		instr_2.dest = temp;
		instr_2.src1 = mangle_var_if_needed(ctx,left);
		instr_2.load_src_is_ptr = true;
		ctx.instructions.push_back(std::move(instr_2));

		instr_3.operation = ir_instruction_t::operation_::ADD;
		instr_3.dest = instr_3.src1 = temp;
		instr_3.src2 = std::to_string(1);
		ctx.instructions.push_back(std::move(instr_3));
		instr.operation = ir_instruction_t::operation_::STORE;
		instr.src1 = temp;
		instr.dest = mangle_var_if_needed(ctx,left);
		ctx.instructions.push_back(std::move(instr));
		ctx.left_is_deref = false;
		return temp;
	case UNARY_OP::DECR:
		if (!expr->is_deref()) {
			instr.operation = ir_instruction_t::operation_::SUB;
			instr.src1 = instr.dest = mangle_var_if_needed(ctx,left);
			instr.src2 = std::to_string(1);
			if(instr.dest[0] != 't')
				instr.store_dest_in_stack = true;
			ctx.instructions.push_back(std::move(instr));
			return left;
		}
		instr_2.operation = ir_instruction_t::operation_::LOAD;
		instr_2.dest = temp;
		instr_2.src1 = mangle_var_if_needed(ctx,left);
		instr_2.load_src_is_ptr = true;
		ctx.instructions.push_back(std::move(instr_2));

		instr_3.operation = ir_instruction_t::operation_::SUB;
		instr_3.dest = instr_3.src1 = temp;
		instr_3.src2 = std::to_string(1);
		ctx.instructions.push_back(std::move(instr_3));
		instr.operation = ir_instruction_t::operation_::STORE;
		instr.src1 = temp;
		instr.dest = mangle_var_if_needed(ctx,left);
		ctx.instructions.push_back(std::move(instr));
		ctx.left_is_deref = false;
		return temp;
	default:
		break;
	}
	destination = ctx.generate_temp();
	instr.dest = destination;
	instr.src1 = mangle_var_if_needed(ctx, left);
	ctx.instructions.push_back(std::move(instr));
	return destination;
}
std::string func_call_expr_t::generate_ir(IR_Gen_Context& ctx) const {
	for(auto& argument : arguments){
		ir_instruction_t instr;
		instr.operation = ir_instruction_t::operation_::ARG;
		instr.src1 = mangle_var_if_needed(ctx,argument->generate_ir(ctx));
		ctx.instructions.push_back(std::move(instr));
	}
	ir_instruction_t call_instr;
	call_instr.operation = ir_instruction_t::operation_::CALL;
	std::string destination = ctx.generate_temp();
	call_instr.dest = destination;
	call_instr.label_id = mangle(id);
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



/*
  /$$$$$$   /$$$$$$  /$$$$$$$$       /$$$$$$$  /$$$$$$$  /$$$$$$ /$$   /$$ /$$$$$$$$ /$$$$$$ /$$   /$$  /$$$$$$ 
 /$$__  $$ /$$__  $$|__  $$__/      | $$__  $$| $$__  $$|_  $$_/| $$$ | $$|__  $$__/|_  $$_/| $$$ | $$ /$$__  $$
| $$  \ $$| $$  \__/   | $$         | $$  \ $$| $$  \ $$  | $$  | $$$$| $$   | $$     | $$  | $$$$| $$| $$  \__/
| $$$$$$$$|  $$$$$$    | $$         | $$$$$$$/| $$$$$$$/  | $$  | $$ $$ $$   | $$     | $$  | $$ $$ $$| $$ /$$$$
| $$__  $$ \____  $$   | $$         | $$____/ | $$__  $$  | $$  | $$  $$$$   | $$     | $$  | $$  $$$$| $$|_  $$
| $$  | $$ /$$  \ $$   | $$         | $$      | $$  \ $$  | $$  | $$\  $$$   | $$     | $$  | $$\  $$$| $$  \ $$
| $$  | $$|  $$$$$$/   | $$         | $$      | $$  | $$ /$$$$$$| $$ \  $$   | $$    /$$$$$$| $$ \  $$|  $$$$$$/
|__/  |__/ \______/    |__/         |__/      |__/  |__/|______/|__/  \__/   |__/   |______/|__/  \__/ \______/ 
                                                                                                                
*/
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
	os << COLOR_LABEL << "unary_op(ptr_depth = " << ptr_depth << "): " << COLOR_OP << unary_op_to_string(op) << COLOR_RESET << "\n";
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

void f3_compiler::func_decl_param_t::print_ast(std::ostream& os, uint32_t indent_level /*= 0*/, bool is_last /*= true*/) const
{
	draw_branch(os, indent_level, is_last);
	os << COLOR_LABEL << "param : " << COLOR_RESET
		<< COLOR_TYPE << type.str() << COLOR_RESET
		<< " " << COLOR_VAR << name << COLOR_RESET << "\n";
}
