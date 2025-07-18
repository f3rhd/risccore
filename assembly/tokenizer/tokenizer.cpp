#include <string>
#include <regex>
#include <unordered_set>
#include <cctype>
#include "tokenizer.hpp"
namespace tokenizer{
    std::vector<Token> tokenize_line_text(const std::string& line_raw){
        std::vector<Token> tokens;

        std::string line = line_raw.substr(0, line_raw.find('#'));

        std::regex pattern(R"([\w\.\-:]+|[,()])");

        auto words_begin = std::sregex_iterator(line.begin(), line.end(), pattern);
        auto words_end = std::sregex_iterator();

        for (std::sregex_iterator i = words_begin; i != words_end;i++){
            std::string token = i->str();

            if(token.back() == ':'){
                tokens.push_back({token.substr(0, token.length() - 1), TOKEN_TYPE::LABEL});
            }
            else if(instruction_look_up::r_type_ops.count(token) || 
                    instruction_look_up::i_type_ops.count(token) ||
                    instruction_look_up::s_type_ops.count(token) ||
                    instruction_look_up::b_type_ops.count(token) ||
                    instruction_look_up::j_type_ops.count(token) ||
                    instruction_look_up::u_type_ops.count(token) || 
                    instruction_look_up::pseudo_ops.count(token)){
                tokens.push_back({token, TOKEN_TYPE::OPERATION});
            }
            else if(instruction_look_up::is_register(token)){
                tokens.push_back({token, TOKEN_TYPE::REGISTER});
            }
            else if(instruction_look_up::is_immediate(token)){
                tokens.push_back({token, TOKEN_TYPE::IMMEDIATE});
            }
            else if(token == ","){
                tokens.push_back({token, TOKEN_TYPE::COMMA});
            }
            else if(token == "("){
                tokens.push_back({token, TOKEN_TYPE::LPAREN});
            }
            else if(token == ")"){
                tokens.push_back({token, TOKEN_TYPE::RPAREN});
            }
            else if(token[0] == '.'){
                tokens.push_back({token, TOKEN_TYPE::DIRECTIVE});
            }
            else {
                tokens.push_back({token, TOKEN_TYPE::IDENTIFIER});
            }
        }
        return tokens;
    }
};