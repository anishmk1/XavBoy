// struct Reg {
//     char *name;
//     uint16_t val;
// }


class CPU {
public:

    // FIXME: move these into a separate register file struct?
    uint16_t AF;
    uint16_t BC;
    uint16_t DE;
    uint16_t HL;
    uint16_t SP;    // Stack Pointer
    uint16_t PC;    // Program Counter

    // Default constructor
    CPU () {
        // value = 0;
        // PC = 10;
    }
    // FIXME: add a destructor

    void print_regs() {
        printf("    |AF: 0x%0x    BC: 0x%0x    DE: 0x%0x    HL: 0x%0x    SP: 0x%0x    PC: 0x%0x|\n",
                            AF,         BC,           DE,          HL,          SP,          PC);
    }


    void execute(uint8_t cmd) {

        if (cmd == 0) {
            std::cout << "  detected: nop" << std::endl;
        } else if (match(cmd, "00xx0011")) {
            // INC R16
            printf("    detected: inc r16\n");
            uint16_t *reg_ptr;
            
            reg_ptr = get_r16(((cmd >> 4) & 0b11));
            (*reg_ptr)++;
        } else if (match(cmd, "00xx1011")) {
            // DEC R16
            printf("    detected: dec r16\n");
            uint16_t *reg_ptr;
            reg_ptr = get_r16(((cmd >> 4) & 0b11));
            (*reg_ptr)--;
        } else if (match(cmd, "00xx1001")) {
            // add hl, r16
            printf("    detected: add hl, r16\n");
            uint16_t *reg_ptr;
            reg_ptr = get_r16(((cmd >> 4) & 0b11));
            HL += (*reg_ptr);

        } 
        
        else {
            std::cout << "  default case" << std::endl;
        }

        print_regs();
    }


    // void print_PC() {
    //     std::cout << "Value of PC: " << PC << std::endl;
    // }
private:
    // Implement wildcard matching - cmd=1010 matches compare="10xx"
    bool match(uint8_t cmd, const char *compare) {
        int cmd_bit, compare_bit;
        for (int i = 0; i < 8; i++) {
            if (compare[i] == 'x') continue;
            compare_bit = (int)compare[i] - 48; // convert char to int with ASCII value of 0
            cmd_bit = (cmd >> (7-i)) & 1;   // get (7-i)-th bit of cmd

            if (cmd_bit != compare_bit) return false;
        }
        return true;
    }
    // Return address of the register (should remain static througout program runtime)
    uint16_t *get_r16(int index) {
        uint16_t *reg_ptr;
        switch (index) {
            case 0:
                reg_ptr = &BC; 
                break;
            case 1:
                reg_ptr = &DE;
                break;
            case 2:
                reg_ptr = &HL;
                break;
            case 3:
                reg_ptr = &SP;
                break;
            default:
                printf ("   Unexpected index to get_r16: %d\n", index);
        }
        // std::cout << "  Returning reg_ptr: " << reg_ptr << std::endl;
        return reg_ptr;
    }

};