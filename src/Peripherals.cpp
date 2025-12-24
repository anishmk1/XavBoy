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

    const uint16_t JOYPAD = 0xff00;    // Joypad register -- 0xFF04
    const uint16_t DIV = 0xff04;    // Divider register -- 0xFF04
    const uint16_t TIMA = 0xff05;   // Timer counter    -- 0xFF05
    const uint16_t TMA = 0xff06;    // Timer modulo     -- 0xFF06
    const uint16_t TAC = 0xff07;    // Timer control    -- 0xFF07

    const uint16_t IE = 0xffFF;     // Interrupt enable -- 0xFFFF
    const uint16_t IF = 0xff0F;     // Interrupt flag   -- 0xFF0F

    MMIO::MMIO () {
        system_counter = 0;
        reset_ime();
    }

    uint8_t MMIO::access(int addr, bool read_nwr, uint8_t val, bool backdoor) {
        if (read_nwr) {
            // ----------------- //
            //    READ ACCESS    //
            // ----------------- //
            uint8_t rd_val = mem->memory[addr];

            // Handle read-only bits for various registers
            if (addr == JOYPAD) {
                rd_val |= 0b11000000;  // JOYPAD[7:6] always read as 1
            } else if (addr == IF) {
                rd_val |= 0b11100000;  // IF[7:5] always read as 1
            } else if (addr == TAC) {
                rd_val |= 0b11111000;  // TAC[7:3] always read as 1
            } else if (addr == 0xFF02) {  // SC - Serial Control
                rd_val |= 0b01111110;  // SC[6:1] always read as 1 (DMG mode)
            }

            return rd_val;
        } else {
            // ----------------- //
            //    WRITE ACCESS   //
            // ----------------- //
            if (addr == DIV) {
                val = 0;  // Writing any value to DIV resets it to 0
            } else if (addr == JOYPAD) {

                if (!backdoor) {
                    uint8_t ro_bits = mem->memory[JOYPAD];
                    ro_bits &= 0b00001111;      // get current lower read only nibble value
                    val |= ro_bits;             // preserve lower nibble

                    joy->update_button_state();
                }
            } 

            mem->memory[addr] = val;

            return 0;
        }
    }

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
        //     debug_file << "incr_timers; mcycles=" << mcycles << "; clk num: " << dbg->instr_cnt << std::endl;
        // } else {
        //     debug_file << "TEST incr_timers with mcycles == 0" << std::endl;
        // }
        

        for (int i = 0; i < mcycles; i++) {
            system_counter++;
            if (system_counter == (1 << 14)) {
                // debug_file << "system counter overflowing; clk num: " << dbg->instr_cnt << std::endl;
                system_counter = 0;       // system_counter should actually only be 14 bits. This models that
            }


            if ((mem->memory[TAC] >> 2) & 0b1) {   // Only increment TIMA if TAC Enable set
                // print ("Inside if statement\n");
                // debug_file << "TAC enable is set; clk num: " << dbg->instr_cnt << std::endl;
                int tac_select = (mem->memory[TAC]) & 0b11;
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
                if (disable_prints == false) {
                    DBG("   TEST Incremented TIMA; Instr_cnt: " << dbg->instr_cnt << " mcycle_cnt: " << dbg->mcycle_cnt << std::endl);
                }
                // debug_file.flush();

                if (system_counter % incr_count == 0) {   // Increment TIMA
                // if (instr_cnt % incr_count == 0) {   // Increment TIMA        // GET INSTR_CNT FROM MAIN.CPP
                    // debug_file << "Incremented TIMA; clk num: " << dbg->instr_cnt << std::endl;
                    mem->memory[TIMA]++;
                    if (mem->memory[TIMA] == 0) {
                        mem->memory[TIMA] = mem->memory[TMA]; // When the value overflows (exceeds $FF) it is reset to the value specified in TMA (FF06) and an interrupt is requested
                        mem->memory[IF] |= TIMER_BIT;

                    }
                }
            } else {
                // debug_file << "TAC select: 0x" << std::hex << mem->memory[TAC] << std::endl;
                // printx ("TAC select : 0x%0x\n", mem->memory[TAC]);
            }


            // DIV Register
            int old_div = mem->memory[DIV];
            mem->memory[DIV] = ((system_counter >> 6) & 0xff);      // DIV is just the "visible part" (bits [13:6]) of system counter
            // Below for Game Boy Color Only
            // bool double_speed_mode = false;
            // int falling_edge_bit = (double_speed_mode) ? (6+4) : (6+5);
            if (((mem->memory[DIV] >> (4+6)) & 0b1) == 0 && 
                (( old_div >> (4+6)) & 0b1) == 1) {     // Detected bit 4 falling edge
                    // 
                    // DIV-APU event
                    // 
            }



            dbg->mcycle_cnt++;
        }

        // mem->memory[DIV]++;

        // FIXME:: Worry about IME flip flop modeling if I want cycle accurate emulation
        IME = IME_ff[1];
        IME_ff[1] = IME_ff[0];
    }

    bool MMIO::exit_halt_mode() {
        if (mem->memory[IF] & mem->memory[IE]) return true;
        else return false;
    }


    bool MMIO::check_interrupts(uint16_t &intr_handler_addr, bool cpu_halted) {
        // debug_file << "check_interrupts: Entered function @ clk num = " << dbg->instr_cnt << std::endl;
        if (dbg->disable_interrupts) {

            return false;
        }

        bool intr_triggered = false;
        uint8_t ie_and_if = mem->memory[IF] & mem->memory[IE];

        // print("check_interrupts: IF=0x%02x, IE=0x%02x, IF&IE=0x%02x, IME=%d, cpu_halted=%d\n",
        //       mem->memory[IF], mem->memory[IE], ie_and_if, IME, cpu_halted);

        if (ie_and_if) {
            // debug_file << "Inside ie_and_if" << std::endl; 
            // WAKE CPU
            if (IME) {
                // setup interrupt handler for execution
                if (ie_and_if & VBLANK_BIT) {
                    reset_ime();
                    intr_handler_addr = 0x40;
                    mem->memory[IF] &= ~VBLANK_BIT;  // Clear IF VBLANK Bit
                    intr_triggered = true;
                    DBG("   Triggering VBLANK interrupt @ clk num = " << dbg->instr_cnt << std::endl); 
                } else if (ie_and_if & LCD_BIT) {
                    reset_ime();
                    intr_handler_addr = 0x48;
                    mem->memory[IF] &= ~LCD_BIT;  // Clear IF LCD/STAT Bit
                    intr_triggered = true;
                    DBG("   Triggering STAT interrupt @ clk num = " << dbg->instr_cnt << std::endl); 
                } else if (ie_and_if & TIMER_BIT) {
                    reset_ime();
                    intr_handler_addr = 0x50;
                    mem->memory[IF] &= ~TIMER_BIT;  // Clear IF Timer Bit
                    intr_triggered = true;
                    DBG("   Triggering TIMER interrupt @ clk num = " << dbg->instr_cnt << std::endl); 
                } else if (ie_and_if & SERIAL_BIT) {
                    reset_ime();
                    intr_handler_addr = 0x58;
                    mem->memory[IF] &= ~SERIAL_BIT;  // Clear IF Serial Bit
                    intr_triggered = true;
                    DBG("   Triggering SERIAL interrupt @ clk num = " << dbg->instr_cnt << std::endl); 
                } else if (ie_and_if & JOYPAD_BIT) {
                    reset_ime();
                    intr_handler_addr = 0x60;
                    mem->memory[IF] &= ~JOYPAD_BIT;  // Clear IF Joypad Bit
                    intr_triggered = true;
                    DBG("   Triggering JOYPAD interrupt @ clk num = " << dbg->instr_cnt << std::endl); 
                } else {
                }
            } else {
                if (cpu_halted) {
                    // can do further processing of the "2 distinct cases"
                }
                // else (i.e IME not set but regular execution of CPU, do nothing)
            }
        } else {
        }

        if (intr_triggered) {
            return true;
        }

        return false;
    }
