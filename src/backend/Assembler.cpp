#include "Assembler.h"
#include "MacroProcessor.h"
#include <iomanip>
#include <cstdint>
#include <vector>

Assembler::Assembler() {
    locationCounter = 0x100;
    startAddress = 0x100;
}

std::string Assembler::trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (std::string::npos == first) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

std::string normalizeLine(std::string line) {
    std::string res = "";
    bool inQuotes = false;
    for (char c : line) {
        if (c == '"') inQuotes = !inQuotes;
        if (c == ',' && !inQuotes) res += " ";
        else res += c;
    }
    return res;
}

std::string cleanToken(std::string t) {
    if (!t.empty() && t.back() == ',') t.pop_back();
    return t;
}

int parseNumber(std::string numStr) {
    if (numStr.empty()) return 0;
    try {
        if (numStr.back() == 'h' || numStr.back() == 'H') {
            numStr.pop_back();
            return std::stoi(numStr, nullptr, 16);
        }
        return std::stoi(numStr);
    } catch (...) { return 0; }
}

int getRegID(std::string r) {
    if (r == "al" || r == "AL") return 0;
    if (r == "ah" || r == "AH") return 1;
    if (r == "bl" || r == "BL") return 2;
    if (r == "bh" || r == "BH") return 3;
    if (r == "cl" || r == "CL") return 4;
    if (r == "ch" || r == "CH") return 5;
    if (r == "dl" || r == "DL") return 6;
    if (r == "dh" || r == "DH") return 7;
    if (r == "ax" || r == "AX") return 0; 
    if (r == "bx" || r == "BX") return 2;
    if (r == "cx" || r == "CX") return 4;
    if (r == "dx" || r == "DX") return 6;
    return -1;
}

bool Assembler::pass1(const std::string& inputFile) {
    std::ifstream inFile(inputFile);
    if (!inFile.is_open()) return false;
    
    std::string line;
    locationCounter = 0x100;
    int dataCounter = 0x800; // Track strings/vars size
    
    while (std::getline(inFile, line)) {
        line = trim(line);
        if (line.empty() || line[0] == ';') continue;
        
        std::string norm = normalizeLine(line);
        std::stringstream ss(norm);
        std::string token; ss >> token;
        if (token.empty()) continue;

        if (token.back() == ':') {
            symbolTable[token.substr(0, token.length() - 1)] = locationCounter;
            ss >> token;
        }
        if (token.empty()) continue;

        if (token[0] == '.') continue;
        if (token == "main" || token == "endp" || token == "end" || token == "include") continue;

        if (token == "org") { int val; ss >> std::hex >> val; locationCounter = val; }
        else if (token == "mov") {
            std::string d, s; ss >> d >> s;
            if (getRegID(d) != -1 && getRegID(s) != -1) locationCounter += 3;
            else locationCounter += 4;
        }
        else if (token == "add" || token == "sub" || token == "cmp") locationCounter += 4;
        else if (token == "jmp" || token == "jz" || token == "jnz") locationCounter += 4;
        else if (token == "int") locationCounter += 2;
        else if (token == "push" || token == "pop" || token == "call" || token == "ret") locationCounter += 4;
        else if (token == "print" || token == "printn") {
            locationCounter += 3; 
        }
        else if (token == "mul" || token == "div") locationCounter += 3;
        else if (token == "lea") locationCounter += 4;
        else {
            std::string next; ss >> next;
            if (next == "db" || next == "DB") {
                symbolTable[token] = 0; // Fixed up in pass2
            }
        }
    }
    return true;
}

bool Assembler::pass2(const std::string& inputFile, const std::string& outputFile) {
    std::ifstream inFile(inputFile);
    std::ofstream outFile(outputFile);
    if (!inFile.is_open() || !outFile.is_open()) return false;

    std::string line;
    locationCounter = 0x100;
    int dataCounter = 0x800;

    outFile << "ADDR CODE" << std::endl;

    while (std::getline(inFile, line)) {
        line = trim(line);
        if (line.empty() || line[0] == ';') continue;
        
        std::string norm = normalizeLine(line);
        // DB Handling
        if (norm.find(" db ") != std::string::npos || norm.find(" DB ") != std::string::npos) {
             std::stringstream ss(norm);
             std::string label, type, valStr;
             ss >> label >> type >> valStr;
             if (type == "db" || type == "DB") {
                 symbolTable[label] = dataCounter;
                 int val = parseNumber(valStr);
                 outFile << std::hex << std::setw(4) << std::setfill('0') << dataCounter 
                         << " " << std::setw(2) << (int)(uint8_t)val << std::endl;
                 dataCounter++;
                 continue;
             }
        }
        
        // DB ? Handling
        std::string dbCheck = norm;
        size_t dbPos = dbCheck.find(" db ");
        if (dbPos == std::string::npos) dbPos = dbCheck.find(" DB ");
        if (dbPos != std::string::npos) {
             std::stringstream ss(dbCheck);
             std::string label, type, valStr;
             ss >> label >> type >> valStr;
             if ((type == "db" || type == "DB") && valStr == "?") {
                 symbolTable[label] = dataCounter;
                 // Initialize to 0 for safety
                 outFile << std::hex << std::setw(4) << std::setfill('0') << dataCounter 
                         << " " << std::setw(2) << 0 << std::endl;
                 dataCounter++;
                 continue;
             }
        }

        size_t colon = line.find(':');
        if (colon != std::string::npos) line = trim(line.substr(colon + 1));
        if (line.empty()) continue;

        std::stringstream ss(normalizeLine(line));
        std::string opcode;
        ss >> opcode;
        if (opcode[0] == '.' || opcode == "main" || opcode == "endp" || opcode == "end" || opcode == "include") continue;

        if (opcode == "org") { int val; ss >> std::hex >> val; locationCounter = val; continue; }

        if (opcode == "mov") {
            std::string destStr, srcStr; ss >> destStr >> srcStr;
            int dest = getRegID(destStr), src = getRegID(srcStr);
            if (src != -1 && dest != -1) {
                outFile << std::hex << std::setw(4) << std::setfill('0') << locationCounter << " 02 " 
                        << std::setw(2) << (int)(uint8_t)dest << " " << std::setw(2) << (int)(uint8_t)src << std::endl;
                locationCounter += 3;
            } else if (dest != -1) {
                if (symbolTable.count(srcStr)) {
                    int addr = symbolTable[srcStr];
                    outFile << std::hex << std::setw(4) << std::setfill('0') << locationCounter << " 05 " 
                            << std::setw(2) << (int)(uint8_t)dest << " " << std::setw(2) << (int)(uint8_t)(addr&0xFF) 
                            << " " << std::setw(2) << (int)(uint8_t)((addr>>8)&0xFF) << std::endl;
                    locationCounter += 4;
                } else {
                    int val = parseNumber(srcStr);
                    outFile << std::hex << std::setw(4) << std::setfill('0') << locationCounter << " 01 " 
                            << std::setw(2) << (int)(uint8_t)dest << " " << std::setw(2) << (int)(uint8_t)(val & 0xFF) 
                            << " " << std::setw(2) << (int)(uint8_t)((val >> 8) & 0xFF) << std::endl;
                    locationCounter += 4;
                }
            } else if (src != -1 && symbolTable.count(destStr)) {
                int addr = symbolTable[destStr];
                outFile << std::hex << std::setw(4) << std::setfill('0') << locationCounter << " 06 " 
                        << std::setw(2) << (int)(uint8_t)(addr&0xFF) << " " << std::setw(2) << (int)(uint8_t)((addr>>8)&0xFF) 
                        << " " << std::setw(2) << (int)(uint8_t)src << std::endl;
                locationCounter += 4;
            }
        }
        else if (opcode == "add" || opcode == "sub" || opcode == "cmp") {
            std::string destStr, srcStr; ss >> destStr >> srcStr;
            int dest = getRegID(destStr), srcID = getRegID(srcStr);
            int op = (opcode == "add") ? 3 : (opcode == "sub") ? 4 : 7;
            if (srcID != -1) outFile << std::hex << std::setw(4) << std::setfill('0') << locationCounter << " 0" << op << " " << std::setw(2) << (int)(uint8_t)dest << " 01 " << std::setw(2) << (int)(uint8_t)srcID << std::endl;
            else {
                 int val = parseNumber(srcStr);
                 outFile << std::hex << std::setw(4) << std::setfill('0') << locationCounter << " 0" << op << " " << std::setw(2) << (int)(uint8_t)dest << " 02 " << std::setw(2) << (int)(uint8_t)(val&0xFF) << std::endl;
            }
            locationCounter += 4; 
        }
        else if (opcode == "mul" || opcode == "div") {
            std::string srcStr; ss >> srcStr;
            int srcID = getRegID(srcStr);
            int op = (opcode == "mul") ? 0x50 : 0x51;
            // Format: ADDR OP REG 00 (REG is src)
            if (srcID != -1) {
                outFile << std::hex << std::setw(4) << std::setfill('0') << locationCounter << " " << op << " " << std::setw(2) << (int)(uint8_t)srcID << " 00" << std::endl;
                locationCounter += 3;
            }
        }
        else if (opcode == "lea") {
            std::string destStr, srcStr; ss >> destStr >> srcStr; // source is variable name
            int destID = getRegID(destStr);
            if (destID != -1 && symbolTable.count(srcStr)) {
                int addr = symbolTable[srcStr];
                outFile << std::hex << std::setw(4) << std::setfill('0') << locationCounter << " 15 " 
                        << std::setw(2) << (int)(uint8_t)destID << " " 
                        << std::setw(2) << (int)(uint8_t)(addr&0xFF) << " " 
                        << std::setw(2) << (int)(uint8_t)((addr>>8)&0xFF) << std::endl;
                locationCounter += 4;
            }
        }
        else if (opcode == "jmp" || opcode == "jz" || opcode == "jnz") {
            int op = (opcode == "jmp") ? 0x40 : (opcode == "jz") ? 0x41 : 0x42;
            std::string lbl; ss >> lbl;
            int addr = symbolTable.count(lbl) ? symbolTable[lbl] : 0;
            outFile << std::hex << std::setw(4) << std::setfill('0') << locationCounter << " " << std::hex << op << " 02 " << std::setw(2) << (addr&0xFF) << " " << std::setw(2) << ((addr>>8)&0xFF) << std::endl;
            locationCounter += 4;
        }
        else if (opcode == "int") {
            std::string arg; ss >> arg; int val = parseNumber(arg);
            outFile << std::hex << std::setw(4) << std::setfill('0') << locationCounter << " 10 " << std::setw(2) << (int)(uint8_t)val << std::endl;
            locationCounter += 2;
        }
        else if (opcode == "print" || opcode == "printn") {
            std::string str;
            std::getline(ss, str);
            size_t start = str.find('"');
            size_t end = str.find_last_of('"');
            if (start != std::string::npos && end != std::string::npos && end > start) {
                std::string content = str.substr(start + 1, end - start - 1);
                int strAddr = dataCounter;
                // Emit PRINTN instruction
                outFile << std::hex << std::setw(4) << std::setfill('0') << locationCounter << " 20 " 
                        << std::setw(2) << (int)(uint8_t)(strAddr&0xFF) << " " << std::setw(2) << (int)(uint8_t)((strAddr>>8)&0xFF) << std::endl;
                
                // Emit String Data at dataCounter
                std::string oldHex = ""; // For debugging
                for (char c : content) {
                    outFile << std::hex << std::setw(4) << std::setfill('0') << dataCounter << " " << std::setw(2) << (int)(uint8_t)c << std::endl;
                    dataCounter++;
                }
                outFile << std::hex << std::setw(4) << std::setfill('0') << dataCounter << " 00" << std::endl; // Null terminator
                dataCounter++;

                locationCounter += 3;
            }
        }
        else if (opcode == "push" || opcode == "pop" || opcode == "call" || opcode == "ret") {
            int op = (opcode == "push") ? 0x30 : (opcode == "pop") ? 0x31 : (opcode == "call") ? 0x32 : 0x33;
            std::string arg; ss >> arg;
            if (op == 0x33) {
                outFile << std::hex << std::setw(4) << std::setfill('0') << locationCounter << " 33 00 00 00" << std::endl;
            } else if (op == 0x32) {
                int addr = symbolTable.count(arg) ? symbolTable[arg] : 0;
                outFile << std::hex << std::setw(4) << std::setfill('0') << locationCounter << " 32 02 " << std::setw(2) << (addr&0xFF) << " " << std::setw(2) << ((addr>>8)&0xFF) << std::endl;
            } else {
                int type = (getRegID(arg) != -1) ? 1 : 2;
                int val = (type == 1) ? getRegID(arg) : parseNumber(arg);
                outFile << std::hex << std::setw(4) << std::setfill('0') << locationCounter << " " << std::hex << op << " " << std::setw(2) << type << " " << std::setw(2) << (val&0xFF) << std::endl;
            }
            locationCounter += 4;
        }
    }
    return true;
}

bool Assembler::assemble(const std::string& inputFile, const std::string& outputFile) {
    MacroProcessor mp;
    std::string expandedFile = "temp_expanded.asm";
    if (!mp.expandMacros(inputFile, expandedFile)) {
        // Fallback if macro processor fails
        expandedFile = inputFile;
    }
    if (!pass1(expandedFile)) return false;
    return pass2(expandedFile, outputFile);
}
void Assembler::initOpcodeTable() {}
