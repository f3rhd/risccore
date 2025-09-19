
#include "lexer/lexer.hpp"
#include "parser/parser.hpp"

#include "../assembler/code_gen/ast_analyser.hpp"
#include "../assembler/code_gen/code_gen.hpp"
#include "../assembler/code_gen/instr_gen.hpp"
#include "../assembler/parser/parser.hpp"
#include "../assembler/preprocessor/preprocessor.hpp"
// TODO : SEMANTIC ANALYSIS 
using namespace  f3_compiler;
int main(int argc, char** argv) {
	if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <input.f3> [--print-ast] [--emit-ir <file>] [--emit-asm-debug <file>] [--emit-asm <file>][--emit-bin <file>]\n";
        return 1;
	}
		const char* input_file = argv[1];
		bool print_ast = false;
		bool asm_debug_output = false;
		bool print_ir = false;
		const char *ir_file = nullptr;
		const char* asm_file = nullptr;
		const char* bin_file = nullptr;
		const char* flow_file = nullptr;
		for (int i = 2; i < argc; i++) {
			if (strcmp(argv[i], "--print-ast") == 0) {
				print_ast = true;
			} else if (strcmp(argv[i], "--emit-asm") == 0 && i + 1 < argc) {
				asm_file = argv[++i];
			} else if (strcmp(argv[i], "--emit-ir") == 0 && i + 1 < argc) {
				ir_file = argv[++i];
				print_ir = true;
			} else if (strcmp(argv[i], "--emit-asm-debug") == 0 && i + 1 < argc) {
				bin_file = argv[++i];
				asm_debug_output = true;
			} else if (strcmp(argv[i], "--emit-bin") == 0 && i + 1 < argc) {
				bin_file = argv[++i];
			}
		}
		Lexer lexer(input_file);
		Parser parser(std::move(const_cast<std::vector<token_t>&>(lexer.get_tokens())));
	auto program = parser.parse_program();
	if (parser.has_error()) {
		exit(EXIT_FAILURE);
	}
	if (print_ast) {
		program.print_ast();
	}
	program.analyse();
	if(program.has_error()){
		exit(EXIT_FAILURE);
	}
	program.generate_IR();
	std::ostringstream asm_stream;
	program.generate_asm(asm_stream);
	if (program.has_error()) {
		exit(EXIT_FAILURE);
	}
	std::string asm_code = asm_stream.str();

	if(ir_file){
		std::ofstream ofs(ir_file);
		program.print_IR(ofs);
	}
	if (asm_file) {
		std::ofstream ofs(asm_file);
		ofs << ".reset_vector:\n";
		ofs << "\tli sp, 0xFFFF\n";
		ofs << "\tcall main\n";
		ofs << asm_code;
	}

	if (bin_file) {

		std::string temp = "out" + std::string(bin_file);
		std::ofstream ofs(temp);
		ofs << ".reset_vector:\n";
		ofs << "\tli sp, 0xFFFF\n";
		ofs << "\tcall main\n";
		ofs << asm_code;
		ofs.close();
		f3_riscv_assembler::Preprocessor asm_prc(temp.c_str());
		f3_riscv_assembler::Parser asm_parser;
		asm_parser.parse_lines(asm_prc.process(asm_debug_output), asm_prc.get_labels());
		f3_riscv_assembler::instr_gen::generator gen;
		gen.generate_instructions(asm_parser.get_ast_nodes());
		f3_riscv_assembler::code_gen::generate_bin_file(bin_file, gen.get_instructions(),asm_debug_output);
		std::remove(temp.c_str());
	}
	return 0;
}