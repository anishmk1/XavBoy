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


class FIFO {
public:
    Pixel pixels[FIFO_DEPTH];   // -----> [0 1 2 ... 15] ----->
    int size = 0;

    bool push(Pixel pxl);
    bool pop(Pixel& pxl);
    void print_contents();
};

class PPU {
    uint8_t ly;
    FIFO pixel_fifo;

    TileType get_pixel_tile_type(int pixel_x);
    void fetch_window_tile(int pixel_x, std::array<uint8_t, 16>& tile_data);
    void fetch_background_tile(int pixel_x, std::array<uint8_t, 16>& tile_data);

public:
    PPUMode mode;   // 0 - HBLANK; 1 - VBLANK; 2 - OAM SCAN; 3 - DRAW PIXELS
    void ppu_tick(int mcycles);
    uint8_t reg_access(int addr, bool read_nwr, uint8_t val, bool backdoor=0);
    void draw_pixels();
    void oam_scan();
    void fetch_pixel(int pixel_x);
    void render_pixel();

    PPU();
    void test_ppu();
};

#endif