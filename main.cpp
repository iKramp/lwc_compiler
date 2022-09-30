#include <iostream>
#include "tokenize/tokenize.h"
#include "parse/parse.h"
#include "generate/generate.h"
#include "createBinary/createBinary.h"
#include <vector>
#include <tuple>


int main() {
    std::ifstream input_file;
    std::ofstream output_file;
    std::vector<std::tuple<token_types, int>> tokens;

    try {
        input_file.open("input.lwc");
        tokenize(input_file, tokens);
        input_file.close();

        Node &root = parse(tokens);

        /*std::string asm_code = generate(root);
        createBinary();*/
    }
    catch (char const* e){
        std::cout << e;
        return 1;
    }
    /*
    output_file.open("output.asm");
    output_file << asm_code;
    output_file.close();*/
}