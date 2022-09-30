#include "createBinary.h"
#include <vector>

void createBinary(){
    std::ifstream file;
    std::vector<std::string> labels;
    std::vector<int> addrs;
    std::string line, asm_str;
    file.open("output.asm");
    int addr = 0;
    while(getline(file, line)){//enumerate labels
        unsigned long pos = line.find('/');
        if(pos != std::string::npos)
            line.erase(pos - 1, line.size() - pos + 1);
        if(line.starts_with("LABEL_")) {
            labels.emplace_back(line.substr(6, line.size() - 6));
            addrs.emplace_back(addr);
            continue;
        }
        asm_str += line + " ";
        addr += 2;
    }
    while(true){
        unsigned long pos = asm_str.find("REF_");
        if(pos == std::string::npos)
            break;
        for(int i = 0; i < labels.size(); i++){
            if(labels[i] == asm_str.substr(pos + 4, labels[i].size())){
                asm_str.erase(pos, labels[i].size() + 4);
                asm_str.insert(pos, std::to_string(addrs[i]));

            }
        }
    }
    int prev_pos = 0;
    /*while(!asm_str.empty()) {
        unsigned long pos = asm_str.find(' ');
        if(asm_str.substr(prev_pos, pos) == "ADD"){
            
        }
    }*/
}