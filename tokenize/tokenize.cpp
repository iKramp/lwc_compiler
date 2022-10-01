#include <algorithm>
#include "tokenize.h"
#include <iostream>
#include <stdexcept>


std::vector<std::string> reserved_keywords = {"var", "func", "return", "if", "while"};
std::vector<std::string> reserved_symbols = {"+", "-", "<<", ">>", "|", "^", "&", "!", "=", "!=", "<=", "<", ">=", ">", "==", "(", ")", "{", "}", " "};

void trim(std::string& param){
    while(param.starts_with(' '))
        param.erase(0, 1);
    while(param.ends_with(' '))
        param.erase(param.end() - 1, param.end());
}

void readFile(std::ifstream& file, std::string& file_str){
    bool in_comment = false;
    unsigned long comment_start = 0;
    std::string line;
    while(getline(file, line)) {
        while(!line.empty()) {
            unsigned long pos = line.find("//");//remove comments
            if (pos != std::string::npos)
                line.erase(pos, line.size() - pos);
            if (in_comment) {
                pos = line.find("*/");
                if (pos == std::string::npos) {
                    line.erase(comment_start, line.length() - comment_start);
                    comment_start = 0;
                }else{
                    line.erase(comment_start, pos - comment_start + 2);
                    in_comment = false;
                    continue;
                }
            }else{
                pos = line.find("/*");
                if (pos != std::string::npos) {
                    in_comment = true;
                    comment_start = pos;
                    continue;
                }
            }
            file_str += line + " ";
            line.clear();
        }
    }
}

void tokenizeSymbol(int id, std::vector<std::tuple<token_types, int>>& tokens){
    if(id < (int)math_tokens::NUM_TYPES)
        tokens.emplace_back(token_types::MATH, id);
    else if(id < (int)math_tokens::NUM_TYPES + (int)logic_tokens::NUM_TYPES)
        tokens.emplace_back(token_types::LOGIC, id - (int)math_tokens::NUM_TYPES);
    else if(id < (int)math_tokens::NUM_TYPES + (int)logic_tokens::NUM_TYPES + (int)symbol_tokens::NUM_TYPES)
        tokens.emplace_back(token_types::SYMBOL, id - ((int)math_tokens::NUM_TYPES + (int)logic_tokens::NUM_TYPES));
}

void splitTokens(std::string& file_str, std::vector<std::string>& string_keys, std::vector<std::tuple<token_types, int>>& tokens){
    while(!file_str.empty()){
        trim(file_str);
        unsigned long pos = std::string::npos;
        int symbol_id = 0;
        for(int i = 0; i < reserved_symbols.size(); i++){
            unsigned long temp = file_str.find(reserved_symbols[i]);
            if(temp < pos) {
                pos = temp;
                symbol_id = i;
            }
        }
        if(pos == 0){
            tokenizeSymbol(symbol_id, tokens);
            file_str.erase(0, reserved_symbols[symbol_id].length());
        }else{
            std::string keyword = file_str.substr(0, pos);
            for(int i = 0; i < reserved_keywords.size(); i++){
                if(keyword == reserved_keywords[i]){
                    tokens.emplace_back((token_types)i, 0);
                    file_str.erase(0, reserved_keywords[i].length());
                    goto cont;
                }
            }
            tokens.emplace_back(token_types::STRING, string_keys.size());
            string_keys.emplace_back(keyword);
            file_str.erase(0, keyword.size());
            cont:;
        }
    }
}

void convertNumbers(std::vector<std::tuple<token_types, int>>& tokens, std::vector<std::string>& string_keys){
    for(auto& token : tokens){
        if(std::get<0>(token) == token_types::STRING){
            std::string& str = string_keys[std::get<1>(token)];
            if(isdigit(str[0])){
                try {
                    int a = std::stoi(str);
                    std::get<0>(token) = token_types::NUMBER;
                    std::get<1>(token) = a;
                }
                catch (...){
                    throw "only numbers can start with a digit";
                }
            }
        }
    }
}

void getFunctions(std::vector<std::tuple<token_types, int>>& tokens, std::vector<std::string>& string_keys, std::vector<std::tuple<std::string, int>>& function_keys){
    for(int i = 0; i < tokens.size(); i++){
        if(std::get<0>(tokens[i]) == token_types::FUNCTION_DEF){
            if(std::get<0>(tokens[i + 1]) != token_types::STRING){
                throw "name expected after function declaration";
            }
            std::get<1>(tokens[i]) = function_keys.size();
            function_keys.emplace_back(string_keys[std::get<1>(tokens[i + 1])], 0);
            tokens.erase(tokens.begin() + i + 1, tokens.begin() + i + 2);
        }
    }
    bool main = false;
    for(int i = 0; i < function_keys.size(); i++){
        for(int j = i + 1; j < function_keys.size(); j++){
            if(std::get<0>(function_keys[i]) == std::get<0>(function_keys[j]))
                throw "double function definition of: " + std::get<0>(function_keys[i]);
        }
        if(std::get<0>(function_keys[i]) == "main") {
            std::swap(function_keys[i], function_keys[0]);
            main = true;
        }
    }
    if(!main)
        throw "no main function";
}

void processVarCreate(std::vector<std::tuple<token_types, int>>& tokens, std::vector<std::string>& string_keys, std::vector<std::tuple<std::string, int>>& var_keys, int pos, int scope){
    if(std::get<0>(tokens[pos + 1]) != token_types::STRING)
        throw "name expected after function declaration";
    std::get<1>(tokens[pos]) = var_keys.size();
    var_keys.emplace_back(string_keys[std::get<1>(tokens[pos + 1])], scope);
    tokens.erase(tokens.begin() + pos + 1, tokens.begin() + pos + 2);
}

void processFuncCall(std::vector<std::tuple<token_types, int>>& tokens, std::vector<std::string>& string_keys, std::vector<std::tuple<std::string, int>>& func_keys, int pos){
    for(int i = 0; i < func_keys.size(); i++){
        if(string_keys[std::get<1>(tokens[pos])] == std::get<0>(func_keys[i])){
            std::get<0>(tokens[pos]) = token_types::FUNCTION_CALL;
            std::get<1>(tokens[pos]) = i;
            return;
        }
    }
    throw string_keys[std::get<1>(tokens[pos])] + " is not a function";
}

void processVar(std::vector<std::tuple<token_types, int>>& tokens, std::vector<std::string>& string_keys, std::vector<std::tuple<std::string, int>>& var_keys, int pos){
    for(int i = 0; i < var_keys.size(); i++){
        if(string_keys[std::get<1>(tokens[pos])] == std::get<0>(var_keys[i])){
            std::get<0>(tokens[pos]) = token_types::VAR;
            std::get<1>(tokens[pos]) = i;
            return;
        }
    }
    throw string_keys[std::get<1>(tokens[pos])] + " is not a variable in this scope";
}

void cleanScopeVars(std::vector<std::tuple<std::string, int>>& var_keys, int scope){
    while(true){
        if(var_keys.empty() || scope >= std::get<1>(var_keys[var_keys.size() - 1]))
            return;
        var_keys.erase(var_keys.end() - 1);
    }
}

void processStrings(std::vector<std::tuple<token_types, int>>& tokens, std::vector<std::string>& string_keys, std::vector<std::tuple<std::string, int>>& var_keys, std::vector<std::tuple<std::string, int>>& function_keys){
    int scope = 0;
    for(int i = 0; i < tokens.size(); i++){
        if(std::get<0>(tokens[i]) == token_types::SYMBOL && std::get<1>(tokens[i]) == (int)symbol_tokens::SCOPE_BRACKET_L)
            scope++;
        else if(std::get<0>(tokens[i]) == token_types::SYMBOL && std::get<1>(tokens[i]) == (int)symbol_tokens::SCOPE_BRACKET_R) {
            scope--;
            cleanScopeVars(var_keys, scope);
        }
        else if(std::get<0>(tokens[i]) == token_types::VAR_CREATE)
            processVarCreate(tokens, string_keys, var_keys, i, scope);
        else if(std::get<0>(tokens[i]) == token_types::STRING) {
            if(std::get<0>(tokens[i + 1]) == token_types::SYMBOL && std::get<1>(tokens[i + 1]) == (int)symbol_tokens::BRACKET_L)
                processFuncCall(tokens, string_keys, function_keys, i);
            else
                processVar(tokens, string_keys, var_keys, i);
        }
    }
}

/*bool preprocess(std::vector<std::tuple<std::string, int>>& function_keys, std::string& file_str){
    bool found = true, main = false;
    unsigned long prev_pos = 0;
    while(found) {
        found = false;
        unsigned long fn;
        fn = file_str.find("func ", prev_pos);
        if(fn != std::string::npos){
            unsigned long pos = file_str.find('(', fn);
            std::string temp = file_str.substr(fn + 5, pos - fn - 5);
            prev_pos = pos;
            trim(temp);
            unsigned long space = temp.find(' ', fn + 5);
            //filter double functions

            if(space != std::string::npos) {
                std::cout << "( expected after function name: " + temp.substr(0, space);
                return false;
            }
            if(temp.empty()) {
                std::cout << "empty function name";
                return false;
            }


            if(temp == "main") {
                function_keys.emplace(function_keys.begin(), temp, 0);
                main = true;
            }
            else
                function_keys.emplace_back(temp, 0);
            found = true;
            continue;
        }
    }
    if(!main) {
        std::cout << "no main function in this program";
        return false;
    }
    else
        return true;
}*/

void tokenize(std::ifstream& file, std::vector<std::tuple<token_types, int>>& tokens){
    std::vector<std::string> string_keys;
    std::vector<std::tuple<std::string, int>> var_keys;
    std::vector<std::tuple<std::string, int>> function_keys;
    std::string file_str;

    readFile(file, file_str);
    splitTokens(file_str, string_keys, tokens);

    convertNumbers(tokens, string_keys);
    getFunctions(tokens, string_keys, function_keys);
    processStrings(tokens, string_keys, var_keys, function_keys);
    //set vars and functions
}