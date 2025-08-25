#include "nodes.hpp"

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
	case f3_compiler::ast_node::BIN_OP::ADD: return "+";
	case f3_compiler::ast_node::BIN_OP::SUB: return "-";
	case f3_compiler::ast_node::BIN_OP::DIV: return "/";
	case f3_compiler::ast_node::BIN_OP::MUL: return "*";
	case f3_compiler::ast_node::BIN_OP::MOD: return "%";
	case f3_compiler::ast_node::BIN_OP::GT: return ">";
	case f3_compiler::ast_node::BIN_OP::LT: return "<";
	case f3_compiler::ast_node::BIN_OP::GTE: return ">=";
	case f3_compiler::ast_node::BIN_OP::LTE: return "<=";
	case f3_compiler::ast_node::BIN_OP::EQUALITY: return "==";
	case f3_compiler::ast_node::BIN_OP::NOT_EQUAL: return "!=";
	case f3_compiler::ast_node::BIN_OP::AND: return "&&";
	case f3_compiler::ast_node::BIN_OP::OR: return "||";
	}
	return "????";
}

static inline std::string unary_op_to_string(f3_compiler::ast_node::UNARY_OP op) {
	switch (op)
	{
	case f3_compiler::ast_node::UNARY_OP::NEG:
		return "-";
	case f3_compiler::ast_node::UNARY_OP::INCR:
		return "++";
	case f3_compiler::ast_node::UNARY_OP::DECR:
		return "--";
	case f3_compiler::ast_node::UNARY_OP::NOT:
		return "!";
	case f3_compiler::ast_node::UNARY_OP::ADDR:
		return "&";
	case f3_compiler::ast_node::UNARY_OP::DEREF:
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
void f3_compiler::ast_node::func_decl_param_t::print_ast(std::ostream& os, uint32_t indent_level /*= 0*/, bool is_last /*= true*/) const
{
	draw_branch(os, indent_level, is_last);
	os << COLOR_LABEL << "param : " << COLOR_RESET
		<< COLOR_TYPE << type.str() << COLOR_RESET
		<< " " << COLOR_VAR << name << COLOR_RESET << "\n";
}

void f3_compiler::ast_node::var_expression_t::print_ast(std::ostream& os, uint32_t indent_level, bool is_last) const
{
	draw_branch(os, indent_level, is_last);
	os << COLOR_LABEL << "variable : " << COLOR_VAR << name << COLOR_RESET << "\n";
}


void f3_compiler::ast_node::integer_literal_t::print_ast(std::ostream& os, uint32_t indent_level, bool is_last) const
{
	draw_branch(os, indent_level, is_last);
	os << COLOR_LABEL << "integer : " << COLOR_LIT << value << COLOR_RESET << "\n";
}

void f3_compiler::ast_node::func_call_expr_t::print_ast(std::ostream& os, uint32_t indent_level /*= 0*/, bool is_last /*= true*/) const
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

void f3_compiler::ast_node::binary_expression_t::print_ast(std::ostream& os, uint32_t indent_level, bool is_last) const
{
	draw_branch(os, indent_level, is_last);
	os << COLOR_LABEL << "binary_op : " << COLOR_OP << bin_op_to_string(op) << COLOR_RESET << "\n";
	if (lhs) lhs->print_ast(os, indent_level + 1, false);
	if (rhs) rhs->print_ast(os, indent_level + 1, true);
}

void f3_compiler::ast_node::unary_expression_t::print_ast(std::ostream& os, uint32_t indent_level, bool is_last) const
{
	draw_branch(os, indent_level, is_last);
	os << COLOR_LABEL << "unary_op : " << COLOR_OP << unary_op_to_string(op) << COLOR_RESET << "\n";
	if (expr) expr->print_ast(os, indent_level + 1, false);
}

void f3_compiler::ast_node::for_range_expression_t::print_ast(std::ostream& os, uint32_t indent_level /* = 0 */, bool is_last /* = true */) const
{
	draw_branch(os, indent_level, is_last);
	os << COLOR_LABEL << "for_range_expression" << COLOR_RESET;

	// Exclusive / inclusive info
	os << " (" << (is_exclusive ? "exclusive" : "inclusive") << ")";
	os << "\n";

	// Start expression
	draw_branch(os, indent_level + 1, false);
	os << COLOR_LABEL << "start" << COLOR_RESET << "\n";
	if (start)
		start->print_ast(os, indent_level + 2, true);

	// Destination expression
	draw_branch(os, indent_level + 1, true);
	os << COLOR_LABEL << "destination" << COLOR_RESET << "\n";
	if (destination)
		destination->print_ast(os, indent_level + 2, true);
}

void f3_compiler::ast_node::assignment_expression_t::print_ast(std::ostream& os, uint32_t indent_level, bool is_last) const
{
	draw_branch(os, indent_level, is_last);
	switch (type) {

	case ASSIGNMENT_TYPE::P:
		os << COLOR_LABEL << "p_assignment(+=)" << COLOR_RESET << "\n";
		break;
	case ASSIGNMENT_TYPE::M:
		os << COLOR_LABEL << "m_assignment(-=)" << COLOR_RESET << "\n";
		break;
	default:
		os << COLOR_LABEL << "assignment(=)" << COLOR_RESET << "\n";
	}
	if (lhs) lhs->print_ast(os, indent_level + 1, false);
	if (rhs) rhs->print_ast(os, indent_level + 1, true);
}

void f3_compiler::ast_node::var_decl_statement_t::print_ast(std::ostream& os, uint32_t indent_level, bool is_last) const
{
	draw_branch(os, indent_level, is_last);
	os << COLOR_LABEL << "variable_decl : " << COLOR_RESET
		<< COLOR_TYPE << type.str() << COLOR_RESET
		<< " " << COLOR_VAR << name << COLOR_RESET << "\n";
	if (rhs) {
		rhs->print_ast(os, indent_level + 1, true);
	}
}

void f3_compiler::ast_node::block_statement_t::print_ast(std::ostream& os, uint32_t indent_level, bool is_last /*= true*/) const
{
	for (size_t i = 0; i < statements.size(); i++) {
		// For each child statement, indent one level deeper
		// Pass 'true' for is_last only if this is the last child
		statements[i]->print_ast(os, indent_level + 1, i == statements.size() - 1);
	}
}

void f3_compiler::ast_node::if_statement_t::print_ast(std::ostream& os, uint32_t indent_level /*= 0*/, bool is_last /*= true*/) const
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

void f3_compiler::ast_node::for_statement_t::print_ast(std::ostream& os, uint32_t indent_level /*= 0*/, bool is_last /*= true*/) const
{
	draw_branch(os, indent_level, is_last);
	os << COLOR_LABEL << "for_statement" << COLOR_RESET << "\n";

	// Range expression
	draw_branch(os, indent_level + 1, false);
	os << COLOR_LABEL << "range" << COLOR_RESET << "\n";
	if (range)
		range->print_ast(os, indent_level + 2, true);

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

void f3_compiler::ast_node::break_statement_t::print_ast(std::ostream& os, uint32_t indent_level /*= 0*/, bool is_last /*= true*/) const
{
	draw_branch(os, indent_level, is_last);
	os << COLOR_LABEL << "break_statement" << COLOR_RESET << "\n";
}

void f3_compiler::ast_node::skip_statement_t::print_ast(std::ostream& os, uint32_t indent_level /*= 0*/, bool is_last /*= true*/) const
{
	draw_branch(os, indent_level, is_last);
	os << COLOR_LABEL << "skip_statement" << COLOR_RESET << "\n";
}

void f3_compiler::ast_node::expr_statement_t::print_ast(std::ostream& os, uint32_t indent_level /*= 0*/, bool is_last /*= true*/) const
{
	draw_branch(os, indent_level, is_last);
	os << COLOR_LABEL << "expr_statement" << COLOR_RESET << "\n";
	if (expr) {
		expr->print_ast(os, indent_level + 1, true);
	}
}

void f3_compiler::ast_node::func_decl_t::print_ast(std::ostream& os, uint32_t indent_level /*= 0*/, bool is_last /*= true*/) const
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
void f3_compiler::ast_node::while_statement_t::print_ast(std::ostream& os, uint32_t indent_level /*= 0*/, bool is_last /*= true*/) const
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

void f3_compiler::ast_node::return_statement_t::print_ast(std::ostream& os, uint32_t indent_level /*= 0*/, bool is_last /*= true*/) const
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
