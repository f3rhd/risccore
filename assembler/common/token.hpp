#pragma once
#include <string>
enum class TOKEN_TYPE {
    OPERATION,
    REGISTER,
    IMMEDIATE,
    LABEL,
    DIRECTIVE,
    COMMA,
    LPAREN,
    RPAREN,
    IDENTIFIER
};
struct Token {
    TOKEN_TYPE type;
    std::string word;
    Token(const std::string& _word, const TOKEN_TYPE _type) : word(_word), type(_type) {}
};
