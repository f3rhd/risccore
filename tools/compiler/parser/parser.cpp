#include "parser.hpp"
using namespace f3_compiler;
using namespace ast_node;
std::unique_ptr<func_decl_t> Parser::parse_function() {

	std::string func_name;
	std::vector<func_decl_param_t> func_arguments;
	std::unique_ptr<block_statement_t> func_body;
	type_t func_return_type;

	expect(TOKEN_TYPE::KW_FUNC, "Missing keyword 'func'");

	advance();

	func_name = parse_identifier(); //@Costyyyyy

	expect(TOKEN_TYPE::LPAREN, "Missing '('. Expected '(' after function identifier");

	// Parse arguments
	func_arguments = parse_func_decl_params();

	// Parse return type
	expect(TOKEN_TYPE::ARROW, "Missing '->'. Function return type must be pointed.");
	// We are -> and we need to advance to the return type
	advance();
	func_return_type = parse_type();

	func_body  = parse_func_body();
	return std::make_unique<func_decl_t>(std::move(func_name), std::move(func_arguments), func_return_type, std::move(func_body));
}

void Parser::make_error(const token_t& cause,const std::string& message){
	_errors.emplace_back(ERROR_CODE::SYNTAX_ERROR, cause,message);
}
void Parser::advance() {
	if (_curr_index >= _tokens.size() - 1)
		return;
	_curr_index++;
	_current_token = &_tokens[_curr_index];
}

const token_t& Parser::peek_next() {
	if (_curr_index == _tokens.size() - 1)
		return _tokens.back();
	return _tokens[_curr_index + 1];
}
std::vector<func_decl_param_t> Parser::parse_func_decl_params() {
	// This function is getting invoked when the curr_token is ( and there is function declaration
	advance();
	std::vector<func_decl_param_t> func_parameters;

	// means there are no arguments
	if (_current_token->type != TOKEN_TYPE::END_OF_FILE && _current_token->type == TOKEN_TYPE::RPAREN) {
		advance();
		return func_parameters;
	}
	while (true) {
		type_t arg_type = parse_type();
		std::string arg_id = parse_identifier();
		func_parameters.emplace_back(arg_type, std::move(arg_id));
		if (_current_token->type == TOKEN_TYPE::END_OF_FILE) {
			make_error(*_current_token, "Missing ')' . Function parameters must be closed by ')'");
			break;
		}
		if (_current_token->type != TOKEN_TYPE::RPAREN) {
			expect(TOKEN_TYPE::COMMA, "Missing ',' . Function argument must be seperated with ','");
		}
		else {
			break;
		}
		advance();
	}
	advance();
	return func_parameters;
}
std::string Parser::parse_identifier() {
	expect(TOKEN_TYPE::IDENTIFIER, "Expected identifier.");
	std::string temp = _current_token->value;
	advance();
	return temp;
}
type_t Parser::parse_type()
{
	type_t::BASE type_base = type_t::BASE::UNKNOWN;
	int32_t type_pointer_depth = 0;
	switch (_current_token->type) {
	case TOKEN_TYPE::KW_INT: type_base = type_t::BASE::INT; break;
	case TOKEN_TYPE::KW_UINT: type_base = type_t::BASE::UINT; break;
	case TOKEN_TYPE::KW_VOID: type_base = type_t::BASE::VOID; break;
	default:
		make_error(*_current_token, { "Unknown type." });
	}
	advance();
	while (_current_token->type != TOKEN_TYPE::END_OF_FILE && _current_token->type == TOKEN_TYPE::STAR) {
		type_pointer_depth++;
		advance();
	}
	return { .base = type_base,.pointer_depth = type_pointer_depth };
}
void Parser::expect(TOKEN_TYPE type, const std::string& error_msg)
{
	if (is_current_token(type))
		return;
	make_error(*_current_token, error_msg);
}
bool Parser::is_current_token(TOKEN_TYPE expected_type)
{
	if (_current_token->type == expected_type)
		return true;
	return false;
}

std::unique_ptr<block_statement_t> Parser::parse_func_body()
{
	return parse_block_statement();
}
std::unique_ptr<block_statement_t> Parser::parse_block_statement() {

	// We are left curly brace at this point 
	std::vector<std::unique_ptr<statement_t>> statements;
	// skip the left curly 
	advance();
	while(!is_current_token(TOKEN_TYPE::END_OF_FILE) && !is_current_token(TOKEN_TYPE::RCURLY)) {

		statements.push_back(std::move(parse_statement()));
	}
	expect(TOKEN_TYPE::RCURLY, "Missing '}'");
	advance();
	return std::make_unique<block_statement_t>(std::move(statements));
}

// c = call() + 2*3 + !(4 > 5 && 5<3 || 23)==2;
// c = 4>3;
std::unique_ptr<expression_t> Parser::parse_expr() {

	return parse_for_range_expr();
}
std::unique_ptr<expression_t> Parser::parse_for_range_expr() {
	auto e = parse_assignment_expr();
	// (i=0...=10)
	if (is_current_token(TOKEN_TYPE::TRIPLE_DOT)) {
		bool is_exclusive = false;
		// skip the tripple dot
		advance();
		if (!is_current_token(TOKEN_TYPE::ASSIGNMENT)) {
			is_exclusive = true;
		}
		else {
			// eat the =
			advance();
		}
		auto right = parse_assignment_expr();
		return std::make_unique<for_range_expression_t>(std::move(e),std::move(right),is_exclusive);
	}
	return e;
}
std::unique_ptr<expression_t> Parser::parse_assignment_expr() {
	auto left = parse_equality_expr();
	if (is_current_token(TOKEN_TYPE::ASSIGNMENT) ||
		is_current_token(TOKEN_TYPE::P_ASSIGNMENT) ||
		is_current_token(TOKEN_TYPE::M_ASSIGNMENT)
		) {
		ASSIGNMENT_TYPE type = ASSIGNMENT_TYPE::UNKNOWN;
		if (!left->is_lvalue()) {
			make_error(peek_before(), "Left side of an assignment should be lvalue.");
		}
		switch (_current_token->type) {
		case TOKEN_TYPE::ASSIGNMENT:  type = ASSIGNMENT_TYPE::NORMAL; break;
		case TOKEN_TYPE::P_ASSIGNMENT: type = ASSIGNMENT_TYPE::P; break;
		case TOKEN_TYPE::M_ASSIGNMENT: type = ASSIGNMENT_TYPE::M; break;
		}
		advance();
		auto right = parse_equality_expr();
		return std::make_unique<assignment_expression_t>(type,std::move(left), std::move(right));
	}
	return left;
}

// a == b == c
std::unique_ptr<expression_t> Parser::parse_equality_expr() {
	auto e = parse_logical_or_expr();
	while (is_current_token(TOKEN_TYPE::EQUAL) || is_current_token(TOKEN_TYPE::NOT_EQUAL)) {
		BIN_OP op = is_current_token(TOKEN_TYPE::EQUAL) ? BIN_OP::EQUALITY : BIN_OP::NOT_EQUAL;
		advance();
		auto right = parse_logical_or_expr();
		e = std::make_unique<binary_expression_t>(op, std::move(e), std::move(right));
	}
	return e;
}

std::unique_ptr<expression_t> Parser::parse_logical_or_expr() {
	auto e = parse_logical_and_expr();
	while (is_current_token(TOKEN_TYPE::DOUBLE_COLUMN) || is_current_token(TOKEN_TYPE::KW_OR)) {
		BIN_OP op = BIN_OP::OR;
		advance();
		auto right = parse_logical_and_expr();
		e = std::make_unique<binary_expression_t>(op, std::move(e), std::move(right));
	}
	return e;
}

std::unique_ptr<expression_t> Parser::parse_logical_and_expr() {
	auto e = parse_relational_expr();
	if (is_current_token(TOKEN_TYPE::DOUBLE_AMPERSAND) || is_current_token(TOKEN_TYPE::KW_AND)) {
		BIN_OP op = BIN_OP::AND;
		advance();
		auto right = parse_relational_expr();
		e = std::make_unique<binary_expression_t>(op, std::move(e), std::move(right));
	}
	return e;
}
// 4 < x < 5
std::unique_ptr<expression_t> Parser::parse_relational_expr() {
	auto e = parse_addition_subtraction();
	while (
		is_current_token(TOKEN_TYPE::GREATER) ||
		is_current_token(TOKEN_TYPE::GRATER_EQUAL) ||
		is_current_token(TOKEN_TYPE::LESS) ||
		is_current_token(TOKEN_TYPE::LESS_EQUAL)
	) {

		BIN_OP op;
		switch (_current_token->type) {
		
		case TOKEN_TYPE::GREATER: op = BIN_OP::GT; break;
		case TOKEN_TYPE::GRATER_EQUAL: op = BIN_OP::GTE; break;
		case TOKEN_TYPE::LESS: op = BIN_OP::LT; break;
		case TOKEN_TYPE::LESS_EQUAL: op = BIN_OP::LTE; break;
		}
		advance();
		auto right = parse_addition_subtraction();
		e = std::make_unique<binary_expression_t>(op, std::move(e), std::move(right));
	}
	return e;
}
// x && y || =  3 || 4
// !x*2
// !x * 2+3 && 4
std::unique_ptr<expression_t> Parser::parse_addition_subtraction() {

	auto e = parse_multiplication_division();
	while (is_current_token(TOKEN_TYPE::MINUS) || is_current_token(TOKEN_TYPE::PLUS)) {
		BIN_OP op = is_current_token(TOKEN_TYPE::MINUS) ? BIN_OP::SUB : BIN_OP::ADD;
		advance();
		auto right = parse_multiplication_division();
		e = std::make_unique<binary_expression_t>(op, std::move(e), std::move(right));
	}
	return e;
}
std::unique_ptr<expression_t> Parser::parse_multiplication_division() {
	auto e = parse_mod();
	while (is_current_token(TOKEN_TYPE::STAR) || is_current_token(TOKEN_TYPE::SLASH)) {

		BIN_OP op = is_current_token(TOKEN_TYPE::STAR) ? BIN_OP::MUL : BIN_OP::DIV;
		advance();
		auto right = parse_mod();
		e = std::make_unique<binary_expression_t>(op, std::move(e), std::move(right));
	}
	return e;
}
std::unique_ptr<expression_t> Parser::parse_mod() {
	auto  e = parse_unary_op();
	while (is_current_token(TOKEN_TYPE::PERCENTAGE)) {

		BIN_OP op = BIN_OP::MOD;
		advance();
		auto right = parse_unary_op();
		e = std::make_unique<binary_expression_t>(op, std::move(e), std::move(right));
	}
	return e;
}
std::unique_ptr<expression_t> Parser::parse_unary_op() {
	// -4 + 3 - 2
	// !(4+3*2)
	while (
		is_current_token(TOKEN_TYPE::INCR) ||
		is_current_token(TOKEN_TYPE::MINUS) ||
		is_current_token(TOKEN_TYPE::DECR) ||
		is_current_token(TOKEN_TYPE::EXCLAMATION) ||
		is_current_token(TOKEN_TYPE::AMPERSAND) || 
		is_current_token(TOKEN_TYPE::STAR)
	) {

		UNARY_OP op;
		switch (_current_token->type) {

		//@Incomplete: There are other operations left
		case TOKEN_TYPE::INCR: op = UNARY_OP::INCR; break;
		case TOKEN_TYPE::MINUS: op = UNARY_OP::NEG; break;
		case TOKEN_TYPE::DECR: op = UNARY_OP::DECR; break;
		case TOKEN_TYPE::EXCLAMATION: op = UNARY_OP::NOT; break;
		case TOKEN_TYPE::AMPERSAND: op = UNARY_OP::ADDR; break;
		case TOKEN_TYPE::STAR: op = UNARY_OP::DEREF;
		}
		advance();
		auto operand = parse_unary_op();
		return std::make_unique<unary_expression_t>(op, std::move(operand));
	}
	return parse_primary_expr();
}
std::vector<std::unique_ptr<expression_t>> Parser::parse_function_call_params() { // This function is also used in parsing of if statement's condition

	// This function is getting invoked when the curr_token is ( and there is function declaration
	advance();
	std::vector<std::unique_ptr<expression_t>> func_parameters;

	// means there are no arguments
	if (_current_token->type != TOKEN_TYPE::END_OF_FILE && _current_token->type == TOKEN_TYPE::RPAREN) {
		advance();
		return func_parameters;
	}
	while (true) {
		func_parameters.push_back(std::move(parse_expr()));
		if (_current_token->type == TOKEN_TYPE::END_OF_FILE) {
			make_error(*_current_token, "Function parameters must be closed by ')'.");
			break;
		}
		if (_current_token->type != TOKEN_TYPE::RPAREN) {
			expect(TOKEN_TYPE::COMMA, "Function parameters must be seperated by ','.");
		}
		else {
			break;
		}
		advance();
	}
	advance();
	return func_parameters;
}
std::unique_ptr<expression_t> Parser::parse_primary_expr() {
	if (is_current_token(TOKEN_TYPE::INT_LITERAL)) {
		auto int_literal_expr = std::make_unique<integer_literal_t>(stol(_current_token->value));
		advance();
		return int_literal_expr;
	}
	if (is_current_token(TOKEN_TYPE::IDENTIFIER)) {
		std::string name = parse_identifier();
		if (!is_current_token(TOKEN_TYPE::END_OF_FILE) && is_current_token(TOKEN_TYPE::LPAREN)) { // function call 
			std::vector<std::unique_ptr<expression_t>> arguments = parse_function_call_params();
			auto func_call_expr = std::make_unique<func_call_expr_t>(std::move(name), std::move(arguments));
			return func_call_expr;
		}
		auto var_expr = std::make_unique<var_expression_t>(std::move(name));
		return var_expr;
	}
	// We are lp
	if (is_current_token(TOKEN_TYPE::LPAREN)) {
		advance();
		auto e = parse_expr();
		expect(TOKEN_TYPE::RPAREN, "Missing matching ')' in expression .");
		//We are rp
		advance();
		return e;
	}
	return nullptr;
}
std::unique_ptr<statement_t> Parser::parse_var_decl_statement() {

	type_t var_type = parse_type();
	std::string identifier = parse_identifier();
	std::unique_ptr<expression_t> right;
	// check whether there is an expression in the declaration such as int x = 4;
	if (is_current_token(TOKEN_TYPE::ASSIGNMENT)) {
		advance();
		right = std::move(parse_expr());
	}
	expect(TOKEN_TYPE::SEMICOLON, "Statement must end with ';'");
	advance();
	return std::make_unique<var_decl_statement_t>(var_type,std::move(identifier),std::move(right));
}
std::unique_ptr<statement_t> Parser::parse_if_statement(bool parsing_while) {

	auto condition = parse_conditional_expr();
	auto body = parse_block_statement();
	// Check whether there is else
	std::unique_ptr<block_statement_t> else_body;
	if (is_current_token(TOKEN_TYPE::KW_ELSE)) {
		advance(); // skip to the left curly
		if(is_current_token(TOKEN_TYPE::LCURLY))
			 else_body = parse_block_statement();
	}
	if(parsing_while)
		return std::make_unique<while_statement_t>(std::move(condition), std::move(body));
	return std::make_unique<if_statement_t>(std::move(condition), std::move(body), std::move(else_body));
}
std::vector<std::unique_ptr<expression_t>> Parser::parse_conditional_expr()
{
	// This is called when the current token is "if"
	advance();

	expect(TOKEN_TYPE::LPAREN, "Missing '('");

	// parse the condition
	auto conditions = parse_function_call_params();

	return conditions;
}
std::unique_ptr<statement_t> Parser::parse_while_statement() {
	return parse_if_statement(true);
}
std::unique_ptr<statement_t> Parser::parse_expr_statement() {
	auto expr = parse_expr();
	expect(TOKEN_TYPE::SEMICOLON, "Statement must end with ';'");
	advance();
	return std::make_unique<expr_statement_t>(std::move(expr));
}
std::unique_ptr<statement_t> Parser::parse_for_statement() {
	// is called when the current token is "for"
	advance();
	expect(TOKEN_TYPE::LPAREN, "Expected '(' after 'for' statement");
	advance(); // skip the lparen

	auto range = parse_expr();
	std::unique_ptr<block_statement_t> body;

	if (is_current_token(TOKEN_TYPE::RPAREN)) {
		advance();
		body = parse_block_statement();
		return std::make_unique<for_statement_t>(std::move(range), std::move(body));
	}
	expect(TOKEN_TYPE::SEMICOLON, "Missing ','");
	advance();
	auto step = parse_expr();
	expect(TOKEN_TYPE::LPAREN, "Expected ')' range statement");
	advance();
	body = parse_block_statement();

	return std::make_unique<for_statement_t>(std::move(range),std::move(step), std::move(body));
}
std::unique_ptr<statement_t> Parser::parse_return_statement() {
	// is called when the current token is 'return'
	advance();

	auto return_expr = parse_expr();
	expect(TOKEN_TYPE::SEMICOLON, "Statement must end with ';'");
	advance();
	return std::make_unique<return_statement_t>(std::move(return_expr));
}
std::unique_ptr<statement_t> Parser::parse_skip_statement() {
	advance();
	expect(TOKEN_TYPE::SEMICOLON,"Statement must end with ';'");
	return std::make_unique<skip_statement_t>();
}
std::unique_ptr<statement_t> Parser::parse_break_statement() {
	advance();
	expect(TOKEN_TYPE::SEMICOLON,"Statement must end with ';'");
	return std::make_unique<break_statement_t>();
}
std::unique_ptr<statement_t> Parser::parse_statement() {

	if (is_current_token(TOKEN_TYPE::END_OF_FILE))
		return nullptr;
	// var decl?
	if (is_current_token(TOKEN_TYPE::KW_INT) ||  is_current_token(TOKEN_TYPE::KW_UINT)) {
		return parse_var_decl_statement();
	}
	// if statement?
	if (is_current_token(TOKEN_TYPE::KW_IF)) {
		return parse_if_statement();
	}
	// while statement?
	if (is_current_token(TOKEN_TYPE::KW_WHILE)) {
		return parse_while_statement();
	}
	// for stmt?
	if (is_current_token(TOKEN_TYPE::KW_FOR)) {
		return parse_for_statement();
	}
	// ret
	if (is_current_token(TOKEN_TYPE::KW_RETURN)) {
		return parse_return_statement();
	}
	// break
	if (is_current_token(TOKEN_TYPE::KW_BREAK)) {
		return parse_break_statement();
	}
	// skip
	if (is_current_token(TOKEN_TYPE::KW_SKIP)) {
		return parse_skip_statement();
	}
	// new block of code
	if (is_current_token(TOKEN_TYPE::LCURLY)) {
		return parse_block_statement();
	}
	return parse_expr_statement();
}
const token_t& Parser::peek_before(){
	if(_curr_index > 0)
		return _tokens[_curr_index - 1];
	return _tokens[0];
}

