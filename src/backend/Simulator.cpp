#include "Simulator.h"

Simulator::Simulator(int memorySize) {
    memory.resize(memorySize, 0);
    AX.X = 0; BX.X = 0; CX.X = 0; DX.X = 0;
    IP = 0;
    running = false;
    ZF = false;
}

uint16_t* Simulator::getRegisterPtr16(const std::string& regName) {
    if (regName == "AX") return &AX.X;
    if (regName == "BX") return &BX.X;
    if (regName == "CX") return &CX.X;
    if (regName == "DX") return &DX.X;
    return nullptr;
}

void Simulator::push(uint16_t val) {
    SP -= 2;
    memory[SP] = val & 0xFF;
    memory[SP+1] = (val >> 8) & 0xFF;
}

uint16_t Simulator::pop() {
    uint8_t low = memory[SP];
    uint8_t high = memory[SP+1];
    SP += 2;
    return (high << 8) | low;
}

bool Simulator::load(const std::string& objectFile) {
    std::ifstream file(objectFile);
    if (!file.is_open()) return false;

    std::string line;
    std::getline(file, line); // Skip Header

    while (std::getline(file, line)) {
        if (line.empty()) continue;
        std::stringstream ss(line);
        std::string addrToken;
        ss >> addrToken;
        if (addrToken.empty()) continue;
        
        try {
            int address = std::stoi(addrToken, nullptr, 16);
            int byteVal;
            while (ss >> std::hex >> byteVal) {
                if (address < memory.size()) {
                    memory[address++] = (uint8_t)byteVal;
                }
            }
        } catch (...) {}
    }
    IP = 0x100; 
    return true;
}

void Simulator::run(bool debugMode) {
    running = true;
    int maxCycles = 5000;
    int cycles = 0;
    if (SP == 0) SP = 0xFFFE;

    if (!debugMode) std::cout << "--- TitanASM Simulation Started (IP=0100) ---" << std::endl;
    else std::cout << "DEBUG_MODE_START" << std::endl;

    while (running && cycles < maxCycles) {
        if (debugMode) {
            std::cout << "DEBUG|" 
                      << std::hex << std::setw(4) << std::setfill('0') << IP << "|"
                      << std::setw(4) << AX.X << "|"
                      << std::setw(4) << BX.X << "|"
                      << std::setw(4) << CX.X << "|"
                      << std::setw(4) << DX.X << "|"
                      << std::setw(4) << SP << "|"
                      << (ZF?"1":"0") << std::endl;
            char cmd; std::cin >> cmd;
            if (cmd == 'q') { running = false; break; }
            if (cmd == 'r') { debugMode = false; }
        }

        uint8_t opcode = memory[IP++];
        switch (opcode) {
            case 0x01: // MOV Reg, Imm
            {
                uint8_t regID = memory[IP++];
                uint16_t val = memory[IP++];
                val |= (memory[IP++] << 8);
                uint8_t* r = nullptr;
                if (regID == 0) r = &AX.L; else if (regID == 1) r = &AX.H;
                else if (regID == 2) r = &BX.L; else if (regID == 3) r = &BX.H;
                else if (regID == 4) r = &CX.L; else if (regID == 5) r = &CX.H;
                else if (regID == 6) r = &DX.L; else if (regID == 7) r = &DX.H;
                if (r) *r = (uint8_t)val;
                break;
            }
            case 0x02: // MOV Reg, Reg
            {
                uint8_t destID = memory[IP++];
                uint8_t srcID = memory[IP++];
                uint8_t srcVal = 0;
                if (srcID == 0) srcVal = AX.L; else if (srcID == 1) srcVal = AX.H;
                else if (srcID == 2) srcVal = BX.L; else if (srcID == 3) srcVal = BX.H;
                else if (srcID == 4) srcVal = CX.L; else if (srcID == 5) srcVal = CX.H;
                else if (srcID == 6) srcVal = DX.L; else if (srcID == 7) srcVal = DX.H;
                uint8_t* d = nullptr;
                if (destID == 0) d = &AX.L; else if (destID == 1) d = &AX.H;
                else if (destID == 2) d = &BX.L; else if (destID == 3) d = &BX.H;
                else if (destID == 4) d = &CX.L; else if (destID == 5) d = &CX.H;
                else if (destID == 6) d = &DX.L; else if (destID == 7) d = &DX.H;
                if (d) *d = srcVal;
                break;
            }
            case 0x03: // ADD
            case 0x04: // SUB
            {
                bool isSub = (opcode == 0x04);
                uint8_t destID = memory[IP++];
                uint8_t type = memory[IP++];
                uint16_t srcVal = memory[IP++];
                if (type == 1) { // Reg
                    uint8_t sID = (uint8_t)srcVal;
                    if (sID == 0) srcVal = AX.L; else if (sID == 1) srcVal = AX.H;
                    else if (sID == 2) srcVal = BX.L; else if (sID == 3) srcVal = BX.H;
                    else if (sID == 4) srcVal = CX.L; else if (sID == 5) srcVal = CX.H;
                    else if (sID == 6) srcVal = DX.L; else if (sID == 7) srcVal = DX.H;
                }
                uint8_t* d = nullptr;
                if (destID == 0) d = &AX.L; else if (destID == 1) d = &AX.H;
                else if (destID == 2) d = &BX.L; else if (destID == 3) d = &BX.H;
                else if (destID == 4) d = &CX.L; else if (destID == 5) d = &CX.H;
                else if (destID == 6) d = &DX.L; else if (destID == 7) d = &DX.H;
                if (d) { 
                    if (isSub) *d -= (uint8_t)srcVal; else *d += (uint8_t)srcVal; 
                    ZF = (*d == 0);
                }
                break;
            }
            case 0x07: // CMP
            {
                uint8_t destID = memory[IP++];
                uint8_t type = memory[IP++];
                uint16_t srcVal = memory[IP++];
                if (type == 1) { // Reg
                    uint8_t sID = (uint8_t)srcVal;
                    if (sID == 0) srcVal = AX.L; else if (sID == 1) srcVal = AX.H;
                    else if (sID == 2) srcVal = BX.L; else if (sID == 3) srcVal = BX.H;
                }
                uint8_t destVal = 0;
                if (destID == 0) destVal = AX.L; else if (destID == 1) destVal = AX.H;
                else if (destID == 2) destVal = BX.L; else if (destID == 3) destVal = BX.H;
                ZF = (destVal == (uint8_t)srcVal);
                break;
            }
            case 0x05: // Load
            {
                 uint8_t destID = memory[IP++];
                 uint16_t addr = memory[IP++];
                 addr |= (memory[IP++] << 8);
                 uint8_t val = memory[addr];
                 if (destID == 0) AX.L = val; else if (destID == 1) AX.H = val;
                 else if (destID == 2) BX.L = val; else if (destID == 3) BX.H = val;
                 break;
            }
            case 0x06: // Store
            {
                 uint16_t addr = memory[IP++];
                 addr |= (memory[IP++] << 8);
                 uint8_t srcID = memory[IP++];
                 uint8_t val = 0;
                 if (srcID == 0) val = AX.L; else if (srcID == 1) val = AX.H;
                 else if (srcID == 2) val = BX.L; else if (srcID == 3) val = BX.H;
                 memory[addr] = val;
                 break;
            }
            case 0x10: // INT
            {
                uint8_t intNo = memory[IP++];
                if (intNo == 0x21) {
                    if (AX.H == 0x4C) running = false;
                    else if (AX.H == 0x01) {
                        if (!debugMode) std::cout << "Input Required: ";
                        char c; std::cin >> c;
                        std::cout << c << std::endl;
                        AX.L = c;
                    }
                    else if (AX.H == 0x02) std::cout << (char)DX.L;
                    else if (AX.H == 0x09) { // String Print
                         uint16_t addr = (DX.X); // Using DS:DX (DS implied same segment)
                         // Since our memory model is flat for now (small model), DX is offset
                         while (addr < memory.size() && memory[addr] != '$') {
                             std::cout << (char)memory[addr++];
                         }
                    }
                }
                break;
            }
            case 0x20: // PRINTN
            {
                uint16_t addr = memory[IP++];
                addr |= (memory[IP++] << 8);
                while (memory[addr] != 0 && memory[addr] != '$') {
                    std::cout << (char)memory[addr++];
                }
                std::cout << std::endl;
                break;
            }
            case 0x30: // PUSH
            {
                uint8_t type = memory[IP++];
                uint16_t val = memory[IP++];
                val |= (memory[IP++] << 8);
                if (type == 1) {
                    if (val == 0) val = AX.X; else if (val == 1) val = BX.X;
                    else if (val == 2) val = CX.X; else if (val == 3) val = DX.X;
                }
                push(val);
                break;
            }
            case 0x31: // POP
            {
                uint8_t type = memory[IP++];
                uint8_t regID = memory[IP++];
                IP++; // Padding
                uint16_t val = pop();
                if (regID == 0) AX.X = val; else if (regID == 1) BX.X = val;
                else if (regID == 2) CX.X = val; else if (regID == 3) DX.X = val;
                break;
            }
            case 0x32: // CALL
            {
                uint8_t type = memory[IP++];
                uint16_t addr = memory[IP++];
                addr |= (memory[IP++] << 8);
                push(IP); IP = addr;
                break;
            }
            case 0x33: // RET
            {
                IP += 3; // Padding
                IP = pop();
                break;
            }
            case 0x40: // JMP
            case 0x41: // JZ
            case 0x42: // JNZ
            {
                uint8_t type = memory[IP++];
                uint16_t addr = memory[IP++];
                addr |= (memory[IP++] << 8);
                if (opcode == 0x40) IP = addr;
                else if (opcode == 0x41 && ZF) IP = addr;
                else if (opcode == 0x42 && !ZF) IP = addr;
                break;
            }
            case 0x50: // MUL r8
            {
                uint8_t srcID = memory[IP++];
                IP++; // Skip 00
                uint8_t srcVal = 0;
                if (srcID == 0) srcVal = AX.L; else if (srcID == 1) srcVal = AX.H;
                else if (srcID == 2) srcVal = BX.L; else if (srcID == 3) srcVal = BX.H;
                else if (srcID == 4) srcVal = CX.L; else if (srcID == 5) srcVal = CX.H;
                else if (srcID == 6) srcVal = DX.L; else if (srcID == 7) srcVal = DX.H;
                
                uint16_t res = (uint16_t)AX.L * (uint16_t)srcVal;
                AX.X = res;
                // Flags not fully implemented but ZF usually updated
                ZF = (AX.X == 0);
                break;
            }
            case 0x51: // DIV r8
            {
                uint8_t srcID = memory[IP++];
                IP++; // Skip 00
                uint8_t srcVal = 0;
                if (srcID == 0) srcVal = AX.L; else if (srcID == 1) srcVal = AX.H;
                else if (srcID == 2) srcVal = BX.L; else if (srcID == 3) srcVal = BX.H;
                else if (srcID == 4) srcVal = CX.L; else if (srcID == 5) srcVal = CX.H;
                else if (srcID == 6) srcVal = DX.L; else if (srcID == 7) srcVal = DX.H;
                
                if (srcVal == 0) {
                     std::cout << "Divide Error" << std::endl;
                     running = false;
                } else {
                     AX.L = AX.X / srcVal; // Quotient
                     AX.H = AX.X % srcVal; // Remainder
                }
                break;
            }
            case 0x15: // LEA
            {
                uint8_t destID = memory[IP++];
                uint16_t addr = memory[IP++];
                addr |= (memory[IP++] << 8);
                
                if (destID == 0) AX.X = addr; // Usually LEA loads to 16-bit reg, we map 0->AX
                else if (destID == 2) BX.X = addr;
                else if (destID == 4) CX.X = addr;
                else if (destID == 6) DX.X = addr;
                break;
            }
            default: running = false; break;
        }
        cycles++;
    }
    if (!debugMode) {
        std::cout << "\n--- Simulation Finished ---" << std::endl;
        std::cout << "Press Enter to exit..." << std::endl;
        std::cin.ignore();
        std::cin.get();
    }
}
