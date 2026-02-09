# XavBoy Performance Optimizations

## Performance Baseline (2026-02-07)

336 frames analyzed (excluding first frame). Average frame time: **23.93ms** (~41.8 FPS).

| Component      | Avg Time (ms) | % of Frame |
|----------------|---------------|------------|
| PPU Draw       | 17.01         | 71.2%      |
| PPU HBlank     | 2.33          | 9.8%       |
| CPU            | 1.49          | 6.2%       |
| PPU OAM Scan   | 1.28          | 5.4%       |
| PPU VBlank     | 0.81          | 3.4%       |
| PPU (other)    | 0.26          | 1.1%       |
| MMIO           | 0.26          | 1.1%       |
| Debugger       | 0.23          | 0.9%       |
| Interrupt      | 0.21          | 0.9%       |

PPU total (Draw + HBlank + OAM Scan + VBlank + other) = ~91% of frame time. PPU Draw alone is the dominant bottleneck at 71.2%.

---

## Critical Issues

### 1. FIFO Pop Array Shifting
**File:** `src/PPU.cpp:58-78`

**Problem:** O(n) array shifting on every pixel pop
```cpp
for (int i = 0; i < DEPTH - 1; ++i) {
    fifo[i] = fifo[i + 1];  // Shifts all elements
}
```

**Impact:** Called 23,040 times per frame (160 pixels × 144 scanlines). Each call shifts up to 15 Pixel structures.

**Fix:** Use a circular queue with head/tail pointers instead of array shifting.

---

### 2. Redundant mem->get() Calls in fetch_background_tile()
**File:** `src/PPU.cpp:323-395`

**Problem:**
- Line 325: `mem->get(REG_LCDC)` - first read
- Lines 339-343: Multiple redundant calls to `mem->get(REG_SCY)` and `mem->get(REG_SCX)`
- Line 361: `mem->get(REG_LCDC)` - re-reads register already cached at line 325
- Lines 374-377: Loop reading tile data with 16 individual `mem->get()` calls

**Impact:** Called 2,880 times per frame (20 tiles/scanline × 144 scanlines)

**Fix:** Cache register values at the start of the function and use bulk memory copy for tile data.

---

### 3. Redundant mem->get() Calls in fetch_window_tile()
**File:** `src/PPU.cpp:398-470`

**Problem:**
- Line 400: `mem->get(REG_LCDC)` - first read
- Line 445: `mem->get(REG_LCDC)` - reads again in same function
- Lines 406-407: Reads REG_WY and REG_WX unconditionally
- Lines 458-461: Loop with 16 individual `mem->get()` calls for tile data

**Fix:** Cache register values and use bulk memory copy.

---

### 4. Redundant mem->get() in fetch_object_tile()
**File:** `src/PPU.cpp:265-321`

**Problem:**
- Line 271: `mem->get(REG_LCDC)` cached
- Lines 298-307: Loop reading 16-32 bytes of tile data with individual `mem->get()` calls

**Fix:** Use bulk memory copy for tile data.

---

### 5. Debug ostringstream Created Per Pixel
**File:** `src/PPU.cpp:505`

**Problem:**
```cpp
std::ostringstream bg_debug_oss, win_debug_oss, obj_debug_oss;
```
Three ostringstream objects created for every pixel, even in release mode.

**Impact:** 23,040 allocations per frame × 3 = 69,120 allocations/frame

**Fix:** Move ostringstream declarations outside the function or use conditional compilation.

---

### 6. OAM Insertion Sort with Nested Loops
**File:** `src/PPU.cpp:206-229`

**Problem:** Insertion sort with nested loops (worst case O(n²))
```cpp
for (int i = 0; i < OBJ_FIFO_DEPTH; i++) {
    if ((obj.x_pos < objects.fifo[i].x_pos) || (i == objects.size)) {
        for (int j = i; j < objects.size; j++) {  // Nested loop
            out_obj = objects.fifo[j];
            objects.fifo[j] = in_obj;
            in_obj = out_obj;
        }
    }
}
```

**Impact:** Called 144 times per frame with up to 40 OAM entries each time.

**Fix:** Use a more efficient sorting algorithm or maintain sorted order differently.

---

### 7. Redundant LCDC Reads in OAM Scan
**File:** `src/PPU.cpp:188`

**Problem:** `mem->get(REG_LCDC)` called for every OAM entry checked (~40 times per scanline)

**Impact:** ~5,760 redundant reads per frame

**Fix:** Cache LCDC value once at the start of oam_scan().

---

## Major Issues

### 8. lcd_status_update() Redundant Reads
**File:** `src/LCD.cpp:22-57`

**Problem:**
```cpp
uint8_t lcd_stat = mem->get(REG_STAT);           // Read 1
if (mem->get(REG_LYC) == mem->get(REG_LY)) {    // Reads 2 & 3
    lcd_stat |= (0x01 << 2);
}
bool ppu_enabled = (mem->get(REG_LCDC) & ...);  // Read 4
```

**Impact:** Called once per dot (~456 dots/scanline × 154 scanlines = ~70,000 times/frame)

**Fix:** Cache register values.

---

### 9. File I/O Every Frame in Debug Mode
**File:** `src/LCD.cpp:170`

**Problem:**
```cpp
#ifndef REL_MODE
lcd->log_framebuffer();  // Opens file, writes 144×160 entries
#endif
```

**Impact:** Massive I/O overhead every frame in debug builds.

**Fix:** Make this opt-in or only log periodically.

---

### 10. Default Build Not Optimized
**File:** `Makefile:30-31`

**Problem:**
```makefile
CXXFLAGS = -DDEBUG_MODE -Wall -Wextra -std=c++17          # No -O3!
CXXRELFLAGS = -DREL_MODE -O3 -Wall -Wextra -std=c++17    # Has -O3
```

Regular `make` uses debug flags without optimization. Only `make release` gets -O3.

**Fix:** Always use `make release` for testing, or add optimization to debug builds.

---

### 11. Debug Output in PPU State Machine
**File:** `src/PPU.cpp:708-794`

**Problem:** Multiple DBG() calls throughout the PPU state machine that runs every dot.

**Fix:** Ensure DBG() compiles to nothing in release mode, including any argument evaluation.

---

## Moderate Issues

### 12. OAM DMA Copy Loop
**File:** `src/PPU.cpp:828-830`

**Problem:**
```cpp
for (int i = 0; i < 0xA0; i++) {
    mem->memory[0xFE00 + i] = mem->memory[source_addr + i];
}
```

**Fix:** Use `std::memcpy()` for 160 bytes.

---

### 13. Memory Stored as std::vector
**File:** `src/Memory.h:15`

**Problem:** Memory access goes through vector's bounds checking and indirection.

**Fix:** Consider using a raw array `uint8_t memory[0x10000]` for faster access.

---

### 14. Global ofstream Objects
**File:** `src/main.cpp:37-41`

**Problem:**
```cpp
std::ofstream logFile;
std::ofstream debug_file;
std::ofstream pixel_map;
std::ofstream serialFile;
```

These add indirection overhead when referenced throughout the codebase.

---

## Quick Wins Summary

| Priority | Fix | Impact |
|----------|-----|--------|
| 1 | Replace FIFO with circular queue | Eliminates ~350,000 array shifts/frame |
| 2 | Cache LCDC/SCX/SCY at scanline start | Eliminates ~100,000 redundant reads |
| 3 | Move ostringstream outside hot loop | Eliminates 69,120 allocations/frame |
| 4 | Use memcpy for tile data | Faster than 16-32 individual reads |
| 5 | Always build with `make release` | Enables -O3 optimizations |
| 6 | Cache registers in lcd_status_update | Eliminates ~70,000 redundant reads |

## Estimated Impact

If all critical and major issues are fixed, expect:
- 3-5x improvement in PPU rendering performance
- Significantly smoother frame rates
- Reduced CPU usage
