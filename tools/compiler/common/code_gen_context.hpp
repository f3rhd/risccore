#pragma  once
#include <sstream>
#include <unordered_map>
#include <memory>

namespace f3_compiler {
	struct type_t {
		enum class BASE { INT, UINT, VOID, UNKNOWN } base = BASE::INT;
		int32_t pointer_depth = 0;

		std::string str() const {
			auto _pointer_depth = [&]() -> std::string {
				std::string str;
				for (int i = 0; i < pointer_depth; i++)
					str += '*';
				return str;
			};
			switch (base) {
			case BASE::INT: return "int" + _pointer_depth();
			case BASE::UINT: return "uint" + _pointer_depth() ;
			case BASE::VOID: return "void" + _pointer_depth() ;
			default: return "int" + _pointer_depth();
			}
		}

	};
	//namespace code_gen {
	//	struct func_info_t {
	//		type_t return_type;
	//		std::vector<type_t> param_types;
	//		func_info_t(std::string&& name_,std::vector<type_t>&& param_types_, type_t return_type_) : param_types(std::move(param_types_)), return_type(return_type_) {}
	//	};
	//	struct var_info_t {
	//		int32_t offset; // offset from current frame pointer
	//		type_t type;
	//		var_info_t(type_t type_, int32_t offset_) : type(type_), offset(offset_) {}
	//	};
	//	using scope_t = std::unordered_map<std::string, var_info_t>;
	//	struct code_gen_ctx_t {
	//		std::ostringstream  code;

	//		std::vector<scope_t> scopes;
	//		std::unordered_map<std::string, func_info_t> functions;

	//		std::vector<std::string> registers = { "t0,","t1","t2","t3","t4","t5","t6" };
	//		std::string alloc_register() {
	//			if (registers.empty()) {
	//				throw std::runtime_error("Ran out of registers.");
	//			}
	//			std::string alloc_reg = std::move(registers.back());
	//			registers.pop_back();
	//			return alloc_reg;
	//		}
	//		void free_register(const std::string& reg_name) {
	//			registers.emplace_back(reg_name);
	//		}

	//		uint32_t frame_size = 0;
	//		void push_scope() { scopes.emplace_back();};
	//		void pop_scope() { scopes.pop_back();}

	//		void add_var(const std::string& name, type_t var_info) {
	//			scopes.back().emplace(name, var_info); //  will be used in var decl statement
	//		}
	//		var_info_t* look_up_var(const std::string& name) {
	//			for (int i = scopes.size() - 1; i >= 0; i--) { // start from current scope 
	//				if (scopes[i].find(name) != scopes[i].end()) {
	//					return &(scopes[i][name]);
	//				}
	//			}
	//			return nullptr;
	//		}

	//	/*
	//		func main() -> void {
	//			int c = 3;
	//			int x = c + 5;
	//			while(x<10) {
	//				int x;
	//				x+=c;
	//				int y;
	//			}
	//			int y = x + 6;
	//		}
	//	*/

	//	};
	//}

	
}