#include <SDL2/SDL.h>
#include <iostream>

#include "LCD.h"

//---------------------------------
//  Compile with:
//      make app:
//      g++ src/App.cpp -o bin/App.out -L /opt/homebrew/opt/sdl2/lib -I /opt/homebrew/opt/sdl2/include -lSDL2
//---------------------------------



int LCD::init_screen() {
    std::cout << "SDL Window\n";

    // Initialize SDL (video subsystem only)
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Create a window
    SDL_Window* window = SDL_CreateWindow(
        "XavBoy",                      // Title
        SDL_WINDOWPOS_CENTERED,        // X position
        SDL_WINDOWPOS_CENTERED,        // Y position
        1000, 900,                     // Width, Height
        SDL_WINDOW_SHOWN               // Flags
    );

    if (!window) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    // Main loop
    bool running = true;
    SDL_Event event;

    while (running) {
        // Poll for events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false; // Window closed
            }
        }

        // (No drawing yet — just an empty window)
    }

    // Clean up
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}