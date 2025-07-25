// Peripherals.h
#ifndef PERIPHERALS_H
#define PERIPHERALS_H

#include <vector>

class Memory;

class MMIO {

    uint16_t system_counter;
    
public:
    bool IME = 0;                   // Interrupt Master Enable
    bool IME_ff[2];
    // uint8_t *mmio;
    std::vector<uint8_t> &mem;

    void set_ime();
    void clear_ime();
    void incr_timers(int mcycles);
    bool check_interrupts(uint16_t &intr_handler_addr);
    uint8_t access(int addr, bool read_nwr, uint8_t val=0);

    MMIO(Memory *memory);

};

#endif // PERIPHERALS_H