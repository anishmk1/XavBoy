// CPU.h
#ifndef CPU_H
#define CPU_H

const bool USE_R8_INDEX = true;
const int NUM_REGS = 6;

struct ExcludeFlags {
    bool no_z = false;
    bool no_n = false;
    bool no_h = false;
    bool no_c = false;
};

enum r16_index_t {AF,BC,DE,HL,SP,PC};
enum r8_index_t {A, B, C, D, E, H, L, hl, F};
enum r16mem_index_t {_BC, _DE, _HLi, _HLd};
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

typedef struct {
    bool interrupt_valid;
    uint16_t handler_addr;
    int wait_cycles;
} InterruptInfo;

// class MIO;
class Memory;

class RegFile {
public:
    Reg regs[NUM_REGS];
    Memory *mem;

    uint16_t debug0;
    uint16_t debug1;
    uint16_t debug2;

    uint16_t get(r16_index_t r16);
    uint8_t get(r8_index_t r8);
    uint16_t get(r16mem_index_t r16mem, bool modify=false);
    void set(r8_index_t r8, uint8_t val);
    void set(r16_index_t r16, uint16_t val);
    bool check_cc(cc_t cc);
    static r8_index_t get_r8(int index);
    static r16_index_t get_r16(int index);
    static r16mem_index_t get_r16mem(int index);
    static r16_index_t get_r16stk(int index);
    static const char* get_reg_name(r8_index_t r8);
    static const char* get_reg_name(r16_index_t r16);
    void print_regs();
};

class CPU {

public:
    Memory *mem;
    RegFile rf;

    CPU (Memory *mem);
    int execute(bool& halt_cpu);

private:
    InterruptInfo intrpt_info;

    bool match(uint8_t cmd, const char *compare);
    bool add16_overflowFromBitX(int bit, uint16_t a=0, uint16_t b=0, uint16_t c=0);
    bool sub_borrowFromBitX(int bit,  uint16_t A=0, uint16_t b=0, uint16_t c=0);
    void compute_flags_add8(int a, int b, ExcludeFlags ex = ExcludeFlags());
    void compute_flags_sub8(int a, int b, ExcludeFlags ex = ExcludeFlags());
    bool get_block2_3_operand(uint8_t cmd, uint8_t &operand, int &mcycles);
    void pop_r16_stack(r16_index_t r16);
    void push_val_stack(uint16_t val);
    bool handle_interrupts();
    bool set_new_interrupts();
    void serial_output();
};

#endif // CPU_H