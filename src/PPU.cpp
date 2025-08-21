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

#include "PPU.h"

void FIFO::push(Pixel num) {

}

Pixel FIFO::pop() {

}




uint8_t PPU::reg_access(int addr, bool read_nwr, uint8_t val) {
    if (addr == 0xFF40) {       // LCD Control (LCDC)
        return 0;
    }
}