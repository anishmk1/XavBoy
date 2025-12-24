#include "Joypad.h"
#include "main.h"

// Pure conditional logic => should reflect immediately in hardware on
// button input
// Runs on 1. writes to JOYP reg or 2. inputs
void Joypad::update_button_state() {

    // Retain upper nibble (ro_bits and select bits) and comb update lower nibble
    uint8_t joyp = mem->get(REG_JOYP);
    uint8_t new_joyp = (joyp & 0b00110000) | 0b11001111;

    // if select buttons then update the lower nibble with buttons bits
    if (((joyp >> JOYP_SEL_BUTTONS_BIT) & 0b1) == 0) {
        if (joy->buttons.START)      new_joyp &= ~(1 << JOYP_START_DOWN_BIT);
        if (joy->buttons.SELECT)     new_joyp &= ~(1 << JOYP_SELECT_UP_BIT);
        if (joy->buttons.B)          new_joyp &= ~(1 << JOYP_B_LEFT_BIT);
        if (joy->buttons.A)          new_joyp &= ~(1 << JOYP_A_RIGHT_BIT);
    }

    // if select dpad then OR update the lower nibble with dpad bits
    if (((joyp >> JOYP_SEL_DPAD_BIT) & 0b1) == 0) {
        if (joy->buttons.D_UP)       new_joyp &= ~(1 << JOYP_SELECT_UP_BIT);
        if (joy->buttons.D_DOWN)     new_joyp &= ~(1 << JOYP_START_DOWN_BIT);
        if (joy->buttons.D_LEFT)     new_joyp &= ~(1 << JOYP_B_LEFT_BIT);
        if (joy->buttons.D_RIGHT)    new_joyp &= ~(1 << JOYP_A_RIGHT_BIT);
    }

    mem->set(REG_JOYP, new_joyp, 1);
}