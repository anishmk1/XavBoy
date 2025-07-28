#include <iostream>
#include <fstream>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <array>
#include <string>

#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <cstring> // For strcmp
#include <cassert>
#include <set>

#include "main.h"
#include "Memory.h"
#include "Peripherals.h"

constexpr uint8_t VBLANK_BIT    = 1 << 0;  // Bit 0
constexpr uint8_t LCD_BIT       = 1 << 1;  // Bit 1  (aka STAT Interrupt)
constexpr uint8_t TIMER_BIT     = 1 << 2;  // Bit 2
constexpr uint8_t SERIAL_BIT    = 1 << 3;  // Bit 3
constexpr uint8_t JOYPAD_BIT    = 1 << 4;  // Bit 4


// class Memory;

/**
 * Implements Memory Mapped I/O Peripheral Regisetrs from 0xFF00 -> 0xFF7F
 */
// class MMIO {

    const uint16_t DIV = 0xff04;    // Divider register -- 0xFF04
    const uint16_t TIMA = 0xff05;   // Timer counter    -- 0xFF05
    const uint16_t TMA = 0xff06;    // Timer modulo     -- 0xFF06
    const uint16_t TAC = 0xff07;    // Timer control    -- 0xFF07

    const uint16_t IE = 0xffFF;     // Interrupt enable -- 0xFFFF
    const uint16_t IF = 0xff0F;     // Interrupt flag   -- 0xFF0F

    MMIO::MMIO (Memory *memory)
        : mem(memory->mem) {
        // this->mem = memory->mem;    // Will be overridden in Memory()
        memory->mmio = this;
        system_counter = 0;
    }

    // void init_mmio(std::vector<uint8_t>& mem){
    //     print ("init_mmio done\n");
    //     this->mmio = &mem[0xFF00];
    // }

    // bool write(int addr, uint8_t val) {
    //     if (mmio.find(addr) != mmio.end()) {
    //         if (addr == DIV) {
    //             mmio[DIV] = 0;  // Writing any value to DIV resets it to 0
    //         } else {
    //             mmio[addr] = val;
    //         }
    //         return true;
    //     } else {
    //         return false;
    //     }
    // }

    uint8_t MMIO::access(int addr, bool read_nwr, uint8_t val) {
        if (read_nwr) {
            // ----------------- //
            //    READ ACCESS    //
            // ----------------- //
            if (addr == 0xff44) {
                return 0x90;    // LY Register - 0x90 (144) is just before VBlank. This tricks games into thinking the screen is ready to draw. Remove once ppu/Vblank is implemented.
            } else {
                return mem[addr];
            }
            // return mem[addr - 0xff00];
        } else {
            // ----------------- //
            //    WRITE ACCESS   //
            // ----------------- //
            if (addr == DIV) {
                mem[addr] = 0;  // Writing any value to DIV resets it to 0
            } else {
                mem[addr] = val;
            }
            // mem[addr - 0xff00] = val;
            return 0;
        }
    }

    // uint8_t read(int addr) {
    //     return mmio.at(addr);
    // }

    void MMIO::set_ime() {
        this->IME_ff[0] = 1;
    }

    void MMIO::clear_ime() {
        this->IME = 0;
    }

    void MMIO::reset_ime() {
        IME = 0;
        IME_ff[0] = 0;
        IME_ff[1] = 0;    
    }

    void MMIO::incr_timers(int mcycles) {

        // system_counter += mcycles;       // This should happen once every M-cycle
        // if (mcycles != 0 ) {
        //     debug_file << "incr_timers; mcycles=" << mcycles << "; clk num: " << dbg->free_clk << std::endl;
        // } else {
        //     debug_file << "TEST incr_timers with mcycles == 0" << std::endl;
        // }
        

        for (int i = 0; i < mcycles; i++) {
            system_counter++;
            if (system_counter == (1 << 14)) {
                // debug_file << "system counter overflowing; clk num: " << dbg->free_clk << std::endl;
                system_counter = 0;       // system_counter should actually only be 14 bits. This models that
            }


            if ((mem[TAC] >> 2) & 0b1) {   // Only increment TIMA if TAC Enable set
                print ("Inside if statement\n");
                debug_file << "TAC enable is set; clk num: " << dbg->free_clk << std::endl;
                int tac_select = (mem[TAC]) & 0b11;
                int incr_count = 0;
                switch (tac_select) {
                    case 0:
                        incr_count = 256;   // 256 M-cycles (* 4 T-cycles)
                        // incr_count = 256 * 4;   // 256 M-cycles (* 4 T-cycles)
                        break;
                    case 1:
                        incr_count = 4;     // 4 M-cycles (* 4 T-cycles)
                        // incr_count = 4 * 4;     // 4 M-cycles (* 4 T-cycles)
                        break;
                    case 2:
                        incr_count = 16;
                        break;
                    case 3:
                        incr_count = 64;
                        break;
                    default:
                        printx ("incr timers - hit default case. Fail\n");
                        std::exit(EXIT_FAILURE);
                        break;
                }
                // debug_file << "TEST Incremented TIMA; clk num: " << dbg->free_clk << std::endl;
                // debug_file.flush();

                if (system_counter % incr_count == 0) {   // Increment TIMA
                // if (free_clk % incr_count == 0) {   // Increment TIMA        // GET FREE CLK FROM MAIN.CPP
                    debug_file << "Incremented TIMA; clk num: " << dbg->free_clk << std::endl;
                    mem[TIMA]++;
                    if (mem[TIMA] == 0) {
                        mem[TIMA] = mem[TMA]; // When the value overflows (exceeds $FF) it is reset to the value specified in TMA (FF06) and an interrupt is requested
                        mem[IF] |= TIMER_BIT;

                    }
                }
            } else {
                // debug_file << "TAC select: 0x" << std::hex << mem[TAC] << std::endl;
                // printx ("TAC select : 0x%0x\n", mem[TAC]);
            }


            // DIV Register
            int old_div = mem[DIV];
            mem[DIV] = ((system_counter >> 6) & 0xff);      // DIV is just the "visible part" (bits [13:6]) of system counter
            // Below for Game Boy Color Only
            // bool double_speed_mode = false;
            // int falling_edge_bit = (double_speed_mode) ? (6+4) : (6+5);
            if (((mem[DIV] >> (4+6)) & 0b1) == 0 && 
                (( old_div >> (4+6)) & 0b1) == 1) {     // Detected bit 4 falling edge
                    // 
                    // DIV-APU event
                    // 
            }



        }

        // mem[DIV]++;

        // FIXME:: Worry about IME flip flop modeling if I want cycle accurate emulation
        IME = IME_ff[1];
        IME_ff[1] = IME_ff[0];
    }

    bool MMIO::exit_halt_mode() {
        if (mem[IF] & mem[IE]) return true;
        else return false;
    }


    bool MMIO::check_interrupts(uint16_t &intr_handler_addr, bool cpu_halted) {
        // if (dbg->free_clk == 151382) dbg->disable_interrupts = true;
        if (dbg->disable_interrupts) return false;
        // return false;
        bool intr_triggered = false;

        uint8_t ie_and_if = mem[IF] & mem[IE];
        if (ie_and_if) {
            // WAKE CPU
            if (IME) {
                // setup interrupt handler for execution
                if (ie_and_if & VBLANK_BIT) {
                    reset_ime();
                    intr_handler_addr = 0x40;
                    mem[IF] &= ~VBLANK_BIT;  // Clear IF VBLANK Bit
                    intr_triggered = true;
                } else if (ie_and_if & LCD_BIT) {
                    reset_ime();
                    intr_handler_addr = 0x48;
                    mem[IF] &= ~LCD_BIT;  // Clear IF LCD/STAT Bit
                    intr_triggered = true;
                } else if (ie_and_if & TIMER_BIT) {
                    // dbg->bp_info.breakpoint = true;
                    // dbg->bp_info.msg = "Hit TIMER Interrupt";
                    reset_ime();
                    intr_handler_addr = 0x50;
                    mem[IF] &= ~TIMER_BIT;  // Clear IF Timer Bit
                    intr_triggered = true;

                } else if (ie_and_if & SERIAL_BIT) {
                    reset_ime();
                    intr_handler_addr = 0x58;
                    mem[IF] &= ~SERIAL_BIT;  // Clear IF Serial Bit
                    intr_triggered = true;

                } else if (ie_and_if & JOYPAD_BIT) {
                    reset_ime();
                    intr_handler_addr = 0x60;
                    mem[IF] &= ~JOYPAD_BIT;  // Clear IF Joypad Bit
                    intr_triggered = true;
                }

                // TODO: maybe add intr handler addrs to a dictionary and replace this
                // code with a loop?
                // for (int i = 0; i < 4; i++) {
                //     if (ie_and_if & (1 << i)) { 

                //         break;
                //     }
                // }

            } else {
                if (cpu_halted) {
                    // can do further processing of the "2 distinct cases"
                }
                // else (i.e IME not set but regular execution of CPU, do nothing)
            }
        }

        if (intr_triggered) return false;

        return false;
    }
