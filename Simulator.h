#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <map>
#include <cstdint>

// 8086 Register Structure
union Register {
    uint16_t X;
    struct {
        uint8_t L;
        uint8_t H;
    };
};

class Simulator {
private:
    std::vector<uint8_t> memory; // Check: Changed to byte-addressable memory for realism? 
                                 // Or stick to Word memory for simplicity?
                                 // emu8086 is byte-addressable. "Hello" is bytes. 
                                 // Let's us byte memory [65536].
    
    Register AX, BX, CX, DX;
    uint16_t IP; // Instruction Pointer (PC)
    
    // Flags
    bool ZF; // Zero Flag
    bool running;

    int getRegisterValue(const std::string& regName);
    void setRegisterValue(const std::string& regName, int value);
    uint8_t* getRegisterPtr8(const std::string& regName); // For AL, AH
    uint16_t* getRegisterPtr16(const std::string& regName); // For AX

public:
    Simulator(int memorySize = 65536);
    bool load(const std::string& objectFile);
    void run();
};

#endif
