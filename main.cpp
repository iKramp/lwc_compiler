#include <iostream>
#include "tokenize/tokenize.h"
#include <fstream>
#include <vector>
#include <tuple>


int main() {
    std::ifstream input_file;
    std::ofstream output_file;
    std::vector<std::tuple<int, int>> tokens;

    input_file.open("input.lwc");
    tokenize(input_file, tokens);

    input_file.close();
    //output_file.open("output.asm");
    //output_file.close();
}
