#pragma once
#include <iostream>
#include <vector>

#include "../common/macro.hpp"
#include "../tokenizer/tokenizer.hpp"
#include <unordered_map>    


class Preprocessor{


    
public:
    Preprocessor(const std::string& source_file_path);
    std::string process();
    void print_macros();

private:
    FILE *_source_file;
    const Macro* get_macro_by_name(const std::string &macro_name);
    std::unordered_map<std::string, Macro> _macro_map;
    std::vector<std::string> get_arguments(const std::vector<Token> &tokens, bool is_macro_line);
};
