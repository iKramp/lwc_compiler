#include <algorithm>
#include "tokenize.h"

enum class token_types{VAR, VAR_CREATE, FUNCTION, FUNCTION_START, MATH, LOGIC, NUMBER};
enum class math_tokens{PLUS, MINUS, BSL, BSR};
enum class logic_tokens{ASSIGN, MORE, LESS, EQUAL};

void tokenizeVar(std::string& line, std::vector<std::tuple<int, int>>& tokens, std::vector<std::tuple<std::string, int>>& var_keys, int scope){
    line.erase(0, 4);
    unsigned long pos = line.find(' ');
    var_keys.emplace_back(line.substr(0, pos), scope);
    line.erase(0, pos);
    tokens.emplace_back((int)token_types::VAR_CREATE, var_keys.size() - 1);
}

void tokenizeFn(std::string& line, std::vector<std::tuple<int, int>>& tokens, std::vector<std::tuple<std::string, int>>& fn_keys, std::vector<std::tuple<std::string, int>>& var_keys, int scope){
    line.erase(0, 9);
    unsigned long pos = line.find('(');
    fn_keys.emplace_back(line.substr(0, pos), 0);
    line.erase(0, pos);
    tokens.emplace_back((int)token_types::FUNCTION_START, fn_keys.size() - 1);
    pos = line.find(')');
    std::string params_str = line.substr(1, pos - 1);
    line.erase(0, pos + 1);
    while(!params_str.empty()){
        std::string param = params_str.substr(0, params_str.find(','));
        params_str.erase(0, param.length() + 1);
        while(param.starts_with(' '))
            param.erase(0, 1);
        while(param.ends_with(' '))
            param.erase(param.end() - 1, param.end());
        var_keys.emplace_back(param, scope);
    }
    std::get<1>(fn_keys[fn_keys.size() - 1]) = var_keys.size();
}


void tokenize(std::ifstream& file, std::vector<std::tuple<int, int>>& tokens){
    bool is_in_function = false;
    int scope_level = 0;
    std::vector<std::tuple<std::string, int>> var_keys;
    std::vector<std::tuple<std::string, int>> function_keys;
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
                else if(line.starts_with("+")){
                    tokens.emplace_back((int)token_types::MATH, (int)math_tokens::PLUS);
                    line.erase(0, 1);
                    continue;
                }
                else if(line.starts_with('-')){
                    tokens.emplace_back((int)token_types::MATH, (int)math_tokens::MINUS);
                    line.erase(0, 1);
                    continue;
                }
                else if(line.starts_with(">>")){
                    tokens.emplace_back((int)token_types::MATH, (int)math_tokens::BSR);
                    line.erase(0, 2);
                    continue;
                }
                else if(line.starts_with("<<")){
                    tokens.emplace_back((int)token_types::MATH, (int)math_tokens::BSL);
                    line.erase(0, 2);
                    continue;
                }
                else if(line.starts_with("==")){
                    tokens.emplace_back((int)token_types::LOGIC, (int)logic_tokens::EQUAL);
                    line.erase(0, 2);
                    continue;
                }
                else if(line.starts_with('=')){
                    tokens.emplace_back((int)token_types::LOGIC, (int)logic_tokens::ASSIGN);
                    line.erase(0, 1);
                    continue;
                }
                else if(line.starts_with('>')){
                    tokens.emplace_back((int)token_types::LOGIC, (int)logic_tokens::MORE);
                    line.erase(0, 1);
                    continue;
                }
                else if(line.starts_with('<')){
                    tokens.emplace_back((int)token_types::LOGIC, (int)logic_tokens::LESS);
                    line.erase(0, 1);
                    continue;
                }
                else if(line.starts_with('}')){
                    scope_level--;
                    line.erase(0, 1);
                    if(scope_level == 0)
                        is_in_function = false;
                    continue;
                }


                //non_reserved keywords (aka numbers and function names)
                unsigned long pos = line.find(' ');
                if(std::ranges::all_of(line.substr(0, pos), [](char c){ return isdigit(c) != 0; })) {
                    tokens.emplace_back((int) token_types::NUMBER, std::stoi(line.substr(0, pos)));
                    line.erase(0, pos);
                    continue;
                }
                bool cont = false;
                for(int i = 0; i < var_keys.size(); i++){
                    if(line.substr(0, pos) == std::get<0>(var_keys[i])) {
                        tokens.emplace_back((int)token_types::VAR, i);
                        line.erase(0, pos);
                        cont = true;
                        break;
                    }
                }
                if(cont)
                    continue;
            }else{
                if(line.starts_with("function ")){
                    is_in_function = true;
                    scope_level++;
                    tokenizeFn(line, tokens, function_keys, var_keys, scope_level);
                    continue;
                }
            }
            int a = 0 / 0;
        }
    }
}