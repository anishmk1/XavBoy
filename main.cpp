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
#include <set>

#include "main.h"
#include "Peripherals.h"
#include "Memory.h"
#include "CPU.h"
#include "Debug.h"

Debug *dbg;
std::ofstream logFile;
bool verbose = false;
bool disable_prints = true;
bool DEBUGGER = false;
bool PRINT_REGS_EN = true;
const bool LOAD_BOOT_ROM = true;    // default: true; ROM includes bytes from addr 0 to 0x100 so Memory will load ROM starting at 0. Most ROMS will have this. Only my own test roms wont. They should be loaded into 0x100 because thats where PC should start from
const bool SKIP_BOOT_ROM = true;    // default: true; Start executing with PC at 0x100. Should mostly be true unless testing actual BOOT ROM execution
const bool GAMEBOY_DOCTOR = true;   // controls when print_regs is run and how it is formatted. Does not affect functionality


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


//------------------------------------------//
//      Emulate the CPU for the Gameboy     //
//------------------------------------------//
int main(int argc, char* argv[]) {
    // char* gb_file = "test-roms/prog_OR.gb";
    if (argc != 1 && argc != 2) {
        std::cerr << "Error opening the file!" << std::endl;
        return 1;
    }
    if (argc == 2) {
        DEBUGGER = (strcmp(argv[1], "--debug") == 0);
        PRINT_REGS_EN = (strcmp(argv[1], "--quiet") != 0);
        // gb_file = argv[1];
    }

    setup_serial_output();

    // FIXME: Confirm that this automatically frees memory when program finishes
    // use valgrind etc
    Memory *mem = new Memory();
    MMIO *mmio  = new MMIO(mem);
    CPU *cpu    = new CPU(mem);
    dbg         = new Debug();

     
    
    //  1 - PASSED
    //  2 - hanging...
    //  3 - PASSED
    //  4 - PASSED
    //  5 - PASSED
    //  6 - PASSED
    //  7 - PASSED -> But log collection ending early: hitting infinite loop (??)
    //  8 - PASSED
    //  9 - PASSED
    // 10 - PASSED
    // 11 - PASSED


    size_t file_size;
    uint8_t *rom_ptr;
    // ------------------------------- BLARGG'S TEST ROMS -------------------------------------------
    // rom_ptr = open_rom("test-roms/gb-test-roms/cpu_instrs/individual/01-special.gb", &file_size);
    rom_ptr = open_rom("test-roms/gb-test-roms/cpu_instrs/individual/02-interrupts.gb", &file_size);
    // rom_ptr = open_rom("test-roms/gb-test-roms/cpu_instrs/individual/03-op sp,hl.gb", &file_size);
    // rom_ptr = open_rom("test-roms/gb-test-roms/cpu_instrs/individual/04-op r,imm.gb", &file_size);
    // rom_ptr = open_rom("test-roms/gb-test-roms/cpu_instrs/individual/05-op rp.gb", &file_size);
    // rom_ptr = open_rom("test-roms/gb-test-roms/cpu_instrs/individual/06-ld r,r.gb", &file_size);
    // rom_ptr = open_rom("test-roms/gb-test-roms/cpu_instrs/individual/07-jr,jp,call,ret,rst.gb", &file_size);
    // rom_ptr = open_rom("test-roms/gb-test-roms/cpu_instrs/individual/08-misc instrs.gb", &file_size);
    // rom_ptr = open_rom("./test-roms/gb-test-roms/cpu_instrs/individual/09-op r,r.gb", &file_size);
    // rom_ptr = open_rom("test-roms/gb-test-roms/cpu_instrs/individual/10-bit ops.gb", &file_size);
    // rom_ptr = open_rom("test-roms/gb-test-roms/cpu_instrs/individual/11-op a,(hl).gb", &file_size);
    // rom_ptr = open_rom("./test-roms/test.gb", &file_size);

    // ----------------------------------- DEBUG ROMS -----------------------------------------------
    // rom_ptr = open_rom("test-roms/blarggs-debug-roms/cpu_instrs_1_debug.gb", &file_size);
    // rom_ptr = open_rom("test-roms/blarggs-debug-roms/cpu_instrs_6_debug.gb", &file_size);
    if (rom_ptr == nullptr) {
        std::cerr << "Error opening the file!" << std::endl;
        return 1;
    }
    mem->load_rom(rom_ptr, file_size);

    if (munmap(rom_ptr, file_size) == -1) {
        perror("Error unmapping file");
    }

    bool halt_cpu = false;      // From CPU when HALT instruction executed
    print ("Starting main loop\n\n");
    
    while (true) {  // main loop
        if (DEBUGGER) dbg->debugger_break(*cpu);

        mmio->incr_timers(++dbg->free_clk);
        if (!halt_cpu) cpu->execute(halt_cpu);

    }
}