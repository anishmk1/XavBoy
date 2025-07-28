// main.h
#ifndef MAIN_H
#define MAIN_H

#include "Debug.h"

#include <chrono>
#include <thread>


extern std::ofstream logFile;
extern std::ofstream debug_file;
extern bool verbose;
extern bool disable_prints;
extern bool DEBUGGER;
extern bool PRINT_REGS_EN;
extern const bool LOAD_BOOT_ROM;    // default: true; ROM includes bytes from addr 0 to 0x100 so Memory will load ROM starting at 0. Most ROMS will have this. Only my own test roms wont. They should be loaded into 0x100 because thats where PC should start from
extern const bool SKIP_BOOT_ROM;    // default: true; Start executing with PC at 0x100. Should mostly be true unless testing actual BOOT ROM execution
extern const bool GAMEBOY_DOCTOR;   // controls when print_regs is run and how it is formatted. Does not affect functionality

extern Debug *dbg;      // Reference to Debug module should be visible from everywhere

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

#endif