#include <fstream>
#include <vector>
#include <tuple>
#pragma once

bool tokenize(std::ifstream& file, std::vector<std::tuple<int, int>>& tokens);

