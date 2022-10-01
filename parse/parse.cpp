#include <string>
#include "parse.h"
#include <iostream>
#include "fstream"


void printTree(Node& base, int indent){
    static std::ofstream output_file;
    static std::string tree = "";

    std::string line;
    for(int i = 0; i < indent - 1; i++)
        line += "   ";
    if(indent > 0) {
        line += token_type_names[base.token[0]] + " ";
        if (base.token[0] == (int) token_types::MATH)
            line += math_type_names[base.token[1]];
        else if (base.token[0] == (int) token_types::LOGIC)
            line += logic_type_names[base.token[1]];
        else if (base.token[0] == (int) token_types::SYMBOL)
            line += symbol_type_names[base.token[1]];
        else
            line += std::to_string(base.token[1]);

        tree += line + "\n";
    }
    for(auto node : base.lower_nodes)
        printTree(*node, indent + 1);

    if(indent == 0) {
        output_file.open("test.txt");
        output_file << tree;
        output_file.close();
    }
}

void parseFunction(Node& base, int pos){

};

Node& parse(const std::vector<std::tuple<token_types, int>>& tokens){
    auto root = new Node(std::tuple<token_types, int>(token_types::VAR_CREATE, 0));
    for(auto token : tokens){
        root->lower_nodes.emplace_back(new Node(token));
    }
    for(int i = 0; i < root->lower_nodes.size(); i++){
        if(root->lower_nodes[i]->token[0] == (int)token_types::FUNCTION_DEF){
            parseFunction(*root, i);
        }else{
            //throw "code cannot be written outside of functions";
        }
    }/*
    for(auto node : root->lower_nodes){
        parseBase(*node);
    }*/
    printTree(*root, 0);
    return *root;
}
