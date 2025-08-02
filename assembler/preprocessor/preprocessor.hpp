#pragma once
#include <iostream>
#include <vector>

#include "../common/macro.hpp"
#include "../tokenizer/tokenizer.hpp"
#include <unordered_map>    


class Preprocessor{


    
public:
    Preprocessor(const std::string& source_file_path);
    std::vector<Line>& process();
    std::unordered_map<std::string,size_t>& get_labels();
private:
    const Macro* get_macro_by_name(const std::string &macro_name);
    std::vector<std::string> get_arguments(const std::vector<Token> &tokens, bool is_macro_line);
    void handle_line(std::vector<Token>&& line_tokens);
    void print_lines();
private:
    uint32_t true_line_number = 0;
    uint32_t memory_row_number = 1; // instruction address
    FILE *_source_file;
    std::vector<Line> _lines;
    std::unordered_map<std::string, Macro> _macro_map;
    std::unordered_map<std::string,uint64_t> _labels;
    void print_macros();
};
