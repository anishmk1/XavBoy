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

class Debug {
public:
    long free_clk;
    long num_steps_left;
    bool run;
    uint8_t tgt_instr;
    BreakpointInfo bp_info;

    bool disable_interrupts = false;

    Debug();
    void debugger_break(CPU &cpu);

};

#endif