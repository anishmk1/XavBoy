// Debug.h
#ifndef DEBUG_H
#define DEBUG_H

#include <string>
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

    bool disable_interrupts = false;

    Debug();
    void set_breakpoint(std::string msg);
    void debugger_break(CPU &cpu);

};

#endif