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


void FIFO::print_contents() {
    DBG("pixel_fifo contents..." << std::endl);
    for (int i = 0; i < FIFO_DEPTH; i++) {
        DBG("     pixels[" << i << "] = {color=" << static_cast<int>(pixels[i].color)
                    << "; valid=" << pixels[i].valid << "}" << std::endl);
    }
}


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

    assert(pxl.valid);  // sanity check we never pop out an invalid Pixel

    return true;
}


PPU::PPU() {
    this->mode = PPUMode::VBLANK;     // PPU should start in this mode once LCD Enabled
}


uint8_t PPU::reg_access(int addr, bool read_nwr, uint8_t val, bool backdoor) {
    if (read_nwr) {
    // ------------------- READS -------------------
        // return 0x90;    // LY Register - 0x90 (144) is just before VBlank. This tricks games into thinking the screen is ready to draw. Remove once ppu/Vblank is implemented.
        return mem->memory[addr];
    } else {
    
    // ------------------- WRITES ------------------

        if (addr == REG_LCDC) {
            if (((mem->memory[addr] & LCDC_ENABLE_BIT) == 1) &&
                ((val & LCDC_ENABLE_BIT) == 0) &&
                (this->mode != PPUMode::VBLANK)) {
                    std::cerr << "FATAL ERROR: Disabling LCD outside of VBLANK period is prohibited" << std::endl;
                    std::exit(EXIT_FAILURE);
            }
        } else if (addr == REG_STAT) {
            // Keep Read-Only bits [2:0] unchanged 
            uint8_t curr_ro_value = mem->get(REG_STAT) & 0b00000111;
            val &= 0b11111000;
            val |= curr_ro_value;
        } else if (addr == REG_LY) {
            if (!backdoor) {
                print ("Attempted write to LY reg [read-only]; Dropping write\n");
                return 0;
            }
        } else if ((addr == REG_WY) || (addr == REG_WX)) {
            if (this->mode != PPUMode::VBLANK) {
                std::cerr << "FATAL ERROR (for now): Writing to WX/WY mid frame causes special behavior. Check this" << std::endl;
                std::exit(EXIT_FAILURE);
            }
        }

        if (addr == REG_LCDC) {
            DBG("Writing to LCDC reg <= 0x" << std::hex << static_cast<int>(val) 
                        << " @ mcycle = " << dbg->mcycle_cnt << std::endl);
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

void PPU::oam_scan() {
    
    // Maybe make use of this function somehow instead of just doing everything in fetch_pixel?
}



void PPU::render_pixel() {
    Pixel pxl;
    // pixel_fifo.print_contents();
    bool pop_result = pixel_fifo.pop(pxl);
    // debug_file << "     [render_pixel] @" << dbg->mcycle_cnt << " mcycles:  pop_pixel_fifo returned " << pop_result << "; LY = " 
    //             << "0x" << std::hex << static_cast<int>(mem->get(REG_LY)) << std::dec << "; framebuff x,y={" << lcd->get_fb_x() << ", " << lcd->get_fb_y() << "}" << std::endl;

    if (pop_result) {
        // Write to SDL framebuffer is equivalent to hardawre drawing to
        // the screen, 1 pixel and
        lcd->write_to_framebuffer(pxl);
    }

}

TileType PPU::get_pixel_tile_type(int pixel_x) {

    if (((mem->get(REG_LCDC) >> LCDC_WINDOW_ENABLE_BIT) & 0b1)) {   // Window Enabled
        uint8_t wy = mem->get(REG_WY);
        uint8_t wx = mem->get(REG_WX);
        if ((ly >= wy) && (pixel_x >= (wx - 7))) {
            return TileType::WINDOW;
        }
    }
    
    return TileType::BACKGROUND;
}


/**
 * Given a pixel x coordinate on the screen - from 0 - 159, fetch and compute the pixel data (Color) and push it to fifo
 */
void PPU::fetch_pixel(int pixel_x) {
    if (pixel_x >= 160) return;     // DRAW_PIXELS runs for 200 dots in XavBoy (arbitrary) so remaining dots just wait

    // Static => Only re-fetch tile_data when pixel_x hits a new tile
    static std::array<uint8_t, 16> tile_data;  // Tile data is 16 bytes : 2bpp * (8x8=64 pixels)
    static uint8_t scy, scx, lcdc;
    static std::array<uint8_t, 16> win_tile_data;
    // 32 = number of tiles along X (32x32 tile indices in the VRAM Tile map)
    ly = mem->get(REG_LY);
    wx = mem->get(REG_WX);
    wy = mem->get(REG_WY);
    uint8_t color_palette = mem->get(REG_BGP);

    uint16_t tile_data_base_addr;
    bool lcdc_4, lcdc_5;

    DBG( "      pxl_x = " << std::dec << pixel_x << "; ");


    // Algorithm
    /**
     * Unoptimized alg:
     *  On every pixel figure out what tile type it is and just fetch the tile data. It will be slow.
     *  But just get the base functionality working
     *  Can optimize later to "decide" whether a new tile fetch is required or not
     */
    bool fetching_new_tile = false;


    static TileType prev_pxl_tile_type = TileType::UNASSIGNED;
    TileType curr_pxl_tile_type;

    curr_pxl_tile_type = get_pixel_tile_type(pixel_x);

    lcdc = mem->get(REG_LCDC);

    // bool fetch_new_tile = ((curr_pxl_tile_type != prev_pxl_tile_type) || 
    //                         ((curr_pxl_tile_type == TileType::BACKGROUND) && ((pixel_x + scx) % 8 == 0)) ||
    //                         ((curr_pxl_tile_type == TileType::WINDOW) && ((pixel_x - (wx-7)) % 8 == 0)));

        // Condition for fetching a new tile


        // 0 1 2 3 4 5 6 7 8 9 10 11
        // b b b w w w w w w w w          => WX = 10. boundary is at 3, 11. 
        // b b b b b b . . . . .  .       => SCX = 2. boundary is at 6. pixel_x = 6 + scx should hit multiples of 8

    uint16_t tile_id_addr;    

    if (curr_pxl_tile_type == TileType::BACKGROUND){
        if (((pixel_x + scx) % 8 == 0)) {
            DBG( "         BG" << std::endl);
            // Fetch background tile

            // The scroll registers are re-read on each tile fetch
            if (pixel_x == 0) {
                scy = mem->get(REG_SCY);
                scx = mem->get(REG_SCX);
            } else {
                scy = mem->get(REG_SCY);
                scx |= (mem->get(REG_SCX) & 0xf8);  // Do not update bottom 3 bits of scx mid-scanline
            }

            // First get which tile to use - tile ID (from 0x9800 to 0x9Bff) (8 bit value)
            // Background tile - Get the tile_id_base_addr based on the current scroll viewport withing 256x256 pixel BG map
            int scroll_adj_tile_ofst_x = (scx + pixel_x) / 8;
            int scroll_adj_tile_ofst_y = (scy + ly) / 8;
            tile_id_addr = 0x9800 + (scroll_adj_tile_ofst_y * 32) + scroll_adj_tile_ofst_x;

            fetching_new_tile = true;
        }
    } else if (curr_pxl_tile_type == TileType::WINDOW) {
        if ((pixel_x - (wx-7)) % 8 == 0) {
            DBG( "         WIN" << std::endl);

            uint16_t tile_id_base_addr = 0x9C00;
            // FIXME: This should respond to LCDC.6 — Window tile map area

            int tile_ofst_x = (pixel_x / 8);
            int tile_ofst_y = (ly / 8);
            // Note: 32 = the number of tiles per row - and each tile id is one byte and occuppies one mem address
            tile_id_addr = tile_id_base_addr + (tile_ofst_y * 32) + tile_ofst_x;

            fetching_new_tile = true;
        }
    }

    if (fetching_new_tile) {
        int tile_id;

        bool lcdc_4 = ((mem->get(REG_LCDC) >> LCDC_BG_WINDOW_TILES_BIT) & 0b1);
        if (lcdc_4 == 1) {
            // The “$8000 method” (Default): with unsigned addressing
            tile_data_base_addr = 0x8000;
            tile_id = static_cast<uint8_t>(mem->get(tile_id_addr));
        } else {
            // The “$8800 method”: with signed addressing
            tile_data_base_addr = 0x9000;
            tile_id = static_cast<int8_t>(mem->get(tile_id_addr));
        }


        uint16_t tile_data_addr = tile_data_base_addr + (tile_id * 16);
        for (int i = 0; i < 16; i++) {
            // Populating each byte of tile data
            tile_data[i] = mem->get(tile_data_addr + i);
        }

        std::string debug_str = "";
        for (int i = 0; i < 16; i++) {
            char buf[4];
            sprintf(buf, "%02X ", tile_data[i]);
            debug_str += buf;
        }
        DBG( "         Tile_data = " << debug_str << std::endl);
    } else {
        DBG( std::endl);
    }

    prev_pxl_tile_type = curr_pxl_tile_type;


    // Compute the pixel to push into the FIFO on this dot - given tiledata and LY value
    int tile_local_y = (scy + ly) % 8;      // which row of the current tile does this pixel lie in?
    uint8_t lsb_byte = tile_data[2*tile_local_y];
    uint8_t msb_byte = tile_data[2*tile_local_y + 1];

    int tile_local_x = (scx + pixel_x) % 8;    // which column of the current tile does this pixel lie in?
    int color_id_lsb = (lsb_byte >> (7 - tile_local_x)) & 0b1;
    int color_id_msb = (msb_byte >> (7 - tile_local_x)) & 0b1;
    int color_id = color_id_lsb + (color_id_msb << 1);

    Pixel pxl;
    pxl.color = static_cast<Color>((color_palette >> (2 * color_id)) & 0b11);
    pxl.valid = true;

    pixel_fifo.push(pxl);

}




// Tick mcycles of the PPU
void PPU::ppu_tick(int mcycles){
    if (CPU_ONLY) return;
    if ((mem->memory[REG_LCDC] & LCDC_ENABLE_BIT) == 0) return;

    static int mcycle_cnt = 0;     // initialized once, persists across calls
    static int dot_cnt    = 0;     // initialized once, persists across calls

    // REMOVE !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    #ifndef REL_MODE
    if (dbg->mcycle_cnt >= 20000000) {
        print ("Run for enough time. Exit\n");
        DBG("Run for enough time. Exit" << std::endl);
        std::exit(EXIT_SUCCESS);
    }
    #endif
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
                if (curr_LY == 0 && dot_cnt == 1) {
                    DBG("Frame: " << dbg->frame_cnt << std::endl);
                    DBG("   SCX = " << static_cast<int>(mem->get(REG_SCX)) << "; SCY = " << static_cast<int>(mem->get(REG_SCY)) << std::endl);
                    DBG("   WX  = " << static_cast<int>(mem->get(REG_WX)) << "; WY = " << static_cast<int>(mem->get(REG_WY)) << std::endl << std::endl);
                    // // -------------------------- REMOVE -------
                    // if (dbg->frame_cnt == 2) {
                    //     std::exit(EXIT_SUCCESS);
                    // }
                    // // -------------------------- REMOVE -------
                }
                if (dot_cnt == 80) {    // Note - 80 might be off by 1..
                    DBG("   [LY = " << std::dec << static_cast<int>(curr_LY) << "] OAM_SCAN: dot_cnt == 80. Moving to DRAW_PIXELS;  @ mcycle = " << dbg->mcycle_cnt);
                    this->mode = PPUMode::DRAW_PIXELS;
                    // Loop through OAM memory and populate first 10 objects
                    // that are visible on this scanline (Y coord)
                    oam_scan();
                }
                break;
            }

            case PPUMode::DRAW_PIXELS: {
                if (dot_cnt == 81) {
                    DBG(std::endl << "   [LY = " << std::dec << static_cast<int>(curr_LY) << "] DRAW_PIXELS: Start fetching and rendering pixels @ mcycle = " << dbg->mcycle_cnt << std::endl);
                }

                render_pixel();

                int pixel_x = (dot_cnt - 81);

                fetch_pixel(pixel_x);   // Pushes each pixel to the fifo

                
                // Note: Fixing DRAW_PIXELS to always take up 200 dots. This is sufficient to run most games since most games just care that VBLANK intrpt is received at the right time
                //       HBLANK timing accuracy generally doesnt matter except for niche raster/parallax effects.
                // TODO: Should actually be variable dot cnt length based on hardware accurate fetcher/fifo modeling. And HBLANK covers the rest - low priority
                if (dot_cnt == 280){
                    DBG(std::endl << "   [LY = " << std::dec << static_cast<int>(curr_LY) << "] DRAW_PIXELS: dot_cnt == 280. Moving to HBLANK @ mcycle = " << dbg->mcycle_cnt << std::endl);

                    this->mode = PPUMode::HBLANK;
                }
                break;
            }

            case PPUMode::HBLANK: {
                if (dot_cnt == 456) {
                    uint8_t new_LY = curr_LY + 1;
                    mem->set(REG_LY, new_LY, 1);

                    dot_cnt = 0;        // Starting new scanline - it's just convenient to restart dot_cnt from 0 to 456..
                    if (new_LY == 144) {
                        DBG("   [LY = " << std::dec << static_cast<int>(curr_LY) << "] HBLANK: dot_cnt == 456. Moving to 1st VBLANK scanline @ mcycle = " << dbg->mcycle_cnt << std::endl);
                        this->mode = PPUMode::VBLANK;

                        // // FIXME: Temp workaround - set 0xFFFA to 1 whenever PPU is in VBLANK mode. And clear once out of VBLANK. Software will use this to poll
                        // mem->set(0xfffa, 1);
                    } else {
                        DBG("   [LY = " << std::dec << static_cast<int>(curr_LY) << "] HBLANK: dot_cnt == 456. Moving to next scanline @ mcycle = " << dbg->mcycle_cnt << std::endl);
                        this->mode = PPUMode::OAM_SCAN;
                    }
                }

                break;
            }

            case PPUMode::VBLANK: {
                if (dot_cnt == 1) {
                    // How was it reaching PC 0040 earlier in the program without this?
                    // Request VBLANK interrupt
                    uint8_t req_vblank = mem->get(REG_IF);
                    req_vblank |= 0x01;
                    mem->set(REG_IF, req_vblank);

                } else if (dot_cnt == 10) {
                    // Finished fetching all the pixel data and framebuffer is populated- this is when screen actually refreshes in hardware. So it's the right time to 
                    if (mcycle_cnt > 10) {
                        // make sure it's not VBLANK out of init
                        lcd->frame_ready = true;
                    }
                } else if (dot_cnt == 456) {
                    DBG("   [LY = " << std::dec << static_cast<int>(curr_LY) << "] VBLANK: dot_cnt == 456. Moving to next VBLANK scanline @ mcycle = " << dbg->mcycle_cnt << std::endl);

                    uint8_t new_LY = curr_LY + 1;
                    if (new_LY == 155) {
                        DBG("   [LY = " << std::dec << static_cast<int>(curr_LY) << "] VBLANK: new_LY == 155; Moving back to scanline 1 OAM_SCAN @ mcycle = " << dbg->mcycle_cnt << std::endl);
                        new_LY = 0;
                        this->mode = PPUMode::OAM_SCAN;

                        // // FIXME: Temp workaround - set 0xFFFA to 1 whenever PPU is in VBLANK mode. And clear once out of VBLANK. Software will use this to poll
                        // mem->set(0xfffa, 0);
                    }

                    mem->set(REG_LY, new_LY, 1);
                    dot_cnt = 0;        // Starting new scanline - it's just convenient to restart dot_cnt from 0 to 456..
                }
                break;
            }

            default:
                break;
        }
    }
}
