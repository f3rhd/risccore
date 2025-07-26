#include <string>
#include <regex>
#include <unordered_set>
#include <cctype>
#include "tokenizer.hpp"
namespace tokenizer {

    std::vector<Token> tokenize_line_text(const std::string& line_raw) {
        std::vector<Token> tokens;
        tokens.reserve(8*sizeof(Token));

        uint64_t comment_pos = line_raw.find('#');
        std::string line = (comment_pos != std::string::npos) ? line_raw.substr(0, comment_pos) : line_raw;

        size_t i = 0;
        while (i < line.size()) {
            char ch = line[i];

            // Skip whitespace
            if (std::isspace(static_cast<unsigned char>(ch))) {
                ++i;
                continue;
            }

            // Handle single-char tokens
            if (ch == ',' || ch == '(' || ch == ')') {
                TOKEN_TYPE type;
                if (ch == ',') type = TOKEN_TYPE::COMMA;
                else if (ch == '(') type = TOKEN_TYPE::LPAREN;
                else type = TOKEN_TYPE::RPAREN;

                tokens.emplace_back(std::string(1, ch), type);
                ++i;
                continue;
            }

            // Match [a-zA-Z0-9_.:-]+
            size_t start = i;
            while (i < line.size() && (std::isalnum(static_cast<unsigned char>(line[i])) || line[i] == '_' || line[i] == '.' || line[i] == '-' || line[i] == ':')) {
                ++i;
            }

            if (start == i)
                continue;

            std::string token = line.substr(start, i - start);

            // Handle label: `label:`
            if (!token.empty() && token.back() == ':') {
                tokens.emplace_back(token.substr(0, token.length() - 1), TOKEN_TYPE::LABEL);
                continue;
            }

            // Determine token type
            if (instruction_look_up::r_type_ops.count(token) ||
                instruction_look_up::i_type_ops.count(token) ||
                instruction_look_up::s_type_ops.count(token) ||
                instruction_look_up::b_type_ops.count(token) ||
                instruction_look_up::j_type_ops.count(token) ||
                instruction_look_up::u_type_ops.count(token) ||
                instruction_look_up::pseudo_ops.count(token)) {
                tokens.emplace_back(token, TOKEN_TYPE::OPERATION);
            }
            else if (instruction_look_up::is_register(token)) {
                tokens.emplace_back(token, TOKEN_TYPE::REGISTER);
            }
            else if (instruction_look_up::is_immediate(token)) {
                tokens.emplace_back(token, TOKEN_TYPE::IMMEDIATE);
            }
            else if (!token.empty() && token[0] == '.') {
                tokens.emplace_back(token, TOKEN_TYPE::DIRECTIVE);
            }
            else {
                tokens.emplace_back(token, TOKEN_TYPE::IDENTIFIER);
            }
        }

        return tokens;
    }

} // namespace tokenizer
