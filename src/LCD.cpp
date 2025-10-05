#include <SDL2/SDL.h>
#include <iostream>
#include <fstream>

#include "LCD.h"
#include "main.h"

//---------------------------------
//  Compile with:
//      make app:
//      g++ src/App.cpp -o bin/App.out -L /opt/homebrew/opt/sdl2/lib -I /opt/homebrew/opt/sdl2/include -lSDL2
//---------------------------------

void LCD::test_write_to_fb() {
    for (int i = 0; i < SCREEN_HEIGHT; i++) {
        for (int j = 0; j < SCREEN_WIDTH; j++) {
            framebuffer[i][j] = dmg_palette[1];
        }
    }
}

int LCD::get_fb_x() {return this->framebuffer_write_ptr_x;}
int LCD::get_fb_y() {return this->framebuffer_write_ptr_y;}


// DRAW PIXELS




void LCD::write_to_framebuffer(Pixel& pxl) {
    // Pixel pixels[8];
    // ppu->pop_pixel_fifo(pixels);

    // for (int i = 0; i < 8; i++) {
    // Pixel pxl = pixels[i];
    uint32_t pxl_rgb = dmg_palette[static_cast<int>(pxl.color)];

    framebuffer[framebuffer_write_ptr_y][framebuffer_write_ptr_x] = pxl_rgb;
    if (dbg->frame_cnt == 1) {
        dbg->last_framebuffer[framebuffer_write_ptr_y][framebuffer_write_ptr_x] = pxl.color;    // SAVE COPY FOR DEBUG
    }

    // Manage frame buffer pointers
    if (framebuffer_write_ptr_x == (SCREEN_WIDTH-1)) {
        // Reached end of current scanline
        framebuffer_write_ptr_x = 0;
        if (framebuffer_write_ptr_y == (SCREEN_HEIGHT-1)) {
            // Reached end of frame - ready to display
            framebuffer_write_ptr_y = 0;
            this->frame_ready = true;
        } else {
            // Frame not finished - move to the next scanline
            framebuffer_write_ptr_y++;
        }
    } else {
        // Current scanline not finished - point to next set of 8 pixels (tile)
        // framebuffer_write_ptr_x += 8;
        framebuffer_write_ptr_x++;
    }
    // }
}

void LCD::close_window() {
    if (CPU_ONLY) return;

    // Print out current framebuffer contents for debug
    // print_framebuffer();

    // Clean up
    SDL_DestroyWindow(window);
    SDL_Quit();

    // std::exit(EXIT_SUCCESS);
}

void LCD::print_framebuffer() {

    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            pixel_map << static_cast<int>(dbg->last_framebuffer[y][x]) << " ";
        }
        pixel_map << std::endl;
    }
}

void LCD::draw_frame() {
    if (CPU_ONLY) return;
    this->frame_ready = false;
    
    // debug_file << "     [draw_frame] Abt to SDL_UpdateTexture" << std::endl;
    // debug_file << "     [draw_frame] texture == nullptr ? " << (texture == nullptr) << std::endl;

    // TODO: Worth exploring:: from SDL Wiki reg SDL_UpdateTexture
    // This is a fairly slow function, intended for use with static textures that do not change often.
    // If the texture is intended to be updated often, it is preferred to create the texture as streaming and use the locking functions referenced below. 
    // While this function will work with streaming textures, for optimization reasons you may not get the pixels back if you lock the texture afterward.
    // Add these checks:
    if (!texture) {
        DBG("Error: texture is NULL\n" << std::endl);
        return;
    }

    // / Right before SDL_QueryTexture
    // debug_file << "     [draw_frame] About to query texture: " << static_cast<void*>(texture) << std::endl;
    // debug_file << "     [draw_frame] Renderer state: " << static_cast<void*>(renderer) << std::endl;

    // Verify texture dimensions
    // int w, h;
    // debug_file << "     Abt to SDL_QueryTexture" << std::endl;
    // int result = SDL_QueryTexture(texture, nullptr, nullptr, &w, &h);
    // if (result != 0) {
    //     debug_file << "SDL_QueryTexture error: " << SDL_GetError() << std::endl;
    // }
    // debug_file << "Texture dimensions: " << w << "x" << h << std::endl;

    // Check if pitch calculation matches texture width
    // int expected_pitch = w * sizeof(uint32_t);
    // int provided_pitch = 160 * sizeof(uint32_t);
    // debug_file << "Expected pitch: " << expected_pitch << std::endl;
    // debug_file << "Provided pitch: " << provided_pitch << std::endl;

    // if (expected_pitch != provided_pitch) {
    //     debug_file << "Warning: Pitch mismatch detected!" << std::endl;
    // }


    // Upload it to texture
    // WHY IS THIS LINE OF CODE SEGFAULTING?
    SDL_UpdateTexture(texture, nullptr, framebuffer, 160 * sizeof(uint32_t));

    // // Set renderer draw color to white (RGBA: 255,255,255,255)
    // SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    // Clear screen
    SDL_RenderClear(renderer);


    // Draw framebuffer texture 
    // 1. Copies the framebuffer to the renderer (background)
    // 2. Changes are made in the background to not cause glitches on the screen
    SDL_RenderCopy(renderer, texture, nullptr, nullptr);

    // Show it
    SDL_RenderPresent(renderer);

    dbg->set_breakpoint("draw_frame");
}


void LCD::init_screen() {
    if (CPU_ONLY) return;

    std::cout << "SDL Window\n";

    // Initialize SDL (video subsystem only)
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return;
    }

    // Create a window
    window = SDL_CreateWindow(
        "XavBoy",                      // Title
        SDL_WINDOWPOS_CENTERED,        // X position
        SDL_WINDOWPOS_CENTERED,        // Y position
        1000, 900,                     // Width, Height
        SDL_WINDOW_SHOWN               // Flags
    );

    // 3. Create a renderer
    renderer = SDL_CreateRenderer(
        window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );
    if (!renderer) {
        std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
        return;
    }

    // Set scaling hint (for crisp pixels) - since framebuffer from Gameboyis only 160x144 pixels but output res is 1000x900
    // Image needs to be scaled up - meaning need to add more pixels.
    // By default SDL uses linear filtering -
    //      Each pixel from framebuffer turns to a block of pixels where the edges of each block "blend into" the adjacent pixels
    //      smooth interpolation, looks blurry. But retro GB pixels should look sharp and crisp
    // Switch to nearest-neighbor scaling - 
    //      Each pixel becomes a block of identical pixels
    //      Leaves sharp jagged edges - looks more faithful to the real Gameboy
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");

    texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA8888,  // matches framebuffer format
        SDL_TEXTUREACCESS_STREAMING,
        160,
        144
    );

    if (!window) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return;
    }

    return;
}