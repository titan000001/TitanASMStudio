#include "Assembler.h"

Assembler::Assembler() {
    locationCounter = 0;
    startAddress = 0;
    initOpcodeTable();
}

void Assembler::initOpcodeTable() {
    // Format: Mnemonic -> {Opcode, Number of Arguments}
    // Using a simple hypothetical ISA for demonstration
    opcodeTable["LOAD"] = {"01", 1};  // Load accumulator from memory
    opcodeTable["STORE"] = {"02", 1}; // Store accumulator to memory
    opcodeTable["ADD"] = {"03", 1};   // Add memory value to accumulator
    opcodeTable["SUB"] = {"04", 1};   // Subtract memory value from accumulator
    opcodeTable["MULT"] = {"05", 1};  // Multiply
    opcodeTable["DIV"] = {"06", 1};   // Divide
    opcodeTable["JMP"] = {"07", 1};   // Unconditional Jump
    opcodeTable["JZ"] = {"08", 1};    // Jump if Zero
    opcodeTable["JNZ"] = {"09", 1};   // Jump if Not Zero
    opcodeTable["HALT"] = {"00", 0};  // Stop execution
}

// Helper to remove whitespace from beginning and end
std::string Assembler::trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (std::string::npos == first) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

// Helper to split string by spaces/tabs
std::vector<std::string> Assembler::split(const std::string& str) {
    std::istringstream iss(str);
    std::vector<std::string> tokens;
    std::string token;
    while (iss >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

bool Assembler::isLabel(const std::string& token) {
    return token.back() == ':';
}

bool Assembler::isComment(const std::string& line) {
    std::string trimmed = trim(line);
    return trimmed.empty() || trimmed[0] == ';';
}

bool Assembler::pass1(const std::string& inputFile) {
    std::ifstream file(inputFile);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open input file " << inputFile << std::endl;
        return false;
    }

    std::string line;
    locationCounter = 0; // Assuming starting at address 0 for simplicity, or look for ORG directive

    std::cout << "Starting Pass 1..." << std::endl;

    while (std::getline(file, line)) {
        if (isComment(line)) continue;

        std::vector<std::string> tokens = split(line);
        if (tokens.empty()) continue;

        // Check for Label
        int tokenIndex = 0;
        if (isLabel(tokens[0])) {
            std::string label = tokens[0].substr(0, tokens[0].length() - 1);
            if (symbolTable.find(label) != symbolTable.end()) {
                std::cerr << "Error: Duplicate label '" << label << "' at address " << locationCounter << std::endl;
                return false;
            }
            symbolTable[label] = locationCounter;
            std::cout << "  Defined Label: " << label << " at " << locationCounter << std::endl;
            tokenIndex++; // Move to next token (instruction)
        }

        if (tokenIndex < tokens.size()) {
            std::string mnemonic = tokens[tokenIndex];
            
            // Handle Directives (Pseudo-ops)
            if (mnemonic == "ORG") {
                if (tokenIndex + 1 < tokens.size()) {
                    locationCounter = std::stoi(tokens[tokenIndex + 1]);
                    startAddress = locationCounter;
                }
            } else if (mnemonic == "DW" || mnemonic == "DB") {
                // Define Word/Byte - increments location counter
                locationCounter++; 
            } else if (opcodeTable.find(mnemonic) != opcodeTable.end()) {
                // It's a valid instruction
                // For this simple ISA, every instruction is 2 words (Opcode + Operand) or 1 word (Opcode only)
                // Let's assume a fixed instruction size of 1 'unit' or dynamic based on args for realism
                // Here: Simple model -> 1 instruction = 1 address increment
                locationCounter++;
            } else {
                std::cerr << "Error: Unknown mnemonic '" << mnemonic << "'" << std::endl;
                // return false; // Optional: Fail on error or continue
            }
        }
    }

    file.close();
    return true;
}

bool Assembler::pass2(const std::string& inputFile, const std::string& outputFile) {
    std::ifstream inFile(inputFile);
    std::ofstream outFile(outputFile);

    if (!inFile.is_open() || !outFile.is_open()) {
        std::cerr << "Error: Could not open files." << std::endl;
        return false;
    }

    std::string line;
    locationCounter = startAddress;

    std::cout << "Starting Pass 2..." << std::endl;
    outFile << "ADDR\tCODE\tSOURCE" << std::endl; // Header

    while (std::getline(inFile, line)) {
        if (isComment(line)) continue;
        
        std::vector<std::string> tokens = split(line);
        if (tokens.empty()) continue;

        std::string originalLine = line;
        int tokenIndex = 0;

        // Skip label if present
        if (isLabel(tokens[0])) {
            tokenIndex++;
        }

        if (tokenIndex >= tokens.size()) continue;

        std::string mnemonic = tokens[tokenIndex];
        std::string objectCode = "";

        if (mnemonic == "ORG") {
             if (tokenIndex + 1 < tokens.size()) {
                locationCounter = std::stoi(tokens[tokenIndex + 1]);
             }
             outFile << "\t\t" << originalLine << std::endl;
             continue; // No object code for ORG
        }
        else if (mnemonic == "DW") {
             // Define Word: Just the value
             if (tokenIndex + 1 < tokens.size()) {
                int value = std::stoi(tokens[tokenIndex + 1]);
                std::stringstream ss;
                ss << std::setfill('0') << std::setw(4) << std::hex << value;
                objectCode = ss.str();
             }
        }
        else if (opcodeTable.find(mnemonic) != opcodeTable.end()) {
            std::string opCode = opcodeTable[mnemonic].first;
            objectCode = opCode;

            // Handle Operand (if any)
            if (opcodeTable[mnemonic].second > 0) {
                if (tokenIndex + 1 < tokens.size()) {
                    std::string operand = tokens[tokenIndex + 1];
                    int address = 0;

                    // Check if operand is a label
                    if (symbolTable.count(operand)) {
                        address = symbolTable[operand];
                    } else {
                        // Try to parse as direct address/number
                        try {
                            address = std::stoi(operand);
                        } catch (...) {
                            address = 0; // Error parsing
                        }
                    }
                    
                    std::stringstream ss;
                    ss << std::setfill('0') << std::setw(2) << std::hex << address; // 2 digit address
                    objectCode += ss.str();
                } else {
                    objectCode += "00"; // Missing operand
                }
            } else {
                objectCode += "00"; // No operand needed (e.g., HALT)
            }
        }

        // Write to output file
        if (!objectCode.empty()) {
            std::stringstream addrSS;
            addrSS << std::setfill('0') << std::setw(4) << std::dec << locationCounter; 
            
            // Convert object code to uppercase for looks
            std::transform(objectCode.begin(), objectCode.end(), objectCode.begin(), ::toupper);
            
            outFile << addrSS.str() << "\t" << objectCode << "\t" << trim(line) << std::endl;
            locationCounter++;
        }
    }

    inFile.close();
    outFile.close();
    return true;
}

bool Assembler::assemble(const std::string& inputFile, const std::string& outputFile) {
    if (!pass1(inputFile)) {
        return false;
    }
    return pass2(inputFile, outputFile);
}
