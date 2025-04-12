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

std::ofstream logFile;
bool verbose = false;
bool disable_prints = false;
const bool LOAD_BOOT_ROM = false;

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

    CPU *cpu = new CPU();
    Reg (&regs)[NUM_REGS] = cpu->rf.regs;

    // FIXME: Confirm that this automatically frees memory when program finishes
    // use valgrind etc
    Memory *mem = new Memory();

    size_t file_size;
    uint8_t *rom_ptr;
    rom_ptr = open_rom("./test-roms/test.gb", &file_size);
    if (rom_ptr == nullptr) {
        std::cerr << "Error opening the file!" << std::endl;
        return 1;
    }
    mem->load_rom(rom_ptr, file_size, LOAD_BOOT_ROM);

    if (munmap(rom_ptr, file_size) == -1) {
        perror("Error unmapping file");
    }

    // main loop
    size_t num_bytes = 0;
    while (num_bytes++ < file_size) {
        // printf ("byte[%d]: 0x%0x\n", i, rom_ptr[i]);
        printf ("PC=0x%0x: cmd=0x%0x\n", regs[PC].val, static_cast<int>(mem->get(regs[PC].val)));

        cpu->execute(mem);
        // increment PC
        regs[PC].val++;
    }
}