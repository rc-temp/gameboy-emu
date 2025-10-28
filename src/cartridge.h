#include <vector>

class Cartridge {
 private:
    std::vector<uint8_t> rom;
    int mbc_type;
 public:
    Cartridge();
    void load(std::string filepath);
    uint8_t read(int address);
    void write(int address, uint8_t val);
};
