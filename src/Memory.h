// Memory.h
#ifndef MEMORY_H
#define MEMORY_H

#include <vector>

class MMIO;

class Memory {
    size_t romSize;
    std::vector<uint8_t> boot_rom;
    bool boot_rom_enabled;

public:
    std::vector<uint8_t> memory;
    // MMIO *mmio;

    Memory();
    uint8_t access_memory_map(int addr, uint8_t val, bool read_nwr, bool backdoor=0);
    void load_boot_rom();
    void load_rom(uint8_t *rom_ptr, size_t file_size);
    uint8_t get(int addr);
    int set(int addr, uint8_t val, bool backdoor=0);
    void dump_mem(int around, int range);
};

#endif // MEMORY_H