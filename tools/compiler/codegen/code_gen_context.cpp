#include "code_gen_context.hpp"
#include <algorithm>
namespace f3_compiler {
	namespace code_gen {
		std::vector<std::string> CodeGen_Context::_registers = { "t0,", "t1","t2","t3","t4","t5","t6" };
		uint32_t CodeGen_Context::label_id = 0;
		std::vector<error_t> CodeGen_Context::_errors;
		std::string CodeGen_Context::allocate_register() {
			std::string allocated_reg = _registers.back();
			_registers.pop_back();
			return allocated_reg;
		}
		std::unordered_map<std::string,func_decl_info_t> CodeGen_Context::_func_decls;
		void CodeGen_Context::free_register(const std::string& reg) {
			if(reg != "zero" && reg != "" && (std::find(_registers.begin(),_registers.end(),reg) == _registers.end()))
				_registers.emplace_back(reg);
		}
		void CodeGen_Context::create_scope() {
			_scopes.emplace_back();
		}

		void CodeGen_Context::push_scope()
		{
			_current_scope_index++;
		}

		void CodeGen_Context::pop_scope() {
			_scopes.pop_back();
			_current_scope_index--;
		}

		void CodeGen_Context::add_var(const std::string& name,var_info_t info)
		{
			if (_scopes.empty()) {
				_scopes.emplace_back();
				_current_scope_index++;
			}
			_scopes.back().emplace(name, info);
		}

		bool CodeGen_Context::had_errors()
		{
			return _errors.size();
		}

		var_info_t CodeGen_Context::get_var_info(const std::string& name)
		{
			for (int32_t i = _current_scope_index; i >= 0; i--) {
				if (_scopes[i].find(name) != _scopes[i].end()) {
					return _scopes[i][name];
				}
			}
			_errors.emplace_back(ERROR_CODE::UNDEFINED_REFERENCE, name, "Undefined reference detected.");
			return {};
		}
		void CodeGen_Context::add_func_decl(const std::string& name, const func_decl_info_t& info)
		{
			_func_decls.emplace(name, info);
		}
		std::string CodeGen_Context::create_label(){
			return ".L" + std::to_string(label_id++);
		}

		func_decl_info_t CodeGen_Context::get_func_decl_info(const std::string& name){
			auto it = _func_decls.find(name); 
			if(it == _func_decls.end()){
				_errors.emplace_back(ERROR_CODE::UNDEFINED_REFERENCE, name, "Undefined reference detected.");
				return {};
			}
			return it->second;
		}


	}; // code_gen
} // f3_compiler
