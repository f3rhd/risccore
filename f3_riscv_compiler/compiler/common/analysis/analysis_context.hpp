#pragma once
#include <string>
#include "../other/f3_type.hpp"
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
        type_t* return_type = nullptr;
        std::vector<func_decl_param_t>* arguments = nullptr;
    };
    struct Analysis_Context{
        type_t get_var_type(const std::string& var_id){

            for (auto it = _scopes.rbegin(); it < _scopes.rend();it++){
                auto &scope = *it;
                if(scope.find(var_id) != scope.end()){
                    return scope[var_id];
                }
            }
            return {type_t::BASE::UNKNOWN, 0};
        }
        void push_stack() { _scopes.emplace_back();}
        void poop_stack() {_scopes.pop_back();}
        analysis_func_decl_info_t* get_func_decl_info(const std::string& id){
            if(_func_decls.find(id) == _func_decls.end()){
                nullptr;
            }
            return &_func_decls[id];
        }

    private:
        std::vector<std::unordered_map<std::string, type_t>> _scopes;
        std::unordered_map<std::string, analysis_func_decl_info_t> _func_decls;
    };

}