#include <vector>
#include <array>
#include <fstream>
#include <string>
#include <cstdint>
#include <iostream>
#include "gameboy-emu.h"

#include "mmu.h"

MMU::MMU() {
    vram.resize(8192);
    eram.resize(8192);
    wram1.resize(4096);
    wram2.resize(4096);
    oam.resize(160);
    not_usable.resize(96);
    io_reg.resize(128);
    hram.resize(127);
}

void MMU::load_boot_rom(std::string filepath) {
    std::ifstream ifd(filepath, std::ios::binary | std::ios::ate);
    std::streamsize size = ifd.tellg();
    ifd.seekg(0, std::ios::beg);
    boot_rom.resize(size);
    ifd.read((char *)boot_rom.data(), size);
}

uint8_t MMU::read(int address) {
    if (address < 0x4000) {
        // 16 KiB ROM bank 00
        if (address < 0x100 && !read(0xFF50)) {
            return boot_rom.at(address);
        } else {
            return this->gameboy->read_cartridge(address);
        }
    } else if (address < 0x8000) {
        // 16 KiB ROM Bank 01–NN
        return this->gameboy->read_cartridge(address);
    } else if (address < 0xA000) {
        // 8 KiB Video RAM (VRAM)
        return vram.at(address - 0x8000);
    } else if (address < 0xC000) {
        // 8 KiB External RAM
        return eram.at(address - 0xA000);
    } else if (address < 0xD000) {
        // 4 KiB Work RAM (WRAM)
        return wram1.at(address - 0xC000);
    } else if (address < 0xE000) {
        // 4 KiB Work RAM (WRAM)
        return wram2.at(address - 0xD000);
    } else if (address < 0xFE00) {
        // Echo RAM (mirror of C000–DDFF)
        throw std::runtime_error("use of this area is prohibited: " + std::to_string(address));
    } else if (address < 0xFEA0) {
        // Object attribute memory (OAM)
        return oam.at(address - 0xFE00);
    } else if (address < 0xFF00) {
        // Not Usable
        // not sure what GB hardware typically does with this
        return 0xFF;
    } else if (address < 0xFF80) {
        // I/O Registers
        if (address == 0xFF00)
            return 0x0F;
        return io_reg.at(address - 0xFF00);
    } else if (address < 0xFFFF) {
        // High RAM (HRAM)
        return hram.at(address - 0xFF80);
    } else {
        // Interrupt Enable register (IE)
        return ie;
    }

    return -1;
}

void MMU::write(int address, uint8_t data) {
    if (address < 0x4000) {
        // 16 KiB ROM bank 00
        gameboy->write_cartridge(address, data);
    } else if (address < 0x8000) {
        // 16 KiB ROM Bank 01–NN
        gameboy->write_cartridge(address, data);
    } else if (address < 0xA000) {
        // 8 KiB Video RAM (VRAM)
        vram.at(address - 0x8000) = data;
    } else if (address < 0xC000) {
        // 8 KiB External RAM
        eram.at(address - 0xA000) = data;
    } else if (address < 0xD000) {
        // 4 KiB Work RAM (WRAM)
        wram1.at(address - 0xC000) = data;
    } else if (address < 0xE000) {
        // 4 KiB Work RAM (WRAM)
        wram2.at(address - 0xD000) = data;
    } else if (address < 0xFE00) {
        // Echo RAM (mirror of C000–DDFF)
        throw std::runtime_error("use of this area is prohibited: " + std::to_string(address));
    } else if (address < 0xFEA0) {
        // Object attribute memory (OAM)
        oam.at(address - 0xFE00) = data;
    } else if (address < 0xFF00) {
        // Not Usable
        // not sure what GB hardware typically does with this
        return;
    } else if (address < 0xFF80) {
        // I/O Registers
        io_reg.at(address - 0xFF00) = data;
    } else if (address < 0xFFFF) {
        // High RAM (HRAM)
        hram.at(address - 0xFF80) = data;
    } else {
        // Interrupt Enable register (IE)
        ie = data;
    }
}
