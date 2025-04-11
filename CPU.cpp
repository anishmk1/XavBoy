const bool USE_R8_INDEX = true;
const int NUM_REGS = 6;

// FIXME: define a macro or something that says `print = if (verbose) printf(...)

enum reg_index_t {AF,BC,DE,HL,SP,PC};

union Reg {
    struct {
        uint8_t lo;
        uint8_t hi;
    };
    uint16_t val;
    struct {
        uint8_t rsvd : 4;       // FIXME: rsvd should always read 0 even if written to
        uint8_t c    : 1;
        uint8_t h    : 1;
        uint8_t n    : 1;
        uint8_t z    : 1;
    } flags;
};


class CPU {
public:

    Reg regs[NUM_REGS] =  {0,0,0,0,0,0};

    // Default constructor
    CPU () {
        if (LOAD_BOOT_ROM) {
            regs[PC].val = 0;
        } else {
            regs[PC].val = 0x100;
        }
    }

    // add a destructor

    void print_regs() {
        printf("    | AF: 0x%0x    BC: 0x%0x     DE: 0x%0x     HL: 0x%0x     SP: 0x%0x     PC: 0x%0x ||   znhc: %d%d%d%d\n\n",
                    regs[AF].val, regs[BC].val, regs[DE].val, regs[HL].val, regs[SP].val, regs[PC].val, regs[AF].flags.z,  regs[AF].flags.n,  regs[AF].flags.h,  regs[AF].flags.c);
    }


    void execute(Memory *mem) {

        // uint16_t *reg_ptr;
        uint8_t cmd = mem->get(regs[PC].val);

        ///////////////////////
        //      BLOCK 0     //
        /////////////////////
        if (cmd == 0) {                                     // nop
            std::cout << "  detected: nop" << std::endl;
        } else if (match(cmd, "00xx0001")) {                // ld r16, imm16
            printf("    detected: ld r16, imm16\n");
            // next 2 bytes are the immediate
            // FIXME: Does PC need to be incremented by 2 as well? Confirm
            uint16_t imm16 = (mem->get(regs[PC].val + 2) << 8) + mem->get(regs[PC].val + 1);    // little endian imm16
            int reg_idx = get_r16(((cmd >> 4) & 0b11));
            regs[reg_idx].val = imm16;
            regs[PC].val += 2;
        } else if (match(cmd, "00xx0010")) {                // ld [r16mem], a
            printf("    detected: ld dest[r16mem], a ----");
            int reg_idx = get_r16(((cmd >> 4) & 0b11));
            mem->set(regs[reg_idx].val, (regs[AF].hi));

            printf(" mem[0x%0x] = 0x%0x\n", regs[reg_idx].val, mem->get(regs[reg_idx].val));
            // assert (mem->get(regs[reg_idx].val) == ((*(get_r16(7, USE_R8_INDEX))) >> 8));  // a == mem[r16]
        } else if (match(cmd, "00xx1010")) {                // ld a, [r16mem]	
            printf("    detected: ld a, source[r16mem]\n");
            // FIXME: Create decode functions. Below only works because firts 2 bits are 00
            int reg_idx = get_r16(((cmd >> 4) & 0b11));
            regs[AF].hi = ((mem->get(regs[reg_idx].val)) << 8);
            
            if (verbose) printf("    log: loaded (from mem) 0x%0x ---> A (0x%0x)\n", regs[reg_idx].val, (regs[AF].val >> 8));
            // assert (mem->get(regs[reg_idx].val) == ((*(get_r16(7, USE_R8_INDEX))) >> 8));  // a == mem[r16]
        } else if (match(cmd, "00001000")) {                // ld [imm16], sp
            printf("    detected ld [imm16], sp\n");
            // next 2 bytes are the immediate
            uint16_t imm16 = (mem->get(regs[PC].val+2) << 8) + mem->get(regs[PC].val+1);
            regs[SP].val = imm16;
            regs[PC].val += 2;

        } else if (match(cmd, "00xx0011")) {                // inc r16
            printf("    detected: inc r16\n");
            
            int reg_idx = get_r16(((cmd >> 4) & 0b11));
            regs[reg_idx].val++;
        } else if (match(cmd, "00xx1011")) {                // dec r16
            printf("    detected: dec r16\n");
            int reg_idx = get_r16(((cmd >> 4) & 0b11));
            regs[reg_idx].val--;
        } else if (match(cmd, "00xx1001")) {                // add hl, r16
            printf("    detected: add hl, r16\n");
            int reg_idx = get_r16(((cmd >> 4) & 0b11));
            regs[HL].val += regs[reg_idx].val;

        } else if (match(cmd, "00xxx100")) {                // inc r8
            printf("    detected: inc r8 ---");
            // use r8 index to get the pointer of the full r16 reg
            // then manipulate the lower/upper byte as needed to emulate 8 bit addition
            // i.e. overflow/underflow of the upper/lower byte should not affect the other byte
            // e.g DE = 03_FF should result in 03_00 after inc r8 of the lower byte
            // FIXME: CONFIRM THIS !!
            int reg_bits = ((cmd >> 3) & 0b111);
            reg_index_t reg_idx = get_r16(reg_bits, USE_R8_INDEX);

            if (is_upper_byte(reg_bits)) {
                regs[reg_idx].hi++;
            } else {
                regs[reg_idx].lo++;
            }
            printf(" incrementing reg %s\n", get_reg_name(reg_bits, USE_R8_INDEX).c_str());
        } else if (match(cmd, "00xxx101")) {                // dec r8
            printf("    detected: dec r8\n");
            int reg_idx = get_r16(((cmd >> 3) & 0b111), USE_R8_INDEX);

            if (is_upper_byte(((cmd >> 3) & 0b111))) {
                regs[reg_idx].hi--;
            } else {
                regs[reg_idx].lo--;
            }
        } else if (match(cmd, "00xxx110")) {                // ld r8, imm8
            printf("    detected: ld r8, imm8 ---\n");
            uint8_t imm8 = mem->get(regs[PC].val + 1);
            int reg_bits = ((cmd >> 3) & 0b111);
            reg_index_t reg = get_r16(reg_bits, USE_R8_INDEX);
            if (is_upper_byte(reg_bits)) {
                regs[reg].hi = imm8;
            } else {
                regs[reg].lo = imm8;
            }

            regs[PC].val++;
        } else if (match(cmd, "00000111")) {                // rlca
            printf("    detected: rlca --- ");
            int msb = regs[AF].hi >> 7;
            regs[AF].hi = regs[AF].hi << 1;
            regs[AF].hi |= msb;     // rotate msb into lsb
            regs[AF].flags.c = msb;       // Update Carry Flag
            printf("rotated left A. carry=msb=%d\n", msb);
        } else if (match(cmd, "00001111")) {                // rrca
            printf("    detected: rrca --- ");
            int lsb = regs[AF].hi & 0b1;
            regs[AF].hi = regs[AF].hi >> 1;
            regs[AF].hi |= (lsb << 7);     // rotate lsb into msb
            regs[AF].flags.c = lsb;       // Update Carry Flag
            printf("rotated right A. carry=lsb=%d\n", lsb);
        } else if (match(cmd, "00010111")) {                // rla
            printf("    detected: rla --- ");
            int msb = regs[AF].hi >> 7;
            regs[AF].hi = regs[AF].hi << 1;
            regs[AF].hi |= regs[AF].flags.c;
            regs[AF].flags.c = msb;
            printf("rotated left A. carry=msb=%d\n", msb);
        } else if (match(cmd, "00011111")) {                // rra
            printf("    detected: rra --- ");
            int lsb = regs[AF].hi & 0b1;
            regs[AF].hi = regs[AF].hi >> 1;
            regs[AF].hi |= (regs[AF].flags.c << 7);
            regs[AF].flags.c = lsb;
            printf("rotated left A. carry=lsb=%d\n", lsb);
        } else if (match(cmd, "00100111")) {                // daa
            printf("    detected: daa --- \n");
            if (regs[AF].flags.n) {
                int adj = 0;
                if (regs[AF].flags.h) adj += 0x6;
                if (regs[AF].flags.c) adj += 0x60;
                regs[AF].hi -= adj;
            } else {
                int adj = 0;
                if (regs[AF].flags.h || ((regs[AF].hi & 0xf) > 0x9)) adj += 0x6;
                if (regs[AF].flags.c || regs[AF].hi > 0x99) {
                    adj += 0x60;
                    regs[AF].flags.c = 1;
                }
                regs[AF].hi += adj;
            }
            if (regs[AF].hi == 0) regs[AF].flags.z = 1;
            regs[AF].flags.h = 0;
            // FIXME: c flag idk what to do
        } else if (match(cmd, "00101111")) {                // cpl
            printf("    detected: cpl --- \n");
            regs[AF].hi = ~regs[AF].hi;
            regs[AF].flags.n = 1;
            regs[AF].flags.h = 1;
        } else if (match(cmd, "00110111")) {                // scf
            printf("    detected: scf\n");
            regs[AF].flags.c = 1;
            regs[AF].flags.n = 0;
            regs[AF].flags.h = 0;
        } else if (match(cmd, "00111111")) {                // ccf
            printf("    detected: ccf\n");
            regs[AF].flags.c = ~regs[AF].flags.c;
            regs[AF].flags.n = 0;
            regs[AF].flags.h = 0;
        } else if (match(cmd, "00011000")) {                // jr imm8
            printf("    detected: jr imm8\n");
            int8_t imm8 = mem->get(regs[PC].val + 1);
            regs[PC].val += imm8;
        } else if (match(cmd, "001xx000")) {                // jr cond, imm8
            printf("    detected: jr cond, imm8\n");
            int cc = (cmd >> 3) & 0b11;
            // FIXME: assumption!! - cc refers to the bit index of the flag 
            if ((regs[AF].lo >> cc) & 0b1) {
                int8_t imm8 = mem->get(regs[PC].val + 1);
                regs[PC].val += imm8;
            }
        }
        else {
            std::cout << "  default case" << std::endl;
        }

        print_regs();
        serial_output(mem);
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

    reg_index_t get_r16(int index, bool index_r8_regs=false) {

        if (index_r8_regs) {
            if (index == 0 || index == 1) return BC;
            if (index == 2 || index == 3) return DE;
            if (index == 4 || index == 5) return HL;
            if (index == 6) {
                printf ("[hl] indexed. Currently not supported");
                exit(1);
            }
            if (index == 7) return AF;
        } else {
            if (index == 0) return BC;
            if (index == 1) return DE;
            if (index == 2) return HL;
            if (index == 3) return SP;
            printf ("   Unexpected index to get_r16: %d\n", index);
            exit(1);
        }

        return AF;
    }

    bool is_upper_byte(int index) {
        if (index == 0 || index == 2 || index == 4 || index == 7) return true;
        if (index == 1 || index == 3 || index == 5) return false;

        printf ("   Unexpected/unsupported index to is_upper_byte: %d\n", index);
        exit(1); 
    }

    std::string get_reg_name(int reg_bits, bool index_r8_regs=false) {
        if (index_r8_regs) {
            if (reg_bits == 0) return "B";
            if (reg_bits == 1) return "C";
            if (reg_bits == 2) return "D";
            if (reg_bits == 3) return "E";
            if (reg_bits == 4) return "H";
            if (reg_bits == 5) return "L";
            if (reg_bits == 6) return "[hl]";
            if (reg_bits == 7) return "A";
            printf ("   Unexpected reg_bits to get_reg_name: %d\n", reg_bits);
            exit(1);
        } else {
            if (reg_bits == 0) return "BC";
            if (reg_bits == 1) return "DE";
            if (reg_bits == 2) return "HL";
            if (reg_bits == 3) return "SP";
            printf ("   Unexpected reg_bits to get_reg_name: %d\n", reg_bits);
            exit(1);
        }
    }

    //    blarggs test - serial output
    void serial_output(Memory *mem) {
        if (mem->get(0xff02) == 0x81) {
            char c = '0' + mem->get(0xff01);
            // printf("%c", c);
            std::clog << c;
            // memory[0xff02] = 0x0;
            mem->set(0xff02, 1);
        }    
    }
};