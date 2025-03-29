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


    void execute(uint8_t cmd) {
        uint16_t *reg_ptr;
        // switch (cmd) {
        //     case 0:     // nop
        //         std::cout << "..nop. Breaking" << std::endl;
        //         break;
        //     case 0b00000011:
        //         std::cout << "..matched binary lit. Breaking" << std::endl;
        //         break;
        // }
        if (cmd == 0) {
            std::cout << "  ..nop. Breaking" << std::endl;
        } else if ((cmd & 0b11) && ((cmd | 0b00110011) == 0b00110011)) {
            std::cout << "  detected inc r16" << std::endl;
            int reg_idx = (cmd & 0b00110000);
            reg_ptr = get_r16(reg_idx);
            (*reg_ptr)++;
            std::cout << "  reg_ptr index = " << reg_idx << "; value = " <<  static_cast<int>(*reg_ptr) << std::endl;
        } else {
            std::cout << "  default case" << std::endl;
        }
    }


    // void print_PC() {
    //     std::cout << "Value of PC: " << PC << std::endl;
    // }
private:
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
                std::cerr << "Unexpected index to get_r16" << std::endl;
        }
        // std::cout << "  Returning reg_ptr: " << reg_ptr << std::endl;
        return reg_ptr;
    }

};