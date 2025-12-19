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
    // In this Lite version, just find string literals for PRINTN and allocate them?
    // Or just do everything in Pass 2 since we don't have backward jumps in the demo?
    // Let's keep Pass 1 empty/minimal or use it for Labels if we were robust.
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
