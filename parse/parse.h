#pragma once
#include <vector>
#include <tuple>
#include "../tokenize/tokenize.h"

struct Node{
    int token[2] = {0, 0};
    std::vector<Node*> lower_nodes;
    explicit Node(std::tuple<token_types, int> ctoken){token[0] = (int)std::get<0>(ctoken); token[1] = std::get<1>(ctoken);}
};

Node& parse(const std::vector<std::tuple<token_types, int>>& tokens);
