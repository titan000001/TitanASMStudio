#include "Assembler.h"
#include <iomanip>

Assembler::Assembler() {
    locationCounter = 0x100; // specific for COM files / emu8086 default
    startAddress = 0x100;
}

// Minimal helpers
std::string Assembler::trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (std::string::npos == first) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

// Helper to remove comma from token (e.g. "AX," -> "AX")
std::string cleanToken(std::string t) {
    if (!t.empty() && t.back() == ',') t.pop_back();
    return t;
}

// Simplified Parse Number (handles 4Ch, 100h)
int parseNumber(std::string numStr) {
    try {
        if (numStr.back() == 'h' || numStr.back() == 'H') {
            numStr.pop_back();
            return std::stoi(numStr, nullptr, 16);
        }
        return std::stoi(numStr);
    } catch (...) { return 0; }
}

bool Assembler::pass1(const std::string& inputFile) {
    std::ifstream inFile(inputFile);
    if (!inFile.is_open()) return false;
    
    std::string line;
    locationCounter = 0x100;
    
    while (std::getline(inFile, line)) {
        line = trim(line);
        if (line.empty() || line[0] == ';') continue;
        
        std::stringstream ss(line);
        std::string token; ss >> token;
        
        // Label Check
        if (token.back() == ':') {
            std::string label = token.substr(0, token.length() - 1);
            symbolTable[label] = locationCounter;
            ss >> token; // Consume label, check next token
        }
        
        if (token.empty()) continue;
        
        // Location Estimation (Must match Pass 2 exactly)
        if (token == "org") {
            int val; ss >> std::hex >> val; locationCounter = val;
        }
        if (token == "mov") locationCounter += 3;
        else if (token == "int") locationCounter += 2;
        else if (token == "printn") locationCounter += 3;
        else if (token == "push") locationCounter += 4;
        else if (token == "pop") locationCounter += 4;
        else if (token == "call") locationCounter += 4;
        else if (token == "ret") locationCounter += 4;
        // ... add others
    }
    return true;
}

bool Assembler::pass2(const std::string& inputFile, const std::string& outputFile) {
    std::ifstream inFile(inputFile);
    std::ofstream outFile(outputFile);
    if (!inFile.is_open() || !outFile.is_open()) return false;

    std::string line;
    locationCounter = 0x100;
    
    // String Data Storage (Simple Bump Allocator starting far ahead)
    int stringHeap = 0x200; // Store strings starting at offset 512

    outFile << "ADDR CODE" << std::endl;

    while (std::getline(inFile, line)) {
        line = trim(line);
        if (line.empty() || line[0] == ';') continue;

        std::stringstream ss(line);
        std::string opcode;
        ss >> opcode;
        
        // Directives to ignore
        if (opcode == "include" || opcode == ".data" || opcode == ".code" 
            || opcode == "main" || opcode == "end" || opcode == "endp") continue;

        if (opcode == "org") {
            std::string arg; ss >> arg;
            locationCounter = parseNumber(arg);
            continue;
        }

        std::string bytes = "";

        if (opcode == "printn") {
            // Format: printn "String"
            // We need to extract the string between quotes
            size_t q1 = line.find('"');
            size_t q2 = line.rfind('"');
            if (q1 != std::string::npos && q2 != std::string::npos && q2 > q1) {
                std::string content = line.substr(q1 + 1, q2 - q1 - 1);
                
                // Opcode for PRINTN: 20
                bytes = "20 "; 
                
                // Write string to memory map (hacky: put it in the output file at heap addr)
                // We encode the Address of the string:
                int sAddr = stringHeap;
                bytes += std::to_string(sAddr & 0xFF) + " " + std::to_string((sAddr >> 8) & 0xFF);
                
                // Advance Heap
                stringHeap += content.length() + 1; // +1 for null terminator
                
                // Emit SIDE-EFFECT: Write the string data to file too?
                // The Simulator Load needs to handle "Address Byte Byte...".
                // We will append the string data at end of file logic or special lines.
                // Output format hack: "ADDRESS BYTE BYTE..."
                
                // Let's write the instruction bytes
                outFile << locationCounter << " " << bytes << std::endl;
                locationCounter += 3;

                // Write the String Data immediately? No, at the specific address.
                outFile << sAddr << " ";
                for (char c : content) outFile << std::hex << (int)c << " ";
                outFile << "00" << std::endl; // Null term
            }
        } 
        else if (opcode == "mov") {
            // mov ah, 4ch
            std::string dest, src;
            ss >> dest >> src;
            dest = cleanToken(dest);
            src = cleanToken(src);

            if ((dest == "ah" || dest == "AH") && src.back() == 'h') {
                // MOV AH, Imm
                int val = parseNumber(src);
                // Opcode 01 (MOV Reg8 Imm), 01 (ID of AH), Val
                outFile << locationCounter << " 01 01 " << std::hex << val << std::endl;
                locationCounter += 3;
            }
        }
        else if (opcode == "int") {
            std::string intNum; ss >> intNum;
            int val = parseNumber(intNum); // 21h -> 33
            
            // Opcode 10 (INT), Val
            outFile << locationCounter << " 10 " << std::hex << val << std::endl;
            locationCounter += 2;
        }
        else if (opcode == "push") {
            // push ax or push 5
            std::string arg; ss >> arg; arg = cleanToken(arg);
            // Opcode 30
            // Format: 30 TYPE VAL
            // Type 1=Reg, 2=Imm
            int type = 2; // Default Imm
            int val = 0;
            if (arg.size() == 2 && (arg[1] == 'x' || arg[1] == 'X')) { // AX, BX..
                 // Reg Mapping: AX=0, BX=2 ?? No, Simulator uses specific ID logic.
                 // Let's reuse MOV logic IDs? 
                 // Simulator mov: 0=AL, 1=AH? Wait, Simulator.cpp mapping is ad-hoc in switch.
                 // Let's standardize register IDs for Titan Upgrade:
                 // 0: AX, 1: BX, 2: CX, 3: DX? (16 bit)
                 type = 1;
                 if (arg == "ax" || arg == "AX") val = 0;
                 else if (arg == "bx" || arg == "BX") val = 1;
                 else if (arg == "cx" || arg == "CX") val = 2;
                 else if (arg == "dx" || arg == "DX") val = 3;
            } else {
                 val = parseNumber(arg);
            }
            outFile << locationCounter << " 30 " << type << " " << std::hex << val << std::endl;
            locationCounter += 4; // 1(Op) + 1(Type) + 2(Val)
        }
        else if (opcode == "pop") {
            // pop ax
            std::string arg; ss >> arg; arg = cleanToken(arg);
            int val = 0; // Reg ID
            if (arg == "ax" || arg == "AX") val = 0;
            else if (arg == "bx" || arg == "BX") val = 1;
            else if (arg == "cx" || arg == "CX") val = 2;
            else if (arg == "dx" || arg == "DX") val = 3;
            
            // Opcode 31
            outFile << locationCounter << " 31 1 " << std::hex << val << std::endl;
            locationCounter += 4;
        }
        else if (opcode == "call") {
             // call Label
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
    }

    inFile.close();
    outFile.close();
    return true;
}

// Simplified Interface
void Assembler::initOpcodeTable() { } 
bool Assembler::assemble(const std::string& inputFile, const std::string& outputFile) {
    return pass2(inputFile, outputFile);
}
