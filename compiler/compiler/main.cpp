#include <cstring>
#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include "../assembler/code_gen/ast_analyser.hpp"
#include "../assembler/code_gen/code_gen.hpp"
#include "../assembler/code_gen/instr_gen.hpp"
#include "../assembler/parser/parser.hpp"
#include "../assembler/preprocessor/preprocessor.hpp"
using namespace  fs_compiler;
int main(int argc, char** argv) {
	if (argc < 2) {
		std::cerr << "Usage: " << argv[0] << " <input.f3> [--print-ast] [--emit-ir-debug <file>] [--emit-ir <file>] [--emit-asm-debug <file>] [--emit-asm <file>][--emit-bin <file>]\n";
		return 1;
	}
	const char* input_file = argv[1];
	bool print_ast = false;
	bool asm_debug_output = false;
	bool ir_debug_print = false;
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
		} else if (strcmp(argv[i], "--emit-asm-debug") == 0 && i + 1 < argc) {
			bin_file = argv[++i];
			asm_debug_output = true;
		} else if (strcmp(argv[i], "--emit-bin") == 0 && i + 1 < argc) {
			bin_file = argv[++i];
		}
		else if (strcmp(argv[i], "--emit-ir-debug") == 0 && i + 1 < argc) {
			 ir_file = argv[++i];
			 ir_debug_print = true;
		}
	}
	Lexer lexer(input_file);
	Parser parser(std::move(const_cast<std::vector<token_t>&>(lexer.get_tokens())));
	auto program = parser.parse_program();
	if (parser.has_error()) {
		exit(EXIT_FAILURE);
	}
	program.analyse();
	if (print_ast) {
		program.print_ast();
		return 0;
	}
	if(program.has_error()){
		exit(EXIT_FAILURE);
	}
	program.generate_IR();

	if (program.has_error()) {
		exit(EXIT_FAILURE);
	}

	if(ir_file){
		if ( !ir_debug_print && strcmp(ir_file, "cout") != 0) {
			std::ofstream ofs(ir_file);
			program.print_IR(ofs);
		}
		else if(ir_debug_print && strcmp(ir_file,"cout") == 0) {
			program.print_liveness_json(std::cout);
		}
		else if (ir_debug_print) {
			std::ofstream ofs(ir_file);
			program.print_liveness_json(ofs);
		}
		else {
			program.print_IR(std::cout);
		}
	}
	if (asm_file) {
		if (strcmp(asm_file, "cout") != 0) {

			std::ostringstream asm_stream;
			program.generate_asm(asm_stream);
			std::string asm_code = asm_stream.str();
			std::ofstream ofs(asm_file);
			ofs << ".reset_vector:\n";
			ofs << "\tli sp, 0xFFFF0\n";
			ofs << "\tcall .main\n";
			ofs << asm_code;
		}
		else {
			std::cout << ".reset_vector:\n";
			std::cout<< "\tli sp, 0xFFFF\n";
			std::cout << "\tcall .main\n";
			program.generate_asm(std::cout);
		}
	}

	if (bin_file) {
		std::ostringstream asm_stream;
		program.generate_asm(asm_stream);
		std::string asm_code = asm_stream.str();
		std::string temp = std::string(bin_file) + ".s";
		{
			std::ofstream ofs(temp);
			if (!ofs) {
				std::cerr << "Error: Could not open " << temp << " for writing.\n";
				return 1 ;
			}
			ofs << ".reset_vector:\n";
			ofs << "\tli sp, 0xFFF0\n";
			ofs << "\tcall .main\n";
			ofs << asm_code;
		}

		riscv_assembler::Preprocessor asm_prc(temp);
		riscv_assembler::Parser asm_parser;
		asm_parser.parse_lines(asm_prc.process(asm_debug_output), asm_prc.get_labels());

		riscv_assembler::instr_gen::generator gen;
		gen.generate_instructions(asm_parser.get_ast_nodes());

		riscv_assembler::code_gen::generate_bin_file(
			bin_file,
			gen.get_instructions(),
			asm_debug_output
		);

		if (std::remove(temp.c_str()) != 0) {
			std::cerr << "Warning: could not remove temp file " << temp << "\n";
		}
	}
	return 0;
}