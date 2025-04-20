const bool USE_R8_INDEX = true;
const int NUM_REGS = 6;

#define print(...)                             \
    do {                                       \
        if (disable_prints == 0)               \
            printf(__VA_ARGS__);               \
    } while (0)


#define printv(...)                                 \
    do {                                            \
        if (disable_prints == 0 && verbose == 1)    \
            printf(__VA_ARGS__);                    \
    } while (0)

// FIXME: Use maps to map index to r8/r16 regs. A lot of functions could use this

enum reg_index_t {AF,BC,DE,HL,SP,PC};
enum r8_index_t {A, B, C, D, E, H, L, hl, F};
enum cc_t {nz, z, nc, c};

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

class RegFile {
public:
    Reg regs[NUM_REGS];
    Memory *mem;        // for [hl]

    uint16_t get(reg_index_t r16) {
        return regs[r16].val;
    }
    uint8_t get(r8_index_t r8) {
        if (r8 == A) return regs[AF].hi;
        if (r8 == B) return regs[BC].hi;
        if (r8 == C) return regs[BC].lo;
        if (r8 == D) return regs[DE].hi;
        if (r8 == E) return regs[DE].lo;
        if (r8 == F) return regs[AF].lo;
        if (r8 == H) return regs[HL].hi;
        if (r8 == L) return regs[HL].lo;
        if (r8 == hl) return mem->get(regs[HL].val);
        print (" Unexpected index to get(r8_index_t) \n");
        exit(1);
    }

    void set(r8_index_t r8, uint8_t val) {
        if (r8 == A) regs[AF].hi = val;
        else if (r8 == B) regs[BC].hi = val;
        else if (r8 == C) regs[BC].lo = val;
        else if (r8 == D) regs[DE].hi = val;
        else if (r8 == E) regs[DE].lo = val;
        else if (r8 == F) regs[AF].lo = val;
        else if (r8 == H) regs[HL].hi = val;
        else if (r8 == L) regs[HL].lo = val;
        else if (r8 == hl) mem->set(regs[HL].val, val);
    }

    void set(reg_index_t r16, uint16_t val) {
        regs[r16].val = val;
    }

    bool check_cc(cc_t cc) {
        if (cc == nz) return (regs[AF].flags.z != 1);
        else if (cc == z) return (regs[AF].flags.z == 1);
        else if (cc == nc) return (regs[AF].flags.c != 1);
        else if (cc == c) return (regs[AF].flags.c == 1);
        print ("   Unexpected index to check_cc: %d\n", cc);
        exit(1);
    }

    static r8_index_t get_r8(int index) {
        if (index == 0) return B;
        if (index == 1) return C;
        if (index == 2) return D;
        if (index == 3) return E;
        if (index == 4) return H;
        if (index == 5) return L;
        if (index == 6) return hl;
        if (index == 7) return A;

        print (" Unexpected index to get_r8: %0d\n", index);
        exit(1);
    }

    static reg_index_t get_r16(int index) {

        if (index == 0) return BC;
        if (index == 1) return DE;
        if (index == 2) return HL;
        if (index == 3) return SP;
        print ("   Unexpected index to get_r16: %d\n", index);
        exit(1);
    }

    static const char* get_reg_name(r8_index_t r8) {
        if (r8 == A) return "A";
        if (r8 == B) return "B";
        if (r8 == C) return "C";
        if (r8 == D) return "D";
        if (r8 == E) return "E";
        if (r8 == F) return "F";
        if (r8 == H) return "H";
        if (r8 == L) return "L";
        if (r8 == hl) return "hl";

        print (" get_reg_name Unexpected r8_index_t\n");
        exit(1);
    }


    static const char* get_reg_name(reg_index_t r16) {
        if (r16 == AF) return "AF";
        if (r16 == BC) return "BC";
        if (r16 == DE) return "DE";
        if (r16 == HL) return "HL";
        if (r16 == SP) return "SP";
        if (r16 == PC) return "PC";

        print (" get_reg_name Unexpected r16_index_t\n");
        exit(1);
    }

    void print_regs() {
        print("    | AF: 0x%0x    BC: 0x%0x     DE: 0x%0x     HL: 0x%0x     SP: 0x%0x     PC: 0x%0x ||   znhc: %d%d%d%d ||   *[hl]: 0x%0x\n\n",
                    regs[AF].val, regs[BC].val, regs[DE].val, regs[HL].val, regs[SP].val, regs[PC].val, regs[AF].flags.z,  regs[AF].flags.n,  regs[AF].flags.h,  regs[AF].flags.c, mem->get(get(HL)));
    }
};


class CPU {
public:

    RegFile rf;

    // Default constructor
    CPU (Memory *mem) {
        if (LOAD_BOOT_ROM) {
            rf.regs[PC].val = 0;
        } else {
            rf.regs[PC].val = 0x100;
        }
        rf.mem = mem;
    }

    // add a destructor


    void execute(Memory *mem) {
        bool nop = false;

        // uint16_t *reg_ptr;
        uint8_t cmd = mem->get(rf.regs[PC].val);

        // increment PC
        rf.regs[PC].val++;

        ///////////////////////
        //      BLOCK 0     //
        /////////////////////
        if (cmd == 0) {                                     // nop
            nop = true;
            print ("    detected: nop\n");
        } else if (match(cmd, "00xx0001")) {                // ld r16, imm16
            print("    detected: ld r16, imm16 ---");
            // next 2 bytes are the immediate
            // FIXME: Does PC need to be incremented by 2 as well? Confirm
            uint16_t imm16 = (mem->get(rf.regs[PC].val + 1) << 8) + mem->get(rf.regs[PC].val);    // little endian imm16
            reg_index_t reg_idx = RegFile::get_r16(((cmd >> 4) & 0b11));
            rf.regs[reg_idx].val = imm16;
            rf.regs[PC].val += 2;

            print (" r[%s] <= 0x%0x\n", RegFile::get_reg_name(reg_idx), imm16);
        } else if (match(cmd, "00xx0010")) {                // ld [r16mem], a
            print("    detected: ld dest[r16mem], a ----");
            int reg_idx = RegFile::get_r16(((cmd >> 4) & 0b11));
            mem->set(rf.regs[reg_idx].val, (rf.regs[AF].hi));

            print(" mem[0x%0x] = 0x%0x\n", rf.regs[reg_idx].val, mem->get(rf.regs[reg_idx].val));
            // assert (mem->get(regs[reg_idx].val) == ((*(get_r16(7, USE_R8_INDEX))) >> 8));  // a == mem[r16]
        } else if (match(cmd, "00xx1010")) {                // ld a, [r16mem]	
            print("    detected: ld a, source[r16mem]\n");
            // FIXME: Create decode functions. Below only works because firts 2 bits are 00
            int reg_idx = RegFile::get_r16(((cmd >> 4) & 0b11));
            rf.regs[AF].hi = ((mem->get(rf.regs[reg_idx].val)) << 8);
            
            printv("    log: loaded (from mem) 0x%0x ---> A (0x%0x)\n", rf.regs[reg_idx].val, (rf.regs[AF].val >> 8));
            // assert (mem->get(regs[reg_idx].val) == ((*(get_r16(7, USE_R8_INDEX))) >> 8));  // a == mem[r16]
        } else if (match(cmd, "00001000")) {                // ld [imm16], sp
            print("    detected ld [imm16], sp\n");
            // next 2 bytes are the immediate
            uint16_t imm16 = (mem->get(rf.regs[PC].val+1) << 8) + mem->get(rf.regs[PC].val);
            rf.regs[SP].val = imm16;
            rf.regs[PC].val += 2;

        } else if (match(cmd, "00xx0011")) {                // inc r16
            print("    detected: inc r16\n");
            
            int reg_idx = RegFile::get_r16(((cmd >> 4) & 0b11));
            rf.regs[reg_idx].val++;
        } else if (match(cmd, "00xx1011")) {                // dec r16
            print("    detected: dec r16\n");
            int reg_idx = RegFile::get_r16(((cmd >> 4) & 0b11));
            rf.regs[reg_idx].val--;
        } else if (match(cmd, "00xx1001")) {                // add hl, r16
            print("    detected: add hl, r16\n");
            int reg_idx = RegFile::get_r16(((cmd >> 4) & 0b11));
            rf.regs[HL].val += rf.regs[reg_idx].val;

        } else if (match(cmd, "00xxx100")) {                // inc r8
            print("    detected: inc r8 ---");
            // use r8 index to get the pointer of the full r16 reg
            // then manipulate the lower/upper byte as needed to emulate 8 bit addition
            // i.e. overflow/underflow of the upper/lower byte should not affect the other byte
            // e.g DE = 03_FF should result in 03_00 after inc r8 of the lower byte
            // FIXME: CONFIRM THIS !!
            int reg_bits = ((cmd >> 3) & 0b111);
            // reg_index_t reg_idx = RegFile::get_r16(reg_bits, USE_R8_INDEX);
            r8_index_t reg_idx = RegFile::get_r8(reg_bits);
            rf.set(reg_idx, rf.get(reg_idx) + 1);

            print(" incrementing reg %s\n", RegFile::get_reg_name(reg_idx));
            // print(" incrementing reg %s\n", RegFile::get_reg_name(reg_bits, USE_R8_INDEX).c_str());
        } else if (match(cmd, "00xxx101")) {                // dec r8
            print("    detected: dec r8 ---");
            int reg_bits = ((cmd >> 3) & 0b111);
            r8_index_t reg_idx = RegFile::get_r8(reg_bits);
            compute_flags_sub8(rf.get(reg_idx), 1);
            rf.set(reg_idx, rf.get(reg_idx) - 1); 

            print(" decrementing reg %s\n", RegFile::get_reg_name(reg_idx));

        } else if (match(cmd, "00xxx110")) {                // ld r8, imm8
            print("    detected: ld r8, imm8 ---");
            uint8_t imm8 = mem->get(rf.regs[PC].val);
            int reg_bits = ((cmd >> 3) & 0b111);
            r8_index_t r8 = RegFile::get_r8(reg_bits);
            rf.set(r8, imm8);

            rf.regs[PC].val++;
            print (" r[%s] <= 0x%0x\n", RegFile::get_reg_name(r8), rf.get(r8));
        } else if (match(cmd, "00000111")) {                // rlca
            print("    detected: rlca --- ");
            int msb = rf.regs[AF].hi >> 7;
            rf.regs[AF].hi = rf.regs[AF].hi << 1;
            rf.regs[AF].hi |= msb;     // rotate msb into lsb
            rf.regs[AF].flags.c = msb;       // Update Carry Flag
            print("rotated left A. carry=msb=%d\n", msb);
        } else if (match(cmd, "00001111")) {                // rrca
            print("    detected: rrca --- ");
            int lsb = rf.regs[AF].hi & 0b1;
            rf.regs[AF].hi = rf.regs[AF].hi >> 1;
            rf.regs[AF].hi |= (lsb << 7);     // rotate lsb into msb
            rf.regs[AF].flags.c = lsb;       // Update Carry Flag
            print("rotated right A. carry=lsb=%d\n", lsb);
        } else if (match(cmd, "00010111")) {                // rla
            print("    detected: rla --- ");
            int msb = rf.regs[AF].hi >> 7;
            rf.regs[AF].hi = rf.regs[AF].hi << 1;
            rf.regs[AF].hi |= rf.regs[AF].flags.c;
            rf.regs[AF].flags.c = msb;
            print("rotated left A. carry=msb=%d\n", msb);
        } else if (match(cmd, "00011111")) {                // rra
            print("    detected: rra --- ");
            int lsb = rf.regs[AF].hi & 0b1;
            rf.regs[AF].hi = rf.regs[AF].hi >> 1;
            rf.regs[AF].hi |= (rf.regs[AF].flags.c << 7);
            rf.regs[AF].flags.c = lsb;
            print("rotated left A. carry=lsb=%d\n", lsb);
        } else if (match(cmd, "00100111")) {                // daa
            print("    detected: daa --- \n");
            if (rf.regs[AF].flags.n) {
                int adj = 0;
                if (rf.regs[AF].flags.h) adj += 0x6;
                if (rf.regs[AF].flags.c) adj += 0x60;
                rf.regs[AF].hi -= adj;
            } else {
                int adj = 0;
                if (rf.regs[AF].flags.h || ((rf.regs[AF].hi & 0xf) > 0x9)) adj += 0x6;
                if (rf.regs[AF].flags.c || rf.regs[AF].hi > 0x99) {
                    adj += 0x60;
                    rf.regs[AF].flags.c = 1;
                }
                rf.regs[AF].hi += adj;
            }
            if (rf.regs[AF].hi == 0) rf.regs[AF].flags.z = 1;
            rf.regs[AF].flags.h = 0;
            // FIXME: c flag idk what to do
        } else if (match(cmd, "00101111")) {                // cpl
            print("    detected: cpl --- \n");
            rf.regs[AF].hi = ~rf.regs[AF].hi;
            rf.regs[AF].flags.n = 1;
            rf.regs[AF].flags.h = 1;
        } else if (match(cmd, "00110111")) {                // scf
            print("    detected: scf\n");
            rf.regs[AF].flags.c = 1;
            rf.regs[AF].flags.n = 0;
            rf.regs[AF].flags.h = 0;
        } else if (match(cmd, "00111111")) {                // ccf
            print("    detected: ccf\n");
            rf.regs[AF].flags.c = ~rf.regs[AF].flags.c;
            rf.regs[AF].flags.n = 0;
            rf.regs[AF].flags.h = 0;
        } else if (match(cmd, "00011000")) {                // jr imm8
            print("    detected: jr imm8 ---");
            int8_t imm8 = mem->get(rf.regs[PC].val++);
            rf.regs[PC].val += imm8;
            print (" PC <= 0x%0x\n", imm8);

            //
            //
            //      currently debugging jr instr - compiler not providing imm in next byte??
            //
            //
        } else if (match(cmd, "001xx000")) {                // jr cond, imm8
            print("    detected: jr cond, imm8 ---");
            cc_t cc = static_cast<cc_t>((cmd >> 3) & 0b11);

           
            if (rf.check_cc(cc)) {
                int8_t imm8 = mem->get(rf.regs[PC].val++);
                rf.regs[PC].val += imm8;
                print (" cc: 0x%0x; branch taken: PC <= 0x%0x\n", cc, imm8);
            } else {
                print (" cc: 0x%0x; branch not taken\n", cc);
            }
        } else if (match(cmd, "00010000")) {                // stop
            print("    detected: stop\n");
            rf.regs[PC].val++;         // stop is NOP but 2 cycles (FIXME: need to pair this with WAKE?)
        } 

        ///////////////////////
        //      BLOCK 1     //
        /////////////////////
        else if (match(cmd, "01xxxxxx")) {                  // ld r8, r8
            if (match(cmd, "01110110")) {
                print("    detected: halt ---- FIXME UNIMPLEMENTED");
            } else {
                print("    detected: ld r8, r8 ----");
                int regd_bits = ((cmd >> 3) & 0b111);
                int regs_bits = (cmd & 0b111);
                r8_index_t reg_d = RegFile::get_r8(regd_bits); //RegFile::get_r16(regd_bits, USE_R8_INDEX);
                r8_index_t reg_s = RegFile::get_r8(regs_bits); //RegFile::get_r16(regs_bits, USE_R8_INDEX);
                rf.set(reg_d, rf.get(reg_s));
                print(" r[%s] <= r[%s](0x%0x)\n", RegFile::get_reg_name(reg_d), RegFile::get_reg_name(reg_s), rf.get(reg_d));
            }
        }
        
        ///////////////////////
        //      BLOCK 2     //
        /////////////////////
        else if (match(cmd, "10000xxx")) {                  // add a, r8
            print ("    detected: add a, r8 ---");
            int reg_bits = (cmd & 0b111);
            r8_index_t r8 = RegFile::get_r8(reg_bits);

            compute_flags_add8(rf.get(A), rf.get(r8));
            rf.regs[AF].hi += rf.get(r8);
            print (" A += r[%s](0x%0x)\n", RegFile::get_reg_name(r8), rf.get(r8));
        } else if (match(cmd, "10001xxx")) {                // adc a, r8
            print ("    detected: adc a, r8 ---");
            int reg_bits = (cmd & 0b111);
            r8_index_t r8 = RegFile::get_r8(reg_bits);

            print (" A += carry(%0d) + r[%s](0x%0x)\n", rf.regs[AF].flags.c, RegFile::get_reg_name(r8), rf.get(r8));
            compute_flags_add8(rf.get(A) + rf.regs[AF].flags.c, rf.get(r8));
            rf.regs[AF].hi += (rf.regs[AF].flags.c + rf.get(r8));
        } else if (match(cmd, "10010xxx")) {                // sub a, r8
            print ("    detected: sub a, r8 ---");
            int reg_bits = (cmd & 0b111);
            r8_index_t r8 = RegFile::get_r8(reg_bits);

            compute_flags_sub8(rf.get(A), rf.get(r8));
            rf.regs[AF].hi -= rf.get(r8);
            print (" A -= r[%s](0x%0x)\n", RegFile::get_reg_name(r8), rf.get(r8));
        } else if (match(cmd, "10011xxx")) {                // sbc a, r8
            print ("    detected: sbc a, r8 ---");
            int reg_bits = (cmd & 0b111);
            r8_index_t r8 = RegFile::get_r8(reg_bits);

            print (" A -= (r[%s]:0x%0x + carry(%0d))\n", RegFile::get_reg_name(r8), rf.get(r8), rf.regs[AF].flags.c);
            compute_flags_sub8(rf.get(A), (rf.get(r8) + rf.regs[AF].flags.c));
            rf.regs[AF].hi -= (rf.get(r8) + rf.regs[AF].flags.c);
        } else if (match(cmd, "10100xxx")) {                // and a, r8
            print ("    detected: and a, r8 ---");
            int reg_bits = (cmd & 0b111);
            r8_index_t r8 = RegFile::get_r8(reg_bits);

            rf.set(A, (rf.get(A) & rf.get(r8)));

            rf.regs[AF].flags.z = (rf.get(A) == 0);
            rf.regs[AF].flags.n = 0;
            rf.regs[AF].flags.h = 1; 
            rf.regs[AF].flags.c = 0;
            print (" A &= r[%s]:0x%0x\n", RegFile::get_reg_name(r8), rf.get(r8));
        }

        else {
            print ("    default case\n");
        }


        if (!nop) rf.print_regs();
        serial_output(mem);
    }


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

    void compute_flags_add8(int a, int b) {
        rf.regs[AF].flags.h = (((a & 0x0F) + (b & 0x0F)) > 0x0F);   // carry at bit 3?
        rf.regs[AF].flags.c = (((a & 0xFF) + (b & 0xFF)) > 0xFF);   // carry at bit 7?
        rf.regs[AF].flags.z = (a + b == 0);
        rf.regs[AF].flags.n = 0;
    }

    void compute_flags_sub8(int a, int b) {
        rf.regs[AF].flags.h = ((b & 0xF) > (a & 0xF));              // borrow from bit 4?
        rf.regs[AF].flags.c = ((b & 0xFF) > (a & 0xFF));            // borrow (i.e r8 > A)?
        rf.regs[AF].flags.z = (a - b == 0);
        rf.regs[AF].flags.n = 1;
    }
    
    //    blarggs test - serial output
    void serial_output(Memory *mem) {
        if (mem->get(0xff02) == 0x81) {
            char c = '0' + mem->get(0xff01);
            // print("%c", c);
            std::clog << c;
            // memory[0xff02] = 0x0;
            mem->set(0xff02, 1);
        }    
    }
};