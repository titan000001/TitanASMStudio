#include "Assembler.h"
#include <iomanip>

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

std::string cleanToken(std::string t) {
    if (!t.empty() && t.back() == ',') t.pop_back();
    return t;
}

int parseNumber(std::string numStr) {
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
    // 16 bit
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
    int dataCounter = 0x800; // Variables start here
    
    while (std::getline(inFile, line)) {
        line = trim(line);
        if (line.empty() || line[0] == ';') continue;
        
        std::stringstream ss(line);
        std::string token; ss >> token;
        
        if (token.empty()) continue; // Safety check forced

        // Label Definition
        if (token.back() == ':') {
            std::string label = token.substr(0, token.length() - 1);
            symbolTable[label] = locationCounter;
            ss >> token; // Move next
        }
        
        if (token.empty()) continue; // Check again after consume

        // Variable Definition (VAR DB VAL)
        // If first token (after label check) is a name, and second is DB
        // Simple heuristic: If token is unknown opcode and next is DB
        if (token != "mov" && token != "add" && token != "sub" && token != "int" && token != "org" && token != "main" && token[0] != '.') {
            std::string nextTok;
            std::streampos oldPos = ss.tellg();
            ss >> nextTok;
            if (nextTok == "db" || nextTok == "DB") {
                 symbolTable[token] = dataCounter++;
                 continue; // It's data
            }
            // Restore? No urgency, P1 is just scanning.
        }
        
        // Location Upates
        if (token.empty()) continue;
        if (token == "org") { int val; ss >> std::hex >> val; locationCounter = val; }
        else if (token == "mov") locationCounter += 4; // Max size
        else if (token == "add" || token == "sub") locationCounter += 6;
        else if (token == "int") locationCounter += 2;
        else if (token == "push" || token == "pop" || token == "call" || token == "ret") locationCounter += 4;
        else if (token == "printn") locationCounter += 3;
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
        

        // Check if Variable Decl (Skip code gen for now, handled in P1)
        // But we DO want to initialize memory in Simulator.
        // If "VAR DB 5", we write "800 5".
        if (line.find(" db ") != std::string::npos || line.find(" DB ") != std::string::npos) {
             std::stringstream ss(line);
             std::string label, type, valStr;
             ss >> label >> type >> valStr;
             if (type == "db" || type == "DB") {
                 int val = parseNumber(valStr);
                 outFile << dataCounter << " " << std::hex << val << std::endl;
                 dataCounter++;
                 continue;
             }
        }

        // Remove Label
        size_t colon = line.find(':');
        if (colon != std::string::npos) line = trim(line.substr(colon + 1));
        if (line.empty()) continue;

        std::stringstream ss(line);
        std::string opcode;
        ss >> opcode;
        
        // Directives
        if (opcode[0] == '.') continue; // .model, .stack, .code
        if (opcode == "main" || opcode == "endp" || opcode == "end" || opcode == "include") continue;

        if (opcode == "org") {
            int val; ss >> std::hex >> val; locationCounter = val; continue;
        }

        if (opcode == "mov") {
            std::string destStr, srcStr;
            ss >> destStr >> srcStr;
            destStr = cleanToken(destStr);
            srcStr = cleanToken(srcStr);
            
            int dest = getRegID(destStr);
            int src = getRegID(srcStr);
            
            if (src != -1 && dest != -1) { // Reg to Reg
                outFile << locationCounter << " 02 " << dest << " " << src << std::endl;
                locationCounter += 3;
            } else if (dest != -1) { // MOV Reg, Imm or Mem
                if (symbolTable.count(srcStr)) {
                    int addr = symbolTable[srcStr];
                    outFile << locationCounter << " 05 " << dest << " " << (addr&0xFF) << " " << ((addr>>8)&0xFF) << std::endl;
                    locationCounter += 4;
                } else {
                    int val = parseNumber(srcStr);
                    outFile << locationCounter << " 01 " << dest << " " << std::hex << (val & 0xFF) << " " << ((val >> 8) & 0xFF) << std::endl;
                    locationCounter += 4;
                }
            } else if (src != -1) { // MOV [Mem], Reg
                 if (symbolTable.count(destStr)) {
                     int addr = symbolTable[destStr];
                     outFile << locationCounter << " 06 " << (addr&0xFF) << " " << ((addr>>8)&0xFF) << " " << src << std::endl;
                     locationCounter += 4; // Op(1)+Addr(2)+Reg(1)
                 }
            }
        }
        else if (opcode == "add" || opcode == "sub") {
            std::string destStr, srcStr;
            ss >> destStr >> srcStr;
            destStr = cleanToken(destStr);
            srcStr = cleanToken(srcStr);
            int dest = getRegID(destStr);
            int op = (opcode == "add") ? 3 : 4;
            int srcID = getRegID(srcStr);
            
            if (srcID != -1) { // Reg
                 outFile << locationCounter << " 0" << op << " " << dest << " 1 " << srcID << " 00" << std::endl;
            } else { // Imm
                 int val = parseNumber(srcStr);
                 outFile << locationCounter << " 0" << op << " " << dest << " 2 " << (val&0xFF) << " " << ((val>>8)&0xFF) << std::endl;
            }
            locationCounter += 6; 
        }
        else if (opcode == "int") {
            std::string arg; ss >> arg; arg = cleanToken(arg);
            int val = parseNumber(arg);
            outFile << locationCounter << " 10 " << std::hex << val << std::endl;
            locationCounter += 2;
        }
        else if (opcode == "push") {
            std::string arg; ss >> arg; arg = cleanToken(arg);
            int type = 2; int val = 0;
            if (getRegID(arg) != -1) { type = 1; val = getRegID(arg); }
            else val = parseNumber(arg);
            outFile << locationCounter << " 30 " << type << " " << std::hex << val << std::endl;
            locationCounter += 4;
        }
        else if (opcode == "pop") {
            std::string arg; ss >> arg; arg = cleanToken(arg);
            int val = getRegID(arg);
            outFile << locationCounter << " 31 1 " << std::hex << val << std::endl;
            locationCounter += 4;
        }
        else if (opcode == "call") {
             std::string lbl; ss >> lbl; lbl = cleanToken(lbl);
             int addr = 0;
             if (symbolTable.count(lbl)) addr = symbolTable[lbl];
             outFile << locationCounter << " 32 2 " << std::hex << addr << std::endl; 
             locationCounter += 4;
        }
        else if (opcode == "ret") {
             outFile << locationCounter << " 33 0 0" << std::endl;
             locationCounter += 4;
        }
        else if (opcode == "printn") {
            size_t q1 = line.find('"');
            size_t q2 = line.rfind('"');
            if (q1 != std::string::npos && q2 != std::string::npos && q2 > q1) {
                std::string content = line.substr(q1 + 1, q2 - q1 - 1);
                outFile << locationCounter << " 20 00 00" << std::endl; // Should be proper address
                locationCounter += 3;
            }
        }
    }
    
    inFile.close();
    outFile.close();
    return true;
}

void Assembler::initOpcodeTable() { } 
bool Assembler::assemble(const std::string& inputFile, const std::string& outputFile) {
    if (!pass1(inputFile)) return false;
    return pass2(inputFile, outputFile);
}
