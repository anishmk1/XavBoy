// LCD.h
#ifndef LCD_H
#define LCD_H

#include <SDL2/SDL.h>

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

public:
    SDL_Window* window;
    SDL_Texture* texture;
    SDL_Renderer* renderer;
    bool frame_ready;

    int init_screen();
    void close_window();
    void write_to_framebuffer();
    void test_write_to_fb();
    void draw_frame();
};


#endif