#include <vector>

const int MEMORY_SIZE = 65536; // 2^16 locations for 16-bit address bus

/*
*   The Game Boy has a 16-bit address bus, which is used to address ROM, RAM, and I/O.
*   Byte addressable memory
*   16-bit access bus -> 2^16 = 65536 addresses
*   So 2^16 * 1 Byte = 2^6 KB memory = 64 KB Memory
*/
class Memory {

    size_t romSize;
    std::vector<uint8_t> mem;
    // std::vector<uint8_t> mem = std::vector<uint8_t>(MEMORY_SIZE, 0);

public:


    Memory () {
        // // init all mem locationss to 0
        // for (int i = 0; i < MEMORY_SIZE; i++) {
        //     mem[i] = 0;
        // }
        mem = std::vector<uint8_t>(MEMORY_SIZE, 0);
    }

    int set(int addr, uint8_t val) {
        if (addr < 0 || addr >= MEMORY_SIZE) {
            std::cerr << "Memory access with out of bounds address: " << addr << std::endl;
            return 1;
        }
        mem[addr] = val;
        return 0;
    }

    uint8_t get(int addr) {
        if (addr < 0 || addr >= MEMORY_SIZE) {
            std::cerr << "Memory access with out of bounds address: " << addr << std::endl;
            return -1;
        }

        return mem[addr];
    }

    void load_rom(uint8_t *rom_ptr, size_t file_size) {
        printf ("Loading ROM into Memory Address 0x0000....\n");
        printf ("   file_size: %lu\n", file_size);
        for (size_t i = 0; i < file_size; i++) {
            set(i, rom_ptr[i]); 
        }
        romSize = file_size;
        printf ("Successfully loaded ROM!\n\n");

    }

    // void dump_ROM() {
    //     printf("")
    // }
};