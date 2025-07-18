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
    //Parsing
    Parser parser;
    std::vector<Line> lines = parser.get_lines(source_file);
    std::vector<AST_Node*> heads = parser.parse_lines(lines);
    parser.set_labels(lines);
    #ifdef DEBUG
        parser.print_labels();
    #endif
    parser.resolve_identifiers(heads);
    //Ast analysis
    if(ast_analyser::analyse_ast_lines(heads) == 1) 
        return 1;
    // Code generation
    std::vector<Instruction> instructions = instr_gen::generate_instructions(heads);
    code_gen::generate_bin_file(argv[2], instructions);

    #ifdef DEBUG
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