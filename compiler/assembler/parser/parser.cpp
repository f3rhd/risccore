#include <iostream>
#include <vector>

#include "../code_gen/ast_analyser.hpp"
#include "../common/macro.hpp"
#include "../common/utils.hpp"
#include "parser.hpp"

namespace f3_riscv_assembler {

	const Token *Parser::peek(std::vector<Token> &line_tokens) {
		if (_token_index < line_tokens.size())
			return &line_tokens[_token_index];
		return nullptr;
	}
	const Token *Parser::eat(std::vector<Token> &line_tokens) {
		if (_token_index < line_tokens.size())
			return &line_tokens[_token_index++];

		return nullptr;
	}

	void Parser::rewind() {
		_token_index = 0;
	}
		Ast_Node *Parser::parse_line(Line &line) {

		const Token *active_token = eat(line.tokens);

		if (active_token == nullptr) {
			return nullptr;
		}
		if (active_token->type == TOKEN_TYPE::LABEL) {
			active_token = eat(line.tokens);
			// This happens in label only lines
			/* main:
				...
			*/
			if (active_token == nullptr) {
				return nullptr;
			}
		}

		switch (active_token->type) {

		case TOKEN_TYPE::OPERATION: {

			Ast_Node *opr_node = utils::make_operation_node(&(active_token->word),
															instruction_look_up::get_opr_type(active_token->word), &line);

			auto current_peek = [&]() -> const Token * { return peek(line.tokens); };

			const Token *next_tok = current_peek();
			if (next_tok != nullptr && next_tok->type == TOKEN_TYPE::COMMA) {
				utils::throw_error_message({"Unexpected comma.", &next_tok->word, &line});
			}

			opr_node->left = parse_line(line);

			next_tok = current_peek();

			// Special-case: jal/jalr acting as pseudo when no comma (single-operand form)
			if ((*(opr_node->str_ptr_value))[0] == 'j') {
				if (opr_node->opr_type == instruction_look_up::OPERATION_TYPE::I_TYPE && next_tok == nullptr)
					opr_node->opr_type = instruction_look_up::OPERATION_TYPE::PSEUDO_TYPE_5;
				else if (opr_node->opr_type == instruction_look_up::OPERATION_TYPE::J_TYPE && next_tok == nullptr)
					opr_node->opr_type = instruction_look_up::OPERATION_TYPE::PSEUDO_TYPE_4;
			}

			auto requires_comma_after_left = [&](instruction_look_up::OPERATION_TYPE t) -> bool {
				// These operation types do not require a comma after the first operand
				if (t == instruction_look_up::OPERATION_TYPE::PSEUDO_TYPE_4 || t == instruction_look_up::OPERATION_TYPE::PSEUDO_TYPE_5 || t == instruction_look_up::OPERATION_TYPE::PSEUDO_TYPE_6)
					return false;
				return true;
			};

			if (requires_comma_after_left(opr_node->opr_type)) {
				if (next_tok != nullptr && next_tok->type == TOKEN_TYPE::COMMA) {
					eat(line.tokens);
				}
				else {
					const std::string *val = opr_node->left ? opr_node->left->str_ptr_value : nullptr;
					utils::throw_error_message({"Expected ',' ", val, &line});
					exit(1);
				}
			}

			opr_node->middle = parse_line(line);

			next_tok = current_peek();

			auto requires_second_separator = [&](instruction_look_up::OPERATION_TYPE t) -> bool {
				// These operation types have only two operands or otherwise don't need a second separator
				if (t == instruction_look_up::OPERATION_TYPE::PSEUDO_TYPE_4 || t == instruction_look_up::OPERATION_TYPE::PSEUDO_TYPE_5 || t == instruction_look_up::OPERATION_TYPE::PSEUDO_TYPE_1 || t == instruction_look_up::OPERATION_TYPE::PSEUDO_TYPE_2 ||
					t == instruction_look_up::OPERATION_TYPE::PSEUDO_TYPE_6 || t == instruction_look_up::OPERATION_TYPE::PSEUDO_TYPE_0 || t == instruction_look_up::OPERATION_TYPE::J_TYPE || t == instruction_look_up::OPERATION_TYPE::U_TYPE)
					return false;
				return true;
			};

			if (requires_second_separator(opr_node->opr_type)) {
				if (next_tok != nullptr &&
					(next_tok->type == TOKEN_TYPE::COMMA || next_tok->type == TOKEN_TYPE::LPAREN)) {
					eat(line.tokens);
				}
				else {
					const std::string *val = opr_node->left ? opr_node->left->str_ptr_value : nullptr;
					utils::throw_error_message({"Expected ',' or '('  ", val, &line});
					exit(1);
				}
			}

			// Parse right operand (if present)
			opr_node->right = parse_line(line);
			return opr_node;
		}

		case TOKEN_TYPE::REGISTER: {
			Ast_Node *reg_node = utils::make_reg_node(&(active_token->word), &line);
			return reg_node;
		}
		case TOKEN_TYPE::IMMEDIATE: {

			Ast_Node *imm_node = utils::make_imm_node(&(active_token->word), &line);
			return imm_node;
		}
		case TOKEN_TYPE::IDENTIFIER: {
			return utils::make_identifier_node(&(active_token->word), &line);
		}
		default:
			return nullptr;
		}
	}
	void Parser::parse_lines(std::vector<Line> &lines, std::unordered_map<std::string, uint32_t> &labels) {

		_heads.reserve(lines.size());
		for (Line &line : lines) {
			rewind();
			Ast_Node *head = parse_line(line);
			if (head != nullptr) {
				resolve_label_identifier(head, labels);
				if (ast_analyser::analyse_line_ast(head) != 1) {
					successful_exit = false;
				}
				_heads.push_back(head);
			}
		}
		if (successful_exit == false) {
			printf("Assembling failed.\n");
			exit(1);
		}
	}
	void Parser::resolve_label_identifier(Ast_Node *head, std::unordered_map<std::string, uint32_t> &labels) {
		Ast_Node *candidate_label_identifier_node;
		switch (head->opr_type) {
			using namespace instruction_look_up;
		case OPERATION_TYPE::PSEUDO_TYPE_3:
		case OPERATION_TYPE::B_TYPE: {
			candidate_label_identifier_node = head->right;
			break;
		}
		case OPERATION_TYPE::J_TYPE:
		case OPERATION_TYPE::PSEUDO_TYPE_2: {
			candidate_label_identifier_node = head->middle;
			break;
		}
		case OPERATION_TYPE::PSEUDO_TYPE_4: {
			candidate_label_identifier_node = head->left;
			break;
		}
		default:
			return;
		}
		if (candidate_label_identifier_node && candidate_label_identifier_node->node_type == AST_NODE_TYPE::IDENTIFIER) {
			auto it = labels.find(*candidate_label_identifier_node->str_ptr_value);
			if (it == labels.end()) {
				candidate_label_identifier_node->identifier_immediate = -1;
				return;
			}
			uint32_t label_row_number = it->second;
			candidate_label_identifier_node->identifier_immediate =
				utils::calculate_offset(label_row_number, candidate_label_identifier_node->line_info->memory_row_number);
		}
	}
	std::vector<Ast_Node *> &Parser::get_ast_nodes() {
		return _heads;
	}
}
