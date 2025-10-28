#include <array>
#include <string>
#include <vector>
#include <iostream>

#include "gameboy-emu.h"
#include "cpu.h"

#include <chrono>
#include <thread>

#include <stdexcept>

// TODO:
// - implement TODOs
// - check the carry arithmetic formulas

const std::array<int, 256> instruction_length = {
    1, 3, 1, 1, 1, 1, 2, 1, 3, 1, 1, 1, 1, 1, 2, 1,
    2, 3, 1, 1, 1, 1, 2, 1, 2, 1, 1, 1, 1, 1, 2, 1,
    2, 3, 1, 1, 1, 1, 2, 1, 2, 1, 1, 1, 1, 1, 2, 1,
    2, 3, 1, 1, 1, 1, 2, 1, 2, 1, 1, 1, 1, 1, 2, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 3, 3, 3, 1, 2, 1, 1, 1, 3, 1, 3, 3, 2, 1,
    1, 1, 3, 0, 3, 1, 2, 1, 1, 1, 3, 0, 3, 0, 2, 1,
    2, 1, 2, 0, 0, 1, 2, 1, 2, 1, 3, 0, 0, 0, 2, 1,
    2, 1, 2, 1, 0, 1, 2, 1, 2, 1, 3, 1, 0, 0, 2, 1,
};

CPU::CPU() {
    IME = false;
    set_IME_delay = 0;
}

int IE_ADDRESS = 0xFFFF;
int IF_ADDRESS = 0xFF0F;
int VBLANK_BIT = 0b1;
int LCD_BIT = 0b01;
int TIMER_BIT = 0b001;
int SERIAL_BIT = 0b0001;
int JOYPAD_BIT = 0b00001;

void CPU::handle_interrupts() {
    if (IME) {
        int IE = gameboy->read_mmu(IE_ADDRESS);
        int IF = gameboy->read_mmu(IF_ADDRESS);

        int flags = IE & IF;

        if (flags & VBLANK_BIT) {
            gameboy->write_mmu(IF_ADDRESS, IF ^ VBLANK_BIT);
            IME = false;
            registers.SP -= 2;
            write_mmu_16(registers.SP, registers.PC);
            registers.PC = 0x40;
            return;
        } else if (flags & LCD_BIT) {
            gameboy->write_mmu(IF_ADDRESS, IF ^ LCD_BIT);
            IME = false;
            registers.SP -= 2;
            write_mmu_16(registers.SP, registers.PC);
            registers.PC = 0x48;
            return;
        } else if (flags & TIMER_BIT) {
            gameboy->write_mmu(IF_ADDRESS, IF ^ TIMER_BIT);
            IME = false;
            registers.SP -= 2;
            write_mmu_16(registers.SP, registers.PC);
            registers.PC = 0x50;
            return;
        } else if (flags & SERIAL_BIT) {
            gameboy->write_mmu(IF_ADDRESS, IF ^ SERIAL_BIT);
            IME = false;
            registers.SP -= 2;
            write_mmu_16(registers.SP, registers.PC);
            registers.PC = 0x58;
            return;
        } else if (flags & JOYPAD_BIT) {
            gameboy->write_mmu(IF_ADDRESS, IF ^ JOYPAD_BIT);
            IME = false;
            registers.SP -= 2;
            write_mmu_16(registers.SP, registers.PC);
            registers.PC = 0x60;
            return;
        }
    }
}


std::vector<uint8_t> CPU::fetch() {
    auto opcode = gameboy->read_mmu(this->registers.PC);
    int num_bytes;
    if (opcode == 0xCB)
        num_bytes = 2;
    else
        num_bytes = instruction_length.at(opcode);
    std::vector<uint8_t> instruction(num_bytes);
    for (int i = 0; i < num_bytes; i++) {
        instruction.at(i) = gameboy->read_mmu(this->registers.PC+i);
    }
    return instruction;
}


r8ptr_t CPU::get_r8(int index) {
    if (index == 0)
        return {&registers.B, false};
    else if (index == 1)
        return {&registers.C, false};
    else if (index == 2)
        return {&registers.D, false};
    else if (index == 3)
        return {&registers.E, false};
    else if (index == 4)
        return {&registers.H, false};
    else if (index == 5)
        return {&registers.L, false};
    else if (index == 6)
        return {&registers.HL, true};
    else if (index == 7)
        return {&registers.A, false};
    else
        throw std::runtime_error("index should be less than 8");
    return {NULL, false};
}

uint8_t CPU::read_r8(r8ptr_t r8) {
    uint8_t val;
    if (r8.is_HL) {
        val = gameboy->read_mmu(*(uint16_t*)r8.ptr);
    } else {
        val = *(uint8_t*)r8.ptr;
    }
    return val;
}

void CPU::write_r8(r8ptr_t r8, uint8_t val) {
    if (r8.is_HL) {
        gameboy->write_mmu(*(uint16_t*)r8.ptr, val);
    } else {
        *(uint8_t*)r8.ptr = val;
    }
}

void CPU::write_mmu_16(int address, uint16_t val) {
    gameboy->write_mmu(address, val & 0xFF);
    gameboy->write_mmu(address + 1, val >> 8);
}

uint16_t CPU::read_mmu_16(int address) {
    uint16_t val = 0;
    val |= gameboy->read_mmu(address);
    val |= gameboy->read_mmu(address + 1) << 8;
    return val;
}

uint16_t* CPU::get_r16(int index) {
    if (index == 0)
        return &registers.BC;
    else if (index == 1)
        return &registers.DE;
    else if (index == 2)
        return &registers.HL;
    else if (index == 3)
        return &registers.SP;
    else
        throw std::runtime_error("index should be less than 4");
    return NULL;
}

uint16_t* CPU::get_r16stk(int index) {
    if (index == 0)
        return &registers.BC;
    else if (index == 1)
        return &registers.DE;
    else if (index == 2)
        return &registers.HL;
    else if (index == 3)
        return &registers.AF;
    else
        throw std::runtime_error("index should be less than 4");
    return NULL;
}

uint16_t CPU::get_r16mem(int index) {
    if (index == 0)
        return registers.BC;
    else if (index == 1)
        return registers.DE;
    else if (index == 2) {
        return registers.HL++;
    } else if (index == 3) {
        return registers.HL--;
    } else
        throw std::runtime_error("index should be less than 4");
}

bool CPU::get_cond(int index) {
    if (index == 0)
        return !registers.get_z();
    else if (index == 1)
        return registers.get_z();
    else if (index == 2)
        return !registers.get_c();
    else if (index == 3)
        return registers.get_c();
    else
        throw std::runtime_error("index should be less than 4");
    return false;
}


int CPU::nop() {
    registers.PC += 1;
    return 4;
}

int CPU::ld_r16_imm16(uint16_t* r16, uint16_t imm16) {
    *r16 = imm16;
    registers.PC += 3;
    return 12;
}

int CPU::ld_r16mem_a(uint16_t r16mem) {
    gameboy->write_mmu(r16mem, registers.A);
    registers.PC += 1;
    return 8;
}

int CPU::ld_a_r16mem(uint16_t r16mem) {
    registers.A = gameboy->read_mmu(r16mem);
    registers.PC += 1;
    return 8;
}

int CPU::ld_imm16_sp(uint16_t imm16) {
    write_mmu_16(imm16, registers.SP);
    registers.PC += 3;
    return 20;
}

int CPU::inc_r16(uint16_t* r16) {
    *r16 += 1;
    registers.PC += 1;
    return 8;
}

int CPU::dec_r16(uint16_t* r16) {
    *r16 -= 1;
    registers.PC += 1;
    return 8;
}

int CPU::add_hl_r16(uint16_t* r16) {
    int temp = registers.HL;
    // stow it in r16temp in case r16 is HL
    int r16temp = *r16;
    registers.HL += *r16;
    registers.set_n(0);
    registers.set_h((((temp & 0xFFF) + (r16temp & 0xFFF)) & 0x1000) == 0x1000);
    registers.set_c(((temp + r16temp) & 0x10000) == 0x10000);
    registers.PC += 1;
    return 8;
}


int CPU::inc_r8(r8ptr_t r8ptr) {
    uint8_t r8 = read_r8(r8ptr);
    r8 += 1;
    registers.set_z(r8 == 0);
    registers.set_n(0);
    registers.set_h((r8 & 0xF) == 0);
    registers.PC += 1;
    write_r8(r8ptr, r8);
    return 4 + (r8ptr.is_HL ? 8 : 0);
}

int CPU::dec_r8(r8ptr_t r8ptr) {
    uint8_t r8 = read_r8(r8ptr);
    r8 -= 1;
    registers.set_z(r8 == 0);
    registers.set_n(1);
    registers.set_h((r8 & 0xF) == 0xF);
    registers.PC += 1;
    write_r8(r8ptr, r8);
    return 4 + (r8ptr.is_HL ? 8 : 0);
}

int CPU::ld_r8_imm8(r8ptr_t r8ptr, uint8_t imm8) {
    registers.PC += 2;
    write_r8(r8ptr, imm8);
    return 8 + (r8ptr.is_HL ? 4 : 0);
}

int CPU::rlca() {
    registers.A = ((registers.A << 1) & 0xFF) | ((registers.A >> 7) & 1);
    registers.set_z(0);
    registers.set_n(0);
    registers.set_h(0);
    registers.set_c((registers.A & 1));
    registers.PC += 1;
    return 4;
}

int CPU::rrca() {
    registers.A = (registers.A >> 1) | ((registers.A & 1) << 7);
    registers.set_z(0);
    registers.set_n(0);
    registers.set_h(0);
    registers.set_c((registers.A >> 7) & 1);
    registers.PC += 1;
    return 4;
}

int CPU::rla() {
    int temp = registers.get_c();
    registers.set_c((registers.A >> 7) & 1);
    registers.A = ((registers.A << 1) & 0xFF) | temp;
    registers.set_z(0);
    registers.set_n(0);
    registers.set_h(0);
    registers.PC += 1;
    return 4;
}

int CPU::rra() {
    int temp = registers.get_c();
    registers.set_c(registers.A & 1);
    registers.A = (registers.A >> 1) | (temp << 7);
    registers.set_z(0);
    registers.set_n(0);
    registers.set_h(0);
    registers.PC += 1;
    return 4;
}

int CPU::daa() {
    // reference: https://forums.nesdev.org/viewtopic.php?p=196282&sid=bc6eb4505d5429dcb3f418c7836e607e#p196282
    // TODO: explain this code better

    // rough explanation:

    // point 1
    // ADD r1 r2 r3
    // point 2
    // DAA r1
    // point 3

    // format: <high 4 bits, low 4 bits>
    // point 1: r2=<a, b>, r3=<c, d>
    // point 2: r1=<a+c+(b+d)//16 mod 16, b+d mod 16>
    // point 3: r1=<a+c+(b+d)//10 mod 10, b+d mod 10>

    // given b+d mod 16, we want b+d mod 10
    // if   b+d<10 => no change
    // elif b+d<16 => add 6
    // else        => add 6

    // similar for second digit

    if (!registers.get_n()) {
        if (registers.get_c() || registers.A > 0x99) {
            registers.A += 0x60;
            registers.set_c(1);
        }
        if (registers.get_h() || (registers.A & 0x0F) > 0x09) {
            registers.A += 0x06;
        }
    } else {
        if (registers.get_c()) {
            registers.A -= 0x60;
        }
        if (registers.get_h()) {
            registers.A -= 0x06;
        }
    }

    registers.set_z(registers.A == 0);
    registers.set_h(0);
    registers.PC += 1;
    return 4;
}

int CPU::cpl() {
    registers.A = ~registers.A & 0xFF;
    registers.set_n(1);
    registers.set_h(1);
    registers.PC += 1;
    return 4;
}

int CPU::scf() {
    registers.set_n(0);
    registers.set_h(0);
    registers.set_c(1);
    registers.PC += 1;
    return 4;
}

int CPU::ccf() {
    registers.set_n(0);
    registers.set_h(0);
    registers.set_c(~registers.get_c() & 1);
    registers.PC += 1;
    return 4;
}

int CPU::jr_imm8(uint8_t imm8) {
    registers.PC += (int8_t)imm8;
    registers.PC += 2;
    return 12;
}

int CPU::jr_cond_imm8(bool cond, uint8_t imm8) {
    if (cond) {
        registers.PC += (int8_t)imm8;
        registers.PC += 2;
        return 12;
    } else {
        registers.PC += 2;
        return 8;
    }
}

int CPU::stop() {
    throw std::runtime_error("this has not been implemented yet");
    return -1;
}

int CPU::ld_r8_r8(r8ptr_t aptr, r8ptr_t bptr) {
    uint8_t b = read_r8(bptr);
    registers.PC += 1;
    write_r8(aptr, b);
    return 4 + (aptr.is_HL ? 4 : 0) + (bptr.is_HL ? 4 : 0);
}

int CPU::halt() {
    throw std::runtime_error("this has not been implemented yet");
    return -1;
}

int CPU::add_a_r8(r8ptr_t r8ptr) {
    uint8_t r8 = read_r8(r8ptr);
    int temp = registers.A;
    uint8_t r8temp = r8;
    registers.A += r8;
    registers.set_z(registers.A == 0);
    registers.set_n(0);
    registers.set_h((((temp & 0xF) + (r8temp & 0xF)) & 0x10) == 0x10);
    registers.set_c(((temp + r8temp) & 0x100) == 0x100);
    registers.PC += 1;
    return 4 + (r8ptr.is_HL ? 4 : 0);
}

int CPU::adc_a_r8(r8ptr_t r8ptr) {
    uint8_t r8 = read_r8(r8ptr);
    int temp = registers.A;
    uint8_t r8temp = r8;
    registers.A += r8 + registers.get_c();
    registers.set_z(registers.A == 0);
    registers.set_n(0);
    registers.set_h((((temp & 0xF) + (r8temp & 0xF) + registers.get_c()) & 0x10) == 0x10);
    registers.set_c(((temp + r8temp + registers.get_c()) & 0x100) == 0x100);
    registers.PC += 1;
    return 4 + (r8ptr.is_HL ? 4 : 0);
}

int CPU::sub_a_r8(r8ptr_t r8ptr) {
    uint8_t r8 = read_r8(r8ptr);
    int temp = registers.A;
    uint8_t r8temp = r8;
    registers.A -= r8;
    registers.set_z(registers.A == 0);
    registers.set_n(1);
    registers.set_h((((temp & 0xF) - (r8temp & 0xF)) & 0x10) == 0x10);
    registers.set_c(((temp - r8temp) & 0x100) == 0x100);
    registers.PC += 1;
    return 4 + (r8ptr.is_HL ? 4 : 0);
}

int CPU::sbc_a_r8(r8ptr_t r8ptr) {
    uint8_t r8 = read_r8(r8ptr);
    int temp = registers.A;
    uint8_t r8temp = r8;
    registers.A -= r8 + registers.get_c();
    registers.set_z(registers.A == 0);
    registers.set_n(1);
    registers.set_h((((temp & 0xF) - (r8temp & 0xF) - registers.get_c()) & 0x10) == 0x10);
    registers.set_c(((temp - r8temp - registers.get_c()) & 0x100) == 0x100);
    registers.PC += 1;
    return 4 + (r8ptr.is_HL ? 4 : 0);
}

int CPU::and_a_r8(r8ptr_t r8ptr) {
    uint8_t r8 = read_r8(r8ptr);
    registers.A &= r8;
    registers.set_z(registers.A == 0);
    registers.set_n(0);
    registers.set_h(1);
    registers.set_c(0);
    registers.PC += 1;
    return 4 + (r8ptr.is_HL ? 4 : 0);
}

int CPU::xor_a_r8(r8ptr_t r8ptr) {
    uint8_t r8 = read_r8(r8ptr);
    registers.A ^= r8;
    registers.set_z(registers.A == 0);
    registers.set_n(0);
    registers.set_h(0);
    registers.set_c(0);
    registers.PC += 1;
    return 4 + (r8ptr.is_HL ? 4 : 0);
}

int CPU::or_a_r8(r8ptr_t r8ptr) {
    uint8_t r8 = read_r8(r8ptr);
    registers.A |= r8;
    registers.set_z(registers.A == 0);
    registers.set_n(0);
    registers.set_h(0);
    registers.set_c(0);
    registers.PC += 1;
    return 4 + (r8ptr.is_HL ? 4 : 0);
}

int CPU::cp_a_r8(r8ptr_t r8ptr) {
    uint8_t r8 = read_r8(r8ptr);
    uint8_t r8temp = r8;
    registers.set_z(registers.A == r8);
    registers.set_n(1);
    registers.set_h((((registers.A & 0xF) - (r8temp & 0xF)) & 0x10) == 0x10);
    registers.set_c(((registers.A - r8temp) & 0x100) == 0x100);
    registers.PC += 1;
    return 4 + (r8ptr.is_HL ? 4 : 0);
}




int CPU::add_a_imm8(uint8_t imm8) {
    int temp = registers.A;
    registers.A += imm8;
    registers.set_z(registers.A == 0);
    registers.set_n(0);
    registers.set_h((((temp & 0xF) + (imm8 & 0xF)) & 0x10) == 0x10);
    registers.set_c(((temp + imm8) & 0x100) == 0x100);
    registers.PC += 2;
    return 8;
}

int CPU::adc_a_imm8(uint8_t imm8) {
    int temp = registers.A;
    registers.A += imm8 + registers.get_c();
    registers.set_z(registers.A == 0);
    registers.set_n(0);
    registers.set_h((((temp & 0xF) + (imm8 & 0xF) + registers.get_c()) & 0x10) == 0x10);
    registers.set_c(((temp + imm8 + registers.get_c()) & 0x100) == 0x100);
    registers.PC += 2;
    return 8;
}

int CPU::sub_a_imm8(uint8_t imm8) {
    int temp = registers.A;
    registers.A -= imm8;
    registers.set_z(registers.A == 0);
    registers.set_n(1);
    registers.set_h((((temp & 0xF) - (imm8 & 0xF)) & 0x10) == 0x10);
    registers.set_c(((temp - imm8) & 0x100) == 0x100);
    registers.PC += 2;
    return 8;
}

int CPU::sbc_a_imm8(uint8_t imm8) {
    int temp = registers.A;
    registers.A -= imm8 + registers.get_c();
    registers.set_z(registers.A == 0);
    registers.set_n(1);
    registers.set_h((((temp & 0xF) - (imm8 & 0xF) - registers.get_c()) & 0x10) == 0x10);
    registers.set_c(((temp - imm8 - registers.get_c()) & 0x100) == 0x100);
    registers.PC += 2;
    return 8;
}

int CPU::and_a_imm8(uint8_t imm8) {
    registers.A &= imm8;
    registers.set_z(registers.A == 0);
    registers.set_n(0);
    registers.set_h(1);
    registers.set_c(0);
    registers.PC += 2;
    return 8;
}

int CPU::xor_a_imm8(uint8_t imm8) {
    registers.A ^= imm8;
    registers.set_z(registers.A == 0);
    registers.set_n(0);
    registers.set_h(0);
    registers.set_c(0);
    registers.PC += 2;
    return 8;
}

int CPU::or_a_imm8(uint8_t imm8) {
    registers.A |= imm8;
    registers.set_z(registers.A == 0);
    registers.set_n(0);
    registers.set_h(0);
    registers.set_c(0);
    registers.PC += 2;
    return 8;
}

int CPU::cp_a_imm8(uint8_t imm8) {
    registers.set_z(registers.A == imm8);
    registers.set_n(1);
    registers.set_h((((registers.A & 0xF) - (imm8 & 0xF)) & 0x10) == 0x10);
    registers.set_c(((registers.A - imm8) & 0x100) == 0x100);
    registers.PC += 2;
    return 8;
}



int CPU::ret_cond(bool cond) {
    if (cond) {
        registers.PC = read_mmu_16(registers.SP);
        registers.SP += 2;
        return 20;
    } else {
        registers.PC += 1;
        return 8;
    }
}

int CPU::ret() {
    registers.PC = read_mmu_16(registers.SP);
    registers.SP += 2;
    return 16;
}

int CPU::reti() {
    registers.PC = read_mmu_16(registers.SP);
    registers.SP += 2;
    set_IME_delay = 2;
    return 16;
}

int CPU::jp_cond_imm16(bool cond, uint16_t imm16) {
    if (cond) {
        registers.PC = imm16;
        return 16;
    } else {
        registers.PC += 3;
        return 12;
    }
}

int CPU::jp_imm16(uint16_t imm16) {
    registers.PC = imm16;
    return 16;
}

int CPU::jp_hl() {
    registers.PC = registers.HL;
    return 4;
}

int CPU::call_cond_imm16(bool cond, uint16_t imm16) {
    registers.PC += 3;
    if (cond) {
        registers.SP -= 2;
        write_mmu_16(registers.SP, registers.PC);
        registers.PC = imm16;
        return 24;
    } else {
        return 12;
    }
}

int CPU::call_imm16(uint16_t imm16) {
    registers.PC += 3;
    registers.SP -= 2;
    write_mmu_16(registers.SP, registers.PC);
    registers.PC = imm16;
    return 24;
}

int CPU::rst_tgt3(int tgt3) {
    registers.PC += 1;
    registers.SP -= 2;
    write_mmu_16(registers.SP, registers.PC);
    registers.PC = tgt3 * 8;
    return 16;
}


int CPU::pop_r16stk(uint16_t* r16) {
    *r16 = read_mmu_16(registers.SP);
    registers.SP += 2;
    registers.PC += 1;
    if (r16 == &registers.AF) {
        // r16 can be AF, which means we overwrite flags, which is weird
        // note that lowest 4 bits of F register should always be 0
        registers.F &= 0xF0;
    }
    return 12;
}

int CPU::push_r16stk(uint16_t* r16) {
    registers.PC += 1;
    registers.SP -= 2;
    write_mmu_16(registers.SP, *r16);
    return 16;
}

int CPU::rlc_r8(r8ptr_t r8ptr) {
    uint8_t r8 = read_r8(r8ptr);
    int temp = r8 >> 7;
    r8 <<= 1;
    r8 |= temp;
    registers.set_z(r8 == 0);
    registers.set_n(0);
    registers.set_h(0);
    registers.set_c(temp);
    write_r8(r8ptr, r8);
    return 8 + (r8ptr.is_HL ? 8 : 0);
}

int CPU::rrc_r8(r8ptr_t r8ptr) {
    uint8_t r8 = read_r8(r8ptr);
    int temp = r8 & 1;
    r8 >>= 1;
    r8 |= temp << 7;
    registers.set_z(r8 == 0);
    registers.set_n(0);
    registers.set_h(0);
    registers.set_c(temp);
    write_r8(r8ptr, r8);
    return 8 + (r8ptr.is_HL ? 8 : 0);
}

int CPU::rl_r8(r8ptr_t r8ptr) {
    uint8_t r8 = read_r8(r8ptr);
    int temp = r8 >> 7;
    r8 <<= 1;
    r8 |= registers.get_c();
    registers.set_z(r8 == 0);
    registers.set_n(0);
    registers.set_h(0);
    registers.set_c(temp);
    write_r8(r8ptr, r8);
    return 8 + (r8ptr.is_HL ? 8 : 0);
}

int CPU::rr_r8(r8ptr_t r8ptr) {
    uint8_t r8 = read_r8(r8ptr);
    int temp = r8 & 1;
    r8 >>= 1;
    r8 |= registers.get_c() << 7;
    registers.set_z(r8 == 0);
    registers.set_n(0);
    registers.set_h(0);
    registers.set_c(temp);
    write_r8(r8ptr, r8);
    return 8 + (r8ptr.is_HL ? 8 : 0);
}

int CPU::sla_r8(r8ptr_t r8ptr) {
    uint8_t r8 = read_r8(r8ptr);
    int temp = r8 >> 7;
    r8 <<= 1;
    registers.set_z(r8 == 0);
    registers.set_n(0);
    registers.set_h(0);
    registers.set_c(temp);
    write_r8(r8ptr, r8);
    return 8 + (r8ptr.is_HL ? 8 : 0);
}

int CPU::sra_r8(r8ptr_t r8ptr) {
    uint8_t r8 = read_r8(r8ptr);
    int temp = r8 & 1;
    // cast to signed to get sign extension
    *(int8_t*)&r8 >>= 1;
    registers.set_z(r8 == 0);
    registers.set_n(0);
    registers.set_h(0);
    registers.set_c(temp);
    write_r8(r8ptr, r8);
    return 8 + (r8ptr.is_HL ? 8 : 0);
}

int CPU::swap_r8(r8ptr_t r8ptr) {
    uint8_t r8 = read_r8(r8ptr);
    r8 = ((r8 & 0xF) << 4) | ((r8 & 0xF0) >> 4);
    registers.set_z(r8 == 0);
    registers.set_n(0);
    registers.set_h(0);
    registers.set_c(0);
    write_r8(r8ptr, r8);
    return 8 + (r8ptr.is_HL ? 8 : 0);
}

int CPU::srl_r8(r8ptr_t r8ptr) {
    uint8_t r8 = read_r8(r8ptr);
    int temp = r8 & 1;
    r8 >>= 1;
    registers.set_z(r8 == 0);
    registers.set_n(0);
    registers.set_h(0);
    registers.set_c(temp);
    write_r8(r8ptr, r8);
    return 8 + (r8ptr.is_HL ? 8 : 0);
}

int CPU::bit_b3_r8(int b3, r8ptr_t r8ptr) {
    uint8_t r8 = read_r8(r8ptr);
    registers.set_z(((r8 >> b3) & 1) == 0);
    registers.set_n(0);
    registers.set_h(1);
    write_r8(r8ptr, r8);
    return 8 + (r8ptr.is_HL ? 8 : 0);
}

int CPU::res_b3_r8(int b3, r8ptr_t r8ptr) {
    uint8_t r8 = read_r8(r8ptr);
    r8 &= 0xFF ^ (1 << b3);
    write_r8(r8ptr, r8);
    return 8 + (r8ptr.is_HL ? 8 : 0);
}

int CPU::set_b3_r8(int b3, r8ptr_t r8ptr) {
    uint8_t r8 = read_r8(r8ptr);
    r8 |= 1 << b3;
    write_r8(r8ptr, r8);
    return 8 + (r8ptr.is_HL ? 8 : 0);
}


int CPU::execute_CB(std::vector<uint8_t> instr) {
    uint8_t opcode = instr.at(1);

    int cycles;
    if ((opcode & 0b11111000) == 0b00000000) {
        cycles = rlc_r8(get_r8(opcode & 0b111));
    } else if ((opcode & 0b11111000) == 0b00001000) {
        cycles = rrc_r8(get_r8(opcode & 0b111));
    } else if ((opcode & 0b11111000) == 0b00010000) {
        cycles = rl_r8(get_r8(opcode & 0b111));
    } else if ((opcode & 0b11111000) == 0b00011000) {
        cycles = rr_r8(get_r8(opcode & 0b111));
    } else if ((opcode & 0b11111000) == 0b00100000) {
        cycles = sla_r8(get_r8(opcode & 0b111));
    } else if ((opcode & 0b11111000) == 0b00101000) {
        cycles = sra_r8(get_r8(opcode & 0b111));
    } else if ((opcode & 0b11111000) == 0b00110000) {
        cycles = swap_r8(get_r8(opcode & 0b111));
    } else if ((opcode & 0b11111000) == 0b00111000) {
        cycles = srl_r8(get_r8(opcode & 0b111));
    }

    else if ((opcode & 0b11000000) == 0b01000000) {
        cycles = bit_b3_r8((opcode >> 3) & 0b111, get_r8(opcode & 0b111));
    } else if ((opcode & 0b11000000) == 0b10000000) {
        cycles = res_b3_r8((opcode >> 3) & 0b111, get_r8(opcode & 0b111));
    } else if ((opcode & 0b11000000) == 0b11000000) {
        cycles = set_b3_r8((opcode >> 3) & 0b111, get_r8(opcode & 0b111));
    } else {
        throw std::runtime_error("opcode not found");
    }

    registers.PC += 2;

    return cycles;

}


int CPU::ldh_cmem_a() {
    gameboy->write_mmu(0xFF00 + registers.C, registers.A);
    registers.PC += 1;
    return 8;
}

int CPU::ldh_imm8_a(uint8_t imm8) {
    gameboy->write_mmu(0xFF00 + imm8, registers.A);
    registers.PC += 2;
    return 12;
}

int CPU::ld_imm16_a(uint16_t imm16) {
    gameboy->write_mmu(imm16, registers.A);
    registers.PC += 3;
    return 16;
}

int CPU::ldh_a_cmem() {
    registers.A = gameboy->read_mmu(0xFF00 + registers.C);
    registers.PC += 1;
    return 8;
}

int CPU::ldh_a_imm8(uint8_t imm8) {
    registers.A = gameboy->read_mmu(0xFF00 + imm8);
    registers.PC += 2;
    return 12;
}

int CPU::ld_a_imm16(uint16_t imm16) {
    registers.A = gameboy->read_mmu(imm16);
    registers.PC += 3;
    return 16;
}

int CPU::add_sp_imm8(uint8_t imm8) {
    uint8_t temp = registers.SP;
    registers.SP += (int8_t)imm8;
    registers.set_z(0);
    registers.set_n(0);
    registers.set_h((((temp & 0xF) + (imm8 & 0xF)) & 0x10) == 0x10);
    registers.set_c(((temp + imm8) & 0x100) == 0x100);
    registers.PC += 2;
    return 16;
}

int CPU::ld_hl_sppimm8(uint8_t imm8) {
    uint8_t temp = registers.SP;
    registers.HL = registers.SP + (int8_t)imm8;
    registers.set_z(0);
    registers.set_n(0);
    registers.set_h((((temp & 0xF) + (imm8 & 0xF)) & 0x10) == 0x10);
    registers.set_c(((temp + imm8) & 0x100) == 0x100);
    registers.PC += 2;
    return 12;
}

int CPU::ld_sp_hl() {
    registers.SP = registers.HL;
    registers.PC += 1;
    return 8;
}


int CPU::di() {
    IME = false;
    registers.PC += 1;
    return 4;
}

int CPU::ei() {
    set_IME_delay = 2;
    registers.PC += 1;
    return 4;
}

void CPU::print_state() {
    uint8_t F = registers.get_z() << 7 | registers.get_n() << 6 | registers.get_h() << 5 | registers.get_c() << 4;
    uint16_t sp = registers.SP;

    printf("A: %02X F: %02X B: %02X C: %02X D: %02X E: %02X H: %02X L: %02X SP: %04X PC: 00:%04X (%02X %02X %02X %02X)\n", registers.A, F, registers.B, registers.C, registers.D, registers.E, registers.H, registers.L, sp, registers.PC, gameboy->read_mmu(registers.PC), gameboy->read_mmu(registers.PC+1), gameboy->read_mmu(registers.PC+2), gameboy->read_mmu(registers.PC+3));

}

void CPU::init(bool skip_boot_rom) {
    if (skip_boot_rom) {
        registers.AF = 0x01B0;
        registers.BC = 0x0013;
        registers.DE = 0x00D8;
        registers.HL = 0x014D;
        registers.SP = 0xFFFE;
        registers.PC = 0x0100;
    } else {
        registers.AF = 0x0000;
        registers.BC = 0x0000;
        registers.DE = 0x0000;
        registers.HL = 0x0000;
        registers.SP = 0x0000;
        registers.PC = 0x0000;
    }
}

int CPU::execute(std::vector<uint8_t> instr) {
    uint8_t opcode = instr.at(0);

    int cycles;

    // BLOCK 0

    if (opcode == 0b00000000) {
        cycles = nop();
    }

    else if ((opcode & 0b11001111) == 0b00000001) {
        cycles = ld_r16_imm16(get_r16(opcode >> 4), *(uint16_t*)&instr.at(1));
    } else if ((opcode & 0b11001111) == 0b00000010) {
        cycles = ld_r16mem_a(get_r16mem(opcode >> 4));
    } else if ((opcode & 0b11001111) == 0b00001010) {
        cycles = ld_a_r16mem(get_r16mem(opcode >> 4));
    } else if (opcode == 0b00001000) {
        cycles = ld_imm16_sp(*(uint16_t*)&instr.at(1));
    }

    else if ((opcode & 0b11001111) == 0b00000011) {
        cycles = inc_r16(get_r16(opcode >> 4));
    } else if ((opcode & 0b11001111) == 0b00001011) {
        cycles = dec_r16(get_r16(opcode >> 4));
    } else if ((opcode & 0b11001111) == 0b00001001) {
        cycles = add_hl_r16(get_r16(opcode >> 4));
    }

    else if ((opcode & 0b11000111) == 0b00000100) {
        cycles = inc_r8(get_r8(opcode >> 3));
    } else if ((opcode & 0b11000111) == 0b00000101) {
        cycles = dec_r8(get_r8(opcode >> 3));
    }

    else if ((opcode & 0b11000111) == 0b00000110) {
        cycles = ld_r8_imm8(get_r8(opcode >> 3), instr.at(1));
    }

    else if (opcode == 0b00000111) {
        cycles = rlca();
    } else if (opcode == 0b00001111) {
        cycles = rrca();
    } else if (opcode == 0b00010111) {
        cycles = rla();
    } else if (opcode == 0b00011111) {
        cycles = rra();
    } else if (opcode == 0b00100111) {
        cycles = daa();
    } else if (opcode == 0b00101111) {
        cycles = cpl();
    } else if (opcode == 0b00110111) {
        cycles = scf();
    } else if (opcode == 0b00111111) {
        cycles = ccf();
    }

    else if (opcode == 0b00011000) {
        cycles = jr_imm8(instr.at(1));
    } else if ((opcode & 0b11100111) == 0b00100000) {
        cycles = jr_cond_imm8(get_cond((opcode >> 3) & 0b11), instr.at(1));
    }

    else if (opcode == 0b00010000) {
        cycles = stop();
    }

    // BLOCK 1

    else if ((opcode & 0b11000000) == 0b01000000) {
        cycles = ld_r8_r8(get_r8((opcode >> 3) & 0b111), get_r8(opcode & 0b111));
    }

    else if (opcode == 0b01110110) {
        cycles = halt();
    }

    // BLOCK 2

    else if ((opcode & 0b11111000) == 0b10000000) {
        cycles = add_a_r8(get_r8(opcode & 0b111));
    } else if ((opcode & 0b11111000) == 0b10001000) {
        cycles = adc_a_r8(get_r8(opcode & 0b111));
    } else if ((opcode & 0b11111000) == 0b10010000) {
        cycles = sub_a_r8(get_r8(opcode & 0b111));
    } else if ((opcode & 0b11111000) == 0b10011000) {
        cycles = sbc_a_r8(get_r8(opcode & 0b111));
    } else if ((opcode & 0b11111000) == 0b10100000) {
        cycles = and_a_r8(get_r8(opcode & 0b111));
    } else if ((opcode & 0b11111000) == 0b10101000) {
        cycles = xor_a_r8(get_r8(opcode & 0b111));
    } else if ((opcode & 0b11111000) == 0b10110000) {
        cycles = or_a_r8(get_r8(opcode & 0b111));
    } else if ((opcode & 0b11111000) == 0b10111000) {
        cycles = cp_a_r8(get_r8(opcode & 0b111));
    }

    // BLOCK 3

    else if (opcode == 0b11000110) {
        cycles = add_a_imm8(instr.at(1));
    } else if (opcode == 0b11001110) {
        cycles = adc_a_imm8(instr.at(1));
    } else if (opcode == 0b11010110) {
        cycles = sub_a_imm8(instr.at(1));
    } else if (opcode == 0b11011110) {
        cycles = sbc_a_imm8(instr.at(1));
    } else if (opcode == 0b11100110) {
        cycles = and_a_imm8(instr.at(1));
    } else if (opcode == 0b11101110) {
        cycles = xor_a_imm8(instr.at(1));
    } else if (opcode == 0b11110110) {
        cycles = or_a_imm8(instr.at(1));
    } else if (opcode == 0b11111110) {
        cycles = cp_a_imm8(instr.at(1));
    }

    else if ((opcode & 0b11100111) == 0b11000000) {
        cycles = ret_cond(get_cond((opcode >> 3) & 0b11));
    } else if (opcode == 0b11001001) {
        cycles = ret();
    } else if (opcode == 0b11011001) {
        cycles = reti();
    } else if ((opcode & 0b11100111) == 0b11000010) {
        cycles = jp_cond_imm16(get_cond((opcode >> 3) & 0b11), *(uint16_t*)&instr.at(1));
    } else if (opcode == 0b11000011) {
        cycles = jp_imm16(*(uint16_t*)&instr.at(1));
    } else if (opcode == 0b11101001) {
        cycles = jp_hl();
    } else if ((opcode & 0b11100111) == 0b11000100) {
        cycles = call_cond_imm16(get_cond((opcode >> 3) & 0b11), *(uint16_t*)&instr.at(1));
    } else if (opcode == 0b11001101) {
        cycles = call_imm16(*(uint16_t*)&instr.at(1));
    } else if ((opcode & 0b11000111) == 0b11000111) {
        cycles = rst_tgt3((opcode >> 3) & 0b111);
    }

    else if ((opcode & 0b11001111) == 0b11000001) {
        cycles = pop_r16stk(get_r16stk((opcode >> 4) & 0b11));
    } else if ((opcode & 0b11001111) == 0b11000101) {
        cycles = push_r16stk(get_r16stk((opcode >> 4) & 0b11));
    }

    else if (opcode == 0b11001011) {
        cycles = execute_CB(instr);
    }

    else if (opcode == 0b11100010) {
        cycles = ldh_cmem_a();
    } else if (opcode == 0b11100000) {
        cycles = ldh_imm8_a(instr.at(1));
    } else if (opcode == 0b11101010) {
        cycles = ld_imm16_a(*(uint16_t*)&instr.at(1));
    } else if (opcode == 0b11110010) {
        cycles = ldh_a_cmem();
    } else if (opcode == 0b11110000) {
        cycles = ldh_a_imm8(instr.at(1));
    } else if (opcode == 0b11111010) {
        cycles = ld_a_imm16(*(uint16_t*)&instr.at(1));
    }

    else if (opcode == 0b11101000) {
        cycles = add_sp_imm8(instr.at(1));
    } else if (opcode == 0b11111000) {
        cycles = ld_hl_sppimm8(instr.at(1));
    } else if (opcode == 0b11111001) {
        cycles = ld_sp_hl();
    }

    else if (opcode == 0b11110011) {
        cycles = di();
    } else if (opcode == 0b11111011) {
        cycles = ei();
    }



    else {
        throw std::runtime_error("opcode not found");
    }

    if (set_IME_delay > 0) {
        set_IME_delay--;
        if (set_IME_delay == 0) {
            IME = true;
        }
    }

    return cycles;
}
