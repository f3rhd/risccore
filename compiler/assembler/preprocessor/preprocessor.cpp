#include "preprocessor.hpp"

#include <iostream>
#include <algorithm>
#include <sstream>
#include <fstream>
namespace riscv_assembler {

	Preprocessor::Preprocessor(const std::string& source_file_path){
		FILE* source_file = fopen(source_file_path.c_str(), "r");
		if(source_file == 0 ){
			printf("Assembler Error : Source file does not exist.");
			exit(EXIT_FAILURE);
		}
		_source_file = source_file;
	}
	std::vector<Line>& Preprocessor::process(bool debug_output)
	{
		char _line[100];
		bool macro_start = false;
		bool macro_finish = false;
		Macro macro;
		macro.arguments.reserve(3);
		macro.definition.reserve(3);
		std::string macro_name;
		while(fgets(_line,sizeof(_line),_source_file)){
			std::vector<Token> line_tokens;
			true_line_number++;
			line_tokens = tokenizer::tokenize_line_text(_line);
			if(line_tokens.size() == 0) continue;
			if(line_tokens[0].type == TOKEN_TYPE::IDENTIFIER){
				const Macro* called_macro = get_macro_by_name(line_tokens[0].word);
				if(called_macro){
					std::vector<std::string> caller_arguments = get_arguments(line_tokens,0);
					if(caller_arguments.size() != const_cast<Macro*>(called_macro)->arguments.size()){
						Line line;
						utils::throw_error_message({"Assembler Error : Macro and caller arguments did not match. Unexpected behavior is possible.", &caller_arguments.back(),&line});
					}
					std::vector<std::string> expanded_lines;
					for (uint32_t i = 0; i < called_macro->definition.size();i++){
						std::string converted_defintion = const_cast<Macro*>(called_macro)->definition[i];
						for (uint32_t j = 0; j < const_cast<Macro*>(called_macro)->arguments.size();j++){
							if(j < caller_arguments.size())
								utils::replace_in_string(converted_defintion, const_cast<Macro *>(called_macro)->arguments[j], caller_arguments[j]); 
						}
						expanded_lines.push_back(std::move(converted_defintion));
					}
					for(const auto& line_str : expanded_lines){
						std::vector<Token> tokens = tokenizer::tokenize_line_text(line_str);
						handle_line(std::move(tokens));
					}
					continue;
				}
			}
			if(!macro_start && line_tokens[0].type != TOKEN_TYPE::DIRECTIVE){
				handle_line(std::move(line_tokens));
				continue;
			}
			if(!macro_start && line_tokens[0].word == ".macro"){
				macro_name.clear();
				macro.arguments.clear();
				macro.definition.clear();
				macro_name = line_tokens[1].word;
				macro.arguments = get_arguments(line_tokens,true);
				macro_start = true;
				macro_finish = false;
				continue;
			}
			if(!macro_finish && macro_start && line_tokens[0].word == ".endm"){
				macro_start = false;
				macro_finish = true;
				_macro_map.emplace(macro_name, macro);
			}
			if(macro_start && !macro_finish){
				macro.definition.emplace_back(_line);
			}
		}
		fclose(_source_file);
		if (debug_output) {
			print_macros();
			print_lines();
		}
		return _lines;
	}
	void Preprocessor::handle_line(std::vector<Token>&& line_tokens){
		_lines.emplace_back();
		Line &line = _lines[_lines.size() - 1];
		line.tokens = line_tokens;

		line.label_str_ptr = utils::get_label_in_line(line);
		line.identifier_str_ptr = utils::get_identifier_in_line(line);
		line.true_row_number = true_line_number;

		// If the line is label-only (first token is LABEL and only one token), do not increment memory_row_number
		   
		line.memory_row_number = memory_row_number;
		if (line.tokens.size() == 1 && line.tokens[0].type == TOKEN_TYPE::LABEL) {
			_labels.emplace(*line.label_str_ptr, line.memory_row_number);
		} else {
			if(line.tokens[0].type == TOKEN_TYPE::LABEL)
				_labels.emplace(*line.label_str_ptr, line.memory_row_number);
			memory_row_number++;
		}
	}
	std::vector<std::string> Preprocessor::get_arguments(const std::vector<Token>& tokens,bool is_macro_line){
		std::vector<std::string> arguments;
		int32_t arguments_start_index = 1;
		if(is_macro_line)
			arguments_start_index = 2;
		for (uint32_t i = arguments_start_index; i < tokens.size(); i++)
		{
			if (tokens[i].word != ",")
				arguments.push_back(tokens[i].word);
		}
		return arguments;
	}
	const Macro* Preprocessor::get_macro_by_name(const std::string& macro_name){
		return _macro_map.find(macro_name) != _macro_map.end() ? &_macro_map[macro_name] : nullptr;
	}
	void Preprocessor::print_macros(){

		for (auto& it: _macro_map) {
			Macro &macro = it.second;
			std::cout << "Macro name: " << it.first << '\n';
			std::cout << "  Arguments:" << '\n';
			for (const std::string& argument : macro.arguments) {
				std::cout << "    - " << argument << '\n';
			}
			std::cout << "  Definition:" << '\n';
			for (const std::string& definition : macro.definition) {
				std::cout << "    " << definition;
				if (definition.empty() || definition.back() != '\n') std::cout << '\n';
			}
			std::cout << std::endl;
		}
	}
	void Preprocessor::print_lines(){

	   //Tokens
	   for(const Line& line : _lines){
		   std::cout << "Line " << line.true_row_number << '(' << line.memory_row_number << ')' << ':' << '\n';
		   for (const Token &token : line.tokens)
		   {
			   std::cout << '\t' << token.word;
		   }
		   std::cout << '\n';
	   }
	}
	std::unordered_map<std::string,uint32_t>& Preprocessor::get_labels(){
		return _labels;
	}
}
