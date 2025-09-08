#pragma  once
#include <sstream>
#include <unordered_map>
#include <memory>
#include "../common/error.hpp"
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
	namespace code_gen {

		namespace flags {
			auto constexpr ALLOCATE_VAR_IN_REG       = 1 << 0; 
			auto constexpr IF_CONDITIONAL_EXPR  = 1 << 1; 
			auto constexpr FOR_RANGE_PHASE_1 = 1 << 2; 
			auto constexpr FOR_RANGE_PHASE_2 = 1 << 3; 
			auto constexpr FOR_RANGE_PHASE_3 = 1 << 4; 
			auto constexpr FOR_CONTROL_KEYWORD = 1 << 5;
			auto constexpr WHILE_CONTROL_KEYWORD = 1 << 6;
		}
		#define HAS_FLAG(f, flag) (((f) & (flag)) != 0)
		struct var_info_t {
			int32_t offset = 0;
			type_t type;
		};
		struct func_decl_info_t {
			std::vector<var_info_t> parameters;
			type_t return_type;
		};
		struct CodeGen_Context {

		public:
			int32_t offset_counter = 0; // logical offset counter
			int32_t frame_size = 0;
			static uint32_t label_id;

		public:
			static void free_register(const std::string& reg);
			static std::string allocate_register();
			static void add_func_decl(const std::string& name, const func_decl_info_t& info);
			static std::string create_label();
			std::vector<std::string> skip_jump_labels;
			std::vector<std::string> break_jump_labels;
			std::vector<std::string> return_jump_labels;

		public:
			void create_scope();
			void push_scope();
			void pop_scope();
			void add_var(const std::string& name,var_info_t info);
			bool had_errors();
			var_info_t get_var_info(const std::string& name);
			static func_decl_info_t get_func_decl_info(const std::string &name);

		private:
			static std::vector<std::string> _registers; ;
			static std::unordered_map<std::string,func_decl_info_t> _func_decls;
			std::vector<std::unordered_map<std::string, var_info_t>> _scopes;
			static std::vector<error_t> _errors;
			int32_t _current_scope_index = -1;
		};
	}
}