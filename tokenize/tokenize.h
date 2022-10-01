#include <fstream>
#include <vector>
#include <tuple>
#pragma once

enum class token_types{VAR_CREATE, FUNCTION_DEF, RETURN, IF, WHILE, VAR, FUNCTION_CALL, MATH, LOGIC, NUMBER, POP, SYMBOL, STRING, NUM_TYPES};
enum class math_tokens{PLUS, MINUS, BSL, BSR, OR, XOR, AND, NOT, NUM_TYPES};
enum class logic_tokens{ASSIGN, NOT_EQ, LESS_EQ, LESS, MORE_EQ, MORE, EQUAL, NUM_TYPES};
enum class symbol_tokens{BRACKET_L, BRACKET_R, SCOPE_BRACKET_L, SCOPE_BRACKET_R, NUM_TYPES};
const std::string token_type_names [13] = {"var_create", "func_def", "return", "if", "while", "var", "func_call", "math", "logic", "number", "pop", "symbol", "string"};
const std::string math_type_names  [9 ] = {"empty", "plus", "minus", "bsl", "bsr", "or", "xor", "and", "not"};
const std::string logic_type_names [7 ] = {"assign", "not_eq", "less_eq", "less", "more_eq", "more", "equal"};
const std::string symbol_type_names[4 ] = {"(", ")", "{", "}"};


void tokenize(std::ifstream& file, std::vector<std::tuple<token_types, int>>& tokens);

