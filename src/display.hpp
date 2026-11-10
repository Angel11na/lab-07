#ifndef _DISPLAY_HPP_
#define _DISPLAY_HPP_

#include "memory.hpp"

namespace display {

    const U16 VRAM_LO = 0x0A00;

    const int VRAM_ROWS = 32;
    const int VRAM_COLS = 64;

    const char VRAM_SYMBOL_FILL = '#';      // 
    const char VRAM_SYMBOL_EMPTY = '.';     //


    void show(const Memory& mem);
    void plot(Memory& mem, int x, int y);
    void unplot(Memory& mem, int x, int y);
    void clear(Memory& mem);
}

struct Sprite { 
    uint8_t x = display::VRAM_COLS+1; 
    uint8_t y = display::VRAM_ROWS+1;
    int8_t vx=0; 
    int8_t vy=0;
    bool alive;
};

#endif