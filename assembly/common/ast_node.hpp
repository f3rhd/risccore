#pragma once
#include "look_up.hpp"
#include <string>
enum class AST_NODE_TYPE
{
    INSTRUCTION,
    REGISTER,
    IMMEDIATE,
    LABEL,
    DIRECTIVE,
    IDENTIFIER
};
struct AST_Node{

    std::string identifier;

    instruction_look_up::INSTRUCTION_TYPE instr_type;
    AST_NODE_TYPE node_type;
    AST_Node *left = nullptr;
    AST_Node *right = nullptr;
    AST_Node *middle = nullptr;

};
