#include <iostream>
#include "tokenize/tokenize.h"
#include "parse/parse.h"
#include "generate/generate.h"
#include <vector>
#include <tuple>


int main() {
    std::ifstream input_file;
    std::ofstream output_file;
    std::vector<std::tuple<int, int>> tokens;

    input_file.open("input.lwc");
    tokenize(input_file, tokens);
    input_file.close();

    Node &root = parse(tokens);

    std::string asm_code = generate(root);

    output_file.open("output.asm");
    output_file << asm_code;
    output_file.close();
}