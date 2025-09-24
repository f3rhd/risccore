#pragma once
#include <string>
#include "../other/f3_type.hpp"
#include "../other/error.hpp"
#include <unordered_map>
#include <vector>
namespace f3_compiler {
    struct func_decl_param_t {
        std::string name;
        type_t type;

        func_decl_param_t(type_t typ, std::string&& id) : name(std::move(id)), type(typ) {}
        void print_ast(std::ostream& os, uint32_t indent_level = 0, bool is_last = true) const;
    };
	struct analysis_func_decl_info_t{

		// These two are pointers as they are gonna point to the main the data of the  func_decl_t node
		const type_t* return_type = nullptr;
		const std::vector<func_decl_param_t>* arguments = nullptr;
	};
	struct Analysis_Context{
		bool in_loop = false;
		bool has_return = false;
		std::string current_func_id;
		type_t get_var_type(const std::string& var_id){

			for (auto it = _scopes.rbegin(); it < _scopes.rend();it++){
				auto &scope = *it;
				if(scope.find(var_id) != scope.end()){
					return scope[var_id];
				}
			}
			return {type_t::BASE::UNKNOWN, 0};
		}
		void push_scope() { _scopes.emplace_back();} 
		void pop_scope() {_scopes.pop_back();}
		void push_func_decl(const std::string &func_id, const analysis_func_decl_info_t &info) { _func_decls[func_id] = info; }
		void add_var(const std::string &var_id,type_t type) { _scopes.back().emplace(var_id,type);}
		void make_error(ERROR_CODE err_code,const std::string &cause,const std::string& msg){
			_errors.emplace_back(err_code, cause, msg);
		}
		analysis_func_decl_info_t* get_func_decl_info(const std::string& id){ 
			if(_func_decls.find(id) == _func_decls.end()){
				return nullptr;
			}
			return &_func_decls[id];
		}
		void reset(){
			_scopes.clear();
		}

		bool has_func_def(const std::string id) {
			return _func_decls.find(id) != _func_decls.end();
		}
		std::vector<error_t>& get_errors(){
			return _errors;
		};
	private:
		std::vector<std::unordered_map<std::string, type_t>> _scopes;
		std::unordered_map<std::string, analysis_func_decl_info_t> _func_decls;
		std::vector<error_t> _errors;
	};

}