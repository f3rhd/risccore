#pragma  once
#include <memory>
#include <string>
#include <vector>
#include <print>
#include <ostream>
#include <istream>
#include <fstream>

#include "../ir/ir_gen_context.hpp"
#include "../analysis/analysis_context.hpp"
#include "../other/f3_type.hpp"

namespace fs_compiler {
	
	namespace ast_node {

		struct node_t {
			virtual ~node_t() = default;
			virtual void print_ast(std::ostream& os, uint32_t indent_level = 0, bool is_last = true) const = 0;
			virtual std::string generate_ir(IR_Gen_Context& ctx) const = 0;
		};

		struct expression_t : node_t {
			virtual bool is_lvalue() const { return false; }
			virtual bool has_call() const { return false; }
			virtual bool is_deref() const { return false; }
			virtual bool is_array_init() const { return false; }
			virtual bool is_integer_literal() const { return false; }
			virtual type_t analyse(Analysis_Context& ctx) const = 0;
		};

		struct var_expression_t : expression_t {
			std::string name;
			bool is_lvalue() const override { return true; }
			var_expression_t(std::string&& id) : name(std::move(id)) {}
			void print_ast(std::ostream& os, uint32_t indent_level, bool is_last) const override;
			std::string generate_ir(IR_Gen_Context& ctx) const override;
			type_t analyse(Analysis_Context &ctx) const override;
		};

		struct integer_literal_t : expression_t {
			int32_t value;
			explicit integer_literal_t(int32_t val) : value(val) {}

			void print_ast(std::ostream& os, uint32_t indent_level, bool is_last) const override;
			bool is_integer_literal() const override { return true; }
			std::string generate_ir(IR_Gen_Context& ctx) const override;
			type_t analyse(Analysis_Context &ctx) const override;
		};
		struct array_initialize_expr_t : expression_t {
			std::vector<std::unique_ptr<expression_t>> elements;
			void print_ast(std::ostream& os, uint32_t indent_level, bool is_last) const override;
			std::string generate_ir(IR_Gen_Context& ctx) const override;
			type_t analyse(Analysis_Context &ctx) const override;
			array_initialize_expr_t(std::vector<std::unique_ptr<expression_t>>&& elements_) : elements(std::move(elements_)) {}
			bool is_array_init() const override { return true; };
		};
		struct func_call_expr_t : expression_t {

			std::string id;
			std::vector<std::unique_ptr<expression_t>> arguments;
			func_call_expr_t(std::string&& id_, std::vector<std::unique_ptr<expression_t>>&& args) : id(std::move(id_)), arguments(std::move(args)) {}
			void print_ast(std::ostream& os, uint32_t indent_level = 0, bool is_last = true) const override;
			bool has_call() const override { return true; }
			std::string generate_ir(IR_Gen_Context& ctx) const override;
			type_t analyse(Analysis_Context &ctx) const override;
		};
		enum class BIN_OP {
			ADD, SUB, DIV, MUL, 
			MOD, GT, LT, GTE, 
			LTE, EQUALITY, 
			NOT_EQUAL, 
			LOGICAL_AND, LOGICAL_OR,
			BIT_AND,BIT_OR,
			BIT_XOR,
			BIT_LEFT_SHIFT,BIT_RIGHT_SHIFT
		};
		enum class UNARY_OP {
			UNKNOWN, NEG, INCR, DECR, NOT_LOGICAL, ADDR, DEREF,BIT_NOT
		};
		struct binary_expression_t : expression_t {
			BIN_OP op;
			std::unique_ptr<expression_t> lhs;
			std::unique_ptr<expression_t> rhs;
			binary_expression_t(BIN_OP opr, std::unique_ptr<expression_t>&& left, std::unique_ptr<expression_t>&& right)
				: op(opr), lhs(std::move(left)), rhs(std::move(right)) {
			}

			void print_ast(std::ostream& os, uint32_t indent_level, bool is_last) const override;
			std::string generate_ir(IR_Gen_Context& ctx) const override;
			type_t analyse(Analysis_Context &ctx) const override;

		};
		struct unary_expression_t : expression_t {
			UNARY_OP op = UNARY_OP::UNKNOWN;
			std::unique_ptr<expression_t> expr;
			int32_t ptr_depth = 0;
			unary_expression_t(UNARY_OP op_, std::unique_ptr<expression_t>&& expr_,int32_t ptr_depth_ = 0) : expr(std::move(expr_)), op(op_), ptr_depth(ptr_depth_){}
			void print_ast(std::ostream& os, uint32_t indent_level, bool is_last) const override;
			bool is_deref() const override {
				return op == UNARY_OP::DEREF;
			};
			std::string generate_ir(IR_Gen_Context& ctx) const override;
			type_t analyse(Analysis_Context &ctx) const override;
		};

		enum class ASSIGNMENT_TYPE {
			UNKNOWN,
			NORMAL,
			ADD_ASSIGN, // +=
			SUBTRACT_ASSIGN, // =
			MULTIPLY_ASSIGN, // *=
			DIVIDE_ASSIGN, // /=
			MOD_ASSIGN // %=
		};
		struct assignment_expression_t : expression_t {
			ASSIGNMENT_TYPE type = ASSIGNMENT_TYPE::UNKNOWN;
			std::unique_ptr<expression_t> lhs;
			std::unique_ptr<expression_t> rhs;
			assignment_expression_t(ASSIGNMENT_TYPE type_, std::unique_ptr<expression_t>&& left, std::unique_ptr<expression_t>&& right)
				: lhs(std::move(left)), rhs(std::move(right)), type(type_) {
			}

			void print_ast(std::ostream& os, uint32_t indent_level, bool is_last) const override;
			std::string generate_ir(IR_Gen_Context& ctx) const override;
			type_t analyse(Analysis_Context &ctx) const override;

		};

		struct statement_t : node_t {
			void print_ast(std::ostream& os, uint32_t indent_level = 0, bool is_last = true) const override = 0;
			std::string generate_ir(IR_Gen_Context& ctx) const override;
			virtual type_t analyse(Analysis_Context& ctx) const = 0;
		};

		struct var_decl_statement_t : statement_t {
			std::string name;
			type_t type;
			std::unique_ptr<expression_t> rhs;
			var_decl_statement_t(type_t typ, std::string&& id, std::unique_ptr<expression_t>&& right) : name(std::move(id)), type(typ), rhs(std::move(right)) {}

			void print_ast(std::ostream& os, uint32_t indent_level, bool is_last) const override;
			std::string generate_ir(IR_Gen_Context& ctx) const override;
			type_t analyse(Analysis_Context &ctx) const override;
		};

		struct block_statement_t : statement_t {
			std::vector<std::unique_ptr<statement_t>> statements;
			block_statement_t(std::vector<std::unique_ptr<statement_t>>&& statements_) : statements(std::move(statements_)) {}
			void print_ast(std::ostream& os, uint32_t indent_level, bool is_last = true) const override;
			std::string generate_ir(IR_Gen_Context& ctx) const override;
			type_t analyse(Analysis_Context &ctx) const override;

		};
		struct if_statement_t : statement_t {
			std::vector<std::unique_ptr<expression_t>> condition;
			std::unique_ptr<block_statement_t> body;
			std::unique_ptr<block_statement_t> else_body;
			if_statement_t(std::vector<std::unique_ptr<expression_t>>&& condition_, std::unique_ptr<block_statement_t>&& body_) :
				condition(std::move(condition_)), body(std::move(body_))
			{
			}
			if_statement_t(std::vector<std::unique_ptr<expression_t>>&& condition_, std::unique_ptr<block_statement_t>&& body_, std::unique_ptr<block_statement_t>&& else_body_) :
				condition(std::move(condition_)), body(std::move(body_)), else_body(std::move(else_body_))
			{
			}
			void print_ast(std::ostream& os, uint32_t indent_level = 0, bool is_last = true) const override;
			std::string generate_ir(IR_Gen_Context& ctx) const override;
			type_t analyse(Analysis_Context &ctx) const override;
		};
		struct while_statement_t : if_statement_t {
			while_statement_t(std::vector<std::unique_ptr<expression_t>>&& condition_,
				std::unique_ptr<block_statement_t>&& body_)
				: if_statement_t(std::move(condition_), std::move(body_)) {
			}

			void print_ast(std::ostream& os, uint32_t indent_level = 0, bool is_last = true) const override;
			std::string generate_ir(IR_Gen_Context& ctx) const override;
			type_t analyse(Analysis_Context &ctx) const override;
		};
		struct for_statement_t : statement_t {
			std::unique_ptr<expression_t> condition;
			std::unique_ptr<expression_t> step;
			std::unique_ptr<block_statement_t> body;
			for_statement_t(
				std::unique_ptr<expression_t>&& range_,
				std::unique_ptr<expression_t>&& step_,
				std::unique_ptr<block_statement_t>&& body_
			) : condition(std::move(range_)), step(std::move(step_)), body(std::move(body_)) {
			}
			for_statement_t(
				std::unique_ptr<expression_t>&& range_,
				std::unique_ptr<block_statement_t>&& body_
			) : condition(std::move(range_)), body(std::move(body_)) {
			}

			void print_ast(std::ostream& os, uint32_t indent_level = 0, bool is_last = true) const override;
			std::string generate_ir(IR_Gen_Context& ctx) const override;
			type_t analyse(Analysis_Context &ctx) const override;
		
		};
		struct return_statement_t : statement_t {
			std::unique_ptr<expression_t> return_expr;
			return_statement_t(std::unique_ptr<expression_t>&& return_expr_) : return_expr(std::move(return_expr_)) {}
			void print_ast(std::ostream& os, uint32_t indent_level = 0, bool is_last = true) const override;
			std::string generate_ir(IR_Gen_Context& ctx) const override;
			type_t analyse(Analysis_Context &ctx) const override;
		};
		struct break_statement_t : statement_t {
			break_statement_t() = default;
			void print_ast(std::ostream& os, uint32_t indent_level = 0, bool is_last = true) const override;
			std::string generate_ir(IR_Gen_Context& ctx) const override;
			type_t analyse(Analysis_Context &ctx) const override;
		};

		struct skip_statement_t : statement_t {
			skip_statement_t() = default;
			void print_ast(std::ostream& os, uint32_t indent_level = 0, bool is_last = true) const override;
			std::string generate_ir(IR_Gen_Context& ctx) const override;
			type_t analyse(Analysis_Context &ctx) const override;
		};

		struct expr_statement_t : statement_t {
			std::unique_ptr<expression_t> expr;
			expr_statement_t(std::unique_ptr<expression_t>&& expr_) : expr(std::move(expr_)) {}
			void print_ast(std::ostream& os, uint32_t indent_level = 0, bool is_last = true) const override;
			std::string generate_ir(IR_Gen_Context& ctx) const override;
			type_t analyse(Analysis_Context &ctx) const override;
		};

		struct func_decl_t : node_t {
			std::string id;
			type_t return_type;
			std::vector<func_decl_param_t> arguments;
			std::unique_ptr<block_statement_t> body;

			void analyse(Analysis_Context & ctx);
			func_decl_t(std::string&& func_name, std::vector<func_decl_param_t>&& args, type_t ret, std::unique_ptr<block_statement_t> body_) :
				id(std::move(func_name)), return_type(ret), body(std::move(body_)), arguments(std::move(args)) {
			}

			void print_ast(std::ostream& os, uint32_t indent_level = 0, bool is_last = true) const override;
			std::string generate_ir(IR_Gen_Context& ctx) const override;
		};
	}
}

