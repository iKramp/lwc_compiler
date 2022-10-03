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

void groupBracket(Node& base, int pos){
    bool scope = base.lower_nodes[pos]->token[1] == (int)symbol_tokens::SCOPE_BRACKET_L;
    pos++;
    while(true){
        if(base.lower_nodes[pos]->token[0] == (int)token_types::SYMBOL && base.lower_nodes[pos]->token[1] == (int)symbol_tokens::BRACKET_R && !scope)
            break;
        if(base.lower_nodes[pos]->token[0] == (int)token_types::SYMBOL && base.lower_nodes[pos]->token[1] == (int)symbol_tokens::SCOPE_BRACKET_R && scope)
            break;
        if(base.lower_nodes[pos]->token[0] == (int)token_types::SYMBOL &&
           (base.lower_nodes[pos]->token[1] == (int)symbol_tokens::SCOPE_BRACKET_L ||
            base.lower_nodes[pos]->token[1] == (int)symbol_tokens::BRACKET_L)){
            groupBracket(base, pos);
        }
        base.lower_nodes[pos - 1]->lower_nodes.emplace_back(base.lower_nodes[pos]);
        base.lower_nodes.erase(base.lower_nodes.begin() + pos, base.lower_nodes.begin() + pos + 1);
    }
    base.lower_nodes.erase(base.lower_nodes.begin() + pos, base.lower_nodes.begin() + pos + 1);
}

void groupBrackets(Node& base){
    for(int i = 0; i < base.lower_nodes.size(); i++){
        if(base.lower_nodes[i]->token[0] == (int)token_types::SYMBOL &&
        (base.lower_nodes[i]->token[1] == (int)symbol_tokens::SCOPE_BRACKET_L ||
        base.lower_nodes[i]->token[1] == (int)symbol_tokens::BRACKET_L)){
            groupBracket(base, i);
        }
    }
}

void parseFunction(Node& base, int pos){
    pos++;
    if(base.lower_nodes[pos]->token[0] != (int)token_types::SYMBOL || base.lower_nodes[pos]->token[1] != (int)symbol_tokens::SCOPE_BRACKET_L)
        throw "{ expected after ) in function definition";
    base.lower_nodes.erase(base.lower_nodes.begin() + pos, base.lower_nodes.begin() + pos + 1);
    int scope_depth = 1;
    while(true){
        if(base.lower_nodes[pos]->token[0] == (int)token_types::SYMBOL && base.lower_nodes[pos]->token[1] == (int)symbol_tokens::SCOPE_BRACKET_L)
            scope_depth++;
        if(base.lower_nodes[pos]->token[0] == (int)token_types::SYMBOL && base.lower_nodes[pos]->token[1] == (int)symbol_tokens::SCOPE_BRACKET_R) {
            scope_depth--;
            if(!scope_depth) {
                base.lower_nodes.erase(base.lower_nodes.begin() + pos, base.lower_nodes.begin() + pos + 1);
                break;
            }
        }
        base.lower_nodes[pos - 1]->lower_nodes.emplace_back(base.lower_nodes[pos]);
        base.lower_nodes.erase(base.lower_nodes.begin() + pos, base.lower_nodes.begin() + pos + 1);
    }
    groupBrackets(*base.lower_nodes[pos - 1]);
}

Node& parse(const std::vector<std::tuple<token_types, int>>& tokens){
    auto root = new Node(std::tuple<token_types, int>(token_types::VAR_CREATE, 0));
    for(auto token : tokens){
        root->lower_nodes.emplace_back(new Node(token));
    }
    for(int i = 0; i < root->lower_nodes.size(); i++){
        if(root->lower_nodes[i]->token[0] == (int)token_types::FUNCTION_DEF){
            parseFunction(*root, i);
        }else{
            throw "code cannot be written outside of functions";
        }
    }/*
    for(auto node : root->lower_nodes){
        parseBase(*node);
    }*/
    printTree(*root, 0);
    return *root;
}
