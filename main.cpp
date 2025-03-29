#include <iostream>
#include <fstream>

#include <cstdint>
// #include <cstdlib>
#include <cstdio>
#include "CPU.cpp"

// emulate the cpu for the gameboy
int main() {
    CPU *cpu = new CPU();

    std::ifstream inputFile("test-roms/prog_OR.gb", std::ios::binary); // Open the file
    if (!inputFile) {
        std::cerr << "Error opening the file!" << std::endl;
        return 1;
    }

    char cmd;
    // main loop
    while (inputFile.read(&cmd, sizeof(cmd))) {
        // std::cout << "PC=" << cpu->PC << ": cmd=" << static_cast<int>(cmd) << std::endl;
        printf ("PC=%d: cmd=0x%0x\n", cpu->PC, static_cast<int>(cmd));

        cpu->execute(static_cast<uint8_t>(cmd));

        // increment PC
        cpu->PC++;
    }

}