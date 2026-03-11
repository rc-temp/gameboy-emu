cmake_minimum_required(VERSION 3.20)
project(GameboyEmu)

# 1. Find the packages
find_package(SDL2 REQUIRED)

# 2. Define your executable
# add_executable(gameboy-emu src/gameboy-emu.cpp)
add_executable(gameboy-emu src/cartridge.cpp src/cpu.cpp src/gameboy-emu.cpp src/mmu.cpp)

# 3. Link the libraries
target_link_libraries(gameboy-emu PRIVATE SDL2::SDL2)
