#include <algorithm>
#include "tokenize.h"

enum class token_types{VAR, VAR_CREATE, FUNCTION_START, FUNCTION, FUNCTION_CALL, MATH, LOGIC, NUMBER};
enum class math_tokens{PLUS, MINUS, BSL, BSR};
enum class logic_tokens{ASSIGN, MORE, LESS, EQUAL};

void trim(std::string& param){
    while(param.starts_with(' '))
        param.erase(0, 1);
    while(param.ends_with(' '))
        param.erase(param.end() - 1, param.end());
    if(param.empty())
        int a = 0 / 0;
}


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
    line.erase(0, pos);
    tokens.emplace_back((int)token_types::FUNCTION_START, fn_keys.size() - 1);
    pos = line.find(')');
    std::string params_str = line.substr(1, pos - 1);
    line.erase(0, pos + 1);
    while(!params_str.empty()){
        std::string param = params_str.substr(0, params_str.find(','));
        params_str.erase(0, param.length() + 1);
        trim(param);
        var_keys.emplace_back(param, scope);
    }
    std::get<1>(fn_keys[fn_keys.size() - 1]) = var_keys.size();
    pos = line.find('{');
    line.erase(0, pos + 1);
}

void tokenizeFnCall(std::string& line, std::vector<std::tuple<int, int>>& tokens, std::vector<std::tuple<std::string, int>>& var_keys){
    unsigned long pos = line.find(')');
    std::string params_str = line.substr(1, pos - 1);
    line.erase(0, pos + 1);
    while(!params_str.empty()){
        std::string param = params_str.substr(0, params_str.find(','));
        params_str.erase(0, param.length() + 1);
        trim(param);
        bool cont = false;
        for(int i = 0; i < var_keys.size(); i++){
            if(param == std::get<0>(var_keys[i])) {
                tokens.emplace_back((int)token_types::VAR, i);
                cont = true;
                break;
            }
        }
        if(!cont)
            int wqef = 0 / 0;
    }
}

void tokenize(std::ifstream& file, std::vector<std::tuple<int, int>>& tokens){
    bool is_in_function = false;
    int scope_level = 0;
    std::vector<std::tuple<std::string, int>> var_keys;
    std::vector<std::tuple<std::string, int>> function_keys;
    std::string line;

    while(getline(file, line)){
        bool found = true;
        while(found) {
            found = false;
            unsigned long fn;
            fn = line.find("function ");
            if(fn != std::string::npos){
                line.erase(0, fn + 8);
                unsigned long pos = line.find('(');
                std::string temp = line.substr(0, pos);
                trim(temp);
                function_keys.emplace_back(temp, 0);
                found = true;
                continue;
            }
        }
    }

    file.clear();
    file.seekg(0);

    while(getline(file, line)){
        while(!line.empty()) {
            trim(line);
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
                    while(!var_keys.empty()){
                        if(std::get<1>(var_keys[var_keys.size() - 1]) > scope_level)
                            var_keys.erase(var_keys.end());
                        else
                            break;
                    }
                    continue;
                }
                else if(line.starts_with('{')){
                    scope_level++;
                    line.erase(0, 1);
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
                pos = line.find('(');
                std::string fn_name = line.substr(0, pos);
                trim(fn_name);
                for(int i = 0; i < function_keys.size(); i++){
                    if(fn_name == std::get<0>(function_keys[i])) {
                        tokens.emplace_back((int)token_types::FUNCTION, i);
                        line.erase(0, pos + 1);
                        tokenizeFnCall(line, tokens, var_keys);
                        tokens.emplace_back((int)token_types::FUNCTION_CALL, i);
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