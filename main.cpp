#include "Assembler.h"
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: assembler <input_file> [output_file]" << std::endl;
        return 1;
    }

    std::string inputFile = argv[1];
    std::string outputFile = "output.obj";

    if (argc >= 3) {
        outputFile = argv[2];
    }

    Assembler myAssembler;
    
    std::cout << "Assembler started for file: " << inputFile << std::endl;
    
    if (myAssembler.assemble(inputFile, outputFile)) {
        std::cout << "Assembly completed successfully!" << std::endl;
        std::cout << "Output written to: " << outputFile << std::endl;
    } else {
        std::cerr << "Assembly failed due to errors." << std::endl;
        return 1;
    }

    return 0;
}
