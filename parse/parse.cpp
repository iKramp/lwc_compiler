#include "parse.h"

enum class token_types{VAR, VAR_CREATE, FUNCTION_START, FUNCTION, FUNCTION_CALL, MATH, LOGIC, NUMBER, END_SPECIAL_SCOPE, IF, WHILE, RETURN};
enum class math_tokens{PLUS, MINUS, BSL, BSR};
enum class logic_tokens{ASSIGN, MORE, LESS, EQUAL, LESS_EQ, MORE_EQ, NOT_EQ};

struct Node{
    int token[2];
    std::vector<Node*> lower_nodes;
    explicit Node(std::tuple<int, int> ctoken){token[0] = std::get<0>(ctoken); token[1] = std::get<1>(ctoken);}
};

void parse(const std::vector<std::tuple<int, int>>& tokens){
    Node root = Node(std::tuple<int, int>(0, 0));
    for(auto token : tokens){
        root.lower_nodes.emplace_back(new Node(token));
    }
    for(int i = 0; i < root.lower_nodes.size(); i++){
        if(root.lower_nodes[i]->token[0] == (int)token_types::FUNCTION_START){
            int level = 0;
            Node& function = *root.lower_nodes[i];
            i++;
            level++;
            while(level > 0){
                if(root.lower_nodes[i]->token[0] == (int)token_types::IF || root.lower_nodes[i]->token[0] == (int)token_types::WHILE)
                    level++;
                if(root.lower_nodes[i]->token[0] == (int)token_types::END_SPECIAL_SCOPE) {
                    root.lower_nodes.erase(root.lower_nodes.begin() + i);
                    level--;
                }else{
                    function.lower_nodes.emplace_back(root.lower_nodes[i]);
                    root.lower_nodes.erase(root.lower_nodes.begin() + i);
                }
            }
            i--;
        }
    }
}
