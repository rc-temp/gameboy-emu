#include <cstdint>

class Cartridge;
class MMU;
class CPU;
class Gameboy {
 public:
    Cartridge* cartridge;
    MMU* mmu;
    CPU* cpu;
    uint8_t read_cartridge(int address);
    uint8_t read_mmu(int address);
    void write_mmu(int address, uint8_t val);
    void write_cartridge(int address, uint8_t val);
};
