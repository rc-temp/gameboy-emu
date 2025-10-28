#include <cstdlib>
#include <chrono>
#include <thread>
#include <fstream>
#include <cstdint>
#include <iostream>
#include <array>
#include <vector>

#include "gameboy-emu.h"
#include "cpu.h"
#include "cartridge.h"
#include "mmu.h"

#include <chrono>

#include <SDL.h>
#include <SDL_timer.h>

int GAMEBOY_DISPLAY_WIDTH = 160;
int GAMEBOY_DISPLAY_HEIGHT = 144;

uint8_t Gameboy::read_cartridge(int address) {
    return cartridge->read(address);
}

uint8_t Gameboy::read_mmu(int address) {
    return mmu->read(address);
}

void Gameboy::write_mmu(int address, uint8_t val) {
    mmu->write(address, val);
}

void Gameboy::write_cartridge(int address, uint8_t val) {
    cartridge->write(address, val);
}

void render_graphics0(SDL_Renderer *renderer, SDL_Surface *surface) {
    // TODO: document what this function does/is for, is it for testing?

    SDL_Rect box;
    box.w = 10;
    box.h = 10;
    box.x = 0;
    box.y = 0;
    // SDL_SetRenderDrawColor(renderer, 0,0,0,0);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 50);
    SDL_RenderClear(renderer);
    //outline rect
    // SDL_SetRenderDrawColor(renderer, r , g , b, a);
    SDL_SetRenderDrawColor(renderer, 10, 20, 30, 40);
    SDL_RenderDrawRect(renderer, &box);
    // SDL_SetRenderDrawColor(renderer, r , g , b, a);
    SDL_SetRenderDrawColor(renderer, 5, 10, 15, 20);
    //fill up rectangle with color
    SDL_RenderFillRect(renderer, &box);
    SDL_RenderPresent(renderer);


    // SDL_RenderClear(rend);
    // SDL_RenderCopy(rend, tex, NULL, &dest);
    // SDL_RenderPresent(rend);
}

void render_graphics1(SDL_Renderer *renderer, SDL_Surface *surface, Gameboy gameboy) {
    // TODO: document what this function does/is for, is it for testing?

    auto memory = *gameboy.mmu;

    for (int i = 0xFE00; i < 0xFEA0; i++) {
        memory.write(i, std::rand() % 256);
    }
    for (int i = 0x8000; i < 0x9000; i++) {
        memory.write(i, std::rand() % 256);
    }

    SDL_RenderClear(renderer);
    std::vector<uint8_t> pixels(8 * 8 * 4, 0);
    SDL_Texture* texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        8,
        8
    );

    for (int i = 0xFE00; i < 0xFEA0;) {
        uint8_t y = memory.read(i++);
        uint8_t x = memory.read(i++);

        uint8_t tile_index = memory.read(i++);
        uint8_t attributes = memory.read(i++);

        int j = 0x8000 + 16 * tile_index;
        for (int k = 0; k < 8; k++) {
            int lo_bits = memory.read(j + 2 * k);
            int hi_bits = memory.read(j + 2 * k + 1);
            for (int l = 0; l < 8; l++) {
                int lo_bit = (lo_bits >> (7 - l)) & 1;
                int hi_bit = (hi_bits >> (7 - l)) & 1;
                int intensity = (hi_bit << 1) | lo_bit;
                pixels.at(4 * (8 * k + l))     = intensity * 255 / 4; // B
                pixels.at(4 * (8 * k + l) + 1) = intensity * 255 / 4; // G
                pixels.at(4 * (8 * k + l) + 2) = intensity * 255 / 4; // R
                pixels.at(4 * (8 * k + l) + 3) = 255; // A //  what to set this to?
            }
        }


        unsigned char* lockedPixels = nullptr;
        int pitch = 0;
        SDL_LockTexture(texture, nullptr, reinterpret_cast< void** >( &lockedPixels ), &pitch);
        std::copy(pixels.begin(), pixels.end(), lockedPixels);
        SDL_UnlockTexture(texture);

        SDL_Rect thing;
        thing.w = 8;
        thing.h = 8;
        thing.x = x;
        thing.y = y;
        SDL_RenderCopy(renderer, texture, nullptr, &thing);

    }
    SDL_RenderPresent(renderer);
    SDL_DestroyTexture(texture);

    // SDL_Rect box;
    // box.w = 10;
    // box.h = 10;
    // box.x = 0;
    // box.y = 0;
    // // SDL_SetRenderDrawColor(renderer, 0,0,0,0);
    // SDL_SetRenderDrawColor(renderer, 255, 255, 255, 50);
    // SDL_RenderClear(renderer);
    // //outline rect
    // // SDL_SetRenderDrawColor(renderer, r , g , b, a);
    // SDL_SetRenderDrawColor(renderer, 10, 20, 30, 40);
    // SDL_RenderDrawRect(renderer, &box);
    // // SDL_SetRenderDrawColor(renderer, r , g , b, a);
    // SDL_SetRenderDrawColor(renderer, 5, 10, 15, 20);
    // //fill up rectangle with color
    // SDL_RenderFillRect(renderer, &box);
    // SDL_RenderPresent(renderer);


    // SDL_RenderClear(rend);
    // SDL_RenderCopy(rend, tex, NULL, &dest);
    // SDL_RenderPresent(rend);
}


void render_graphics2(SDL_Renderer *renderer, SDL_Surface *surface, SDL_Texture *texture,std::vector<uint8_t> pixels, Gameboy gameboy) {
    // TODO: document what this function does/is for, is it for testing?

    auto mmu = *gameboy.mmu;

    // // sprite addresses
    // for (int i = 0xFE00; i < 0xFEA0; i++) {
    //     mmu.write(i, std::rand() % 256);
    // }
    // for (int i = 0x8000; i < 0x9000; i++) {
    //     mmu.write(i, std::rand() % 256);
    // }

    // // background addresses
    // for (int i = 0x8000; i < 0x9800; i++) {
    //     mmu.write(i, std::rand() % 256);
    // }
    // for (int i = 0x9800; i < 0xA000; i++) {
    //     mmu.write(i, std::rand() % 256);
    // }


    // do pixel stuff
    uint8_t lcdc = mmu.read(0xFF40);
    int tile_map_base = 0x9800;
    if (lcdc & (1 << 3))
        tile_map_base = 0x9C00;

    int tile_data_base = 0x8800;
    if (lcdc & (1 << 4))
        tile_data_base = 0x8000;

    uint8_t scy = mmu.read(0xFF42);
    uint8_t scx = mmu.read(0xFF43);

    for (int j = 0; j < GAMEBOY_DISPLAY_HEIGHT; j++) {
        int tile_map_y = ((scy + j) % 256) / 8;
        for (int i = 0; i < GAMEBOY_DISPLAY_WIDTH; i++) {
            int tile_map_x = ((scx + i) % 256) / 8;
            int tile_map_index = tile_map_y * 32 + tile_map_x;

            uint8_t tile_data_index = mmu.read(tile_map_base + tile_map_index);

            int tile_data_pointer;
            if (tile_data_base == 0x8000)
                tile_data_pointer = tile_data_base + tile_data_index * 16;
            else
                tile_data_pointer = tile_data_base + ((int8_t)tile_data_index) * 16;

            int x_offset = (scx + i) % 8;
            int y_offset = (scy + j) % 8;

            int lo_bits = mmu.read(tile_data_pointer + 2 * y_offset);
            int hi_bits = mmu.read(tile_data_pointer + 2 * y_offset + 1);

            int lo_bit = (lo_bits >> (7 - x_offset)) & 1;
            int hi_bit = (hi_bits >> (7 - x_offset)) & 1;
            int intensity = 3 - ((hi_bit << 1) | lo_bit);
            pixels.at(4 * (160 * j + i))     = intensity * 255 / 3; // B
            pixels.at(4 * (160 * j + i) + 1) = intensity * 255 / 3; // G
            pixels.at(4 * (160 * j + i) + 2) = intensity * 255 / 3; // R
            pixels.at(4 * (160 * j + i) + 3) = 255; // A //  what to set this to?

        }
    }

    SDL_RenderClear(renderer);

    // write directly to surface instead? but how?

    unsigned char* locked_pixels = nullptr;
    int pitch = 0;
    // is reinterpret_cast necessary?
    SDL_LockTexture(texture, nullptr, reinterpret_cast<void**>(&locked_pixels), &pitch);
    // std::copy_n(pixels.data(), pixels.size(), locked_pixels);
    std::copy(pixels.begin(), pixels.end(), locked_pixels);
    SDL_UnlockTexture(texture);

    SDL_RenderCopy(renderer, texture, nullptr, nullptr);
    SDL_RenderPresent(renderer);

}


int main(int argc, char *argv[]) {
    if (argc < 2 || argc > 3) {
        std::cerr << "usage: gameboy-emu rom_file [boot_rom]\n";
        return 1;
    }

    Gameboy gameboy = Gameboy();

    auto cartridge = Cartridge();
    cartridge.load(std::string(argv[1]));
    gameboy.cartridge = &cartridge;

    auto mmu = MMU();
    gameboy.mmu = &mmu;
    mmu.gameboy = &gameboy;
    if (argc == 3) {
        mmu.load_boot_rom(std::string(argv[2]));
    } else {
        // disable bootrom, necessary for passing blargg test 07
        gameboy.write_mmu(0xFF50, 1);
    }

    auto cpu = CPU();
    gameboy.cpu = &cpu;
    cpu.gameboy = &gameboy;

    std::cerr << "starting execution" << std::endl;
    int cycles = 0;

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        printf("error initializing SDL: %s\n", SDL_GetError());
    }
    SDL_Window* win = SDL_CreateWindow("Gameboy",
                                       SDL_WINDOWPOS_CENTERED,
                                       SDL_WINDOWPOS_CENTERED,
                                       GAMEBOY_DISPLAY_WIDTH,
                                       GAMEBOY_DISPLAY_HEIGHT,
                                       0);
    if (win == NULL) {
        fprintf(stderr, "SDL window failed to initialise: %s\n", SDL_GetError());
        return 1;
    }

    bool is_running = true;
    SDL_Event event;

    SDL_Renderer* renderer = SDL_CreateRenderer(win, -1, 0);
    SDL_Surface* surface;
    SDL_Texture* texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        GAMEBOY_DISPLAY_WIDTH,
        GAMEBOY_DISPLAY_HEIGHT
    );
    std::vector<uint8_t> pixels(160 * 144 * 4, 0);


    cpu.init(false);
    // cpu.init(true);

    int lcdy_cycles = 0;
    // gameboy.write_mmu(0xFF44, 0x00);
    gameboy.write_mmu(0xFF44, 0x90);

    int total_cycles = 0;

    std::chrono::time_point<std::chrono::high_resolution_clock> start, stop;
    std::chrono::nanoseconds duration;

    SDL_JoystickEventState(SDL_IGNORE);

    start = std::chrono::high_resolution_clock::now();

    bool started = false;

    // SDL_PollEvent(&event);
    while (is_running) {

        // cpu.print_state();
        cpu.handle_interrupts();

        // fetch instruction
        auto instr = cpu.fetch();

        // execute instruction
        int instr_cycles = cpu.execute(instr);

        cycles += instr_cycles;
        lcdy_cycles += instr_cycles;
        total_cycles += instr_cycles;

        if (lcdy_cycles >= 456) {
            int temp = gameboy.read_mmu(0xFF44) + 1;
            if (temp >= 154)
                temp = 0;
            gameboy.write_mmu(0xFF44, temp);
            lcdy_cycles -= 456;
        }

        int cycles_per_frame = 70224;
        int cycles_per_second = 4194304;

        if (cycles > cycles_per_frame) {
            // TODO: getting keyboard state should happen when
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) {
                    is_running = false;
                }
            }

            cycles -= cycles_per_frame;
            render_graphics2(renderer, surface, texture, pixels, gameboy);

            std::this_thread::sleep_until(start + std::chrono::nanoseconds(16742706));
            // std::this_thread::sleep_until(start + std::chrono::nanoseconds(15500000));
            stop = std::chrono::high_resolution_clock::now();
            duration = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start);
            std::cout << "FPS: " << 1L * 1000 * 1000 * 1000 / duration.count() << std::endl;
            start = std::chrono::high_resolution_clock::now();
        }

    }

    SDL_DestroyTexture(texture);

    return 0;
}
