#include <fstream>
#include <map>
#include <vector>

enum class tokens{VAR, FUNCTION, ASSIGN};

void tokenize(std::ifstream& file, std::vector<int>& tokens){
    std::map<std::string, short> var_keys;
    std::map<std::string, short> function_keys;
    std::string line;
    while(getline(file, line)){

    }
}