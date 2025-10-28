#include <array>
#include <cstdint>

class Gameboy;
class MMU {
 private:
    std::vector<uint8_t> vram;
    std::vector<uint8_t> eram;
    std::vector<uint8_t> wram1;
    std::vector<uint8_t> wram2;
    std::vector<uint8_t> oam;
    std::vector<uint8_t> not_usable;
    std::vector<uint8_t> io_reg;
    std::vector<uint8_t> hram;
    uint8_t ie;
    std::vector<uint8_t> boot_rom;
 public:
    MMU();
    Gameboy* gameboy;
    uint8_t read(int address);
    void write(int address, uint8_t data);
    void load_boot_rom(std::string filepath);
};
