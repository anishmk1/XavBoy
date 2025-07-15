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

constexpr uint8_t TIMER_BIT = 1 << 2;  // Bit 2


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

    // // MemoryMappedArray<int, 10, 0xFF00> mmio;
    // // std::array<uint8_t, 10> mmio;
    // std::unordered_map<uint16_t, uint8_t> mmio = {
    //     {DIV, 0x18}, {TIMA, 0x00},
    //     {TMA, 0x00}, {TAC, 0xF8},
    //     {IE, 0x00}, {IF, 0xE1}
    // };

// public:
    // bool IME = 0;                   // Interrupt Master Enable
    // bool IME_ff[2];
    // uint8_t *mmio;
    // std::vector<uint8_t> &mem;

    MMIO::MMIO (Memory *memory)
        : mem(memory->mem) {
        // this->mem = memory->mem;    // Will be overridden in Memory()
        memory->mmio = this;
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

    void MMIO::incr_timers(int free_clk) {
        
        if ((mem[TAC] >> 2) & 0b1) {   // Only increment TIMA if TAC Enable set
            print ("Inside if statement\n");
            int tac_select = (mem[TAC]) & 0b11;
            int incr_count = 0;
            switch (tac_select) {
                case 0:
                    incr_count = 256 * 4;   // 256 M-cycles (* 4 T-cycles)
                    break;
                case 1:
                    incr_count = 4 * 4;     // 4 M-cycles (* 4 T-cycles)
                    break;
                case 2:
                    incr_count = 16 * 4;
                    break;
                case 3:
                    incr_count = 64 * 4;
                    break;
                default:
                    printx ("incr timers - hit default case. Fail\n");
                    std::exit(EXIT_FAILURE);
                    break;
            }

            if (free_clk % incr_count == 0) {   // Increment TIMA
                mem[TIMA]++;
                if (mem[TIMA] == 0) {
                    mem[TIMA] = mem[TMA]; // When the value overflows (exceeds $FF) it is reset to the value specified in TMA (FF06) and an interrupt is requested
                    mem[IF] |= TIMER_BIT;

                }
            }
        }

        mem[DIV]++;

        IME = IME_ff[1];
        IME_ff[1] = IME_ff[0];
    }

    bool MMIO::check_interrupts(uint16_t &intr_handler_addr) {
        // if (dbg->free_clk == 151382) dbg->disable_interrupts = true;    // FIXME: get rid of this line
        if (dbg->disable_interrupts) return false;

        if (IME) {
            // FIXME: change this to do [IE] & [IF] and process the result based on interrupt priotity
            // Use the TIMER_BIT etc to parse the result
            // FIXME: ONly checking timer interrupt for now
            if ((mem[IE] & TIMER_BIT) && (mem[IF] & TIMER_BIT)) {
                // print ("mem[IE]=0x%0x; mem[IF]=0x%0x\n", mem[IE], mem[IF]);
                // print ("mem[IE] & TIMER_BIT =0x%0x; mem[IF] & TIMER_BIT =0x%0x\n", mem[IE] & TIMER_BIT, mem[IF] & TIMER_BIT);
                print ("Detected TIMER Interrupt\n");
                intr_handler_addr = 0x50;
                mem[IF] &= ~TIMER_BIT;  // Clear IF Timer Bit

                IME = 0;    // Reset IME
                IME_ff[0] = 0;
                IME_ff[1] = 0;

                // dbg->breakpoint = true;
                dbg->bp_info.breakpoint = true;
                dbg->bp_info.msg = "Hit TIMER Interrupt";
                return true;
            }
        }

        return false;
    }



// };