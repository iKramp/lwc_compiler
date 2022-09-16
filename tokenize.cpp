#include <algorithm>
#include "tokenize.h"

enum class token_types{VAR, FUNCTION, MATH, LOGIC, NUMBER};
enum class math_tokens{PLUS, MINUS, BSL, BSR};
enum class logic_tokens{ASSIGN, MORE, LESS, EQUAL};

void tokenizeVar(std::string& line, std::vector<std::tuple<int, int>>& tokens, std::vector<std::tuple<std::string, int>>& var_keys, int scope){
    line.erase(0, 4);
    unsigned long pos = line.find(' ');
    var_keys.emplace_back(line.substr(0, pos), scope);
    line.erase(0, pos);
    if(line.starts_with(" =")){
        tokens.emplace_back((int)token_types::VAR, var_keys.size() - 1);
        tokens.emplace_back((int)token_types::MATH, (int)logic_tokens::ASSIGN);
        line.erase(0, 3);
    }
}





void tokenize(std::ifstream& file, std::vector<std::tuple<int, int>>& tokens){
    bool is_in_function = false;
    int scope_level = 0;
    std::vector<std::tuple<std::string, int>> var_keys;
    std::vector<std::string> function_keys;
    std::string line;
    while(getline(file, line)){
        while(!line.empty()) {
            if(line.starts_with(' ')) {
                line.erase(0, 1);
                continue;
            }
            if(line.starts_with(';')){
                line.clear();
                continue;
            }
            if(is_in_function){
                if(line.starts_with("var")) {
                    tokenizeVar(line, tokens, var_keys, scope_level);
                    continue;
                }
                else if(line.starts_with('}')){
                    scope_level--;
                    line.erase(0, 1);
                    if(scope_level == 0)
                        is_in_function = false;
                    continue;
                }
                unsigned long pos = line.find(' ');

                if(std::ranges::all_of(line.substr(0, pos), [](char c){ return isdigit(c) != 0; }))
                    tokens.emplace_back((int)token_types::NUMBER, std::stoi(line.substr(0, pos)));
                line.erase(0, pos);
            }else{
                if(line.starts_with("function ")){
                    is_in_function = true;
                    scope_level++;
                    line.clear();
                    continue;
                }
            }
            line.erase(0, 1);//later syntax error, as it didn't recognise anything
        }
    }
}