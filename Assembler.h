#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <algorithm>
#include <iomanip>

class Assembler {
private:
    // Opcode Table: Maps mnemonics to their opcode values (and instruction format info if needed)
    std::map<std::string, std::pair<std::string, int>> opcodeTable;

    // Symbol Table: Maps labels to their memory addresses
    std::map<std::string, int> symbolTable;

    // Current location counter
    int locationCounter;

    // Starting address of the program
    int startAddress;

    // Helper to initialize opcodes
    void initOpcodeTable();

    // Helper methods
    std::string trim(const std::string& str);
    std::vector<std::string> split(const std::string& str);
    bool isLabel(const std::string& token);
    bool isComment(const std::string& line);

    // Pass 1: Define symbols
    bool pass1(const std::string& inputFile);

    // Pass 2: Generate object code
    bool pass2(const std::string& inputFile, const std::string& outputFile);

public:
    Assembler();
    bool assemble(const std::string& inputFile, const std::string& outputFile);
};

#endif
