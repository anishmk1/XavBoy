#include <iostream>
#include <fstream>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <array>
#include <string>

#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <cstring> // For strcmp
#include <cassert>

#define printx(...)                            \
    do {                                       \
        printf(__VA_ARGS__);                   \
    } while (0)


#define print(...)                             \
    do {                                       \
        if (disable_prints == 0)               \
            printf(__VA_ARGS__);               \
    } while (0)


#define printv(...)                                 \
    do {                                            \
        if (disable_prints == 0 && verbose == 1)    \
            printf(__VA_ARGS__);                    \
    } while (0)


std::ofstream logFile;
bool verbose = false;
bool disable_prints = true;
const bool LOAD_BOOT_ROM = true;    // default: true; ROM includes bytes from addr 0 to 0x100 so Memory will load ROM starting at 0. Most ROMS will have this. Only my own test roms wont. They should be loaded into 0x100 because thats where PC should start from
const bool SKIP_BOOT_ROM = true;    // default: true; Start executing with PC at 0x100. Should mostly be true unless testing actual BOOT ROM execution
const bool GAMEBOY_DOCTOR = true;   // controls when print_regs is run and how it is formatted. Does not affect functionality

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

void setup_serial_output() {
    logFile.open("log.txt");
    std::clog.rdbuf(logFile.rdbuf());  // Redirect clog to file

    std::clog << "Serial Output Window....\n\n";
}

// emulate the cpu for the gameboy
int main(int argc, char* argv[]) {
    // char* gb_file = "test-roms/prog_OR.gb";
    if (argc != 1 && argc != 2) {
        std::cerr << "Error opening the file!" << std::endl;
        return 1;
    }
    if (argc == 2) {
        verbose = (strcmp(argv[1], "--debug") == 0);
        // gb_file = argv[1];
    }

    setup_serial_output();

    // FIXME: Confirm that this automatically frees memory when program finishes
    // use valgrind etc
    Memory *mem = new Memory();
    CPU *cpu = new CPU(mem);
    Reg (&regs)[NUM_REGS] = cpu->rf.regs;


    size_t file_size;
    uint8_t *rom_ptr;
    rom_ptr = open_rom("./test-roms/gb-test-roms/cpu_instrs/individual/04-op r,imm.gb", &file_size);
    // rom_ptr = open_rom("./test-roms/test.gb", &file_size);
    if (rom_ptr == nullptr) {
        std::cerr << "Error opening the file!" << std::endl;
        return 1;
    }
    mem->load_rom(rom_ptr, file_size);

    if (munmap(rom_ptr, file_size) == -1) {
        perror("Error unmapping file");
    }

    // main loop
    size_t num_bytes = 0;
    while (num_bytes++ < file_size) {
        // printf ("byte[%d]: 0x%0x\n", i, rom_ptr[i]);
        print ("PC=0x%0x: cmd=0x%0x\n", regs[PC].val, static_cast<int>(mem->get(regs[PC].val)));

        cpu->execute();
    }
}