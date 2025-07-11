// Debug.h
#ifndef DEBUG_H
#define DEBUG_H

class CPU;

class Debug {
public:
    long free_clk;
    long num_steps_left;
    bool run;
    uint8_t tgt_instr;
    bool breakpoint = false;
    bool disable_interrupts = false;

    Debug();
    void debugger_break(CPU &cpu);

};

#endif