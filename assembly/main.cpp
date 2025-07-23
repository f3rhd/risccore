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
    instr_gen::generator gen;
    gen.generate_instructions(parser.get_ast_nodes());
    code_gen::generate_bin_file(argv[2],gen.get_instructions());
    printf("Assembled successfully");
}