#include "preprocessor.hpp"

#include <iostream>
#include <algorithm>
Preprocessor::Preprocessor(const std::string& source_file_path){

    FILE* source_file = fopen(source_file_path.c_str(), "r");
    if(source_file == 0 ){
        printf("Source file does not exist.");
        exit(EXIT_FAILURE);
    }
    _source_file = source_file;
}
std::string Preprocessor::process()
{
    FILE *processed_file = fopen("processed.s", "wb");
    char _line[500];
    bool macro_start = false;
    bool macro_finish = false;
    Macro macro;
    uint32_t line_counter = 0;
    std::string macro_name;
    while(fgets(_line,sizeof(_line),_source_file) != 0){
        line_counter++;
        std::vector<Token> line_tokens = tokenizer::tokenize_line_text(_line);
        if(line_tokens.size() == 0) continue;
        if(line_tokens[0].type == TOKEN_TYPE::IDENTIFIER){

            const Macro* called_macro = get_macro_by_name(line_tokens[0].word);
            if(called_macro){
                std::vector<std::string> caller_arguments = get_arguments(line_tokens,0);
                if(caller_arguments.size() != const_cast<Macro*>(called_macro)->arguments.size()){
                    Line line;
                    line.tokens = line_tokens;
                    line.true_row_number = line_counter;
                    utils::throw_error_message({"Macro and caller arguments did not match. Unexpected behavior is possible.", &caller_arguments.back(),&line});
                }
                for (uint32_t i = 0; i < called_macro->definition.size();i++){
                    std::string converted_defintion = const_cast<Macro*>(called_macro)->definition[i];
                    for (uint32_t j = 0; j < const_cast<Macro*>(called_macro)->arguments.size();j++){

                        if(j < caller_arguments.size())
                            replace_in_string(converted_defintion, const_cast<Macro *>(called_macro)->arguments[j], caller_arguments[j]); 
                    }
                    fprintf(processed_file, "%s", converted_defintion.c_str());
                }
                continue;
            }
        }
        if(!macro_start && line_tokens[0].type != TOKEN_TYPE::DIRECTIVE){
            fprintf(processed_file, "%s", _line);
            continue;
        }
        if(!macro_start && line_tokens[0].word == ".macro"){

            macro_name.clear();
            macro.arguments.clear();
            macro.definition.clear();
            macro_name = line_tokens[1].word;
            // means that this is macro definition line
            macro.arguments = get_arguments(line_tokens,true);
            macro_start = true;
            macro_finish = false;
            // means that print the macro beginning
            fprintf(processed_file, "%s", _line);
            continue;
        }
        if(!macro_finish && macro_start &&line_tokens[0].word == ".endm"){

            macro_start = false;
            macro_finish = true;
            _macro_map.insert({macro_name, macro});
        }
        if(macro_start && !macro_finish){
            macro.definition.emplace_back(_line);
        }
        fprintf(processed_file, "%s", _line);
    }
    fclose(_source_file);
    fclose(processed_file);
    #ifdef PRINT_MACROS
        print_macros();
    #endif
    return "processed.s";
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
void Preprocessor::replace_in_string(std::string &_str, const std::string &from, const std::string &to){
    size_t start_pos = 0;
    while((start_pos = _str.find(from, start_pos)) != std::string::npos) {
        _str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Move past the replaced part
    }
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