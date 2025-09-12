#include "program.hpp"
namespace f3_compiler {
	void Program::generate_IR() {

		IR_Gen_Context ctx;
		for (auto& func : _functions) {
			func->generate_ir(ctx);
		}
		instructions = std::move(ctx.instructions);
	}

	void Program::print_IR(std::ostream& os){

		for(auto& instr : instructions){
			os << instr.to_string() << '\n';
		}
	}
	bool Program::has_error() const
	{
		return had_error;
	}

	void Program::print_ast()
	{
		for (auto& func : _functions) {
			func->print_ast(std::cout);
		}
	}

}