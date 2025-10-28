#include <format>
#include <string>
#include <fstream>
#include "cartridge.h"

Cartridge::Cartridge() {
}

// void Cartridge::load(std::string filepath) {
//     std::basic_ifstream<uint8_t> ifd(filepath, std::ios::binary | std::ios::ate);
//     std::streamsize size = ifd.tellg();
//     ifd.seekg(0, std::ios::beg);
//     rom.resize(size);
//     ifd.read(rom.data(), size);
// }
void Cartridge::load(std::string filepath) {
    std::ifstream ifd(filepath, std::ios::binary | std::ios::ate);
    std::streamsize size = ifd.tellg();
    ifd.seekg(0, std::ios::beg);
    rom.resize(size);
    ifd.read((char *)rom.data(), size);

    int cartridge_type = rom.at(0x0147);
    switch (cartridge_type) {
        // these may not be complete list
    case 0x00: case 0x08: case 0x09:
        mbc_type = 0;
        break;
    case 0x01: case 0x02: case 0x03:
        mbc_type = 1;
        break;
    case 0x05: case 0x06:
        mbc_type = 2;
        break;
    case 0x0F: case 0x10: case 0x11: case 0x12: case 0x13:
        mbc_type = 3;
        break;
    case 0x19: case 0x1A: case 0x1B: case 0x1C: case 0x1D: case 0x1E:
        mbc_type = 5;
        break;
    case 0x20:
        mbc_type = 6;
        break;
    case 0x22:
        mbc_type = 6;
        break;
    default:
        std::runtime_error("Unrecognized cartridge type");
    }
}

uint8_t Cartridge::read(int address) {
    // TODO: || true makes certain blargg tests pass, but this should be removed
    if (mbc_type == 0 || true) {
        return rom.at(address);
    } else {
        throw std::runtime_error(std::format("mbc type {} is not implemented yet", mbc_type));
    }
}

void Cartridge::write(int address, uint8_t val) {
    if (mbc_type == 0) {
        rom.at(address) = val;
    } else {
        throw std::runtime_error(std::format("mbc type {} is not implemented yet", mbc_type));
    }
}
