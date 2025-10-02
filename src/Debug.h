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
    int num_main_loops;
    std::chrono::high_resolution_clock::time_point curr_frame_draw_time;
} PerfMetrics;

class Debug {
public:
    Color last_framebuffer[144][160];

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

    bool disable_interrupts = false;

    std::chrono::high_resolution_clock::time_point last_frame_time;

    Debug();
    void set_breakpoint(std::string msg);
    void debugger_break(CPU &cpu);
    void log_frame_timing();
    void log_duration(std::chrono::time_point<std::chrono::high_resolution_clock> before, std::chrono::time_point<std::chrono::high_resolution_clock> after, std::string msg);

};

#endif