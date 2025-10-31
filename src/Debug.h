// Debug.h
#ifndef DEBUG_H
#define DEBUG_H

#include <string>
#include <chrono>
#include <fstream>
#include "PPU.h"

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
    double frame_mmio_time_ms;
    double frame_lcd_time_ms;
    double frame_sdl_events_time_ms;
    double frame_debugger_time_ms;
    double frame_interrupt_time_ms;
    double frame_other_time_ms;

    // For tracking main loop sections
    std::chrono::high_resolution_clock::time_point last_section_time;

    bool csv_logging_enabled;
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
    BreakpointInfo bp_info;
    PerfMetrics perf;

    Color last_framebuffer[144][160];

    bool disable_interrupts = false;

    Debug();
    void set_breakpoint(std::string msg);
    void debugger_break(CPU &cpu);

    // CSV performance logging methods
    void init_csv_logging();
    void start_frame_timing();
    void log_component_timing(const std::string& component, double time_ms);
    void finalize_frame_timing();
    void write_csv_header();
    void write_csv_row(const FramePerfData& data);

    // Detailed profiling methods
    void start_section_timing();
    void end_section_timing(const std::string& section_name);

};

#endif