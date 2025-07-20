#pragma once

#include "line.hpp"

#include <string>
#include <iostream>
struct Error_Message{

    std::string message;
    const std::string* error_causing_word;
    Line *error_causing_line;
};

