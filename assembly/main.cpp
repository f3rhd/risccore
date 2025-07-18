#include "tokenizer/tokenizer.hpp"
#include "parser/parser.hpp"
#include "common/line.hpp"
#include "code_gen/ast_analyser.hpp"
#include <iostream>
int main(int argc, char** argv){
    FILE *source_file = fopen("test.sm", "r");
    if(source_file == 0){
        printf("zort");
        return 1;
    }
    Parser parser;
    std::vector<Line> lines = utils::get_lines(source_file);
    std::vector<AST_Node*> heads = parser.parse_lines(lines);
    if(ast_analyser::analyse_ast_lines(heads) == 1)
        return 1;
    parser.resolve_identifiers(heads);
}