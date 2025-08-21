// PPU.h
#ifndef PPU_H
#define PPU_H

#define FIFO_DEPTH 16

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
    int mode;   // 0 - HBLANK; 1 - VBLANK; 2 - OAM SCAN; 3 - DRAW PIXELS
    uint8_t reg_access(int addr, bool read_nwr, uint8_t val);
};

#endif