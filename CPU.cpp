const bool USE_R8_INDEX = true;
const int NUM_REGS = 6;


// FIXME: Use maps to map index to r8/r16 regs. A lot of functions could use this

enum r16_index_t {AF,BC,DE,HL,SP,PC};
enum r8_index_t {A, B, C, D, E, H, L, hl, F};
enum r16mem_index_t {_BC, _DE, _HLi, _HLd};
enum cc_t {nz, z, nc, c};

struct ExcludeFlags {
    bool no_z = false;
    bool no_n = false;
    bool no_h = false;
    bool no_c = false;
};

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

    // FIXME: move this somewhere that makes sense
    uint16_t debug0;
    uint16_t debug1;
    uint16_t debug2;

    uint16_t get(r16_index_t r16) {
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
    uint16_t get(r16mem_index_t r16mem, bool modify=false) {
        if (r16mem == _BC) return regs[BC].val;
        if (r16mem == _DE) return regs[DE].val;
        if (r16mem == _HLi) return ((modify) ? regs[HL].val++ : regs[HL].val);
        if (r16mem == _HLd) return ((modify) ? regs[HL].val-- : regs[HL].val);
        printx (" Unexpected index to get(r16mem_index_t) \n");
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

    void set(r16_index_t r16, uint16_t val) {
        regs[r16].val = val;
    }

    bool check_cc(cc_t cc) {
        if (cc == nz) return (regs[AF].flags.z == 0);
        else if (cc == z) return (regs[AF].flags.z == 1);
        else if (cc == nc) {
            print ("    regs[AF].flags.c = %0d\n", regs[AF].flags.c);
            // print ("    regs[AF].flags = 0x%0x\n", regs[AF].flags);
            print ("    regs[AF].lo = 0x%0x\n", regs[AF].lo);
            print ("    regs[AF].val[4] = %0d\n", ((regs[AF].val >> 4) & 0x1));
            return (regs[AF].flags.c == 0);
        }
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

    static r16_index_t get_r16(int index) {

        if (index == 0) return BC;
        if (index == 1) return DE;
        if (index == 2) return HL;
        if (index == 3) return SP;
        printx ("   Unexpected index to get_r16: %d\n", index);
        exit(1);
    }

    static r16mem_index_t get_r16mem(int index) {
        if (index == 0) return _BC;
        if (index == 1) return _DE;
        if (index == 2) return _HLi;
        if (index == 3) return _HLd;
        printx ("   Unexpected index to get_r16mem: %0d\n", index);
        exit(1);
    }

    static r16_index_t get_r16stk(int index) {
        if (index == 0) return BC;
        if (index == 1) return DE;
        if (index == 2) return HL;
        if (index == 3) return AF;
        printx ("   Unexpected index to get_r16stk: %0d\n", index);
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


    static const char* get_reg_name(r16_index_t r16) {
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
        if (PRINT_REGS_EN == 0) return;
        if (GAMEBOY_DOCTOR) {
            printv ("A:%02X F:%02X B:%02X C:%02X D:%02X E:%02X H:%02X L:%02X SP:%02X PC:%04X PCMEM:%02X,%02X,%02X,%02X\n",
                    get(A), get(F), get(B), get(C), get(D), get(E), get(H), get(L), get(SP), get(PC),
                    mem->get(get(PC)), mem->get(get(PC)+1), mem->get(get(PC)+2), mem->get(get(PC)+3));
        } else {
            print ("    | AF: 0x%0x    BC: 0x%0x     DE: 0x%0x     HL: 0x%0x     SP: 0x%0x     PC: 0x%0x ||   znhc: %d%d%d%d ||   *[hl]: 0x%0x\n\n",
                        regs[AF].val, regs[BC].val, regs[DE].val, regs[HL].val, regs[SP].val, regs[PC].val, regs[AF].flags.z,  regs[AF].flags.n,  regs[AF].flags.h,  regs[AF].flags.c, mem->get(get(HL)));
        }
    }
};


class CPU {
public:

    Memory *mem;
    RegFile rf;



    // Default constructor
    CPU (Memory *mem) {
        rf.mem = mem;
        this->mem = mem;

        if (SKIP_BOOT_ROM) {
            // Load initial system state after BOOT_ROM 
            rf.set(A, 0x01);
            rf.set(F, 0xB0);     // FIXME: In gameboy doctor docs: F	0xB0 (or CH-Z if managing flags individually)
            rf.set(B, 0x00);
            rf.set(C, 0x13);
            rf.set(D, 0x00);
            rf.set(E, 0xD8);
            rf.set(H, 0x01);
            rf.set(L, 0x4D);
            rf.set(SP, 0xFFFE);
            rf.set(PC, 0x100);
        } else {
            rf.regs[PC].val = 0;
        }
    }

    // add a destructor


    void execute() {
        bool nop = false;

        // uint16_t *reg_ptr;
        uint8_t cmd = mem->get(rf.regs[PC].val);

        if (GAMEBOY_DOCTOR) {
            rf.print_regs();
        }
        // check for infinite loop
        if ((cmd == 0x18) && (mem->get(rf.get(PC) + 1) == 0xFE)) {
            print ("Detected Infinite loop. Exiting sim\n");
            std::exit(EXIT_SUCCESS);
        }

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
            r16_index_t reg_idx = RegFile::get_r16(((cmd >> 4) & 0b11));
            rf.regs[reg_idx].val = imm16;
            rf.regs[PC].val += 2;

            print (" r[%s] <= 0x%0x\n", RegFile::get_reg_name(reg_idx), imm16);
        } else if (match(cmd, "00xx0010")) {                // ld [r16mem], a
            print("    detected: ld dest[r16mem], a ----");
            r16mem_index_t reg_idx = RegFile::get_r16mem(((cmd >> 4) & 0b11));
            mem->set(rf.get(reg_idx, true), rf.regs[AF].hi);

            // assert (mem->get(regs[reg_idx].val) == ((*(get_r16(7, USE_R8_INDEX))) >> 8));  // a == mem[r16]
        } else if (match(cmd, "00xx1010")) {                // ld a, [r16mem]	
            print("    detected: ld a, source[r16mem]\n");
            r16mem_index_t reg_idx = RegFile::get_r16mem(((cmd >> 4) & 0b11));
            int addr = rf.get(reg_idx, true); // increment HL after reading

            rf.regs[AF].hi = (mem->get(addr));
            
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
            r16_index_t reg_idx = RegFile::get_r16(((cmd >> 4) & 0b11));
            rf.regs[AF].flags.n = 0;
            rf.regs[AF].flags.h = add16_overflowFromBitX(11, rf.get(HL), rf.get(reg_idx));
            rf.regs[AF].flags.c = add16_overflowFromBitX(15, rf.get(HL), rf.get(reg_idx));

            rf.regs[HL].val += rf.regs[reg_idx].val;

        } else if (match(cmd, "00xxx100")) {                // inc r8
            print("    detected: inc r8 ---");
            int reg_bits = ((cmd >> 3) & 0b111);
            r8_index_t reg_idx = RegFile::get_r8(reg_bits);
            compute_flags_add8(rf.get(reg_idx), 1, ExcludeFlags{.no_c = true});
            rf.set(reg_idx, rf.get(reg_idx) + 1);

            print(" incrementing reg %s\n", RegFile::get_reg_name(reg_idx));
            // print(" incrementing reg %s\n", RegFile::get_reg_name(reg_bits, USE_R8_INDEX).c_str());
        } else if (match(cmd, "00xxx101")) {                // dec r8
            print("    detected: dec r8 ---");
            int reg_bits = ((cmd >> 3) & 0b111);
            r8_index_t reg_idx = RegFile::get_r8(reg_bits);
            compute_flags_sub8(rf.get(reg_idx), 1, ExcludeFlags{.no_c = true});
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
            rf.regs[AF].flags.z = 0;
            rf.regs[AF].flags.n = 0;
            rf.regs[AF].flags.h = 0;
            print("rotated left A. carry=msb=%d\n", msb);
        } else if (match(cmd, "00001111")) {                // rrca
            print("    detected: rrca --- ");
            int lsb = rf.regs[AF].hi & 0b1;
            rf.regs[AF].hi = rf.regs[AF].hi >> 1;
            rf.regs[AF].hi |= (lsb << 7);     // rotate lsb into msb
            rf.regs[AF].flags.c = lsb;       // Update Carry Flag
            rf.regs[AF].flags.z = 0;
            rf.regs[AF].flags.n = 0;
            rf.regs[AF].flags.h = 0;
            print("rotated right A. carry=lsb=%d\n", lsb);
        } else if (match(cmd, "00010111")) {                // rla
            print("    detected: rla --- ");
            int msb = rf.regs[AF].hi >> 7;
            rf.regs[AF].hi = rf.regs[AF].hi << 1;
            rf.regs[AF].hi |= rf.regs[AF].flags.c;
            rf.regs[AF].flags.c = msb;
            rf.regs[AF].flags.z = 0;
            rf.regs[AF].flags.n = 0;
            rf.regs[AF].flags.h = 0;
            print("rotated left A. carry=msb=%d\n", msb);
        } else if (match(cmd, "00011111")) {                // rra
            print("    detected: rra --- ");
            uint8_t val = rf.get(A);
            int lsb = val & 0b1;
            val = val >> 1;
            val |= (rf.regs[AF].flags.c << 7);
            rf.set(A, val);
            rf.regs[AF].flags.c = lsb;
            rf.regs[AF].flags.z = 0;
            rf.regs[AF].flags.n = 0;
            rf.regs[AF].flags.h = 0;
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
            // if (rf.regs[AF].hi == 0) rf.regs[AF].flags.z = 1;
            rf.regs[AF].flags.z = (rf.get(A) == 0);
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
            int8_t imm8 = mem->get(rf.regs[PC].val);
            rf.regs[PC].val++;          // JR instr is 2 bytes long. So PC is incremented once to cover reading the imm8. Then the actual jump is done
            rf.regs[PC].val += imm8;
            print (" PC += 0x%0x\n", imm8);

            //
            //
            //      DOCUMENTING PAST ISSUE: 
            //      currently debugging jr instr - RGBASM compiler not providing imm in next byte??
            //      FOr some reason, imm8 is placed somewhere way later than the next byte when compiled with RGBASM
            //
            //
        } else if (match(cmd, "001xx000")) {                // jr cond, imm8
            print("    detected: jr cond, imm8 ---");
            cc_t cc = static_cast<cc_t>((cmd >> 3) & 0b11);
            int8_t imm8 = mem->get(rf.regs[PC].val);
            rf.regs[PC].val++;      // JR instr is 2 bytes long. So PC is incremented once regardless of branch taken/not to cover reading the imm8. Then the actual jump is done

            // if ((cc != nc && rf.check_cc(cc)) ||
            //     (cc == nc && (((rf.regs[AF].lo >> 4) & 0x1) == 0))) {
            if (rf.check_cc(cc)) {
                rf.regs[PC].val += imm8;
                print (" cc: 0x%0x; branch taken: PC <= 0x%0x\n", cc, imm8);
            } else {
                print (" cc: 0x%0x; branch not taken\n", cc);
            }
        } else if (match(cmd, "00010000")) {                // stop
            printx ("    detected: stop\n");
            rf.regs[PC].val++;         // stop is NOP but 2 cycles (FIXME: need to pair this with WAKE?)
            std::exit(EXIT_SUCCESS);
        } 

        ///////////////////////
        //      BLOCK 1     //
        /////////////////////
        else if (match(cmd, "01xxxxxx")) {                  // ld r8, r8
            if (match(cmd, "01110110")) {
                printx ("    detected: HALT");
                std::exit(EXIT_SUCCESS);
            } else {
                print("    detected: ld r8, r8 ----");
                int regd_bits = ((cmd >> 3) & 0b111);
                int regs_bits = (cmd & 0b111);
                r8_index_t reg_d = RegFile::get_r8(regd_bits); //RegFile::get_r16(regd_bits, USE_R8_INDEX);
                r8_index_t reg_s = RegFile::get_r8(regs_bits); //RegFile::get_r16(regs_bits, USE_R8_INDEX);
                rf.set(reg_d, rf.get(reg_s));
            }
        }
        
        /////////////////////////////////
        //      BLOCK 2 / BLOCK 3     //
        ///////////////////////////////
        else if (match(cmd, "10000xxx") || match(cmd, "11000110")) {                  // add a, r8          add a, imm8
            uint8_t operand;
            if (get_block2_3_operand(cmd, operand)) print ("    detected: add a, imm8 ---");
            else print ("    detected: add a, r8 ---");            
            
            compute_flags_add8(rf.get(A), operand);

            rf.regs[AF].hi += operand;
            print (" A += (0x%0x)\n", operand);
        } else if (match(cmd, "10001xxx") || match(cmd, "11001110")) {                // adc a, r8          adc a, imm8
            uint8_t operand;
            if (get_block2_3_operand(cmd, operand)) print ("    detected: adc a, imm8 ---");
            else print ("    detected: adc a, r8 ---");

            uint8_t val = rf.get(A) + rf.regs[AF].flags.c + operand;

            // compute_flags_add8(rf.get(A) + rf.regs[AF].flags.c, operand);
            rf.regs[AF].flags.n = 0;
            rf.regs[AF].flags.z = (val == 0);
            rf.regs[AF].flags.h = add16_overflowFromBitX(3, rf.get(A), rf.regs[AF].flags.c, operand);
            rf.regs[AF].flags.c = add16_overflowFromBitX(7, rf.get(A), rf.regs[AF].flags.c, operand);
            rf.set(A, val);
            // rf.regs[AF].hi += (rf.regs[AF].flags.c + operand);
        } else if (match(cmd, "10010xxx") || match(cmd, "11010110")) {                // sub a, r8          sub a, imm8
            uint8_t operand;
            if (get_block2_3_operand(cmd, operand)) print ("    detected: sub a, imm8 ---");
            else print ("    detected: sub a, r8 ---");

            compute_flags_sub8(rf.get(A), operand);
            rf.regs[AF].hi -= operand;
            print (" A -= (0x%0x)\n", operand);
        } else if (match(cmd, "10011xxx") || match(cmd, "11011110")) {                // sbc a, r8          sbc a, imm8
            uint8_t operand;
            if (get_block2_3_operand(cmd, operand)) print ("    detected: sbc a, imm8 ---");
            else print ("    detected: sbc a, r8 ---");

            int carry = rf.regs[AF].flags.c;
            rf.regs[AF].flags.n = 1;
            rf.regs[AF].flags.z = (static_cast<uint8_t>(rf.get(A) - operand) - carry == 0);
            rf.regs[AF].flags.h = sub_borrowFromBitX(4, rf.get(A), operand, carry);
            rf.regs[AF].flags.c = ((operand + carry) > rf.get(A));
            rf.regs[AF].hi -= (operand + carry);
        } else if (match(cmd, "10100xxx") || match(cmd, "11100110")) {                // and a, r8          and a, imm8 
            uint8_t operand;
            if (get_block2_3_operand(cmd, operand)) print ("    detected: and a, imm8 ---");
            else print ("    detected: and a, r8 ---");

            rf.set(A, (rf.get(A) & operand));

            rf.regs[AF].flags.z = (rf.get(A) == 0);
            rf.regs[AF].flags.n = 0;
            rf.regs[AF].flags.h = 1; 
            rf.regs[AF].flags.c = 0;
            print (" A &= 0x%0x\n", operand);
        } else if (match(cmd, "10101xxx") || match(cmd, "11101110")) {                // xor a, r8          xor a, imm8
            uint8_t operand;
            if (get_block2_3_operand(cmd, operand)) print ("    detected: xor a, imm8 ---");
            else print ("    detected: xor a, r8 ---");

            rf.set(A, (rf.get(A) ^ operand));

            rf.regs[AF].flags.z = (rf.get(A) == 0);
            rf.regs[AF].flags.n = 0;
            rf.regs[AF].flags.h = 0; 
            rf.regs[AF].flags.c = 0;
            print (" A ^= 0x%0x\n", operand);
        } else if (match(cmd, "10110xxx") || match(cmd, "11110110")) {                // or a, r8           or a, imm8
            uint8_t operand;
            if (get_block2_3_operand(cmd, operand)) print ("    detected: or a, imm8 ---");
            else print ("    detected: or a, r8 ---");

            rf.set(A, (rf.get(A) | operand));

            rf.regs[AF].flags.z = (rf.get(A) == 0);
            rf.regs[AF].flags.n = 0;
            rf.regs[AF].flags.h = 0; 
            rf.regs[AF].flags.c = 0;
            print (" A |= 0x%0x\n", operand);
        } else if (match(cmd, "10111xxx") || match(cmd, "11111110")) {                // cp a, r8           cp a, imm8
            uint8_t operand;
            if (get_block2_3_operand(cmd, operand)) print ("    detected: cp a, imm8 ---");
            else print ("    detected: cp a, r8 ---\n");

            compute_flags_sub8(rf.get(A), operand);
        } 

        ///////////////////////
        //      BLOCK 3     //
        /////////////////////
        else if (match(cmd, "110xx000")) {                  // ret cond
            print ("    detected: ret cond\n");
            cc_t cc = static_cast<cc_t>((cmd >> 3) & 0b11);

            if (rf.check_cc(cc)) {
                pop_r16_stack(PC);      // ret ~= POP PC (Move PC to return address:head of stack; then incr stack pointer to "pop")
            }
        } else if (match(cmd, "11001001")) {                // ret
            print ("    detected: ret\n");
            
            
            // printx ("around 0xdffd\n");
            // mem->dump_mem(0xdffd);
            // printx ("around 0x00cd\n");
            // mem->dump_mem(0x00cd);
            pop_r16_stack(PC);      // ret ~= POP PC (Move PC to return address:head of stack; then incr stack pointer to "pop")
        } else if (match(cmd, "11011001")) {                // reti
            print ("    detected: reti\n");

            pop_r16_stack(PC);
            mem->mmio->set_ime();    // Enable Interrupts next cycle

        } else if (match(cmd, "110xx010")) {                // jp cond, imm16
            print ("    detected: jp cond, imm16\n");
            cc_t cc = static_cast<cc_t>((cmd >> 3) & 0b11);

            if (rf.check_cc(cc)) {
                uint16_t imm16 = (mem->get(rf.regs[PC].val + 1) << 8) + mem->get(rf.regs[PC].val);    // little endian imm16
                rf.set(PC, imm16);
            } else {
                rf.regs[PC].val += 2;
            }
        } else if (match(cmd, "11000011")) {                // jp imm16
            print ("    detected: jp cond, imm16\n");
            uint16_t imm16 = (mem->get(rf.regs[PC].val + 1) << 8) + mem->get(rf.regs[PC].val);    // little endian imm16
            rf.set(PC, imm16);
        } else if (match(cmd, "11101001")) {                // jp hl
            print ("    detected: jp hl\n");
            rf.set(PC, rf.get(HL));
        } else if (match(cmd, "110xx100")) {                // call cond, imm16
            print ("    detected: call cond, imm16\n");

            cc_t cc = static_cast<cc_t>((cmd >> 3) & 0b11);
            uint16_t imm16 = (mem->get(rf.regs[PC].val + 1) << 8) + mem->get(rf.regs[PC].val);      // little endian imm16
            rf.regs[PC].val += 2;

            if (rf.check_cc(cc)) {
                uint16_t return_addr = rf.regs[PC].val;    // little endian retn addr

                push_val_stack(return_addr);
                rf.set(PC, imm16);
            }
        } else if (match(cmd, "11001101")) {                // call imm16
            print ("    detected: call imm16\n");
            // call ~= push addr of next instr (return instr) onto the stack and then jump to imm16

            // call is 3 bytes. So PC, PC + 1 refers to the imm16 bytes of the instr
            // And PC + 2 refers to the byte after the call instr - return addr
            uint16_t imm16 = (mem->get(rf.regs[PC].val + 1) << 8) + mem->get(rf.regs[PC].val);      // little endian imm16
            rf.regs[PC].val += 2;
            uint16_t return_addr = rf.regs[PC].val;


            push_val_stack(return_addr);
            rf.set(PC, imm16);

        } else if (match(cmd, "11xxx111")) {                // rst tgt3
            print ("    detected: rst tgt3\n");

            uint16_t target_addr = (((cmd >> 3) & 0x111) << 3);     // tgt3 encodes the target address divided by 8.
            uint16_t return_addr = (mem->get(rf.get(PC) + 3) << 8) + (mem->get(rf.get(PC) + 2));    // little endian retn addr

            push_val_stack(return_addr);
            rf.set(PC, target_addr);
        } else if (match(cmd, "11xx0001")) {                // pop r16stk
            print ("    detected: pop r16stk\n");

            r16_index_t r16stk = RegFile::get_r16stk(((cmd >> 4) & 0b11));
            pop_r16_stack(r16stk);

        } else if (match(cmd, "11xx0101")) {                // push r16stk
            print ("    detected: push r16stk ---");

            r16_index_t r16stk = RegFile::get_r16stk(((cmd >> 4) & 0b11));
            uint16_t val = rf.get(r16stk);
            push_val_stack(val);

            print (" r16stk=%0d; val=0x%0x; SP=0x%0x\n", r16stk, val, rf.get(SP));
        } 
        

        
        /////////////////////////////////
        //      $CB PREFIX INSTRS     //
        ///////////////////////////////
        
        else if (match(cmd, "11001011")) {                  // $CB
            print ("    detected: cb\n");

            uint8_t cb_op = mem->get(rf.get(PC));
            r8_index_t r8 = RegFile::get_r8(cb_op & 0b111);

            if (match(cb_op, "00000xxx")) {         // rlc r8
                print("    detected: rlc r8\n");
                uint8_t val = rf.get(r8);
                uint8_t msb = (val >> 7) & 1;
                val = ((val << 1) | msb) & 0xFF;
                rf.set(r8, val);
                rf.regs[AF].flags.z = (val == 0);
                rf.regs[AF].flags.n = 0;
                rf.regs[AF].flags.h = 0;
                rf.regs[AF].flags.c = msb;
            } else if (match(cb_op, "00001xxx")) {  // rrc r8
                print("    detected: rrc r8\n");
                uint8_t val = rf.get(r8);
                uint8_t lsb = val & 1;
                val = ((val >> 1) | (lsb << 7)) & 0xFF;
                rf.set(r8, val);
                rf.regs[AF].flags.z = (val == 0);
                rf.regs[AF].flags.n = 0;
                rf.regs[AF].flags.h = 0;
                rf.regs[AF].flags.c = lsb;
            } else if (match(cb_op, "00010xxx")) {  // rl r8
                print("    detected: rl r8\n");
                uint8_t val = rf.get(r8);
                uint8_t msb = (val >> 7) & 1;
                val = ((val << 1) | rf.regs[AF].flags.c) & 0xFF;
                rf.set(r8, val);
                rf.regs[AF].flags.z = (val == 0);
                rf.regs[AF].flags.n = 0;
                rf.regs[AF].flags.h = 0;
                rf.regs[AF].flags.c = msb;
            } else if (match(cb_op, "00011xxx")) {  // rr r8
                print("    detected: rr r8\n");
                uint8_t val = rf.get(r8);
                uint8_t lsb = val & 1;
                val = ((val >> 1) | (rf.regs[AF].flags.c << 7)) & 0xFF;
                rf.set(r8, val);
                rf.regs[AF].flags.z = (val == 0);
                rf.regs[AF].flags.n = 0;
                rf.regs[AF].flags.h = 0;
                rf.regs[AF].flags.c = lsb;
            } else if (match(cb_op, "00100xxx")) {  // sla r8
                print("    detected: sla r8\n");
                uint8_t val = rf.get(r8);
                uint8_t msb = (val >> 7) & 1;
                val = (val << 1) & 0xFF;
                rf.set(r8, val);
                rf.regs[AF].flags.z = (val == 0);
                rf.regs[AF].flags.n = 0;
                rf.regs[AF].flags.h = 0;
                rf.regs[AF].flags.c = msb;
            } else if (match(cb_op, "00101xxx")) {  // sra r8
                print("    detected: sra r8\n");
                uint8_t val = rf.get(r8);
                uint8_t msb = val & 0x80;
                uint8_t lsb = val & 1;
                val = ((val >> 1) | msb) & 0xFF;
                rf.set(r8, val);
                rf.regs[AF].flags.z = (val == 0);
                rf.regs[AF].flags.n = 0;
                rf.regs[AF].flags.h = 0;
                rf.regs[AF].flags.c = lsb;
            } else if (match(cb_op, "00110xxx")) {  // swap r8
                print("    detected: swap r8\n");
                uint8_t val = rf.get(r8);
                val = ((val & 0xF) << 4) | ((val & 0xF0) >> 4);
                rf.set(r8, val);
                rf.regs[AF].flags.z = (val == 0);
                rf.regs[AF].flags.n = 0;
                rf.regs[AF].flags.h = 0;
                rf.regs[AF].flags.c = 0;
            } else if (match(cb_op, "00111xxx")) {  // srl r8
                print("    detected: srl r8\n");
                uint8_t val = rf.get(r8);
                uint8_t lsb = val & 1;
                val = (val >> 1) & 0xFF;
                rf.set(r8, val);
                rf.regs[AF].flags.z = (val == 0);
                rf.regs[AF].flags.n = 0;
                rf.regs[AF].flags.h = 0;
                rf.regs[AF].flags.c = lsb;
            } else if (match(cb_op, "01xxxxxx")) {  // bit b3, r8
                int b3 = (cb_op >> 3) & 0b111;
                print("    detected: bit %d, r8\n", b3);
                uint8_t val = rf.get(r8);
                rf.regs[AF].flags.z = ((val & (1 << b3)) == 0);
                rf.regs[AF].flags.n = 0;
                rf.regs[AF].flags.h = 1;
            } else if (match(cb_op, "10xxxxxx")) {  // res b3, r8
                int b3 = (cb_op >> 3) & 0b111;
                print("    detected: res %d, r8\n", b3);
                uint8_t val = rf.get(r8);
                val &= ~(1 << b3);
                rf.set(r8, val);
            } else if (match(cb_op, "11xxxxxx")) {  // set b3, r8
                int b3 = (cb_op >> 3) & 0b111;
                print("    detected: set %d, r8\n", b3);
                uint8_t val = rf.get(r8);
                val |= (1 << b3);
                rf.set(r8, val);
            } else {
                printx("    default case in cb: 0x%02x\n", cb_op);
            }
            rf.regs[PC].val++;

        } else if (match(cmd, "11100010")) {                // ldh [c], a
            print ("    detected: ldh [c], a\n");

            mem->set(0xff00 + rf.get(C), rf.get(A));
        } else if (match(cmd, "11100000")) {                // ldh imm8, a
            print ("    detected: ldh imm8, a\n");

            uint8_t imm8 = mem->get(rf.get(PC)); 
            rf.regs[PC].val += 1;
            mem->set(0xff00 + imm8, rf.get(A));
        } else if (match(cmd, "11101010")) {                // ld [imm16], a
            print ("    detected: ld [imm16], a\n");

            uint16_t imm16 = (mem->get(rf.regs[PC].val + 1) << 8) + mem->get(rf.regs[PC].val);      // little endian imm16
            rf.regs[PC].val += 2;
            mem->set(imm16, rf.get(A));
        } else if (match(cmd, "11110010")) {                // ldh a, [c]
            print ("    detected: ldh a, [c]\n");

            rf.set(A, mem->get(0xff00 + rf.get(C)));
        } else if (match(cmd, "11110000")) {                // ldh a, [imm8]
            print ("    detected: ldh a, [imm8]\n");

            uint8_t imm8 = mem->get(rf.get(PC));
            rf.regs[PC].val++;
            rf.set(A, mem->get(0xff00 + imm8));
        } else if (match(cmd, "11111010")) {                // ld a, [imm16]
            print ("    detected: ld a, [imm16]\n");

            uint16_t imm16 = (mem->get(rf.regs[PC].val + 1) << 8) + mem->get(rf.regs[PC].val);      // little endian imm16
            rf.regs[PC].val += 2;
            rf.set(A, mem->get(imm16));     // NOTE: this corresponds to LD A,[n16] of rgbds gb docs. There's no guard for addr (ff00 -> ffff) like in the ldh instrs
        } else if (match(cmd, "11101000")) {                // add sp, imm8
            print ("    detected: add sp, imm8\n");

            int8_t imm8 = mem->get(rf.get(PC));     // signed imm8
            rf.regs[PC].val++;

            // FIXME: does this work? Passing in rf.get(SP) : 16 bit value into 8 bit compute add flags
            compute_flags_add8(rf.get(SP), imm8, ExcludeFlags{.no_z = true});
            rf.regs[AF].flags.z = 0;

            rf.set(SP, rf.get(SP) + imm8);
        } else if (match(cmd, "11111000")) {                // ld hl, sp + imm8
            print ("    detected: ld hl, sp + imm8\n");

            int8_t imm8 = mem->get(rf.get(PC));     // signed imm8]
            rf.regs[PC].val++;

            // FIXME: does this work? Passing in rf.get(SP) : 16 bit value into 8 bit compute add flags
            compute_flags_add8(rf.get(SP), imm8, ExcludeFlags{.no_z = true});
            rf.regs[AF].flags.z = 0;

            rf.set(HL, rf.get(SP) + imm8);
        } else if (match(cmd, "11111001")) {                // ld sp, hl
            print ("    detected: ld sp, hl\n");
            
            rf.set(SP, rf.get(HL));
        } else if (match(cmd, "11110011")) {                // di
            print ("    detected: di\n");

            mem->mmio->clear_ime();
        } else if (match(cmd, "11111011")) {                // ei
            print ("    detected: ei\n");

            mem->mmio->set_ime();
        } 

        else {
            std::set<uint8_t> hardlock_ops = {
                0xD3, 0xDB, 0xDD, 0xE3, 0xE4, 0xEB, 0xEC, 0xED, 0xF4, 0xFC, 0xFD
            };
            if (hardlock_ops.count(cmd)) {
                std::cerr << "Detected Hard Lock Opcode: 0x" << std::hex << cmd << std::endl;
                std::exit(EXIT_FAILURE);
            }

            printx ("    default case. Opcode: 0x%0x\n", cmd);
        }

        // Zero out lower nibble of Flags reg
        rf.regs[AF].lo &= 0b11110000;


        if (!nop && !GAMEBOY_DOCTOR) rf.print_regs();

        serial_output();

        check_and_handle_interrupts();
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

    bool add16_overflowFromBitX(int bit, uint16_t a=0, uint16_t b=0, uint16_t c=0) {
        if (bit > 15) {
            printx ("Invalid bit provided to add16_overflow\n");
            std::exit(EXIT_FAILURE);
        }

        // Mask out bits other than X to 0.
        a &= ((1 << (bit+1)) - 1);
        b &= ((1 << (bit+1)) - 1);
        c &= ((1 << (bit+1)) - 1);

        // Add result --> if bit X+1 got set that means it carried through bit X
        int result = a + b + c;
        if ((result >> (bit+1)) & 0x1) {
            return true;
        }
        
        return false;
    }

    bool sub_borrowFromBitX(int bit,  uint16_t A=0, uint16_t b=0, uint16_t c=0) {
        if (bit < 0 || bit > 15) {
            printx ("Invalid bit=%d provided to sub_borrowFromBitX\n", bit);
            std::exit(EXIT_FAILURE);
        }

        uint16_t mask = ((1 << bit) - 1);

        // Mask out bits other than X-1 to 0
        if ((A & mask) < (b & mask)) return true;

        A = A - b;

        if ((A & mask) < (c & mask)) return true;

        return false;
    }

    void compute_flags_add8(int a, int b, ExcludeFlags ex = ExcludeFlags()) {
        if (!ex.no_z) rf.regs[AF].flags.z = ((((a & 0xFF) + (b & 0xFF)) & 0xFF) == 0);
        if (!ex.no_n) rf.regs[AF].flags.n = 0;
        if (!ex.no_h) rf.regs[AF].flags.h = (((a & 0x0F) + (b & 0x0F)) > 0x0F);   // carry at bit 3?
        if (!ex.no_c) rf.regs[AF].flags.c = (((a & 0xFF) + (b & 0xFF)) > 0xFF);   // carry at bit 7?
    }

    void compute_flags_sub8(int a, int b, ExcludeFlags ex = ExcludeFlags()) {
        if (!ex.no_z) rf.regs[AF].flags.z = ((((a & 0xFF) - (b & 0xFF)) & 0xFF) == 0);
        if (!ex.no_n) rf.regs[AF].flags.n = 1;
        if (!ex.no_h) rf.regs[AF].flags.h = ((b & 0xF) > (a & 0xF));              // borrow from bit 4?
        if (!ex.no_c) rf.regs[AF].flags.c = ((b & 0xFF) > (a & 0xFF));            // borrow (i.e r8 > A)?
    }

    bool get_block2_3_operand(uint8_t cmd, uint8_t &operand) {
        bool imm_or_reg;
        if ((cmd >> 6) & 0b1) {
            operand = mem->get(rf.regs[PC].val);     // imm8
            imm_or_reg = true;
            rf.regs[PC].val += 1;
        } else {
            int reg_bits = (cmd & 0b111);
            r8_index_t r8 = RegFile::get_r8(reg_bits);
            operand = rf.get(r8);
            imm_or_reg = false;
        }

        return imm_or_reg;
    }

    void pop_r16_stack(r16_index_t r16) {
        // load [SP] into r16       (in little endian)
        rf.regs[r16].lo = mem->get(rf.get(SP));
        rf.set(SP, rf.get(SP) + 1);
        rf.regs[r16].hi = mem->get(rf.get(SP));
        rf.set(SP, rf.get(SP) + 1);

        // // increment SP (stack grows from high mem to low mem - so incrementing head of the stack shrinks the stack)
        // rf.set(SP, rf.get(SP) + 2);
    }

    /**
     * Higher addresses
        0xffff  | ff
        0xfffe  | cd
        0xfffd  | ab  <---- SP (retn addr abcd)
        0xfffc  | ff
        .
        .
        .
        0x0000
     * Lower addresses
     */

    void push_val_stack(uint16_t val) {

        // Write retn addr to stack then update stack pointer (grows from high addr to low addr)
        // 1. Write lower byte of retn addr to SP - 1
        // mem->set(rf.get(SP) - 1, (val & 0xff));
        // // 2. Write higher byte of retn addr to SP - 2
        // mem->set(rf.get(SP) - 1, ((val >> 8) & 0xff));
        // // 3. Set SP = SP - 2;
        // rf.set(SP, rf.get(SP) - 2);

        rf.set(SP, rf.get(SP) - 1);   // decrement SP
        mem->set(rf.get(SP), (val >> 8) & 0xff);
        rf.set(SP, rf.get(SP) - 1);   // decrement SP
        mem->set(rf.get(SP), val & 0xff);
    }

    void check_and_handle_interrupts() {
        uint16_t intr_handler_addr;

        if (mem->mmio->check_interrupts(intr_handler_addr)) {
            push_val_stack(rf.regs[PC].val);
            rf.regs[PC].val = intr_handler_addr; 
        }
    }
    
    //    blarggs test - serial output
    void serial_output() {
        if (mem->get(0xff02) == 0x81) {
            char c = static_cast<char>(mem->get(0xff01));
            std::clog << c;
            // printx ("%c", c);
            // memory[0xff02] = 0x0;
            mem->set(0xff02, 1);
        }    
    }
};