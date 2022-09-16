#include <fstream>
#include <vector>
#include <tuple>
#pragma once

void tokenizeVar(std::string& line, std::vector<std::tuple<int, int>>& tokens, std::vector<std::string>& var_keys, int scope);
void tokenize(std::ifstream& file, std::vector<std::tuple<int, int>>& tokens);
