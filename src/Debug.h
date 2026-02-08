// Debug.h
#ifndef DEBUG_H
#define DEBUG_H

#include <string>
#include <chrono>
#include <fstream>
#include "PPU.h"

// ============================================================
// Performance Profiling
// ============================================================
// Enable with: -DPERF_PROFILING in compiler flags
// Usage:
//   PERF_NEW_FRAME(dbg)                    - Call at frame boundary
//   PERF_SECTION(dbg, PerfSection::CPU)    - Call after a section completes
// ============================================================

enum class PerfSection {
    CPU,
    PPU,
    PPU_OAM_SCAN,
    PPU_DRAW_PIXELS,
    PPU_HBLANK,
    PPU_VBLANK,
    MMIO,
    LCD,
    SDL_EVENTS,
    DEBUGGER,
    INTERRUPT,
    OTHER
};

#ifdef PERF_PROFILING
    #define PERF_NEW_FRAME(dbg)             do { (dbg)->new_frame_timing(); } while(0)
    #define PERF_SECTION(dbg, section)      do { (dbg)->end_section_timing(section); } while(0)
#else
    #define PERF_NEW_FRAME(dbg)             do { } while(0)
    #define PERF_SECTION(dbg, section)      do { } while(0)
#endif

class CPU;

typedef struct {
    bool breakpoint;
    std::string msg;
    bool disable_breakpoints;
} BreakpointInfo;

typedef struct {
    uint16_t pc;
    bool valid;
} TargetPC;

typedef struct {
    double cpu_time_ms;
    double ppu_time_ms;
    double ppu_oam_scan_time_ms;
    double ppu_draw_pixels_time_ms;
    double ppu_hblank_time_ms;
    double ppu_vblank_time_ms;
    double mmio_time_ms;
    double lcd_time_ms;
    double sdl_events_time_ms;
    double debugger_time_ms;
    double interrupt_time_ms;
    double other_time_ms;
    double total_frame_time_ms;
} FramePerfData;

typedef struct {
    int num_main_loops;
    std::chrono::high_resolution_clock::time_point curr_frame_draw_time;

    // Per-frame timing accumulators
    std::chrono::high_resolution_clock::time_point frame_start_time;
    double frame_cpu_time_ms;
    double frame_ppu_time_ms;
    double frame_ppu_oam_scan_time_ms;
    double frame_ppu_draw_pixels_time_ms;
    double frame_ppu_hblank_time_ms;
    double frame_ppu_vblank_time_ms;
    double frame_mmio_time_ms;
    double frame_lcd_time_ms;
    double frame_sdl_events_time_ms;
    double frame_debugger_time_ms;
    double frame_interrupt_time_ms;
    double frame_other_time_ms;

    // For tracking main loop sections
    std::chrono::high_resolution_clock::time_point last_section_time;

    bool csv_logging_enabled;
    std::ofstream csv_file;  // Keep file open for entire runtime
} PerfMetrics;

class Debug {
public:

    unsigned long instr_cnt = 0;
    unsigned long mcycle_cnt = 0;
    unsigned long frame_cnt = 0;
    long num_steps_left;
    bool run;
    uint8_t tgt_instr;
    // uint8_t tgt_pc;
    TargetPC tgt_pc;
    unsigned long tgt_frame_cnt;
    bool tgt_frame_valid;
    BreakpointInfo bp_info;
    PerfMetrics perf;

    Color last_framebuffer[144][160];

    bool disable_interrupts = false;

    Debug();
    void set_breakpoint(std::string msg);
    void debugger_break();
    void check_for_breakpoints();

    // CSV performance logging methods
    void init_csv_logging();
    void new_frame_timing();  // Ends previous frame (if any) and starts new one
    void log_component_timing(PerfSection section, double time_ms);
    void write_csv_row(const FramePerfData& data);

    // Detailed profiling methods
    void end_section_timing(PerfSection section);

};

#endif