// main.h
#ifndef MAIN_H
#define MAIN_H

#include "Debug.h"
#include "PPU.h"
#include "Memory.h"
#include "Peripherals.h"
#include "LCD.h"

#include <chrono>
#include <thread>


// LCD Register addresses
const uint16_t REG_LCDC = 0xff40;   // LCD control
const uint16_t REG_STAT = 0xff41;   // LCD status
const uint16_t REG_LY   = 0xff44;   // LCD Y co-ordinate [read-only]
const uint16_t REG_LYC  = 0xff45;   // LY compare
const uint16_t REG_BGP  = 0xff47;   // BG Palette Data
const uint16_t REG_SCY  = 0xff42;   // Background viewport Y position
const uint16_t REG_SCX  = 0xff43;   // Background viewport X position
const uint16_t REG_WY   = 0xff4a;   // Window Y position
const uint16_t REG_WX   = 0xff4b;   // Window X position + 7

const uint16_t REG_IF   = 0xff0f;   // Interrupt Flag
const uint16_t REG_IE   = 0xffff;   // Interrupt Enable

// LCDC Register fields
constexpr uint8_t LCDC_ENABLE_BIT            = 1 << 7;   // Bit 7
constexpr uint8_t LCDC_WINDOW_TILEMAP_BIT    = 6;   // Bit 6
constexpr uint8_t LCDC_WINDOW_ENABLE_BIT     = 5;   // Bit 5
constexpr uint8_t LCDC_BG_WINDOW_TILES_BIT   = 4;   // Bit 4
// constexpr uint8_t LCDC_BG_TILEMAP_BIT        = 1 << 3;   // Bit 3
constexpr uint8_t LCDC_OBJ_SIZE_BIT          = 2;   // Bit 2
constexpr uint8_t LCDC_OBJ_ENABLE_BIT        = 1;   // Bit 1
constexpr uint8_t LCDC_BG_WINDOW_ENABLE_BIT  = 0;   // Bit 0   BG & Window enable / priority


enum ObjDataFormat : uint8_t {
    OBJ_Y_POS       = 0,
    OBJ_X_POS       = 1,
    OBJ_TILE_IDX    = 2,
    OBJ_ATTR_FLAGS  = 3
};


extern std::string rom_path;
extern std::ofstream logFile;
extern std::ofstream debug_file;
extern std::ofstream pixel_map;
extern bool verbose;
extern bool disable_prints;
extern bool DEBUGGER;
extern bool PRINT_REGS_EN;
extern bool CPU_ONLY;
extern bool SKIP_BOOT_ROM;          // default: true; Start executing with PC at 0x100. Should mostly be true unless testing actual BOOT ROM execution
extern const bool LOAD_BOOT_ROM;    // default: true; ROM includes bytes from addr 0 to 0x100 so Memory will load ROM starting at 0. Most ROMS will have this. Only my own test roms wont. They should be loaded into 0x100 because thats where PC should start from
extern const bool GAMEBOY_DOCTOR;   // controls when print_regs is run and how it is formatted. Does not affect functionality

extern PPU *ppu;        // Globally referencable PPU module
extern Memory *mem;      // Globally referencable Memory 
extern MMIO *mmio;
extern Debug *dbg;      // Reference to Debug module should be visible from everywhere
extern LCD *lcd;
extern CPU *cpu;

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


#ifdef DEBUG_MODE
    #define DBG(x) do { debug_file << x; } while(0)
#else
    #define DBG(x) do {} while(0)
#endif

// #define PXL(x) do { pixel_map << x; } while (0)

#endif