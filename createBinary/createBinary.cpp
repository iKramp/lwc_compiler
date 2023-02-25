#include "createBinary.h"
#include <map>
#include <vector>
#define ull unsigned long long

//map of instructions and binary
const std::map<std::string, std::string> bin_codes = {
        {"NOP",  "00000000"},
        {"NULL", "00000000"},
        {"ADD",  "00000001"},
        {"SUB",  "00000010"},
        {"BSL",  "00000011"},
        {"BSR",  "00000100"},
        {"OR",   "00000101"},
        {"XOR",  "00000110"},
        {"AND",  "00000111"},
        {"NOT",  "00001000"},
        {"XNOR", "00001001"},
        {"NAND", "00001010"},
        {"MEMW", "00001011"},
        {"MEMR", "00001100"},
        {"MOV",  "00001110"},
        {"CMP",  "00001111"},
        {"JMP",  "00010000"},
        {"GPUDRAW", "00010001"},
        {"GPUDISPLAY", "00010010"},
        {"PUSH", "00010011"},
        {"POP", "00010100"},
        {"SETSTACKPTR", "00010101"},
        {"GETSTACKPTR", "00010110"},
};

std::string trim(std::string str){
    while(str[0] == ' ' || str[0] == '\n')
        str.erase(0, 1);
    while(str[str.size() - 1] == ' ' || str[str.size() - 1] == '\n')
        str.erase(str.size() - 1, 1);
    return str;
}

//function that converts an int to 8 bit binary
std::string to8BitBin(int num){
    std::string bin = "";
    for(int i = 0; i < 8; i++){
        bin += std::to_string(num % 2);
        num /= 2;
    }
    std::reverse(bin.begin(), bin.end());
    return bin;
}

//function that converts an int to 16 bit binary
std::string to16BitBin(int num){
    std::string bin = "";
    for(int i = 0; i < 16; i++){
        bin += std::to_string(num % 2);
        num /= 2;
    }
    std::reverse(bin.begin(), bin.end());
    return bin;
}

void createBinary(){
    std::ifstream file;
    std::vector<std::string> labels;
    std::vector<int> addrs;
    std::string line, asm_str;
    file.open("output.asm");
    int addr = 0;
    while(getline(file, line)){//enumerate labels
        ull pos = line.find('/');
        if(pos != std::string::npos)
            line.erase(pos - 1, line.size() - pos + 1);
        if(line.starts_with("LABEL_")) {
            labels.emplace_back(trim(line.substr(6, line.size() - 6)));
            addrs.emplace_back(addr);
            continue;
        }
        asm_str += line + " \n";
        addr += 2;
    }
    while(true){
        ull pos = asm_str.find("REF_");
        if(pos == std::string::npos)
            break;
        std::string ref = trim(asm_str.substr(pos + 4, asm_str.find(' ', pos) - pos - 4));
        for(int i = 0; i < labels.size(); i++){
            if(labels[i] == ref){
                asm_str.erase(pos, labels[i].size() + 4);
                asm_str.insert(pos, std::to_string(addrs[i]));
            }
        }
    }
    file.close();
    std::ofstream ofile;
    ofile.open("output.asm");
    ofile << asm_str;
    ofile.close();
    std::string bin_str = "";
    int counter = 0;
    while(!asm_str.empty()){
        if(asm_str.empty())
            break;
        //special case for imm because it has a num spanning 2 bytes
        if(asm_str.starts_with("IMM")){
            asm_str.erase(0, 3);
            bin_str += "00001101";
            asm_str = trim(asm_str);
            int num = std::stoi(asm_str.substr(0, asm_str.find(' ')));
            bin_str += to8BitBin(num);
            asm_str = trim(asm_str.substr(asm_str.find(' '), asm_str.size() - asm_str.find(' ')));
            num = std::stoi(asm_str.substr(0, asm_str.find(' ')));
            bin_str += "\n" + to16BitBin(num);
            asm_str = trim(asm_str.substr(asm_str.find(' '), asm_str.size() - asm_str.find(' ')));
            counter += 4;
        } else {
            ull pos = asm_str.find(' ');
            if(pos == std::string::npos)
                pos = asm_str.size();
            std::string instr = asm_str.substr(0, pos);
            asm_str = trim(asm_str.substr(pos, asm_str.size() - pos));
            if(bin_codes.contains(instr))
                bin_str += bin_codes.at(instr);
            else
                bin_str += to8BitBin(std::stoi(instr));
            counter++;
        }
        if(counter % 2 == 0)
            bin_str += "\n";
    }
    ofile.open("output.bin");
    ofile << bin_str;
    ofile.close();
}