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
#include "PPU.h"
#include "Debug.h"

Debug *dbg;
Memory *mem;
PPU *ppu;
MMIO *mmio;
std::ofstream logFile;
std::ofstream debug_file;
bool verbose = false;
bool disable_prints = true;
bool DEBUGGER = false;
bool PRINT_REGS_EN = true;
const bool LOAD_BOOT_ROM = true;    // default: true; ROM includes bytes from addr 0 to 0x100 so Memory will load ROM starting at 0. Most ROMS will have this. Only my own test roms wont. They should be loaded into 0x100 because thats where PC should start from
const bool SKIP_BOOT_ROM = true;    // default: true; Start executing with PC at 0x100. Should mostly be true unless testing actual BOOT ROM execution
const bool GAMEBOY_DOCTOR = true;   // controls when print_regs is run and how it is formatted. Does not affect functionality

const double GAMEBOY_CPU_FREQ_HZ = 4194304.0; // 4.194304 MHz


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
    logFile.open("./logs/log.txt");
    std::clog.rdbuf(logFile.rdbuf());  // Redirect clog to file

    std::clog << "Serial Output Window....\n\n";
}


// Wait for some real world duration equivalent to the specified number 
// of machine cycles at the given clock frequency (Hz)
void wait_cycles(int mcycles, double clock_frequency_hz) {
    // Each machine cycle = 4 clock cycles
    const int clocks_per_mcycle = 4;

    // Total clock cycles
    double total_clocks = mcycles * clocks_per_mcycle;

    // Duration of one clock cycle (in seconds)
    double seconds_per_cycle = 1.0 / clock_frequency_hz;

    // Total duration to wait (in seconds)
    double wait_time_seconds = total_clocks * seconds_per_cycle;

    // Convert to chrono duration
    auto wait_duration = std::chrono::duration<double>(wait_time_seconds);

    std::this_thread::sleep_for(wait_duration);
}


//------------------------------------------//
//      Emulate the CPU for the Gameboy     //
//------------------------------------------//
int main(int argc, char* argv[]) {
    // char* gb_file = "test-roms/prog_OR.gb";
    // if (argc != 1 && argc != 2) {
    //     std::cerr << "Error opening the file!" << std::endl;
    //     return 1;
    // }
    // if (argc == 2) {
    //     DEBUGGER = (strcmp(argv[1], "--debug") == 0);
    //     PRINT_REGS_EN = (strcmp(argv[1], "--quiet") != 0);
    //     // gb_file = argv[1];
    // }

    setup_serial_output();
    // Separate debug log file
    debug_file.open("logs/debug.log");

    // FIXME: Confirm that this automatically frees memory when program finishes
    // use valgrind etc
    // Memory *mem = new Memory();
    // MMIO *mmio  = new MMIO(mem);
    mem = new Memory();
    mmio  = new MMIO();
    CPU *cpu    = new CPU(mem);
    ppu = new PPU();
    dbg = new Debug();

     
    
    //  1 - PASSED
    //  2 - PASSED
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
    // rom_ptr = open_rom("test-roms/gb-test-roms/cpu_instrs/cpu_instrs.gb", &file_size);
    // rom_ptr = open_rom("test-roms/gb-test-roms/cpu_instrs/individual/01-special.gb", &file_size);
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
    // rom_ptr = open_rom("test-roms/blarggs-debug-roms/cpu_instrs_2_debug.gb", &file_size);
    // rom_ptr = open_rom("test-roms/blarggs-debug-roms/cpu_instrs_6_debug.gb", &file_size);

    // ------------------------------- GRAPHICS TEST ROMS -------------------------------------------
    // rom_ptr = open_rom("test-roms/graphics-test-roms/simple_infinite_loop.gb", &file_size);

    // Note: To produce Debug roms (With .sym dbeugger symbols)
    //      cd XavBoy/test-roms/gb-test-roms/cpu_instrs/source
    //      wla-gb -D DEBUG -o ../../../blarggs-debug-roms/test.o 02-interrupts.s
    //      cd ../../../blarggs-debug-roms
    //      wlalink -S -v ../gb-test-roms/cpu_instrs/source/linkfile cpu_instrs_2_debug.gb

    // Use ROM path and flags from command line if provided, otherwise use default
    std::string rom_path = "test-roms/gb-test-roms/cpu_instrs/individual/02-interrupts.gb";
    
    // std::string rom_path = "test-roms/gb-test-roms/cpu_instrs/individual/02-interrupts.gb";
    // std::cerr << "argc = " << argc << std::endl;
    for (int i = 1; i < argc; ++i) {
        // printx ("From main.cpp: Currently looking at argv[%0d] = %s\n", i, argv[i]);
        // std::cerr << "From main.cpp: Currently looking at argv[" << i << "] = " << argv[i] << std::endl;
        if (strcmp(argv[i], "--debug") == 0) {
            DEBUGGER = true;
        } else if (strcmp(argv[i], "--quiet") == 0) {
            PRINT_REGS_EN = false;
        } else if (argv[i][0] != '-') {
            rom_path = std::string(argv[i]);
            // printx ("From main.cpp: rom_path = %s\n", rom_path.c_str());
            // std::cerr << "From main.cpp: rom_path = " << rom_path << std::endl;
        }
    }
    // std::cerr << "Trying to open ROM: " << rom_path << std::endl;
    rom_ptr = open_rom(rom_path.c_str(), &file_size);

    if (rom_ptr == nullptr) {
        std::cerr << "Error opening the file!" << std::endl;
        return 1;
    }
    mem->load_rom(rom_ptr, file_size);

    if (munmap(rom_ptr, file_size) == -1) {
        perror("Error unmapping file");
    }

    // bool halt_cpu = false;      // From CPU when HALT instruction executed
    print ("Starting main loop\n\n");
    debug_file << "Starting main loop" << std::endl;
    
    while (true) {  // main loop
        cpu->rf.debug0 = mmio->IME;
        cpu->rf.debug1 = mmio->IME_ff[0];
        cpu->rf.debug2 = mmio->IME_ff[1];
        dbg->debugger_break(*cpu);


        // Jul 25th Debug Notes
        // Line 151347 ---> Some general TIMER interrupt testing. Passed this initially but code re-org brought it back
        // Line 152036 ---> HALT instr happens around here. Need to confirm cpu is able to wake back up
        // 
        // make is not terminating - most probs because TIMA is not incrementing
        // TAC is never enabled. SO thtas why
        // According to source file, TAC should be enabled for interrupt_addr@set_test 4
        // But on enabling compile with symbols, the resultant PC addr for this doesnt actually have the instructions
        // that are supposed to be there at interrupt_addr@set_test 4
        // Maybe need some sanity testing on the symbols...




        int mcycles = 1;
        if (!cpu->halt_mode) {
            mcycles = cpu->execute();
            assert(GAMEBOY_CPU_FREQ_HZ);
            // wait_cycles(mcycles, GAMEBOY_CPU_FREQ_HZ);
        }
        mmio->incr_timers(mcycles);

        // IE && IF != 0 wakes up CPU from halt mode
        if (cpu->halt_mode) {

            if (mmio->exit_halt_mode()) {
                cpu->halt_mode = false;
            }

            // More Interrupt handling - this time when handling halted Cpu
            cpu->rf.debug0 = mmio->IME;
            if (mmio->check_interrupts(cpu->intrpt_info.handler_addr, 1)) {
                cpu->intrpt_info.interrupt_valid = true;
                cpu->intrpt_info.wait_cycles = 1;
            }
        }
        
        // if (halt_cpu && mmio->exit_halt_mode(cpu->intrpt_info.handler_addr)) {
        //     halt_cpu = false;
        //     cpu->intrpt_info.interrupt_valid = true;
        //     cpu->intrpt_info.wait_cycles = 0;
        // }

    }
}