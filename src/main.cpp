#include <iostream>
#include <fstream>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <array>
#include <string>
#include <iomanip>
#include <csignal>

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
CPU *cpu;
std::ofstream logFile;
std::ofstream debug_file;
std::ofstream pixel_map;
std::string rom_path;
bool verbose = false;
bool disable_prints = true;
bool DEBUGGER = false;
bool PRINT_REGS_EN = true;
bool CPU_ONLY = false;
bool SKIP_BOOT_ROM = false;         // default: false; Ninentdo logo boot rom runs by default on startup
const bool LOAD_BOOT_ROM = true;    // default: true; ROM includes bytes from addr 0 to 0x100 so Memory will load ROM starting at 0. Most ROMS will have this. Only my own test roms wont. They should be loaded into 0x100 because thats where PC should start from
const bool GAMEBOY_DOCTOR = true;   // controls when print_regs is run and how it is formatted. Does not affect functionality

// const double GAMEBOY_CPU_FREQ_HZ = 4194304.0; // 4.194304 MHz


// FIXME:: Just use a simple ifstream/fread() into a. I decided on mmap before realizing I needed to load the
// entire rom into memory anyway.. Maybe implementing MBCs will change this at some point.. But for now keep this simple
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

// Poll for events
// SDL_PollEvent checks the queue of events. Since multiple events can be queued up per frame
// Loop exits once all events are popped off the q
bool sdl_event_loop(bool& main_loop_running) {
    if (CPU_ONLY) return false;

    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            main_loop_running = false; // Window closed

        }
    }

    if (main_loop_running == false) {
        DBG("CLOSING SDL WINDOW" << std::endl);
        lcd->close_window();
        // goto exit_main;
        std::exit(EXIT_SUCCESS);
        // return true;
    }
    return false;
}

//------------------------------------------//
//            Emulate the Gameboy           //
//------------------------------------------//
int emulate(int argc, char* argv[]) {
    // setup_serial_output();

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
    // rom_path = "test-roms/graphics-test-roms/color_bands_lcdc4_0.gb";
    // rom_path = "test-roms/graphics-test-roms/color_bands_scroll.gb";
    // rom_path = "test-roms/graphics-test-roms/color_columns_scroll.gb";
    // rom_path = "test-roms/graphics-test-roms/color_bands_with_window.gb";
    // rom_path = "test-roms/graphics-test-roms/color_bands_with_moving_window.gb";
    rom_path = "test-roms/graphics-test-roms/simple_objects.gb";
    // rom_path = "test-roms/graphics-test-roms/simple_infinite_loop.gb";

    // ---------------------------------------- GAMES ------------------------------------------------
    // rom_path = "../GameBoy_ROMS/Tetris (World) (Rev 1).gb";


    // Note: To produce Debug roms (With .sym dbeugger symbols)
    //      cd XavBoy/test-roms/gb-test-roms/cpu_instrs/source
    //      wla-gb -D DEBUG -o ../../../blarggs-debug-roms/test.o 02-interrupts.s
    //      cd ../../../blarggs-debug-roms
    //      wlalink -S -v ../gb-test-roms/cpu_instrs/source/linkfile cpu_instrs_2_debug.gb
    
    for (int i = 1; i < argc; ++i) {
        if (argv[i][0] != '-') {
            rom_path = std::string(argv[i]);
        }
    }
    rom_ptr = open_rom(rom_path.c_str(), &file_size);

    if (rom_ptr == nullptr) {
        std::cerr << "Error opening the file!" << rom_path << std::endl;
        return 1;
    }

    // Load boot ROM first
    mem->load_boot_rom();

    // Then load game ROM
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

    // Initialize CSV logging
    dbg->init_csv_logging();

    bool main_loop_running = true;
    // bool frame_timing_started = false;

    while (main_loop_running) {  // main loop

        // Start frame timing if this is the beginning of a new frame
        // if (!frame_timing_started) {
        //     dbg->start_frame_timing();
        //     frame_timing_started = true;
        // }

        // SDL events are now polled once per frame (when lcd->frame_ready is true)
        // instead of every main loop iteration to improve WSL2 performance

        dbg->perf.num_main_loops++;

        lcd->lcd_status_update();
        if (lcd->frame_ready) {
            // Poll SDL events once per frame for better WSL2 performance
            // dbg->start_section_timing();
            if (sdl_event_loop(main_loop_running) == true) {
                return 0;
            }
            // dbg->end_section_timing("sdl_events");

            // dbg->start_section_timing();
            lcd->draw_frame();
            dbg->frame_cnt++;
            dbg->perf.num_main_loops = 0;
            // dbg->end_section_timing("lcd");

            // Finalize frame timing and write to CSV
            // dbg->finalize_frame_timing();
            // frame_timing_started = false;  // Reset for next frame
        }

        // BREAKPOINT SETTING
        if (mem->get(0xfffe) == 0xac) {
            // dbg->bp_info.breakpoint = true;
            // dbg->bp_info.msg = "BPT: In .updateWX after storing a in RAM";
            dbg->set_breakpoint("BPT 2: In .updateWX after storing a in RAM");
        } else if (mem->get(0xfffe) == 0xab) {
            dbg->set_breakpoint("BPT 1: In VBlankHandler");
        } else if (mem->get(0xfffe) == 0xaa) {
            dbg->set_breakpoint("BPT 0: Before VBlankHandler Label");
        }

        // dbg->start_section_timing();
        dbg->debugger_break();
        // dbg->end_section_timing("debugger");


        int mcycles = 1;
        if (!cpu->halt_mode) {
            // dbg->start_section_timing();
            mcycles = cpu->execute();
            // dbg->end_section_timing("cpu");
        }

        // dbg->start_section_timing();
        ppu->ppu_tick(mcycles);
        // dbg->end_section_timing("ppu");

        // dbg->start_section_timing();
        mmio->incr_timers(mcycles);
        // dbg->end_section_timing("mmio");

        // IE && IF != 0 wakes up CPU from halt mode
        // dbg->start_section_timing();
        if (cpu->halt_mode) {

            if (mmio->exit_halt_mode()) {
                cpu->halt_mode = false;
            }

            // More Interrupt handling - this time when handling halted Cpu
            if (mmio->check_interrupts(cpu->intrpt_info.handler_addr, 1)) {
                cpu->intrpt_info.interrupt_valid = true;
                cpu->intrpt_info.wait_cycles = 1;
            }
        }
        // dbg->end_section_timing("interrupt");

        // Track any remaining unaccounted time in this main loop iteration
        // dbg->start_section_timing();
        // dbg->end_section_timing("other");
    }

    return 0;
}


 void handle_sigterm(int) {
    std::cerr << "Program timed out. Dumping stats...\n";

    printx("Total Frame Count = %0ld\n", dbg->frame_cnt);
    // Print profiling info, flush buffers, clean up...
    std::exit(124); // convention: 124 = timeout exit code
}



int main(int argc, char* argv[]) {
    // Register handler for timeout signal
    std::signal(SIGTERM, handle_sigterm);
    // also catch Ctrl+C for consistency
    std::signal(SIGINT, handle_sigterm);

    // setup_serial_output();
    // Separate debug log file
#ifdef DEBUG_MODE
    debug_file.open("logs/debug.log");
    pixel_map.open("logs/pixel_map.log");
#endif

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--step") == 0) {
            DEBUGGER = true;
        } else if (strcmp(argv[i], "--quiet") == 0) {
            PRINT_REGS_EN = false;
        } else if (strcmp(argv[i], "--cpu_only") == 0) {
            CPU_ONLY = true;
        } else if (strcmp(argv[i], "--skip-boot-rom") == 0) {
            SKIP_BOOT_ROM = true;
        }
    }

    mem         = new Memory();     // FIXME: Change "mem" reference to "mmu". Then memory field can be renamed back to "mem"
    mmio        = new MMIO();
    cpu         = new CPU();
    ppu         = new PPU();
    dbg         = new Debug();
    lcd         = new LCD();

    lcd->init_screen();
     
    emulate(argc, argv);

    lcd->close_window();
    delete lcd;
    delete dbg;
    delete ppu;
    delete cpu;
    delete mmio;
    delete mem;
    pthread_exit(NULL);
}