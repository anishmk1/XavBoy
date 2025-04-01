#include <iostream>
#include <fstream>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <cstring> // For strcmp
#include <cassert>

bool verbose = false;

#include "Memory.cpp"
#include "CPU.cpp"

uint8_t *open_rom(const char* file_path, size_t *file_size) {
    int fd = open(file_path, O_RDONLY);
    if (fd == -1) {
        perror("Error opening file");
        return nullptr;
    }

    // Get the size of the file
    struct stat file_stat;
    if (fstat(fd, &file_stat) == -1) {
        perror("Error getting file size");
        close(fd);
        return nullptr;
    }
    *file_size = file_stat.st_size;

    // Memory-map the file into memory
    void* mapped_file = mmap(NULL, *file_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (mapped_file == MAP_FAILED) {
        perror("Error memory-mapping file");
        close(fd);
        return nullptr;
    }

    // Use the memory-mapped file (it’s treated like a regular pointer to a byte array)
    uint8_t* file_data = static_cast<uint8_t*>(mapped_file);

    return file_data;
}

// emulate the cpu for the gameboy
int main(int argc, char* argv[]) {
    if (argc != 1 && argc != 2) {
        std::cerr << "Error opening the file!" << std::endl;
        return 1;
    }
    if (argc == 2) {
        verbose = (strcmp(argv[1], "--debug") == 0);
    }

    CPU *cpu = new CPU();

    // FIXME: Confirm that this automatically frees memory when program finishes
    // use valgrind etc
    Memory *mem = new Memory();

    size_t file_size;
    uint8_t *rom_ptr;
    rom_ptr = open_rom("test-roms/prog_OR.gb", &file_size);
    if (rom_ptr == nullptr) {
        std::cerr << "Error opening the file!" << std::endl;
        return 1;
    }

    printf ("file_size: %lu\n", file_size);
    // main loop
    // for (int i = 0; i < file_size; i++) {
    while (cpu->PC < file_size) {
        // printf ("byte[%d]: 0x%0x\n", i, rom_ptr[i]);
        uint8_t cmd = rom_ptr[cpu->PC];
        printf ("PC=%d: cmd=0x%0x\n", cpu->PC, static_cast<int>(cmd));

        cpu->execute(rom_ptr, mem);
        // increment PC
        cpu->PC++;
    }

    if (munmap(rom_ptr, file_size) == -1) {
        perror("Error unmapping file");
    }
}