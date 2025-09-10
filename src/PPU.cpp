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
// constexpr uint8_t LCDC_WINDOW_TILEMAP_BIT    = 1 << 6;   // Bit 6
// constexpr uint8_t LCDC_WINDOW_ENABLE_BIT     = 1 << 5;   // Bit 5
// constexpr uint8_t LCDC_BG_WINDOW_TILES_BIT   = 1 << 4;   // Bit 4
// constexpr uint8_t LCDC_BG_TILEMAP_BIT        = 1 << 3;   // Bit 3
// constexpr uint8_t LCDC_OBJ_SIZE_BIT          = 1 << 2;   // Bit 2
// constexpr uint8_t LCDC_OBJ_ENABLE_BIT        = 1 << 1;   // Bit 1
// constexpr uint8_t LCDC_BG_WINDOW_ENABLE_BIT  = 1 << 0;   // Bit 0   BG & Window enable / priority



bool FIFO::push(Pixel pxl) {

    if (size >= FIFO_DEPTH) {
        // debug_file << "[FIFO Push] FIFO Full detected; size = " << size << std::endl;
        return false; // FIFO full
    }

    pixels[size] = pxl;  // Insert at the end
    size++;
    // debug_file << "[FIFO Push] Pushed pxl.color = " << static_cast<int>(pxl.color) << "; val" << std::endl;
    return true;
}

bool FIFO::pop(Pixel& pxl) {

    if (size == 0) {
        return false;
    }

    // Retrieve the pixel from the FRONT (index 0)
    pxl = pixels[0];

    // Shift everything to the left
    for (int i = 0; i < FIFO_DEPTH - 1; ++i) {
        pixels[i] = pixels[i + 1];
    }

    // Clear the last pixel (now empty)
    pixels[FIFO_DEPTH - 1] = Pixel{};
    size--;



    // debug_file << "Inside pop() for pxl.color=" << static_cast<int>(pxl.color) << std::endl;
    debug_file  << "     Inside pop() for pxl.color=" << static_cast<int>(pxl.color) << "; pxl.valid =" 
                << pxl.valid << "; framebuffer x,y = {" << lcd->get_fb_x() << ", " << lcd->get_fb_y() << "}" << std::endl;
    // assert(pxl.valid);  // sanity check we never pop out an invalid Pixel
    if (pxl.valid == false) {
        debug_file << "POPPED PXL INVALID" << std::endl;
    }

    return true;
}


PPU::PPU() {
    this->mode = PPUMode::VBLANK;     // PPU should start in this mode once LCD Enabled
}


uint8_t PPU::reg_access(int addr, bool read_nwr, uint8_t val, bool backdoor) {
    if (read_nwr) {         // Read
//     return 0x90;    // LY Register - 0x90 (144) is just before VBlank. This tricks games into thinking the screen is ready to draw. Remove once ppu/Vblank is implemented.
        return mem->memory[addr];
    } else {                // Write
        if (addr == REG_LCDC) {
            if (((mem->memory[addr] & LCDC_ENABLE_BIT) == 1) &&
                ((val & LCDC_ENABLE_BIT) == 0) &&
                (this->mode != PPUMode::VBLANK)) {
                    std::cerr << "FATAL ERROR: Disabling LCD outside of VBLANK period is prohibited" << std::endl;
            }
        } else if (addr == REG_LY) {
            if (!backdoor) {
                print ("Attempted write to LY reg [read-only]; Dropping write\n");
                return 0;
            }
        }

        mem->memory[addr] = val;
        return 0;
    }
}

bool PPU::pop_pixel_fifo(Pixel& pxl) {
    return pixel_fifo.pop(pxl);
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



void PPU::render_pixel() {
    Pixel pxl;
    if (pop_pixel_fifo(pxl)) {
        // Write to SDL framebuffer is equivalent to hardawre drawing to
        // the screen, 1 pixel and
        lcd->write_to_framebuffer(pxl);
        debug_file << "     [render_pixel] Wrote above (popped) pxl to framebuffer" << std::endl;
    } else {
        debug_file << "     [render_pixel] pop_pixel_fifo returned false" << std::endl;
    }

}

/**
 * Given a pixel x coordinate - from 0 - 159, fetch and compute the pixel data (Color) and push it to fifo
 */
void PPU::fetch_pixel(int pixel_x) {
    if (pixel_x >= 160) return;     // Note: This is not needed once HBLANK is implemented


    std::array<uint8_t, 16> tile_data;  // Tile data is 16 bytes : 2bpp * (8x8=64 pixels)
    // 32 = number of tiles along X (32x32 tile indices in the VRAM Tile map)
    uint8_t ly = mem->get(REG_LY);
    uint8_t color_palette = mem->get(REG_BGP);

    int tile_x = pixel_x / 8;

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

    // Compute the pixel to push into the FIFO on this dot - given tiledata and LY value
    int tile_local_y = ly % 8;      // ly is y coord across all tiles - what is the y relative to the current tile?
    uint8_t lsb_byte = tile_data[2*tile_local_y];
    uint8_t msb_byte = tile_data[2*tile_local_y + 1];

    int pixel_local_x = pixel_x % 8;
    int color_id_lsb = (lsb_byte >> (7 - pixel_local_x)) & 0b1;
    int color_id_msb = (msb_byte >> (7 - pixel_local_x)) & 0b1;
    int color_id = color_id_lsb + (color_id_msb << 1);

    Pixel pxl;
    pxl.color = static_cast<Color>((color_palette >> (2 * color_id)) & 0b11);
    pxl.valid = true;

    pixel_fifo.push(pxl);

}




// Tick mcycles of the PPU
void PPU::ppu_tick(int mcycles){
    if ((mem->memory[REG_LCDC] & LCDC_ENABLE_BIT) == 0) return;

    static int mcycle_cnt = 0;     // initialized once, persists across calls
    static int dot_cnt    = 0;     // initialized once, persists across calls

    // REMOVE !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    if (dbg->mcycle_cnt >= 20000000) {
        print ("Run for enough time. Exit\n");
        std::exit(EXIT_SUCCESS);
    }
    // REMOVE !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    
    mcycle_cnt += mcycles;
    if (mcycle_cnt >= 4) {
        // debug_file << "Detected DOT" << std::endl;
        mcycle_cnt %= 4;

        // Detected DOT
        dot_cnt++;
        // if (dot_cnt)

        uint8_t curr_LY = mem->get(REG_LY);

        // ------------------- //
        //  PPU state machine  //
        // ------------------- //
        switch (this->mode) {
            case PPUMode::OAM_SCAN: {
                if (dot_cnt == 80) {    // Note - 80 might be off by 1..
                    debug_file << "[LY = " << static_cast<int>(curr_LY) << "] OAM_SCAN: dot_cnt == 80. Moving to DRAW_PIXELS;  @ mcycle = " << dbg->mcycle_cnt << std::endl;
                    this->mode = PPUMode::DRAW_PIXELS;
                    // Loop through OAM memory and populate first 10 objects
                    // that are visible on this scanline (Y coord)
                    // oam_scan();
                }
                break;
            }

            case PPUMode::DRAW_PIXELS: {

                render_pixel();

                int pixel_x = (dot_cnt - 81);
                // debug_file << "     [ppu_tick] Before fetch pickel" << std::endl;
                fetch_pixel(pixel_x);   // Pushes each pixel to the fifo
                // debug_file << "     [ppu_tick] After fetch pickel" << std::endl;

                
                if (dot_cnt == 456){     // FIXME: Should actually be variable dot cnt length. And HBLANK should make up the rest
                    debug_file << "[LY = " << static_cast<int>(curr_LY) << "] DRAM_PIXELS: dot_cnt == 456. Moving on to next scanline @ mcycle = " << dbg->mcycle_cnt << std::endl;

                    uint8_t new_LY = curr_LY + 1;
                    mem->set(REG_LY, new_LY, 1);
                    dot_cnt = 0;        // Starting new scanline - it's just convenient to restart dot_cnt from 0 to 456..

                    // lcd->update_framebuffer_ptrs(curr_LY, );
                    // lcd->write_to_framebuffer();

                    // FIXME: pretending HBLANK doesnt exist for now..
                    if (new_LY == 144) {
                        this->mode = PPUMode::VBLANK;
                    } else {
                        this->mode = PPUMode::OAM_SCAN;
                    }
                }
                break;
            }

            case PPUMode::VBLANK: {
                if (dot_cnt == 1) {
                    // Finished fetching all the pixel data and framebuffer is populated- this is when screen actually refreshes in hardware. So it's the right time to 
                    lcd->frame_ready = true;
                    // lcd->test_write_to_fb();
                } else if (dot_cnt == 456) {
                    debug_file << "[LY = " << static_cast<int>(curr_LY) << "] VBLANK: dot_cnt == 456. Moving to next VBLANK scanline @ mcycle = " << dbg->mcycle_cnt << std::endl;
                    uint8_t new_LY = curr_LY + 1;
                    if (new_LY == 146) {
                    // if (new_LY == 155) {     FIXME: uncomment this. Changing this so it spends less time in VBLANK for debug
                        debug_file << "[LY = " << static_cast<int>(curr_LY) << "] VBLANK: new_LY == 155; Moving back to scanline 1 OAM_SCAN @ mcycle = " << dbg->mcycle_cnt << std::endl;
                        new_LY = 0;
                        this->mode = PPUMode::OAM_SCAN;
                    }

                    mem->set(REG_LY, new_LY, 1);
                    dot_cnt = 0;        // Starting new scanline - it's just convenient to restart dot_cnt from 0 to 456..
                }
                break;
            }

            default:
                break;
        }

        debug_file << "     [ppu_tick] End of function" << std::endl;
    }
}
