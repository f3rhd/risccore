
#include "lexer/lexer.hpp"
#include "parser/parser.hpp"

#include "../assembler/code_gen/ast_analyser.hpp"
#include "../assembler/code_gen/code_gen.hpp"
#include "../assembler/code_gen/instr_gen.hpp"
#include "../assembler/parser/parser.hpp"
#include "../assembler/preprocessor/preprocessor.hpp"
using namespace  f3_compiler;
int main(int argc, char** argv) {
	if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <input.f3> [--print-ast] [--emit-asm <file>] [--emit-bin <file>]\n";
        return 1;
		}
		const char* input_file = argv[1];
		bool print_ast = false;
		const char* asm_file = nullptr;
		const char* bin_file = nullptr;
		for (int i = 2; i < argc; i++) {
			if (strcmp(argv[i], "--print-ast") == 0) {
				print_ast = true;
			} else if (strcmp(argv[i], "--emit-asm") == 0 && i + 1 < argc) {
				asm_file = argv[++i];
			} else if (strcmp(argv[i], "--emit-bin") == 0 && i + 1 < argc) {
				bin_file = argv[++i];
			}
		}
		Lexer lexer(input_file);
		Parser parser(std::move(const_cast<std::vector<token_t>&>(lexer.get_tokens())));
		if (!parser.no_error()) {
			exit(EXIT_FAILURE);
		}
	auto program = parser.parse_program();
	if (print_ast) {
		program.print_ast();
	}

	std::ostringstream asm_stream;
	program.generate(asm_stream);
	if (program.has_error()) {
		exit(EXIT_FAILURE);
	}
	std::string asm_code = asm_stream.str();

	if (asm_file) {
		std::ofstream ofs(asm_file);
		ofs << asm_code;
	}

	if (bin_file) {

		std::ofstream ofs("program.s");
		ofs << asm_code;
		ofs.close();
		f3_riscv_assembler::Preprocessor asm_prc("program.s");
		f3_riscv_assembler::Parser asm_parser;
		asm_parser.parse_lines(asm_prc.process(), asm_prc.get_labels());
		f3_riscv_assembler::instr_gen::generator gen;
		gen.generate_instructions(asm_parser.get_ast_nodes());
		f3_riscv_assembler::code_gen::generate_bin_file(bin_file, gen.get_instructions());
		std::remove("program.s");
	}
	return 0;
}