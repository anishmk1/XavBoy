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

#define printx(...)                            \
    do {                                       \
        printf(__VA_ARGS__);                   \
    } while (0)


#define printv(...)                                 \
    do {                                            \
        if (verbose == 1)    \
            printf(__VA_ARGS__);                    \
    } while (0)

#define print(...)                             \
    do {                                       \
        if (disable_prints == 0)               \
            printf(__VA_ARGS__);               \
    } while (0)


std::ofstream logFile;
bool verbose = true;
bool disable_prints = true;
bool DEBUGGER = false;
bool PRINT_REGS_EN = true;
const bool LOAD_BOOT_ROM = true;    // default: true; ROM includes bytes from addr 0 to 0x100 so Memory will load ROM starting at 0. Most ROMS will have this. Only my own test roms wont. They should be loaded into 0x100 because thats where PC should start from
const bool SKIP_BOOT_ROM = true;    // default: true; Start executing with PC at 0x100. Should mostly be true unless testing actual BOOT ROM execution
const bool GAMEBOY_DOCTOR = true;   // controls when print_regs is run and how it is formatted. Does not affect functionality

#include "Peripherals.cpp"
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

void debugger_break(long &num_steps_left, CPU &cpu) {
    bool exit_debugger = false;
    if (num_steps_left == 0) {
        // Break CPU execution for debugging
        do  {
            PRINT_REGS_EN = true;
            printx (">> ");
            std::string dbg_cmd;
            std::cin >> dbg_cmd;

            if (dbg_cmd[0] == 'h') {    // help
                printx ("Debug commands: 'sx'(step); 'r'(run); 'p'(print regs); 'mxxxx'(Read mem addr)\n");
            } else if (std::tolower(dbg_cmd[0]) == 's') {
                // prints state of regs and PC THAT WERE EXECUTED
                if (std::islower(dbg_cmd[0])) PRINT_REGS_EN = false; // Step quietly (dont print regs with each step)
                
                if (dbg_cmd.size() > 1) {
                    int step_input = std::stoi(dbg_cmd.substr(1));
                    num_steps_left = (step_input == 0) ? 0 : (step_input - 1);
                }
                exit_debugger = true;
                // PRINT_REGS_EN
            
            } else if (dbg_cmd[0] == 'e') {
                disable_prints = !disable_prints; // enable prints
                printx ("disable_prints <= %0d\n", disable_prints);
            } else if (dbg_cmd[0] == 'p') {
                // prints state of regs and pc ABOUT TO BE EXECUTED
                cpu.rf.print_regs();
            } else if (dbg_cmd[0] == 'm') {
                if (dbg_cmd.size() == 5) {
                    int addr = std::stoi(dbg_cmd.substr(1), nullptr, 16);
                    uint16_t val = cpu.mem->get(addr);
                    printx ("mem[0x%0x] = 0x%0x\n", addr, val);
                } else {
                    printx ("Please provide a 4 digit hex address with m: mxxxx\n");
                }
            } else if (dbg_cmd[0] == 'd') {
                if (dbg_cmd[1] == '0') printx ("debug0 = 0x%0x\n", cpu.rf.debug0);
                else if (dbg_cmd[1] == '1') printx ("debug1 = 0x%0x\n", cpu.rf.debug1);
                else if (dbg_cmd[1] == '2') printx ("debug2 = 0x%0x\n", cpu.rf.debug2);
            } else if (dbg_cmd[0] == 'r') {
                DEBUGGER = false;   // disable debugger for rest of the run
                exit_debugger = true;
            }
            
        } while (exit_debugger == false);

    } else {
        num_steps_left--;
    }
}

// emulate the cpu for the gameboy
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
    MMIO *mmio = new MMIO();
    Memory *mem = new Memory(mmio);
    CPU *cpu = new CPU(mem);


    /**
     * INC H                        // 0x24
     * LD mem[0xff00 + 0x80], A=88  // 0xE0/0x80
     * LD A, mem[0xff00 + 0x82]     // 0xF0/0x82
     * XOR A, A, (HL)               // 0xAE
     * INC H                        // 0x24
     * 
     * 
     *  $DEF8 is the base address for all instrs being tested 
     * 
     */

     
    
    //  1 - hanging...
    //  2 - hanging...
    //  3 - hanging...
    //  4 - PASSED
    //  5 - PASSED
    //  6 - PASSED
    //  7 - detected HALT
    //  8 - hanging...
    //  9 - PASSED
    // 10 - PASSED
    // 11 - PASSED


    size_t file_size;
    uint8_t *rom_ptr;
    // ------------------------------- BLARGG'S TEST ROMS -------------------------------------------
    rom_ptr = open_rom("test-roms/gb-test-roms/cpu_instrs/individual/01-special.gb", &file_size);
    // rom_ptr = open_rom("test-roms/gb-test-roms/cpu_instrs/individual/02-interrupts.gb", &file_size);
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

    // main loop
    int free_clk = 0;
    // size_t num_bytes = 0;
    // while (num_bytes++ < file_size) {
    long num_steps_left = 0;
    while (true) {
    // while (free_clk < 2335) {
        if (DEBUGGER) debugger_break(num_steps_left, *cpu);

        mmio->incr_timers(++free_clk);
        cpu->execute();

    }
}