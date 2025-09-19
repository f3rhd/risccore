#pragma once
#include <string>
#include <cstdint>
struct type_t {
		// success is used in analysis
		enum class BASE { INT, UINT, VOID, UNKNOWN,SUCCESS} base = BASE::VOID;
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