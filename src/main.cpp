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

#include <SDL2/SDL.h>

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
LCD *lcd;
std::ofstream logFile;
std::ofstream debug_file;
std::ofstream pixel_map;
bool verbose = false;
bool disable_prints = true;
bool DEBUGGER = false;
bool PRINT_REGS_EN = true;
bool CPU_ONLY = false;
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

// Poll for events
// SDL_PollEvent checks the queue of events. Since multiple events can be queued up per frame
// Loop exits once all events are popped off the q
void sdl_event_loop(bool& main_loop_running) {
    if (CPU_ONLY) return;

    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            main_loop_running = false; // Window closed

            DBG("CLOSING SDL WINDOW" << std::endl);
            lcd->close_window();
        }
    }
}

//------------------------------------------//
//            Emulate the Gameboy           //
//------------------------------------------//
int emulate(int argc, char* argv[]) {
    setup_serial_output();

    CPU *cpu    = new CPU(mem);

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
    std::string rom_path;
    // Use ROM path and flags from command line if provided, otherwise use default
    // ------------------------------- BLARGG'S TEST ROMS -------------------------------------------
    // rom_path = "test-roms/gb-test-roms/cpu_instrs/cpu_instrs.gb";
    // rom_path = "test-roms/gb-test-roms/cpu_instrs/individual/01-special.gb";
    // rom_path = "test-roms/gb-test-roms/cpu_instrs/individual/02-interrupts.gb";
    // rom_path = "test-roms/gb-test-roms/cpu_instrs/individual/03-op sp,hl.gb";
    // rom_path = "test-roms/gb-test-roms/cpu_instrs/individual/04-op r,imm.gb";
    // rom_path = "test-roms/gb-test-roms/cpu_instrs/individual/05-op rp.gb";
    // rom_path = "test-roms/gb-test-roms/cpu_instrs/individual/06-ld r,r.gb";
    // rom_path = "test-roms/gb-test-roms/cpu_instrs/individual/07-jr,jp,call,ret,rst.gb";
    // rom_path = "test-roms/gb-test-roms/cpu_instrs/individual/08-misc instrs.gb";
    // rom_path = "test-roms/gb-test-roms/cpu_instrs/individual/09-op r,r.gb";
    // rom_path = "test-roms/gb-test-roms/cpu_instrs/individual/10-bit ops.gb";
    // rom_path = "test-roms/gb-test-roms/cpu_instrs/individual/11-op a,(hl).gb";
    // rom_path = "test-roms/test.gb";

    // ----------------------------------- DEBUG ROMS -----------------------------------------------
    // rom_path = "test-roms/blarggs-debug-roms/cpu_instrs_1_debug.gb";
    // rom_path = "test-roms/blarggs-debug-roms/cpu_instrs_2_debug.gb";
    // rom_path = "test-roms/blarggs-debug-roms/cpu_instrs_6_debug.gb";

    // ------------------------------- GRAPHICS TEST ROMS -------------------------------------------
    // rom_path = "test-roms/graphics-test-roms/blank_screen.gb";
    // rom_path = "test-roms/graphics-test-roms/color_bands.gb";
    rom_path = "test-roms/graphics-test-roms/color_bands_scroll.gb";
    // rom_path = "test-roms/graphics-test-roms/simple_infinite_loop.gb";

    // Note: To produce Debug roms (With .sym dbeugger symbols)
    //      cd XavBoy/test-roms/gb-test-roms/cpu_instrs/source
    //      wla-gb -D DEBUG -o ../../../blarggs-debug-roms/test.o 02-interrupts.s
    //      cd ../../../blarggs-debug-roms
    //      wlalink -S -v ../gb-test-roms/cpu_instrs/source/linkfile cpu_instrs_2_debug.gb
    
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--debug") == 0) {
            DEBUGGER = true;
        } else if (strcmp(argv[i], "--quiet") == 0) {
            PRINT_REGS_EN = false;
        } else if (strcmp(argv[i], "--cpu_only") == 0) {
            CPU_ONLY = true;
        } else if (argv[i][0] != '-') {
            rom_path = std::string(argv[i]);
        }
    }
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
    #ifdef DEBUG_MODE
        std::cout << "Debug messages enabled." << std::endl;
        DBG("Starting main loop" << std::endl);
    #else
        DBG("Debug messages disabled." << std::endl);
    #endif
    
    bool main_loop_running = true;
    while (main_loop_running) {  // main loop

        // SDL Main Loop - Polls and services SDL Events like interacting with App window
        // In CPU Only mode, this function exits immediately and main_loop_running will never dsiable
        // So program will run indefinitely (or until forever loop detected??)
        sdl_event_loop(main_loop_running);

        if (lcd->frame_ready) {
            lcd->draw_frame();
        }

        // if ((mem->get(REG_LCDC) & LCDC_ENABLE_BIT) != 0) {
        //     dbg->bp_info.breakpoint = true;
        //     dbg->bp_info.msg = "LCD Turned on";
        // }
        // if ((mem->get(0xfffe) == 0xab) && (hit_bp_1 == false)) {
        //     hit_bp_1 = true;
        //     dbg->bp_info.breakpoint = true;
        //     dbg->bp_info.msg = "mem[FFFE] <= 0xAB";
        // }
        if (mem->get(0xfffe) == 0xac) {
            dbg->bp_info.breakpoint = true;
            dbg->bp_info.msg = "BPT 2: In ExitLoop after enabling LCD";
        }
        // if (mem->get(REG_BGP) == 0xe4) {
        //     dbg->bp_info.breakpoint = true;
        //     dbg->bp_info.msg = "Palette reg set";
        // }
        // if (dbg->mcycle_cnt == 500) {
        //     dbg->bp_info.breakpoint = true;
        //     dbg->bp_info.msg = "Hit cycle 500";
        // }


        cpu->rf.debug0 = mmio->IME;
        cpu->rf.debug1 = mmio->IME_ff[0];
        cpu->rf.debug2 = mmio->IME_ff[1];
        dbg->debugger_break(*cpu);


        int mcycles = 1;
        if (!cpu->halt_mode) {
            mcycles = cpu->execute();
            assert(GAMEBOY_CPU_FREQ_HZ);
            // wait_cycles(mcycles, GAMEBOY_CPU_FREQ_HZ);
        }
        ppu->ppu_tick(mcycles);
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

    return 0;
}

int main(int argc, char* argv[]) {

    // setup_serial_output();
    // Separate debug log file
#ifdef DEBUG_MODE
    debug_file.open("logs/debug.log");
    pixel_map.open("logs/pixel_map.log");
#endif

    // FIXME: Confirm that this automatically frees memory when program finishes
    // use valgrind etc
    mem         = new Memory();     // FIXME: Change "mem" reference to "mmu". Then memory field can be renamed back to "mem"
    mmio        = new MMIO();
    // CPU *cpu    = new CPU(mem);
    ppu         = new PPU();
    dbg         = new Debug();
    lcd         = new LCD();

    lcd->init_screen();
     
    emulate(argc, argv);
}