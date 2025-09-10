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
	if (_current_token->type != TOKEN_TYPE::END_OF_FILE) {
		_curr_index++;
		_current_token = &_tokens[_curr_index];
	}
	if(_current_token->type == TOKEN_TYPE::END_OF_FILE){
		_current_token = &_tokens[_curr_index-1];
		_curr_index--;
	}
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
		if (_current_token->type == TOKEN_TYPE::ARROW || _current_token->type == TOKEN_TYPE::LCURLY) {
			make_error(*_current_token, "Missing ')' . Function parameters must be closed by ')'");
			break;
		}
		if (_current_token->type != TOKEN_TYPE::RPAREN) {
			expect(TOKEN_TYPE::COMMA, "Missing ',' . Function arguments must be seperated by ','");
		}
		else {
			advance();
			break;
		}
		advance();
	}
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
	if (current_token_is(type))
		return;
	make_error(*_current_token, error_msg);
}
bool Parser::current_token_is(TOKEN_TYPE expected_type)
{
	if (_current_token->type == expected_type)
		return true;
	return false;
}

std::unique_ptr<block_statement_t> Parser::parse_func_body()
{
	return parse_block_statement();
}
// @call : current token is left curly
std::unique_ptr<block_statement_t> Parser::parse_block_statement() {

	std::vector<std::unique_ptr<statement_t>> statements;
	while (_current_token->type != TOKEN_TYPE::LCURLY) {
		make_error(*_current_token, "Unexpected ");
		advance();
	}
	// skip the left curly 
	advance();
	while(!current_token_is(TOKEN_TYPE::END_OF_FILE) && !current_token_is(TOKEN_TYPE::RCURLY)) {

		statements.push_back(std::move(parse_statement()));
	}
	expect(TOKEN_TYPE::RCURLY, "Missing '}'");
	advance();
	return std::make_unique<block_statement_t>(std::move(statements));
}

std::unique_ptr<expression_t> Parser::parse_expr() {

	return parse_for_range_expr();
}
std::unique_ptr<expression_t> Parser::parse_for_range_expr() {
	auto e = parse_assignment_expr();
	// (i=0...=10)
	if (current_token_is(TOKEN_TYPE::TRIPLE_DOT)) {
		bool is_exclusive = false;
		// skip the tripple dot
		advance();
		if (!current_token_is(TOKEN_TYPE::ASSIGNMENT)) {
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
	if (current_token_is(TOKEN_TYPE::ASSIGNMENT) ||
		current_token_is(TOKEN_TYPE::P_ASSIGNMENT) ||
		current_token_is(TOKEN_TYPE::M_ASSIGNMENT)
		) {
		ASSIGNMENT_TYPE type = ASSIGNMENT_TYPE::UNKNOWN;
		if ( left != nullptr && !left->is_lvalue()) {
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

std::unique_ptr<expression_t> Parser::parse_equality_expr() {
	auto e = parse_logical_or_expr();
	while (current_token_is(TOKEN_TYPE::EQUAL) || current_token_is(TOKEN_TYPE::NOT_EQUAL)) {
		BIN_OP op = current_token_is(TOKEN_TYPE::EQUAL) ? BIN_OP::EQUALITY : BIN_OP::NOT_EQUAL;
		advance();
		auto right = parse_logical_or_expr();
		e = std::make_unique<binary_expression_t>(op, std::move(e), std::move(right));
	}
	return e;
}

std::unique_ptr<expression_t> Parser::parse_logical_or_expr() {
	auto e = parse_logical_and_expr();
	while (current_token_is(TOKEN_TYPE::DOUBLE_COLUMN) || current_token_is(TOKEN_TYPE::KW_OR)) {
		BIN_OP op = BIN_OP::OR;
		advance();
		auto right = parse_logical_and_expr();
		e = std::make_unique<binary_expression_t>(op, std::move(e), std::move(right));
	}
	return e;
}

std::unique_ptr<expression_t> Parser::parse_logical_and_expr() {
	auto e = parse_relational_expr();
	if (current_token_is(TOKEN_TYPE::DOUBLE_AMPERSAND) || current_token_is(TOKEN_TYPE::KW_AND)) {
		BIN_OP op = BIN_OP::AND;
		advance();
		auto right = parse_relational_expr();
		e = std::make_unique<binary_expression_t>(op, std::move(e), std::move(right));
	}
	return e;
}
std::unique_ptr<expression_t> Parser::parse_relational_expr() {
	auto e = parse_addition_subtraction();
	while (
		current_token_is(TOKEN_TYPE::GREATER) ||
		current_token_is(TOKEN_TYPE::GRATER_EQUAL) ||
		current_token_is(TOKEN_TYPE::LESS) ||
		current_token_is(TOKEN_TYPE::LESS_EQUAL)
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
std::unique_ptr<expression_t> Parser::parse_addition_subtraction() {

	auto e = parse_multiplication_division();
	while (current_token_is(TOKEN_TYPE::MINUS) || current_token_is(TOKEN_TYPE::PLUS)) {
		BIN_OP op = current_token_is(TOKEN_TYPE::MINUS) ? BIN_OP::SUB : BIN_OP::ADD;
		advance();
		auto right = parse_multiplication_division();
		e = std::make_unique<binary_expression_t>(op, std::move(e), std::move(right));
	}
	return e;
}
std::unique_ptr<expression_t> Parser::parse_multiplication_division() {
	auto e = parse_mod();
	while (current_token_is(TOKEN_TYPE::STAR) || current_token_is(TOKEN_TYPE::SLASH)) {

		BIN_OP op = current_token_is(TOKEN_TYPE::STAR) ? BIN_OP::MUL : BIN_OP::DIV;
		advance();
		auto right = parse_mod();
		e = std::make_unique<binary_expression_t>(op, std::move(e), std::move(right));
	}
	return e;
}
std::unique_ptr<expression_t> Parser::parse_mod() {
	auto  e = parse_unary_op();
	while (current_token_is(TOKEN_TYPE::PERCENTAGE)) {

		BIN_OP op = BIN_OP::MOD;
		advance();
		auto right = parse_unary_op();
		e = std::make_unique<binary_expression_t>(op, std::move(e), std::move(right));
	}
	return e;
}
std::unique_ptr<expression_t> Parser::parse_unary_op() {
	// Handle prefix unary operators
	if (
		current_token_is(TOKEN_TYPE::DOUBLE_PLUS) ||
		current_token_is(TOKEN_TYPE::MINUS) ||
		current_token_is(TOKEN_TYPE::DOUBLE_MINUS) ||
		current_token_is(TOKEN_TYPE::EXCLAMATION) ||
		current_token_is(TOKEN_TYPE::AMPERSAND) ||
		current_token_is(TOKEN_TYPE::STAR)
		) {
		UNARY_OP op = UNARY_OP::UNKNOWN;
		switch (_current_token->type) {
		case TOKEN_TYPE::DOUBLE_PLUS: op = UNARY_OP::INCR; break;
		case TOKEN_TYPE::MINUS: op = UNARY_OP::NEG; break;
		case TOKEN_TYPE::DOUBLE_MINUS: op = UNARY_OP::DECR; break;
		case TOKEN_TYPE::EXCLAMATION: op = UNARY_OP::NOT; break;
		case TOKEN_TYPE::AMPERSAND: op = UNARY_OP::ADDR; break;
		case TOKEN_TYPE::STAR: op = UNARY_OP::DEREF; break;
		default:
			make_error(*_current_token, "Unknown unary operator.");
			break;
		}
		advance();
		auto operand = parse_unary_op();
		return std::make_unique<unary_expression_t>(op, std::move(operand));
	}

	// Parse the primary expression
	auto expr = parse_primary_expr();

	// Handle postfix unary operators (e.g., x++, x--)
	while (
		current_token_is(TOKEN_TYPE::DOUBLE_PLUS) ||
		current_token_is(TOKEN_TYPE::DOUBLE_MINUS)
		) {
		UNARY_OP op = UNARY_OP::UNKNOWN;
		switch (_current_token->type) {
		case TOKEN_TYPE::DOUBLE_PLUS: op = UNARY_OP::INCR; break;
		case TOKEN_TYPE::DOUBLE_MINUS: op = UNARY_OP::DECR; break;
		default:
			make_error(*_current_token, "Unknown postfix unary operator.");
			break;
		}
		advance();
		// For postfix, wrap the current expr
		expr = std::make_unique<unary_expression_t>(op, std::move(expr));
	}

	return expr;
}

std::vector<std::unique_ptr<expression_t>> Parser::parse_function_call_params() { // This function is also used in parsing of if,while statement's condition

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
		if (_current_token->type == TOKEN_TYPE::LCURLY) {
			make_error(*_current_token, " Expected closing ')'.");
			break;
		}
		if (_current_token->type != TOKEN_TYPE::RPAREN) {
			expect(TOKEN_TYPE::COMMA, "Expressions must be seperated by ','.");
		}
		else {
			advance();
			break;
		}
		advance();
	}
	return func_parameters;
}
std::unique_ptr<expression_t> Parser::parse_primary_expr() {
	if (current_token_is(TOKEN_TYPE::INT_LITERAL)) {
		auto int_literal_expr = std::make_unique<integer_literal_t>(stol(_current_token->value));
		advance();
		return int_literal_expr;
	}
	if (current_token_is(TOKEN_TYPE::IDENTIFIER)) {
		std::string name = parse_identifier();
		if (!current_token_is(TOKEN_TYPE::END_OF_FILE) && current_token_is(TOKEN_TYPE::LPAREN)) { // function call 
			std::vector<std::unique_ptr<expression_t>> arguments = parse_function_call_params();
			auto func_call_expr = std::make_unique<func_call_expr_t>(std::move(name), std::move(arguments));
			return func_call_expr;
		}
		auto var_expr = std::make_unique<var_expression_t>(std::move(name));
		return var_expr;
	}
	// We are lp
	if (current_token_is(TOKEN_TYPE::LPAREN)) {
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
	if (current_token_is(TOKEN_TYPE::ASSIGNMENT)) {
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
	if (current_token_is(TOKEN_TYPE::KW_ELSE)) {
		advance(); // skip to the left curly
		if(current_token_is(TOKEN_TYPE::LCURLY))
			 else_body = parse_block_statement();
	}
	if(parsing_while)
		return std::make_unique<while_statement_t>(std::move(condition), std::move(body));
	return std::make_unique<if_statement_t>(std::move(condition), std::move(body), std::move(else_body));
}
// @call : current token is "if"
std::vector<std::unique_ptr<expression_t>> Parser::parse_conditional_expr()
{
	advance();

	expect(TOKEN_TYPE::LPAREN, "Missing '('");

	// parse the condition
	auto conditions = parse_function_call_params();

	return conditions;
}
// @call : current token is while
std::unique_ptr<statement_t> Parser::parse_while_statement() {
	return parse_if_statement(true);
}
std::unique_ptr<statement_t> Parser::parse_expr_statement() {
	auto expr = parse_expr();
	expect(TOKEN_TYPE::SEMICOLON, "Statement must end with ';'");
	advance();
	return std::make_unique<expr_statement_t>(std::move(expr));
}

// @call :  called when the current token is "for"
std::unique_ptr<statement_t> Parser::parse_for_statement() {
	advance();
	expect(TOKEN_TYPE::LPAREN, "Expected '(' after 'for' statement");
	advance(); // skip the lparen

	auto range = parse_expr();
	std::unique_ptr<block_statement_t> body;

	if (current_token_is(TOKEN_TYPE::RPAREN)) {
		advance();
		body = parse_block_statement();
		return std::make_unique<for_statement_t>(std::move(range), std::move(body));
	}
	expect(TOKEN_TYPE::SEMICOLON, "Missing ','");
	advance();
	auto step_expr = parse_expr();
	expect(TOKEN_TYPE::RPAREN, "Expected ')' range statement");
	advance();
	body = parse_block_statement();

	return std::make_unique<for_statement_t>(std::move(range),std::move(step_expr), std::move(body));
}

// @call : current token is return
std::unique_ptr<statement_t> Parser::parse_return_statement() {
	advance();

	auto return_expr = parse_expr();
	expect(TOKEN_TYPE::SEMICOLON, "Statement must end with ';'");
	advance();
	return std::make_unique<return_statement_t>(std::move(return_expr));
}
// @call : current token is skip
std::unique_ptr<statement_t> Parser::parse_skip_statement() {
	advance();
	expect(TOKEN_TYPE::SEMICOLON,"Statement must end with ';'");
	return std::make_unique<skip_statement_t>();
}
// @call : current token is break
std::unique_ptr<statement_t> Parser::parse_break_statement() {
	advance();
	expect(TOKEN_TYPE::SEMICOLON,"Statement must end with ';'");
	return std::make_unique<break_statement_t>();
}
std::unique_ptr<statement_t> Parser::parse_statement() {

	if (current_token_is(TOKEN_TYPE::END_OF_FILE))
		return nullptr;
	// var decl?
	if (current_token_is(TOKEN_TYPE::KW_INT) || current_token_is(TOKEN_TYPE::KW_UINT)) {
		return parse_var_decl_statement();
	}
	// if statement?
	if (current_token_is(TOKEN_TYPE::KW_IF)) {
		return parse_if_statement();
	}
	// while statement?
	if (current_token_is(TOKEN_TYPE::KW_WHILE)) {
		return parse_while_statement();
	}
	// for stmt?
	if (current_token_is(TOKEN_TYPE::KW_FOR)) {
		return parse_for_statement();
	}
	// ret
	if (current_token_is(TOKEN_TYPE::KW_RETURN)) {
		return parse_return_statement();
	}
	// break
	if (current_token_is(TOKEN_TYPE::KW_BREAK)) {
		return parse_break_statement();
	}
	// skip
	if (current_token_is(TOKEN_TYPE::KW_SKIP)) {
		return parse_skip_statement();
	}
	// new block of code
	if (current_token_is(TOKEN_TYPE::LCURLY)) {
		return parse_block_statement();
	}
	return parse_expr_statement();
}
const token_t& Parser::peek_before(){
	if(_curr_index > 0)
		return _tokens[_curr_index - 1];
	return _tokens[0];
}

bool Parser::no_error()
{
	return _errors.empty();
}

Program Parser::parse_program()
{
	std::vector<std::unique_ptr<func_decl_t>> funcs;
	while (current_token_is(TOKEN_TYPE::KW_FUNC)) {
		auto func = parse_function();
		funcs.push_back(std::move(func));
	}
	return {std::move(funcs), };
}

void Program::generate_IR() {

	IR_Gen_Context ctx;
	for (auto& func : _functions) {
		func->generate_ir(ctx);
	}
	instructions = std::move(ctx.instructions);
}

void Program::print_IR(std::ostream& os){

	for(auto& instr : instructions){
		os << instr.to_string() << '\n';
	}
}
bool Program::has_error() const
{
	return had_error;
}

void Program::print_ast()
{
	for (auto& func : _functions) {
		func->print_ast(std::cout);
	}
}

