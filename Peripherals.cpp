// union InterruptFlagRegister {
//     uint8_t value;

//     struct {
//         unsigned vblank  : 1;  // Bit 0
//         unsigned lcd     : 1;  // Bit 1
//         unsigned timer   : 1;  // Bit 2
//         unsigned serial  : 1;  // Bit 3
//         unsigned joypad  : 1;  // Bit 4
//         unsigned rsvd    : 3;  // Bits 5–7 (typically unused/reserved)
//     };

//     void clear() {
//         value = 0;
//     }
// };

constexpr uint8_t TIMER_BIT = 1 << 2;  // Bit 2



/**
 * Implements Memory Mapped I/O Peripheral Regisetrs from 0xFF00 -> 0xFF7F
 */
class MMIO {

    const uint16_t DIV = 0xFF04;    // Divider register
    const uint16_t TIMA = 0xFF05;   // Timer counter
    const uint16_t TMA = 0xFF06;    // Timer modulo
    const uint16_t TAC = 0xFF07;    // Timer control

    const uint16_t IE = 0xFFFF;     // Interrupt enable
    const uint16_t IF = 0xFF0F;     // Interrupt flag

    bool IME = 0;                   // Interrupt Master Enable
    bool IME_ff[2];

    // MemoryMappedArray<int, 10, 0xFF00> mmio;
    // std::array<uint8_t, 10> mmio;
    std::unordered_map<uint16_t, uint8_t> mmio = {
        {DIV, 0x18}, {TIMA, 0x00},
        {TMA, 0x00}, {TAC, 0xF8},
        {IE, 0x00}, {IF, 0xE1}
    };

public:

    bool write(int addr, uint8_t val) {
        if (mmio.find(addr) != mmio.end()) {
            if (addr == DIV) {
                mmio[DIV] = 0;  // Writing any value to DIV resets it to 0
            } else {
                mmio[addr] = val;
            }
            return true;
        } else {
            return false;
        }
    }

    uint8_t read(int addr) {
        return mmio.at(addr);
    }

    void set_ime() {
        this->IME_ff[0] = 1;
    }

    void clear_ime() {
        this->IME = 0;
    }

    void incr_timers(int free_clk) {
        if ((mmio[TAC] >> 2) & 0b1) {   // Only increment TIMA if TAC Enable set
            int tac_select = (mmio[TAC]) & 0b11;
            int incr_count = 0;
            switch (tac_select) {
                case 0:
                    incr_count = 256'000'000;
                    break;
                case 1:
                    incr_count = 4'000'000;
                    break;
                case 2:
                    incr_count = 16'000'000;
                    break;
                case 3:
                    incr_count = 64'000'000;
                    break;
                default:
                    printx ("incr timers - hit default case. Fail\n");
                    std::exit(EXIT_FAILURE);
                    break;
            }

            if (free_clk % incr_count == 0) {   // Increment TIMA
                mmio[TIMA]++;
                if (mmio[TIMA] == 0) {
                    mmio[TIMA] = mmio[TMA]; // When the value overflows (exceeds $FF) it is reset to the value specified in TMA (FF06) and an interrupt is requested
                    // InterruptFlagRegister *if_reg = reinterpret_cast<InterruptFlagRegister*>(&mmio[IF]);
                    // if_reg->timer = 1;
                    mmio[IF] |= TIMER_BIT;

                }
            }
        }

        mmio[DIV]++;

        IME = IME_ff[1];
        IME_ff[1] = IME_ff[0];
    }

    bool check_interrupts(uint16_t &intr_handler_addr) {
        if (IME) {
            // FIXME: ONly checking timer interrupt for now
            if ((mmio[IE] & TIMER_BIT) && (mmio[IF] & TIMER_BIT)) {
                intr_handler_addr = 0x50;
                return true;
            }
        }

        return false;
    }



};