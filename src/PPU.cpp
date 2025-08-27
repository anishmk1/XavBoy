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

// LCD Register addresses
const uint16_t REG_LCDC = 0xff40;   // LCD control
const uint16_t REG_LY   = 0xff44;   // LCD Y co-ordinate [read-only]
const uint16_t REG_BGP  = 0xff47;   // BG Palette Data

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


// Tile::Tile(std::array<uint8_t, 16>& tile_data) {
//     // Construct a 2D array of Pixels based on the Tile Data Format
//     for (int i = 0; i < 8; i++) {   // For each row
//         for (int j = 0; j < 8; j++) {

//             // LSBs of the Color IDs come from the 1st Byte
//             uint8_t lsb_byte = tile_data[2*i]; // tile_data[i + (j/4)];
//             int color_id_lsb = (lsb_byte >> (7-j)) & 0b1;
//             uint8_t msb_byte = tile_data[2*i + 1];
//             int color_id_msb = (msb_byte >> (7-j)) & 0b1;
//             int color_id = color_id_lsb + (color_id_msb << 1);

//             // Using the color id, get the actual color by indexing into the palette
//             Pixel p;
//             uint8_t color_palette = mem->get(REG_BGP);
//             p.color = static_cast<Color>((color_palette >> (2*color_id)) & 0b11);
//             pixel_grid[i][j] = p;
//         }
//     }
// }



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


/**
 * 
 * GAMEBOY GRAPHICS CONCEPTS >>
 * 
 * TILEs are 8x8 grids of pixels. They are like presets which are stored individulaly in memory. You can have a tree tile, a water tile, some other texture tiles, or even some sprite tiles 
 * 
 * TILEMAPs are grids of numbers - the whole gameboy scdreen is taken up by the tile map - 32 x 32 tiles big. (Actually a good chunk of the tilemap is not even in the visible screen. And gameboy le
 * lets you change which part of the tilemap is visible - See Background Scrolling.)
 * EAch tile in the tilemap has a nunmber and this number is used to index into the Tile Map Memory (idk what this is called) and pull out that "preset" tile
 * So if you want a screen full of water, you can have a single water tile in memory (which takes up 16 bytes i think) in location 8 for example
 * And then just populate your tile map with the number 8.
 * 
 * Each entry in the tilemap can store an 8 bit Value to reference a tile preset in memory. So thats 0-255 tiles. That's limiting. So thats why gameboy allows up to 360 tiles
 * But the trick to be able to use the extra ones is to change the "TILE ADDRESSING MODE". You can change which tiles you address in memory with certain numbers
 * 
 * Another trick is gameboy offers a way to swap between 2 SEPARATE TILE MAPS quickly.
 * 
 * Background Scrolling:
 * Change which part of the Tilemap is visible on screen. SCX and SCY == (0,0)  means show the top left of the tile map
 *   Programming notes - when the player moves to the right, the game needs to update the tilemap just outside and to the right
 *   of the visible scope. And once the player reaches the end of the tilemap, game needs to update the leftmost (SCY=0) of the 
 *   tile map since SCY will loop the visible screen back around to the left of the tilemap
 * 
 * Window - Part 4 of the series by System of Levers. Not needed for blank screen
 *
 * 
 */



 /**
  * BLANK SCREEN PLAN:
  *     The color palette is set such that all values in the tilemap map to color grey. Doesnt matter what junk is in the tilemap memory
  *     No objects or sprites.
  *     The only thing to worry about now is how to get the background pixels. (SCX and SCY are both 0,0)
  *     So what is the 
  */




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

    std::array<uint8_t, 16> tile_data;  // Tile data is 16 bytes : 2bpp * (8x8=64 pixels)
    // 32 = number of tiles along X (32x32 tile indices in the VRAM Tile map)
    uint8_t ly = mem->get(REG_LY);
    uint8_t color_palette = mem->get(REG_BGP);
    for (int tile_x = 0; tile_x < 32; tile_x++) {
        // First get which tile to use - tile ID (from 0x9800 to 0x9Bff) (8 bit value)
        uint16_t tile_id_addr = 0x9800 + tile_x + (0 /*scanline 0 times number of X indices in a scanline*/);
        uint8_t tile_id = mem->get(tile_id_addr);   // 8 bit number (0-255)

        // Then fetch what that tile looks like from Memory.
        // Get the 16 byte data (each pixel is 2 bits. 8x8 = 64 pixels => 128 bits => 128/8 = 16 bytes)
        uint16_t tile_data_addr = 0x9000 + (tile_id * 16);       // 0x9000 = base addr for VRAM Tile Data for (BG/WIN when LCDC.4 is 0 and Using unsigned tile addressign mode)
        for (int i = 0; i < 16; i++) {
            // Populating each byte of tile data
            tile_data[i] = mem->get(tile_data_addr);
        }


        // COnstruct a list of 8 Pixels to push into the Pixel FIFO - given current tiledata and LY value
        int tile_local_y = ly % 8;      // ly is y coord across all tiles - what is the y relative to the current tile?
        uint8_t lsb_byte = tile_data[2*tile_local_y];
        uint8_t msb_byte = tile_data[2*tile_local_y + 1];
        for (int i = 0; i < 8; i++) {   // Loop through each column of the current tile
            int color_id_lsb = (lsb_byte >> (7-i)) & 0b1;
            int color_id_msb = (msb_byte >> (7-i)) & 0b1;
            int color_id = color_id_lsb + (color_id_msb << 1); 

            Pixel pxl;
            pxl.color = static_cast<Color>((color_palette >> (2*color_id)) & 0b11);

            pixel_fifo.push(pxl);   // FIXME: Is the ordering of this correct? It's technically pushing in 0th Pixel first.
        }
    }

    


    mcycle_cnt++;
    if (mcycle_cnt >= 4) {
        mcycle_cnt = 0;

        // Detected DOT
        dot_cnt++;
    }
}





void PPU::test_ppu() {
    printx ("Testing PPU\n");

    
}