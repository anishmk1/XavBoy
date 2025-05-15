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
        int rom_start_addr = (LOAD_BOOT_ROM) ? 0 : 0x100;
        print ("Loading ROM into Memory Address 0x%0x....\n", rom_start_addr);
        print ("   file_size: %lu\n", file_size);
        for (size_t i = 0; i < file_size; i++) {
            set(i + rom_start_addr, rom_ptr[i]); 
        }
        romSize = file_size;
        print ("Successfully loaded ROM!\n\n");

        // dump_mem(300);
    }

    void dump_mem(int around = 25, int range = 50) {
        int start = around - (range / 2);
        int end = around + (range / 2);
        for (int i = start; i < end; i++) {
            if (i % 16 == 0) printx ("\n");
            printx ("mem[0x%0x]=0x%0x;  ", i, mem[i]);
        }
        printx ("\n\n");
    }

    // void dump_ROM() {
    //     printf("")
    // }
};