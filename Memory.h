// Memory.h
#ifndef MEMORY_H
#define MEMORY_H

class MMIO;

class Memory {
    size_t romSize;

public:
    std::vector<uint8_t> mem;
    MMIO *mmio;

    Memory();
    uint8_t access_memory_map(int addr, uint8_t val, bool read_nwr);
    void load_rom(uint8_t *rom_ptr, size_t file_size);
    uint8_t get(int addr);
    int set(int addr, uint8_t val);
    void dump_mem(int around, int range);
};

#endif // MEMORY_H