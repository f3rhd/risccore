#pragma once
#include <string>
#include <cstdint>
auto constexpr ARRAY_NOT_INITIALIZED = -1;
auto constexpr ARRAY_SIZE_IMPLICIT = -2;
struct type_t {
		// success is used in analysis
		enum class base { INT, VOID, UNKNOWN,SUCCESS} base = base::UNKNOWN;
		int32_t pointer_depth = 0;
		int32_t array_size = ARRAY_NOT_INITIALIZED;
		std::string str() const {
			auto _pointer_depth = [&]() -> std::string {
				std::string str;
				for (int i = 0; i < pointer_depth; i++)
					str += '*';
				return str;
			};
			switch (base) {
			case base::INT: return "int" + _pointer_depth();
			case base::VOID: return "void" + _pointer_depth() ;
			default: return "int" + _pointer_depth();
			}
		}

	};