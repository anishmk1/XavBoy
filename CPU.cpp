// struct Reg {
//     char *name;
//     uint16_t val;
// }
// #define USE_R8_INDEX true;
const bool USE_R8_INDEX = true;


class CPU {
public:

    // FIXME: MAKE THIS AN ARRAY OF REGS. EACH ONE IS A STRUCT/CLASS WITH A NAME AND A VALUE
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
        printf("    |AF: 0x%0x    BC: 0x%0x    DE: 0x%0x    HL: 0x%0x    SP: 0x%0x    PC: 0x%0x|\n\n",
                            AF,         BC,           DE,          HL,          SP,          PC);
    }


    void execute(Memory *mem) {

        uint16_t *reg_ptr;
        uint8_t cmd = mem->get(PC);

        ///////////////////////
        //      BLOCK 0     //
        /////////////////////
        if (cmd == 0) {
            std::cout << "  detected: nop" << std::endl;
        } else if (match(cmd, "00xx0001")) {                // ld r16, imm16
            printf("    detected: ld r16, imm16\n");
            // next 2 bytes are the immediate
            // FIXME: Does PC need to be incremented by 2 as well? Confirm
            uint16_t imm16 = (mem->get(PC+1) << 8) + mem->get(PC+2);
            reg_ptr = get_r16(((cmd >> 4) & 0b11));
            (*reg_ptr) = imm16;
            PC += 2;
        } else if (match(cmd, "00xx0010")) {                // ld [r16mem], a
            printf("    detected: ld dest[r16mem], a\n");
            reg_ptr = get_r16(((cmd >> 4) & 0b11));
            mem->set((*reg_ptr), (AF >> 8));

            assert (mem->get((*reg_ptr)) == ((*(get_r16(7, USE_R8_INDEX))) >> 8));  // a == mem[r16]
        } else if (match(cmd, "00xx1010")) {                // ld a, [r16mem]	
            printf("    detected: ld a, source[r16mem]\n");
            // FIXME: Create decode functions. Below only works because firts 2 bits are 00
            reg_ptr = get_r16(((cmd >> 4) & 0b11));
            AF &= 0x00ff;       // zero out A and dont touch F
            AF |= ((mem->get((*reg_ptr))) << 8);
            
            if (verbose) printf("    log: loaded (from mem) 0x%0x ---> A (0x%0x)\n", *reg_ptr, (AF >> 8));
            assert (mem->get((*reg_ptr)) == ((*(get_r16(7, USE_R8_INDEX))) >> 8));  // a == mem[r16]
        } else if (match(cmd, "00001000")) {                // ld [imm16], sp
            printf("    detected ld [imm16], sp\n");
            // next 2 bytes are the immediate
            uint16_t imm16 = (mem->get(PC+1) << 8) + mem->get(PC+2);
            SP = imm16;
            PC += 2;

        } else if (match(cmd, "00xx0011")) {                // inc r16
            printf("    detected: inc r16\n");
            
            reg_ptr = get_r16(((cmd >> 4) & 0b11));
            (*reg_ptr)++;
        } else if (match(cmd, "00xx1011")) {                // dec r16
            printf("    detected: dec r16\n");
            reg_ptr = get_r16(((cmd >> 4) & 0b11));
            (*reg_ptr)--;
        } else if (match(cmd, "00xx1001")) {                // add hl, r16
            printf("    detected: add hl, r16\n");
            reg_ptr = get_r16(((cmd >> 4) & 0b11));
            HL += (*reg_ptr);

        } else if (match(cmd, "00xxx100")) {                // inc r8
            printf("    detected: inc r8\n");
            // use r8 index to get the pointer of the full r16 reg
            // then manipulate the lower/upper byte as needed to emulate 8 bit addition
            // i.e. overflow/underflow of the upper/lower byte should not affect the other byte
            // e.g DE = 03_FF should result in 03_00 after inc r8 of the lower byte
            // FIXME: CONFIRM THIS !!
            int reg_idx = ((cmd >> 3) & 0b111);
            reg_ptr = get_r16(reg_idx, USE_R8_INDEX);

            if (is_upper_byte(reg_idx)) {
                uint16_t r8_byte_mask = 0xff00;
                uint8_t byte = ((*reg_ptr) & r8_byte_mask) >> 8;
                byte++;
                (*reg_ptr) &= (~r8_byte_mask);      // clear out the addressed byte of the r16
                (*reg_ptr) |= (byte << 8);
            } else {
                uint16_t r8_byte_mask = 0x00ff;
                uint8_t byte = ((*reg_ptr) & r8_byte_mask);
                byte++;
                (*reg_ptr) &= (~r8_byte_mask);      // clear out the addressed byte of the r16
                (*reg_ptr) |= byte;   
            }
        } else if (match(cmd, "00xxx101")) {                // dec r8
            printf("    detected: dec r8\n");
            int reg_idx = ((cmd >> 3) & 0b111);
            reg_ptr = get_r16(reg_idx, USE_R8_INDEX);

            if (is_upper_byte(reg_idx)) {
                uint16_t r8_byte_mask = 0xff00;
                uint8_t byte = ((*reg_ptr) & r8_byte_mask) >> 8;
                byte--;
                (*reg_ptr) &= (~r8_byte_mask);      // clear out the addressed byte of the r16
                (*reg_ptr) |= (byte << 8);
            } else {
                uint16_t r8_byte_mask = 0x00ff;
                uint8_t byte = ((*reg_ptr) & r8_byte_mask);
                byte--;
                (*reg_ptr) &= (~r8_byte_mask);      // clear out the addressed byte of the r16
                (*reg_ptr) |= byte;   
            }
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

    // Return address of the register (full 16-bit) (address should remain static througout program runtime)
    uint16_t *get_r16(int index, bool index_r8_regs=false) {

        if (index_r8_regs) {
            if (index == 0 || index == 1) return &BC;
            if (index == 2 || index == 3) return &DE;
            if (index == 4 || index == 5) return &HL;
            if (index == 6) {
                printf ("[hl] indexed. Currently not supported");
                exit(1);
            }
            if (index == 7) return &AF;
        } else {
            if (index == 0) return &BC;
            if (index == 1) return &DE;
            if (index == 2) return &HL;
            if (index == 3) return &SP;
            printf ("   Unexpected index to get_r16: %d\n", index);
            exit(1);
        }

        return &AF;
    }

    bool is_upper_byte(int index) {
        if (index == 0 || index == 2 || index == 4 || index == 7) return true;
        if (index == 1 || index == 3 || index == 5) return false;

        printf ("   Unexpected/unsupported index to is_upper_byte: %d\n", index);
        exit(1); 
    }
};