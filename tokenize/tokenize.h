#include <fstream>
#include <vector>
#include <tuple>
#pragma once

void tokenizeVar(std::string& line, std::vector<std::tuple<int, int>>& tokens, std::vector<std::tuple<std::string, int>>& var_keys, int scope);
void tokenizeFn(std::string& line, std::vector<std::tuple<int, int>>& tokens, std::vector<std::tuple<std::string, int>>& fn_keys, std::vector<std::tuple<std::string, int>>& var_keys, int scope);
void tokenize(std::ifstream& file, std::vector<std::tuple<int, int>>& tokens);
void tokenizeFnCall(std::string& line, std::vector<std::tuple<int, int>>& tokens, std::vector<std::tuple<std::string, int>>& var_keys);

