#include "tokenizer/tokenizer.hpp"
#include "parser/parser.hpp"
#include "common/line.hpp"
#include <iostream>
int main(int argc, char** argv){
    FILE *source_file = fopen("test.sm", "r");

    if(source_file == 0){
        printf("zort");
        return 1;
    }
    std::vector<Line> lines = utils::get_lines(source_file);

    for(Line& line : lines){

        for(Token& token : line.tokens){

            std::cout << token.word << std::endl;
        }
    }
}