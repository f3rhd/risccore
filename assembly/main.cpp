#include <iostream>

#include "parser/parser.hpp"
#include "code_gen/ast_analyser.hpp"
#include "code_gen/instr_gen.hpp"
#include "code_gen/code_gen.hpp"

int main(int argc, char** argv){
    FILE *source_file = fopen(argv[1], "r");
    if(source_file == 0){
        printf("Source file does not exist.");
        return 1;
    }
    Parser parser;
    parser.run(source_file);
    if(ast_analyser::analyse_ast_lines(parser.get_ast_nodes()) == 1) 
        return 1;
    instr_gen::generator gen;
    gen.generate_instructions(parser.get_ast_nodes());
    code_gen::generate_bin_file(argv[2],gen.get_instructions());

    #ifdef PRINT_INSTR
    for(Instruction& instr : instructions){
        printf("opcode: %u, funct3: %u, funct7: %u, rd: %u, rs1: %u, rs2: %u, imm: %d\n",
            instr.opcode,
            instr.func3,
            instr.func7,
            instr.rd,
            instr.rs1,
            instr.rs2,
            instr.imm);
    }
    #endif
    printf("Assembled successfully");
}