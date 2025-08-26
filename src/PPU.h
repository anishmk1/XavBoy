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
    int color;              //  a value between 0 and 3
    int palette;            // on CGB a value between 0 and 7 and on DMG this only applies to objects
    int sprite_priority;    // on CGB this is the OAM index for the object and on DMG this doesn’t exist
    int bg_priotiy;         // holds the value of the OBJ-to-BG Priority bit
} Pixel;

class FIFO {
public:
    Pixel pixels[FIFO_DEPTH];

    void push(Pixel num);
    Pixel pop();
};

class PPU {
public:
    PPUMode mode;   // 0 - HBLANK; 1 - VBLANK; 2 - OAM SCAN; 3 - DRAW PIXELS
    void ppu_tick();
    uint8_t reg_access(int addr, bool read_nwr, uint8_t val);
};

#endif