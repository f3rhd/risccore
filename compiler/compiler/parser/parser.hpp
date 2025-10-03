#pragma  once
#include "../common/other/token.hpp"
#include "../common/other/error.hpp"
#include "../program/program.hpp"
#include "../common/ast/nodes.hpp"
#include "../common/analysis/analysis_context.hpp"
namespace fs_compiler {
	class Parser {
	public:
		Parser(std::vector<token_t>&& tokens) : _tokens(std::move(tokens)), _current_token(&_tokens[0]) {}
		Program															parse_program();
		bool															has_error();
	private:
		std::unique_ptr<ast_node::func_decl_t>							parse_function();
		void															advance();
		void															make_error(const token_t& cause,const std::string& message);
		bool															current_token_is(TOKEN_TYPE expected_type);
		void															expect(TOKEN_TYPE type,const std::string& error_msg);
		const token_t&													peek_next();
		const token_t&													peek_before(uint32_t i = 1);
		std::vector<func_decl_param_t>									parse_func_decl_params();
		std::string														parse_identifier();
		std::unique_ptr<ast_node::block_statement_t> 					parse_func_body();
		std::unique_ptr<ast_node::block_statement_t>					parse_block_statement();
		std::unique_ptr<ast_node::expression_t> 						parse_logical_shift();
		std::unique_ptr<ast_node::expression_t> 						parse_bit_and();
		std::unique_ptr<ast_node::expression_t> 						parse_bit_xor();
		std::unique_ptr<ast_node::expression_t> 						parse_bit_or();
		std::vector<std::unique_ptr<ast_node::expression_t>> 			parse_function_call_params();
		std::unique_ptr<ast_node::expression_t> 						parse_assignment_expr();
		std::unique_ptr<ast_node::expression_t> 						parse_primary_expr();
		std::unique_ptr<ast_node::expression_t> 						parse_expr();
		std::unique_ptr<ast_node::expression_t> 						parse_for_range_expr();
		std::unique_ptr<ast_node::statement_t> 							parse_var_decl_statement();
		std::unique_ptr<ast_node::statement_t> 							parse_if_statement(bool parsing_while = false);
		std::unique_ptr<ast_node::statement_t> 							parse_statement();
		std::vector<std::unique_ptr<ast_node::expression_t>> 			parse_conditional_expr();
		std::unique_ptr<ast_node::expression_t>							parse_array_initialize_expr();
		std::unique_ptr<ast_node::statement_t> 							parse_while_statement();
		std::unique_ptr<ast_node::statement_t> 							parse_return_statement();
		std::unique_ptr<ast_node::statement_t> 							parse_skip_statement();
		std::unique_ptr<ast_node::statement_t> 							parse_break_statement();
		std::unique_ptr<ast_node::statement_t> 							parse_expr_statement();
		std::unique_ptr<ast_node::statement_t> 							parse_for_statement();
		std::unique_ptr<ast_node::expression_t> 						parse_equality_expr();
		std::unique_ptr<ast_node::expression_t> 						parse_relational_expr();
		std::unique_ptr<ast_node::expression_t> 						parse_logical_or_expr();
		std::unique_ptr<ast_node::expression_t> 						parse_logical_and_expr();
		std::unique_ptr<ast_node::expression_t> 						parse_addition_subtraction();
		std::unique_ptr<ast_node::expression_t> 						parse_multiplication_division();
		std::unique_ptr<ast_node::expression_t> 						parse_mod();
		std::unique_ptr<ast_node::expression_t> 						parse_unary_op();
		type_t 															parse_type();

	private:
		std::vector<token_t> _tokens;
		const token_t *_current_token = nullptr;
		size_t _curr_index = 0;
		std::vector<error_t> _errors;
	};

}