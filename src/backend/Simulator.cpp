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

// Stack Helpers (Little Endian in Memory)
void Simulator::push(uint16_t val) {
    SP -= 2;
    memory[SP] = val & 0xFF;        // Low Byte
    memory[SP+1] = (val >> 8) & 0xFF; // High Byte
}

uint16_t Simulator::pop() {
    uint8_t low = memory[SP];
    uint8_t high = memory[SP+1];
    SP += 2;
    return (high << 8) | low;
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
        std::string addrToken;
        ss >> addrToken;
        if (addrToken.empty()) continue;
        
        try {
            address = std::stoi(addrToken);
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


void Simulator::run(bool debugMode) { // Changed signature
    running = true;
    int maxCycles = 5000;
    int cycles = 0;
    
    // Initialize Stack Pointer if 0 (Safety)
    if (SP == 0) SP = 0xFFFE;

    if (!debugMode) std::cout << "--- emu8086 Simulation Started (IP=" << std::hex << IP << ") ---" << std::endl;
    else std::cout << "DEBUG_MODE_START" << std::endl;

    while (running && cycles < maxCycles) {
        if (debugMode) {
            // Print State: DEBUG|IP|AX|BX|CX|DX|SP|ZF
            std::cout << "DEBUG|" 
                      << std::hex << std::setw(4) << std::setfill('0') << IP << "|"
                      << std::setw(4) << AX.X << "|"
                      << std::setw(4) << BX.X << "|"
                      << std::setw(4) << CX.X << "|"
                      << std::setw(4) << DX.X << "|"
                      << std::setw(4) << SP << "|"
                      << (ZF?"1":"0") << std::endl;
            
            // Wait for command
            char cmd;
            std::cin >> cmd;
            if (cmd == 'q') { running = false; break; }
            if (cmd == 'r') { debugMode = false; } // Continue without pausing
            // if 's', convert to just continuing loop once
        }

        // Fetch Opcode
        uint8_t opcode = memory[IP];
        IP++;

        switch (opcode) {
            case 0x01: // MOV Reg, Imm
            {
                uint8_t regID = memory[IP++];
                uint16_t val = 0;
                // Check if 8-bit or 16-bit imm? Assembler should specify.
                // Assuming simplified protocol: if regID < 8, it's 8-bit.
                // We read 1 byte for 8-bit reg? 
                // Let's assume Assembler always emits 16-bit val for simplicity in parsing, 
                // but we cast down.
                // Or better: read type byte?
                // Let's stick to: "MOV Reg ID, Val (16-bit)"
                val = memory[IP++];
                val |= (memory[IP++] << 8); // Read 16-bit Imm
                
                if (regID == 0) AX.L = (uint8_t)val;
                else if (regID == 1) AX.H = (uint8_t)val;
                else if (regID == 2) BX.L = (uint8_t)val;
                else if (regID == 3) BX.H = (uint8_t)val;
                else if (regID == 4) CX.L = (uint8_t)val;
                else if (regID == 5) CX.H = (uint8_t)val;
                else if (regID == 6) DX.L = (uint8_t)val;
                else if (regID == 7) DX.H = (uint8_t)val;
                // 16-bit Access?
                // Left out for brevity unless requested.
                break;
            }
            case 0x02: // MOV Reg1, Reg2
            {
                uint8_t destID = memory[IP++];
                uint8_t srcID = memory[IP++];
                
                uint8_t val = 0;
                // Read Src
                if (srcID == 0) val = AX.L;
                else if (srcID == 1) val = AX.H;
                else if (srcID == 2) val = BX.L;
                else if (srcID == 3) val = BX.H;
                else if (srcID == 4) val = CX.L;
                else if (srcID == 5) val = CX.H;
                else if (srcID == 6) val = DX.L;
                else if (srcID == 7) val = DX.H;
                
                // Write Dest
                if (destID == 0) AX.L = val;
                else if (destID == 1) AX.H = val;
                else if (destID == 2) BX.L = val;
                else if (destID == 3) BX.H = val;
                else if (destID == 4) CX.L = val;
                else if (destID == 5) CX.H = val;
                else if (destID == 6) DX.L = val;
                else if (destID == 7) DX.H = val;
                break;
            }
            case 0x03: // ADD Dest, Src (Reg, Reg/Imm)
            case 0x04: // SUB Dest, Src
            {
                 bool isSub = (opcode == 0x04);
                 uint8_t destID = memory[IP++];
                 uint8_t type = memory[IP++]; // 1=Reg, 2=Imm
                 uint16_t srcVal = 0;
                 
                 if (type == 1) { // Reg
                     uint8_t srcID = memory[IP++];
                     memory[IP++]; // Padding to align?
                     if (srcID == 0) srcVal = AX.L;
                     else if (srcID == 1) srcVal = AX.H;
                     else if (srcID == 2) srcVal = BX.L;
                     else if (srcID == 3) srcVal = BX.H;
                     else if (srcID == 4) srcVal = CX.L;
                     else if (srcID == 5) srcVal = CX.H;
                     else if (srcID == 6) srcVal = DX.L;
                     else if (srcID == 7) srcVal = DX.H;
                 } else { // Imm
                     srcVal = memory[IP++];
                     srcVal |= (memory[IP++] << 8);
                 }
                 
                 // Perform Op on Dest
                 uint8_t* destPtr = nullptr;
                 if (destID == 0) destPtr = &AX.L;
                 else if (destID == 1) destPtr = &AX.H;
                 else if (destID == 2) destPtr = &BX.L;
                 else if (destID == 3) destPtr = &BX.H;
                 else if (destID == 4) destPtr = &CX.L;
                 else if (destID == 5) destPtr = &CX.H;
                 else if (destID == 6) destPtr = &DX.L;
                 else if (destID == 7) destPtr = &DX.H;
                 
                 if (destPtr) {
                     if (isSub) *destPtr -= (uint8_t)srcVal;
                     else *destPtr += (uint8_t)srcVal;
                 }
                 break;
            }

            case 0x05: // MOV Reg, [Addr] (Load)
            {
                 uint8_t destID = memory[IP++];
                 uint8_t low = memory[IP++];
                 uint8_t high = memory[IP++];
                 uint16_t addr = (high << 8) | low;
                 
                 uint8_t val = memory[addr];
                 
                 if (destID == 0) AX.L = val;
                 else if (destID == 1) AX.H = val;
                 else if (destID == 2) BX.L = val;
                 else if (destID == 3) BX.H = val;
                 else if (destID == 4) CX.L = val;
                 else if (destID == 5) CX.H = val;
                 else if (destID == 6) DX.L = val;
                 else if (destID == 7) DX.H = val;
                 break;
            }
            case 0x06: // MOV [Addr], Reg (Store)
            {
                 uint8_t addrL = memory[IP++];
                 uint8_t addrH = memory[IP++];
                 uint16_t addr = (addrH << 8) | addrL;
                 uint8_t srcID = memory[IP++];
                 
                 uint8_t val = 0;
                 if (srcID == 0) val = AX.L;
                 else if (srcID == 1) val = AX.H;
                 else if (srcID == 2) val = BX.L;
                 else if (srcID == 3) val = BX.H;
                 else if (srcID == 4) val = CX.L;
                 else if (srcID == 5) val = CX.H;
                 else if (srcID == 6) val = DX.L;
                 else if (srcID == 7) val = DX.H;
                 
                 memory[addr] = val;
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
                        // Logic simplified
                        std::cout << "StringOutput Stub";
                    }
                    else if (AX.H == 0x01) { // Input Char
                        char c;
                        if (debugMode) std::cout << "\n[INPUT_REQ]";
                        std::cin >> c;
                        AX.L = c; // Store in AL
                    }
                    else if (AX.H == 0x02) { // Output Char from DL
                        std::cout << (char)DX.L;
                    }
                }
                break;
            }
            case 0x20: // PRINTN Address
            {
                uint8_t msgL = memory[IP++];
                uint8_t msgH = memory[IP++];
                uint16_t msgAddr = (msgH << 8) | msgL;
                
                while (memory[msgAddr] != 0 && memory[msgAddr] != '$') {
                    if (memory[msgAddr] == '"') { msgAddr++; continue; } 
                    std::cout << (char)memory[msgAddr]; // This goes to stdout
                    msgAddr++;
                }
                if (!debugMode) std::cout << std::endl;
                else std::cout << "\n[CONSOLE_OUTPUT_NEWLINE]\n"; // Special marker for debug?
                break;
            }
            case 0x30: // PUSH Type Val
            {
                uint8_t type = memory[IP++]; // 1=Reg, 2=Imm
                uint16_t val = 0;
                if (type == 1) { // Reg
                     // Re-reading logic (simplified for diff)
                     IP--; 
                     uint8_t regIDByte = memory[IP++];
                     uint8_t padding = memory[IP++]; 
                     uint16_t* r = nullptr;
                     if (regIDByte == 0) r = &AX.X;
                     else if (regIDByte == 1) r = &BX.X;
                     else if (regIDByte == 2) r = &CX.X;
                     else if (regIDByte == 3) r = &DX.X;
                     if (r) val = *r;
                } else {
                     uint8_t low = memory[IP++];
                     uint8_t high = memory[IP++];
                     val = (high << 8) | low;
                }
                push(val);
                break;
            }
            case 0x31: // POP Reg
            {
                 uint8_t type = memory[IP++];
                 uint8_t regID = memory[IP++];
                 uint8_t padding = memory[IP++]; 
                 uint16_t val = pop();
                 uint16_t* r = nullptr;
                 if (regID == 0) r = &AX.X;
                 else if (regID == 1) r = &BX.X;
                 else if (regID == 2) r = &CX.X;
                 else if (regID == 3) r = &DX.X;
                 if (r) *r = val;
                 break;
            }
            case 0x32: // CALL Addr
            {
                 uint8_t type = memory[IP++];
                 uint8_t low = memory[IP++];
                 uint8_t high = memory[IP++];
                 uint16_t addr = (high << 8) | low;
                 push(IP); 
                 IP = addr; 
                 break;
            }
            case 0x33: // RET
            {
                 uint8_t pad1 = memory[IP++];
                 uint8_t pad2 = memory[IP++];
                 IP = pop();
                 break;
            }

            case 0x00: // HALT 
                if (memory[IP] == 0 && memory[IP+1] == 0) running = false;
                break;
            
            default:
                break;
        }
        cycles++;
    }
    
    if (!debugMode) std::cout << "--- Simulation Finished ---" << std::endl;
}
