#pragma once
#include <vector>
#include <tuple>

struct Node{
    int token[2];
    std::vector<Node*> lower_nodes;
    explicit Node(std::tuple<int, int> ctoken){token[0] = std::get<0>(ctoken); token[1] = std::get<1>(ctoken);}
};

Node& parse(const std::vector<std::tuple<int, int>>& tokens);
