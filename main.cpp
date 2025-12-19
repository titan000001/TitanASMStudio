#include "Assembler.h"
#include "Simulator.h"
#include <iostream>
#include <cstring>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: assembler <input_file> [output_file]" << std::endl;
        std::cout << "Usage: assembler -run <object_file>" << std::endl;
        return 1;
    }

    // Check for Simulator Mode
    if (strcmp(argv[1], "-run") == 0) {
        if (argc < 3) {
            std::cout << "Error: Please specify object file to run." << std::endl;
            return 1;
        }
        std::string objFile = argv[2];
        Simulator cpu;
        if (cpu.load(objFile)) {
            cpu.run();
        } else {
            std::cout << "Simulation failed to load." << std::endl;
        }
        return 0;
    }

    // Assembler Mode
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
