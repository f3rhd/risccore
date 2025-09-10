#include "code_gen/ast_analyser.hpp"
#include "code_gen/code_gen.hpp"
#include "code_gen/instr_gen.hpp"
#include "parser/parser.hpp"
#include "preprocessor/preprocessor.hpp"
#ifndef STATIC_BUILD
using namespace f3_riscv_assembler;
int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <input.s> <output.bin>\n", argv[0]);
        return 1;
    }
    Preprocessor prc(argv[1]);
    Parser parser;
    parser.parse_lines(prc.process(), prc.get_labels());
    instr_gen::generator gen;
    gen.generate_instructions(parser.get_ast_nodes());
    code_gen::generate_bin_file(argv[2], gen.get_instructions());
}
#endif
