// LCD.h
#ifndef LCD_H
#define LCD_H

#include <SDL2/SDL.h>

#include "PPU.h"

// 32-bit pixels: 0xRRGGBBAA
constexpr int SCREEN_WIDTH  = 160;
constexpr int SCREEN_HEIGHT = 144;

class LCD {

    int framebuffer_write_ptr_x;
    int framebuffer_write_ptr_y;
    uint32_t framebuffer[SCREEN_HEIGHT][SCREEN_WIDTH];
    uint32_t dmg_palette[4] = {     // Encode colors in RGBA8888 format
        0xFFFFFFFF, // White
        0xAAAAAAFF, // Light gray
        0x555555FF, // Dark gray
        0x000000FF  // Black
    };

    void print_framebuffer();

public:
    SDL_Window* window      = nullptr;
    SDL_Texture* texture    = nullptr;
    SDL_Renderer* renderer  = nullptr;
    bool frame_ready;

    void init_screen();
    void close_window();
    void write_to_framebuffer(Pixel& pxl);
    void test_write_to_fb();
    void draw_frame();
    int get_fb_x();
    int get_fb_y();
};


#endif