#include <string>
#include "parse.h"
#include <iostream>
#include "fstream"


bool isToken(Node& node, math_tokens token){
    return node.token[0] == (int)token_types::MATH && node.token[1] == (int)token;
}

bool isToken(Node& node, logic_tokens token){
    return node.token[0] == (int)token_types::LOGIC && node.token[1] == (int)token;
}

bool isToken(Node& node, symbol_tokens token){
    return node.token[0] == (int)token_types::SYMBOL && node.token[1] == (int)token;
}

bool isToken(Node& node, token_types token){
    return node.token[0] == (int)token;
}

std::string getTokenName(Node& token){
    if(isToken(token, token_types::MATH))
        return math_type_names[token.token[1]];
    if(isToken(token, token_types::LOGIC))
        return logic_type_names[token.token[1]];
    if(isToken(token, token_types::SYMBOL))
        return symbol_type_names[token.token[1]];
    return token_type_names[token.token[0]];
}

bool isAcceptableForOperation(Node& base, int stage){
    if(stage >= 0)
        if(isToken(base, token_types::FUNCTION_CALL) ||
           isToken(base, token_types::NUMBER) ||
           isToken(base, token_types::VAR) ||
           isToken(base, symbol_tokens::BRACKET_L))
            return true;
    if(stage >= 1)
        if(isToken(base, math_tokens::NOT))
            return true;
    if(stage >= 2)
        if(isToken(base, math_tokens::PLUS) ||
        isToken(base, math_tokens::MINUS))
            return true;
    if(stage >= 3)
        if(isToken(base, math_tokens::BSL) ||
           isToken(base, math_tokens::BSR))
            return true;
    if(stage >= 4)
        if(isToken(base, logic_tokens::LESS_EQ) ||
           isToken(base, logic_tokens::LESS) ||
           isToken(base, logic_tokens::MORE_EQ) ||
           isToken(base, logic_tokens::MORE) ||
           isToken(base, logic_tokens::EQUAL) ||
           isToken(base, logic_tokens::NOT_EQ))
            return true;
    if(stage >= 5)
        if(isToken(base, math_tokens::AND) ||
           isToken(base, math_tokens::XOR) ||
           isToken(base, math_tokens::OR))
            return true;
    return false;
}

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

void parseFnCalls(Node& base){
    for(int pos = 0; pos < base.lower_nodes.size(); pos++) {
        if(isToken(*base.lower_nodes[pos], token_types::FUNCTION_CALL)) {
            while(!base.lower_nodes[pos + 1]->lower_nodes.empty()){
                base.lower_nodes[pos]->lower_nodes.emplace_back(base.lower_nodes[pos + 1]->lower_nodes[0]);
                base.lower_nodes[pos + 1]->lower_nodes.erase(base.lower_nodes[pos + 1]->lower_nodes.begin(), base.lower_nodes[pos + 1]->lower_nodes.begin() + 1);
            }
            base.lower_nodes.erase(base.lower_nodes.begin() + pos + 1, base.lower_nodes.begin() + pos + 2);
        }
    }
}

void groupBracket(Node& base, int pos){
    bool scope = base.lower_nodes[pos]->token[1] == (int)symbol_tokens::SCOPE_BRACKET_L;
    pos++;
    while(true){
        if(isToken(*base.lower_nodes[pos], symbol_tokens::COMMA)){
            base.lower_nodes.erase(base.lower_nodes.begin() + pos, base.lower_nodes.begin() + pos + 1);
        }
        if(isToken(*base.lower_nodes[pos], symbol_tokens::BRACKET_R) && !scope)
            break;
        if(isToken(*base.lower_nodes[pos], symbol_tokens::SCOPE_BRACKET_R) && scope)
            break;
        if(isToken(*base.lower_nodes[pos], symbol_tokens::SCOPE_BRACKET_L) || isToken(*base.lower_nodes[pos], symbol_tokens::BRACKET_L)){
            groupBracket(base, pos);
        }
        base.lower_nodes[pos - 1]->lower_nodes.emplace_back(base.lower_nodes[pos]);
        base.lower_nodes.erase(base.lower_nodes.begin() + pos, base.lower_nodes.begin() + pos + 1);
    }
    base.lower_nodes.erase(base.lower_nodes.begin() + pos, base.lower_nodes.begin() + pos + 1);
    parseFnCalls(*base.lower_nodes[pos - 1]);
}

void groupBrackets(Node& base){
    for(int i = 0; i < base.lower_nodes.size(); i++){
        if(isToken(*base.lower_nodes[i], symbol_tokens::SCOPE_BRACKET_L) || isToken(*base.lower_nodes[i], symbol_tokens::BRACKET_L)){
            groupBracket(base, i);
        }
    }
}

void parseMathLogicToken(Node& base, int pos, int stage){
    if(!isAcceptableForOperation(*base.lower_nodes[pos + 1], stage)){
        throw "function, variable, number or expression in () expected after operator: " + getTokenName(*base.lower_nodes[pos]);
    }
    if(!isAcceptableForOperation(*base.lower_nodes[pos - 1], stage + 1)){
        throw "function, variable, number or expression in () expected before operator: " + getTokenName(*base.lower_nodes[pos]);
    }
    base.lower_nodes[pos]->lower_nodes.emplace_back(base.lower_nodes[pos + 1]);
    base.lower_nodes.erase(base.lower_nodes.begin() + pos + 1, base.lower_nodes.begin() + pos + 2);

    base.lower_nodes[pos]->lower_nodes.emplace_back(base.lower_nodes[pos - 1]);
    base.lower_nodes.erase(base.lower_nodes.begin() + pos - 1, base.lower_nodes.begin() + pos);
}

void parseNot(Node& base, int pos){
    if(isAcceptableForOperation(*base.lower_nodes[pos + 1], 0)) {
        base.lower_nodes[pos]->lower_nodes.emplace_back(base.lower_nodes[pos + 1]);
        base.lower_nodes.erase(base.lower_nodes.begin() + pos + 1, base.lower_nodes.begin() + pos + 2);
        return;
    }
    throw "function, variable, number or expression in () expected after !";
}

void parseExpressions(Node& base){
    for(auto & lower_node : base.lower_nodes){//parse inner symbols
        if(isToken(*lower_node, symbol_tokens::BRACKET_L) || isToken(*lower_node, symbol_tokens::SCOPE_BRACKET_L) || isToken(*lower_node, token_types::FUNCTION_CALL))
            parseExpressions(*lower_node);
    }
    for(int i = 0; i < base.lower_nodes.size(); i++){//parse !
        if(isToken(*base.lower_nodes[i], math_tokens::NOT))
            parseNot(base, i);
    }
    for(int i = 0; i < base.lower_nodes.size(); i++){//parse +/-
        if(isToken(*base.lower_nodes[i], math_tokens::PLUS) || isToken(*base.lower_nodes[i], math_tokens::MINUS)) {
            parseMathLogicToken(base, i, 1);
            i--;
        }
    }
    for(int i = 0; i < base.lower_nodes.size(); i++){//parse <</>>
        if(isToken(*base.lower_nodes[i], math_tokens::BSL) || isToken(*base.lower_nodes[i], math_tokens::BSR)) {
            parseMathLogicToken(base, i, 2);
            i--;
        }
    }
    for(int i = 0; i < base.lower_nodes.size(); i++){//parse < <= > >= == !=
        if(isToken(*base.lower_nodes[i], token_types::LOGIC) && !isToken(*base.lower_nodes[i], logic_tokens::ASSIGN)) {
            parseMathLogicToken(base, i, 3);
            i--;
        }
    }
    for(int i = 0; i < base.lower_nodes.size(); i++){//parse | & ^
        if(isToken(*base.lower_nodes[i], math_tokens::XOR) || isToken(*base.lower_nodes[i], math_tokens::AND) || isToken(*base.lower_nodes[i], math_tokens::OR)) {
            parseMathLogicToken(base, i, 4);
            i--;
        }
    }
}

void parseIfWhile(Node& base, int pos){
    if(!isToken(*base.lower_nodes[pos + 1], symbol_tokens::BRACKET_L))
        throw "(cond) expected after if/while";
    if(base.lower_nodes[pos + 1]->lower_nodes.size() != 1 || //not 1 arg
    !isToken(*base.lower_nodes[pos + 1]->lower_nodes[0], token_types::LOGIC) || //not logic
    !((int)base.lower_nodes[pos + 1]->lower_nodes[0]->token[1] >= (int)logic_tokens::NOT_EQ && //not comparison
    (int)base.lower_nodes[pos + 1]->lower_nodes[0]->token[1] < (int)logic_tokens::NUM_TYPES))
        throw "a single condition expression expected in () after if/while";
    base.lower_nodes[pos]->lower_nodes.emplace_back(base.lower_nodes[pos + 1]);
    base.lower_nodes.erase(base.lower_nodes.begin() + pos + 1, base.lower_nodes.begin() + pos + 2);
    if(!isToken(*base.lower_nodes[pos + 1], symbol_tokens::SCOPE_BRACKET_L))
        throw "scope expected after condition after if/while";
    base.lower_nodes[pos]->lower_nodes.emplace_back(base.lower_nodes[pos + 1]);
    base.lower_nodes.erase(base.lower_nodes.begin() + pos + 1, base.lower_nodes.begin() + pos + 2);
}

void parseAssign(Node& base, int pos){
    if(!isToken(*base.lower_nodes[pos - 1], token_types::VAR_CREATE) && !isToken(*base.lower_nodes[pos - 1], token_types::VAR))
        throw "var name expected before = operator";
    base.lower_nodes[pos]->lower_nodes.emplace_back(base.lower_nodes[pos - 1]);
    base.lower_nodes.erase(base.lower_nodes.begin() + pos - 1, base.lower_nodes.begin() + pos);
    pos--;
    if(!isAcceptableForOperation(*base.lower_nodes[pos + 1], 5))
        throw "expression expected after = operator";
    base.lower_nodes[pos]->lower_nodes.emplace_back(base.lower_nodes[pos + 1]);
    base.lower_nodes.erase(base.lower_nodes.begin() + pos + 1, base.lower_nodes.begin() + pos + 2);
}

void parseScopes(Node& base){
    for(auto & lower_node : base.lower_nodes){
        if(isToken(*lower_node, symbol_tokens::SCOPE_BRACKET_L)){
            parseScopes(*lower_node);
        }
    }
    for(int i = 0; i < base.lower_nodes.size(); i++){
        if(isToken(*base.lower_nodes[i], token_types::RETURN)){
            if(!isAcceptableForOperation(*base.lower_nodes[i + 1], 5))
                throw "expression expected after return statement";
            base.lower_nodes[i]->lower_nodes.emplace_back(base.lower_nodes[i + 1]);
            base.lower_nodes.erase(base.lower_nodes.begin() + i + 1, base.lower_nodes.begin() + i + 2);
        }
    }
    for(int i = 0; i < base.lower_nodes.size(); i++){
        if(isToken(*base.lower_nodes[i], token_types::IF) || isToken(*base.lower_nodes[i], token_types::WHILE)){
            parseIfWhile(base, i);
        }
    }
    for(int i = 0; i < base.lower_nodes.size(); i++){
        if(isToken(*base.lower_nodes[i], logic_tokens::ASSIGN)){
            parseAssign(base, i);
        }
    }
}

void parseFunction(Node& base, int pos){
    pos++;
    if(!isToken(*base.lower_nodes[pos], symbol_tokens::SCOPE_BRACKET_L))
        throw "{ expected after ) in function definition";
    base.lower_nodes.erase(base.lower_nodes.begin() + pos, base.lower_nodes.begin() + pos + 1);
    int scope_depth = 1;
    while(true){
        if(isToken(*base.lower_nodes[pos], symbol_tokens::SCOPE_BRACKET_L))
            scope_depth++;
        if(isToken(*base.lower_nodes[pos], symbol_tokens::SCOPE_BRACKET_R)) {
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
    parseFnCalls(*base.lower_nodes[pos - 1]);
    parseExpressions(*base.lower_nodes[pos - 1]);
    parseScopes(*base.lower_nodes[pos - 1]);
    //parse assign
    //pretty much done
}

Node& parse(const std::vector<std::tuple<token_types, int>>& tokens){
    auto root = new Node(std::tuple<token_types, int>(token_types::VAR_CREATE, 0));
    for(auto token : tokens){
        root->lower_nodes.emplace_back(new Node(token));
    }
    for(int i = 0; i < root->lower_nodes.size(); i++){
        if(isToken(*root->lower_nodes[i], token_types::FUNCTION_DEF)){
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
