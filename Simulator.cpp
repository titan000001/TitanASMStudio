#include "Simulator.h"

Simulator::Simulator(int memorySize) {
    memory.resize(memorySize, 0);
    AX.X = 0; BX.X = 0; CX.X = 0; DX.X = 0;
    IP = 0;
    running = false;
    ZF = false;
}

// Helpers for Register Access
uint16_t* Simulator::getRegisterPtr16(const std::string& regName) {
    if (regName == "AX") return &AX.X;
    if (regName == "BX") return &BX.X;
    if (regName == "CX") return &CX.X;
    if (regName == "DX") return &DX.X;
    return nullptr;
}

uint8_t* Simulator::getRegisterPtr8(const std::string& regName) {
    if (regName == "AL") return &AX.L;
    if (regName == "AH") return &AX.H;
    if (regName == "BL") return &BX.L;
    if (regName == "BH") return &BX.H;
    if (regName == "CL") return &CX.L;
    if (regName == "CH") return &CX.H;
    if (regName == "DL") return &DX.L;
    if (regName == "DH") return &DX.H;
    return nullptr;
}

bool Simulator::load(const std::string& objectFile) {
    std::ifstream file(objectFile);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open object file." << std::endl;
        return false;
    }

    std::string line;
    std::getline(file, line); // Skip Header

    while (std::getline(file, line)) {
        if (line.empty()) continue;
        std::stringstream ss(line);
        std::string type, valStr;
        int address;
        
        // Format: ADDR  TYPE  VALUE
        // Types:
        // CODE <OP> <ARGS...>
        // DATA <VAL>
        // We need a consistent format from the new Assembler.
        // Let's assume the Assembler will produce:
        // ADDRESS  OPCODE  OP1_TYPE OP1_VAL  OP2_TYPE OP2_VAL
        
        // Because I haven't written the assembler yet, I will define the contract here.
        // Assembler will write integers to memory directly.
        // This load function just reads "Address Value" pairs for simplicity?
        // Or "Address Opcode Operands..."
        
        std::string addrToken;
        ss >> addrToken;
        if (addrToken.empty()) continue;
        
        try {
            address = std::stoi(addrToken);
            
            // Read bytes into memory
            int byteVal;
            while (ss >> std::hex >> byteVal) {
                if (address < memory.size()) {
                    memory[address++] = (uint8_t)byteVal;
                }
            }
        } catch (...) {}
    }
    
    // Set Entry Point (Default 0x100 for COM format / emu8086)
    IP = 0x100; 
    return true;
}

void Simulator::run() {
    running = true;
    int maxCycles = 5000;
    int cycles = 0;
    
    std::cout << "--- emu8086 Simulation Started (IP=" << std::hex << IP << ") ---" << std::endl;

    while (running && cycles < maxCycles) {
        // Fetch Opcode
        uint8_t opcode = memory[IP];
        IP++;

        // NOTE: This is a Simplified Opcode Set, NOT real x86 machine code.
        // 0x01 = MOV Reg, Imm
        // 0x02 = MOV Reg, Reg
        // 0x10 = INT
        // 0x20 = PRINTN String Literal (Address)
        // 0x00 = HALT
        
        switch (opcode) {
            case 0x01: // MOV Reg8, Imm8
            {
                uint8_t regID = memory[IP++];
                uint8_t val = memory[IP++];
                // Map ID to AH, AL... 0=AL, 1=AH...
                // Hardcoding mapping for demo: 0=AL, 1=AH
                if (regID == 1) AX.H = val; // MOV AH, Imm
                break;
            }
            case 0x10: // INT Imm8
            {
                uint8_t intNo = memory[IP++];
                if (intNo == 0x21) {
                    if (AX.H == 0x4C) { // Exit
                        running = false;
                    }
                    else if (AX.H == 0x09) { // Print String at DX
                        // Not implemented for this exact demo, using PRINTN instead
                    }
                }
                break;
            }
            case 0x20: // PRINTN Address (16-bit)
            {
                uint8_t msgL = memory[IP++];
                uint8_t msgH = memory[IP++];
                uint16_t msgAddr = (msgH << 8) | msgL;
                
                // Print string at msgAddr until \0 or '$'
                while (memory[msgAddr] != 0 && memory[msgAddr] != '$') {
                    if (memory[msgAddr] == '"') { msgAddr++; continue; } // wrapper
                    std::cout << (char)memory[msgAddr];
                    msgAddr++;
                }
                std::cout << std::endl;
                break;
            }
            case 0x00: // HALT/NOP
                if (memory[IP] == 0 && memory[IP+1] == 0) running = false; // Safety
                break;
            
            default:
                // Assume data or padding
                break;
        }
        cycles++;
    }
    
    std::cout << "--- Simulation Finished ---" << std::endl;
}
