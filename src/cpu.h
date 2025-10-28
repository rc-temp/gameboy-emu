#include <functional>
#include <vector>
#include <array>

struct Registers {
    // TODO: can this be cleaned up to use bit flags for the F register?
    union {
        uint16_t AF;
        struct {
            uint8_t F;
            uint8_t A;
        };
    };

    union {
        uint16_t BC;
        struct {
            uint8_t C;
            uint8_t B;
        };
    };

    union {
        uint16_t DE;
        struct {
            uint8_t E;
            uint8_t D;
        };
    };

    union {
        uint16_t HL;
        struct {
            uint8_t L;
            uint8_t H;
        };
    };

    uint16_t SP;
    uint16_t PC;

    int get_z() {
        return (F >> 7) & 1;
    }

    int get_n() {
        return (F >> 6) & 1;
    }

    int get_h() {
        return (F >> 5) & 1;
    }

    int get_c() {
        return (F >> 4) & 1;
    }

    void set_z(int cond) {
        if (cond)
            F |= 1 << 7;
        else
            F &= 0xF0 ^ (1 << 7);
    }

    void set_n(int cond) {
        if (cond)
            F |= 1 << 6;
        else
            F &= 0xF0 ^ (1 << 6);
    }

    void set_h(int cond) {
        if (cond)
            F |= 1 << 5;
        else
            F &= 0xF0 ^ (1 << 5);
    }

    void set_c(int cond) {
        if (cond)
            F |= 1 << 4;
        else
            F &= 0xF0 ^ (1 << 4);
    }

};

struct r8ptr_t {
    void* ptr;
    bool is_HL;
};

class Gameboy;
class CPU {
 public:
    Gameboy* gameboy;
    Registers registers;

    CPU();
    std::vector<uint8_t> fetch();
    int execute(std::vector<uint8_t> instr);
    void init(bool skip_boot_rom);
    void print_state();
    void handle_interrupts();


 private:
    bool IME;
    int set_IME_delay;

    void write_mmu_16(int address, uint16_t val);
    uint16_t read_mmu_16(int address);

    uint8_t read_r8(r8ptr_t r8);
    void write_r8(r8ptr_t r8, uint8_t val);
    r8ptr_t get_r8(int index);
    uint16_t* get_r16(int index);
    uint16_t* get_r16stk(int index);
    uint16_t get_r16mem(int index);
    bool get_cond(int index);

    int nop();
    int ld_r16_imm16(uint16_t* r16, uint16_t imm16);
    int ld_r16mem_a(uint16_t r16mem);
    int ld_a_r16mem(uint16_t r16mem);
    int ld_imm16_sp(uint16_t imm16);
    int inc_r16(uint16_t* r16);
    int dec_r16(uint16_t* r16);
    int add_hl_r16(uint16_t* r16);
    int inc_r8(r8ptr_t r8ptr);
    int dec_r8(r8ptr_t r8ptr);
    int ld_r8_imm8(r8ptr_t r8ptr, uint8_t imm8);
    int rlca();
    int rrca();
    int rla();
    int rra();
    int daa();
    int cpl();
    int scf();
    int ccf();
    int jr_imm8(uint8_t imm8);
    int jr_cond_imm8(bool cond, uint8_t imm8);
    int stop();
    int ld_r8_r8(r8ptr_t aptr, r8ptr_t bptr);
    int halt();
    int add_a_r8(r8ptr_t r8ptr);
    int adc_a_r8(r8ptr_t r8ptr);
    int sub_a_r8(r8ptr_t r8ptr);
    int sbc_a_r8(r8ptr_t r8ptr);
    int and_a_r8(r8ptr_t r8ptr);
    int xor_a_r8(r8ptr_t r8ptr);
    int or_a_r8(r8ptr_t r8ptr);
    int cp_a_r8(r8ptr_t r8ptr);

    int add_a_imm8(uint8_t imm8);
    int adc_a_imm8(uint8_t imm8);
    int sub_a_imm8(uint8_t imm8);
    int sbc_a_imm8(uint8_t imm8);
    int and_a_imm8(uint8_t imm8);
    int xor_a_imm8(uint8_t imm8);
    int or_a_imm8(uint8_t imm8);
    int cp_a_imm8(uint8_t imm8);

    int ret_cond(bool cond);
    int ret();
    int reti();
    int jp_cond_imm16(bool cond, uint16_t imm16);
    int jp_imm16(uint16_t imm16);
    int jp_hl();
    int call_cond_imm16(bool cond, uint16_t imm16);
    int call_imm16(uint16_t imm16);
    int rst_tgt3(int tgt3);

    int pop_r16stk(uint16_t* r16);
    int push_r16stk(uint16_t* r16);

    int execute_CB(std::vector<uint8_t> instr);

    int rlc_r8(r8ptr_t r8ptr);
    int rrc_r8(r8ptr_t r8ptr);
    int rl_r8(r8ptr_t r8ptr);
    int rr_r8(r8ptr_t r8ptr);
    int sla_r8(r8ptr_t r8ptr);
    int sra_r8(r8ptr_t r8ptr);
    int swap_r8(r8ptr_t r8ptr);
    int srl_r8(r8ptr_t r8ptr);

    int bit_b3_r8(int b3, r8ptr_t r8ptr);
    int res_b3_r8(int b3, r8ptr_t r8ptr);
    int set_b3_r8(int b3, r8ptr_t r8ptr);


    int ldh_cmem_a();
    int ldh_imm8_a(uint8_t imm8);
    int ld_imm16_a(uint16_t imm16);
    int ldh_a_cmem();
    int ldh_a_imm8(uint8_t imm8);
    int ld_a_imm16(uint16_t imm16);

    int add_sp_imm8(uint8_t imm8);
    int ld_hl_sppimm8(uint8_t imm8);
    int ld_sp_hl();

    int di();
    int ei();
};
