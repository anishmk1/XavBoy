// PPU.h
#ifndef PPU_H
#define PPU_H

#include <array>

#define PXL_FIFO_DEPTH 16
#define OBJ_FIFO_DEPTH 10

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

enum class TileType : int {
    BACKGROUND  = 0,
    WINDOW      = 1,
    OBJECT      = 2,
    UNASSIGNED  = 3
};

typedef struct {
    Color color;            // a value between 0 and 3
    int palette;            // on DMG this only applies to objects
    // int sprite_priority;    // on DMG this doesn’t exist
    int bg_priority;        // holds the value of the OBJ-to-BG Priority bit
    bool valid;
} Pixel;

typedef struct {
    uint8_t x_pos;
    uint8_t y_pos;
    uint8_t tile_index;
    bool priority;
    bool y_flip;
    bool x_flip;
    bool dmg_palette;   // 0 = OBP0, 1 = OBP1
    // bool bank;
    // bool cgb_palette;
} Object;


template <typename T, int DEPTH>
class FIFO {
public:
    T fifo[DEPTH];   // -----> [0 1 2 ... 15] ----->
    int size = 0;

    bool push(T el);
    bool pop(T& el);
    void print_contents();
    void clear_fifo();
};

class PPU {
    uint8_t ly;
    uint8_t wx;
    uint8_t wy;

    int obj_i;
    FIFO<Pixel, PXL_FIFO_DEPTH> pixels;
    FIFO<Object, OBJ_FIFO_DEPTH> objects;

    TileType get_fallback_tile_type(int pixel_x);

public:
    PPUMode mode;
    void ppu_tick(int mcycles);
    uint8_t reg_access(int addr, bool read_nwr, uint8_t val, bool backdoor=0);
    void draw_pixels();
    void oam_scan();
    void fetch_pixel(int pixel_x);
    void render_pixel();
    int get_object_color_id(int pixel_x, std::ostringstream& obj_debug_oss);

    PPU();
};

#endif