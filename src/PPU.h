// PPU.h
#ifndef PPU_H
#define PPU_H

#define FIFO_DEPTH 16

enum class Color : uint8_t {
    WHITE       = 0,
    LIGHT_GREY  = 1,
    DARK_GREY   = 2,
    BLACK       = 3
};

enum class ColorIndex : uint8_t {
    COLOR_0 = 0,
    COLOR_1 = 1,
    COLOR_2 = 2,
    COLOR_3 = 3
};

enum class PPUMode : uint8_t {
    HBLANK      = 0,
    VBLANK      = 1,
    OAM_SCAN    = 2,
    DRAW_PIXELS = 3
};

typedef struct {
    Color color;            // a value between 0 and 3
    int palette;            // on DMG this only applies to objects
    // int sprite_priority;    // on DMG this doesn’t exist
    int bg_priority;        // holds the value of the OBJ-to-BG Priority bit
} Pixel;

// class Tile {
// public:
//     Pixel pixel_grid[8][8]; // [row][column]

//     Tile(std::array<uint8_t, 16>& tile_data);
// };

class FIFO {
public:
    Pixel pixels[FIFO_DEPTH];

    void push(Pixel num);
    Pixel pop();
};

class PPU {
public:
    FIFO pixel_fifo;
    PPUMode mode;   // 0 - HBLANK; 1 - VBLANK; 2 - OAM SCAN; 3 - DRAW PIXELS
    void ppu_tick(int mcycles);
    uint8_t reg_access(int addr, bool read_nwr, uint8_t val, bool backdoor=0);
    void draw_pixels();

    PPU();
    void test_ppu();
};

#endif