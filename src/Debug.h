// Debug.h
#ifndef DEBUG_H
#define DEBUG_H

#include <string>

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
    // long free_clk;

    void* texture_ptr_val;
    long instr_cnt;
    long mcycle_cnt;
    long num_steps_left;
    bool run;
    uint8_t tgt_instr;
    // uint8_t tgt_pc;
    TargetPC tgt_pc;
    BreakpointInfo bp_info;

    bool disable_interrupts = false;

    Debug();
    void debugger_break(CPU &cpu);

};

#endif