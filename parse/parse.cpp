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


void fillBase(Node& base, int base_pos){
    Node& scope = *base.lower_nodes[base_pos];
    while(true){
        if(base.lower_nodes[base_pos + 1]->token[0] == (int)token_types::IF || base.lower_nodes[base_pos + 1]->token[0] == (int)token_types::WHILE)
            fillBase(base, base_pos + 1);
        if(base.lower_nodes[base_pos + 1]->token[0] == (int)token_types::STRING) {//end special scope
            base.lower_nodes.erase(base.lower_nodes.begin() + base_pos + 1);
            break;
        }
        scope.lower_nodes.emplace_back(base.lower_nodes[base_pos + 1]);
        base.lower_nodes.erase(base.lower_nodes.begin() + base_pos + 1);
    }
}

void parseFnCall(Node& base, int fn_pos){
    int arg_pos = fn_pos + 1;
    while(base.lower_nodes[arg_pos]->token[0] != (int)token_types::FUNCTION_CALL){
        base.lower_nodes[fn_pos]->lower_nodes.emplace_back(base.lower_nodes[arg_pos]);
        base.lower_nodes.erase(base.lower_nodes.begin() + arg_pos);
    }
    base.lower_nodes.erase(base.lower_nodes.begin() + arg_pos);
}

void parseMath(Node& base, int pos){
    pos += 2;
    if(base.lower_nodes[pos - 1]->token[0] == (int)token_types::STRING)//function
        parseFnCall(base, pos - 1);

    while(base.lower_nodes[pos]->token[0] == (int)token_types::MATH) {
        if(base.lower_nodes[pos + 1]->token[0] == (int)token_types::STRING)//function
            parseFnCall(base, pos + 1);

        base.lower_nodes[pos]->lower_nodes.emplace_back(base.lower_nodes[pos - 1]);
        base.lower_nodes[pos]->lower_nodes.emplace_back(base.lower_nodes[pos + 1]);
        base.lower_nodes.erase(base.lower_nodes.begin() + pos + 1);
        base.lower_nodes.erase(base.lower_nodes.begin() + pos - 1);
    }
}


void parseBase(Node& base){
    for(int i = 0; i < base.lower_nodes.size(); i++){
        if(base.lower_nodes[i]->token[0] == (int)token_types::IF || base.lower_nodes[i]->token[0] == (int)token_types::WHILE) {
            base.lower_nodes[i]->lower_nodes[1]->lower_nodes.emplace_back(base.lower_nodes[i]->lower_nodes[0]);
            base.lower_nodes[i]->lower_nodes[1]->lower_nodes.emplace_back(base.lower_nodes[i]->lower_nodes[2]);
            base.lower_nodes[i]->lower_nodes.erase(base.lower_nodes[i]->lower_nodes.begin() + 2);
            base.lower_nodes[i]->lower_nodes.erase(base.lower_nodes[i]->lower_nodes.begin());
            parseBase(*base.lower_nodes[i]);
            continue;
        }

        if(base.lower_nodes[i]->token[0] == (int)token_types::VAR_CREATE || base.lower_nodes[i]->token[0] == (int)token_types::VAR)
            if(base.lower_nodes[i + 1]->token[0] == (int)token_types::LOGIC){
                base.lower_nodes[i + 1]->lower_nodes.emplace_back(base.lower_nodes[i]);
                base.lower_nodes.erase(base.lower_nodes.begin() + i);
                parseMath(base, i);
                base.lower_nodes[i]->lower_nodes.emplace_back(base.lower_nodes[i + 1]);
                base.lower_nodes.erase(base.lower_nodes.begin() + i + 1);
                continue;
            }

        if(base.lower_nodes[i]->token[0] == (int)token_types::STRING){//function
            parseFnCall(base, i);
            continue;
        }

        if(base.lower_nodes[i]->token[0] == (int)token_types::RETURN) {
            base.lower_nodes[i]->lower_nodes.emplace_back(base.lower_nodes[i + 1]);
            base.lower_nodes.erase(base.lower_nodes.begin() + i + 1);
            continue;
        }
    }
}


Node& parse(const std::vector<std::tuple<token_types, int>>& tokens){
    auto root = new Node(std::tuple<token_types, int>(token_types::VAR_CREATE, 0));
    for(auto token : tokens){
        root->lower_nodes.emplace_back(new Node(token));
    }
    /*for(int i = 0; i < root->lower_nodes.size(); i++){
        if(root->lower_nodes[i]->token[0] == (int)token_types::FUNCTION_START){
            fillBase(*root, i);
        }
    }
    for(auto node : root->lower_nodes){
        parseBase(*node);
    }*/
    printTree(*root, 0);
    return *root;
}
