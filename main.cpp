#include <iostream>
#include <fstream>

#include <cstdint>
#include "CPU.cpp"

// emulate the cpu for the gameboy
int main() {
    std::cout << "hello" << std::endl;

    CPU *cpu = new CPU();



    std::ifstream inputFile("test-roms/prog_OR.gb", std::ios::binary); // Open the file
    if (!inputFile) {
        std::cerr << "Error opening the file!" << std::endl;
        return 1;
    }

    char cmd;
    // main loop
    while (inputFile.read(&cmd, sizeof(cmd))) {
        std::cout << "PC=" << cpu->PC << ": cmd=" << static_cast<int>(cmd) << std::endl;

        cpu->execute(static_cast<uint8_t>(cmd));

        // increment PC
        cpu->PC++;
    }

}