#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
using namespace  f3_compiler;
int main(int argc, char** argv) {
    //if (argc < 3) {
    //    fprintf(stderr, "Usage: %s <input.s> <output.bin>\n", argv[0]);
    //    return 1;
    //}
	Lexer lexer("tests/general_test.f3");
	Parser parser(std::move(const_cast<std::vector<token_t>&>(lexer.get_tokens())));
	auto func = parser.parse_function();
	if (parser.no_error()) {
		func->print_ast(std::cout);
	}
	else {
		exit(EXIT_FAILURE);
	}
	return 0;
}