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
#include "Peripherals.h"
#include "Memory.h"


const int MEMORY_SIZE = 65536; // 2^16 locations for 16-bit address bus

/*
*   The Game Boy has a 16-bit address bus, which is used to address ROM, RAM, and I/O.
*   Byte addressable memory
*   16-bit access bus -> 2^16 = 65536 addresses
*   So 2^16 * 1 Byte = 2^6 KB memory = 64 KB Memory
*/
// class Memory {

//     size_t romSize;

// public:
    // std::vector<uint8_t> memory;
    // MMIO *mmio;

    Memory::Memory () {
        // this->mmio = mmio;
        // mmio->mmio = &mem[0xFF00];      // MMIO region starts here
        // mmio->mem = mem;
        // mmio->mmio = &mem[0xFF00];
        // mmio->init_mmio(mem[0xFF00]);
        // init all mem locations to 0
        memory = std::vector<uint8_t>(MEMORY_SIZE, 0);

        // Load Boot ROM -- Note these values change based on GameBoy version.
        memory[0xFF00] = 0xCF; // P1	
        memory[0xFF01] = 0x00; // SB	
        memory[0xFF02] = 0x7E; // SC	
        memory[0xFF04] = 0x18; // DIV
        memory[0xFF05] = 0x00; // TIMA
        memory[0xFF06] = 0x00; // TMA
        memory[0xFF07] = 0xF8; // TAC
        memory[0xFF0F] = 0xE1; // IF
        memory[0xFF10] = 0x80; // NR10
        memory[0xFF11] = 0xBF; // NR11
        memory[0xFF12] = 0xF3; // NR12
        memory[0xFF13] = 0xFF; // NR13
        memory[0xFF14] = 0xBF; // NR14
        memory[0xFF16] = 0x3F; // NR21
        memory[0xFF17] = 0x00; // NR22
        memory[0xFF18] = 0xFF; // NR23
        memory[0xFF19] = 0xBF; // NR24
        memory[0xFF1A] = 0x7F; // NR30
        memory[0xFF1B] = 0xFF; // NR31
        memory[0xFF1C] = 0x9F; // NR32
        memory[0xFF1D] = 0xFF; // NR33
        memory[0xFF1E] = 0xBF; // NR34
        memory[0xFF20] = 0xFF; // NR41
        memory[0xFF21] = 0x00; // NR42
        memory[0xFF22] = 0x00; // NR43
        memory[0xFF23] = 0xBF; // NR44
        memory[0xFF24] = 0x77; // NR50
        memory[0xFF25] = 0xF3; // NR51
        memory[0xFF26] = 0xF1; // NR52
        memory[0xFF40] = 0x91; // LCDC
        memory[0xFF41] = 0x81; // STAT
        memory[0xFF42] = 0x00; // SCY
        memory[0xFF43] = 0x00; // SCX
        memory[0xFF44] = 0x91; // LY
        memory[0xFF45] = 0x00; // LYC
        memory[0xFF46] = 0xFF; // DMA
        memory[0xFF47] = 0xFC; // BGP
        memory[0xFF48] = 0x00; // OBP0
        memory[0xFF49] = 0x00; // OBP1
        memory[0xFF4A] = 0x00; // WY
        memory[0xFF4B] = 0x00; // WX
        memory[0xFF4D] = 0x7E; // KEY1
        memory[0xFF4F] = 0xFE; // VBK
        memory[0xFF51] = 0xFF; // HDMA1
        memory[0xFF52] = 0xFF; // HDMA2
        memory[0xFF53] = 0xFF; // HDMA3
        memory[0xFF54] = 0xFF; // HDMA4
        memory[0xFF55] = 0xFF; // HDMA5
        memory[0xFF56] = 0x3E; // RP
        memory[0xFF68] = 0x00; // BCPS
        memory[0xFF69] = 0x00; // BCPD
        memory[0xFF6A] = 0x00; // OCPS
        memory[0xFF6B] = 0x00; // OCPD
        memory[0xFF70] = 0xF8; // SVBK
        memory[0xFFFF] = 0x00; // IE
    }

    // FIXME:: when LCD is ON and PPU is actively drawing (aka not HBLANK or VBLANK) cpu should not be able to access VRAM or OAM. And vice versa (i think)
    uint8_t Memory::access_memory_map(int addr, uint8_t val, bool read_nwr) {
        if (addr < 0) {
            std::cerr << "Memory access with out of bounds address: " << addr << std::endl;
            return 1;
        } else if (addr <= 0x3FFF) {          // 16 KiB ROM bank 00
            if (read_nwr) return memory[addr];
            else return 1;  // printx ("Attempted write-access to ROM addr: 0x%0x\n", addr);
        } else if (addr <= 0x7FFF) {          // 16 KiB ROM bank 01–NN
            if (read_nwr) return memory[addr];
            else return 1;  // printx ("Attempted write-access to ROM addr: 0x%0x\n", addr);
        } else if (addr <= 0x9FFF) {          // 8 KiB Video RAM (VRAM) 
            if (read_nwr) return memory[addr];
            else {
                memory[addr] = val;
                return 0;
            }
        } else if (addr <= 0xBFFF) {          // 8 KiB External RAM
            if (read_nwr) return memory[addr];
            else {
                memory[addr] = val;
                return 0;
            }
        } else if (addr <= 0xDFFF) {          // 4 KiB Work RAM (WRAM)
            if (read_nwr) return memory[addr];
            else {
                memory[addr] = val;
                return 0;
            }
        } else if (addr <= 0xFDFF) {          // Echo RAM
            addr = 0xC000 + (addr - 0xE000);    // Remap addresses (E000 -> FDFF) to mirror (C000 -> DDFF)

            if (read_nwr) return memory[addr];
            else {
                memory[addr] = val;
                return 0;
            }
        } else if (addr <= 0xFE9F) {          // Object attribute memory (OAM)
            // Consists of 40 entries - each 4 bytes
            if (read_nwr) return memory[addr];
            else {
                memory[addr] = val;
                return 0;
            }
        } else if (addr <= 0xFEFF) {          // Not Usable
            //Nintendo says use of this area is prohibited.
            // std::cerr << "Memory access to \"Not Usable\" addr space: 0x" << std::hex << addr << std::endl;
            // return 1;
            if (read_nwr) return memory[addr];
            else {
                memory[addr] = val;
                return 0;
            }
        } else if (addr <= 0xFF7F) {          // I/O Registers
            // FIXME: Maybe change this so it's just all done in Memory block? Not sure the clean way to do this...
            if (addr <= 0xFF3F) {
                return mmio->access(addr, read_nwr, val);
            } else if (addr <= 0xFF48) {            // LCD Registers
                return ppu->reg_access(addr, read_nwr, val);
            } else {
                return mmio->access(addr, read_nwr, val);
            }
            // return mmio->access(addr, read_nwr, val);

        } else if (addr <= 0xFFFE) {          // High RAM (HRAM)
            if (read_nwr) return memory[addr];
            else {
                memory[addr] = val;
                return 0;
            }
        } else if (addr == 0xFFFF) {          // Interrupt Enable register (IE) 
            if (read_nwr) return memory[addr];
            else {
                memory[addr] = val;
                return 0;
            }
        } else {
            std::cerr << "Memory access with out of bounds address: 0x" << std::hex << addr << std::endl;
            return -1;
        }
    }

    int Memory::set(int addr, uint8_t val) {
        // if (addr == 0xff0f) {
        //     dbg->bp_info.breakpoint = true;
        //     dbg->bp_info.msg = "Setting memory addr 0xff0f";
        // }
        // if (addr == 0xff07) {
        //     dbg->bp_info.breakpoint = true;
        //     dbg->bp_info.msg = "Setting memory addr 0xff07";
        // }
        // if (addr == 0x4244) {
        //     dbg->bp_info.breakpoint = true;
        //     dbg->bp_info.msg = "Setting memory addr 0x4244";
        // }


        return access_memory_map(addr, val, 0);
    }

    uint8_t Memory::get(int addr) {
        return access_memory_map(addr, 0xaf, 1);
    }

    void Memory::load_rom(uint8_t *rom_ptr, size_t file_size) {
        int rom_start_addr = (LOAD_BOOT_ROM) ? 0 : 0x100;
        print ("Loading ROM into Memory Address 0x%0x....\n", rom_start_addr);
        print ("   file_size: %lu\n", file_size);
        for (size_t i = 0; i < file_size; i++) {
            memory[i + rom_start_addr] = rom_ptr[i];
            // if (i + rom_start_addr == 0x4244) {
            //     printx ("mem[0x%0x] <= 0x%0x\n", i + rom_start_addr, mem[i + rom_start_addr]);
            // }
        }
        romSize = file_size;
        print ("Successfully loaded ROM!\n\n");

        // dump_mem(300);
    }

    void Memory::dump_mem(int around, int range) {
        int start = around - (range / 2);
        int end = around + (range / 2);
        for (int i = start; i < end; i++) {
            if (i % 16 == 0) printx ("\n");
            printx ("mem[0x%0x]=0x%0x;  ", i, memory[i]);
        }
        printx ("\n\n");
    }

    // void dump_ROM() {
    //     printf("")
    // }
// };