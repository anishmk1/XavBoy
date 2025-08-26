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
#include "PPU.h"

//
const uint16_t REG_LCDC = 0xff40;   // LCD control
const uint16_t REG_LY   = 0xff44;   // LCD Y co-ordinate [read-only]

// LCDC Register fields
constexpr uint8_t LCDC_ENABLE_BIT            = 1 << 7;   // Bit 7
constexpr uint8_t LCDC_WINDOW_TILEMAP_BIT    = 1 << 6;   // Bit 6
constexpr uint8_t LCDC_WINDOW_ENABLE_BIT     = 1 << 5;   // Bit 5
constexpr uint8_t LCDC_BG_WINDOW_TILES_BIT   = 1 << 4;   // Bit 4
constexpr uint8_t LCDC_BG_TILEMAP_BIT        = 1 << 3;   // Bit 3
constexpr uint8_t LCDC_OBJ_SIZE_BIT          = 1 << 2;   // Bit 2
constexpr uint8_t LCDC_OBJ_ENABLE_BIT        = 1 << 1;   // Bit 1
constexpr uint8_t LCDC_BG_WINDOW_ENABLE_BIT  = 1 << 0;   // Bit 0   BG & Window enable / priority



void FIFO::push(Pixel num) {

}

Pixel FIFO::pop() {

}




uint8_t PPU::reg_access(int addr, bool read_nwr, uint8_t val) {
    if (read_nwr) {         // Read
//     return 0x90;    // LY Register - 0x90 (144) is just before VBlank. This tricks games into thinking the screen is ready to draw. Remove once ppu/Vblank is implemented.
        return mem->memory[addr];
    } else {                // Write
        if (addr == REG_LCDC) {
            if ((mem->memory[addr] & LCDC_ENABLE_BIT == 1) &&
                (val & LCDC_ENABLE_BIT == 0) &&
                (this->mode != PPUMode::VBLANK)) {
                    std::cerr << "FATAL ERROR: Disabling LCD outside of VBLANK period is prohibited" << std::endl;
            }
        } else if (addr == REG_LY) {
            print ("Attempted write to LY reg [read-only]; Dropping write\n");
            return 0;
        }

        mem->memory[addr] = val;
        return 0;
    }
}

// Single mcycle of the PPU
void PPU::ppu_tick(){
    if (mem->memory[REG_LCDC] & LCDC_ENABLE_BIT == 0) return;

    static int mcycle_cnt = 0;     // initialized once, persists across calls
    static int dot_cnt    = 0;     // initialized once, persists across calls

    
    // BEGIN SCANLINE
    // mem->memory[REG_LY]
    this->mode = PPUMode::OAM_SCAN;     // Stay in this mode for 80 dots (=4 m-cycles)
    if (dot_cnt == 1) {

    }
    // Loop through OAM memory and populate first 10 objects
    // that are visible on this scanline (Y coord)
    // oam_scan();
    // for (int i = 0; i < 40; i++) {
        // TODO: implement this... Ignore object fetching for now until we get blank background

        
    // }

    




    mcycle_cnt++;
    if (mcycle_cnt >= 4) {
        mcycle_cnt = 0;

        // Detected DOT
        dot_cnt++;
    }
}