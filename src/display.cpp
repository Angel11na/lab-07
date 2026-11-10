#include <iostream>
using namespace std;

#include "display.hpp"


namespace display {

    void show(const Memory& mem) {
        for (int y = 0; y < VRAM_ROWS; y++) {
            for (int x = 0; x < VRAM_COLS; x++) {
                Byte cell = mem.data[VRAM_LO + (y * 8) + (x / 8)];
                bool lit = (cell >> (7 - (x % 8))) & 1;
                cout << (lit ? VRAM_SYMBOL_FILL : VRAM_SYMBOL_EMPTY);
            }
            cout << '\n';
        }
    }

    static void plot_op(Memory& mem, int x, int y, bool set = true){
        if (x < 0 || x >= VRAM_COLS || y < 0 || y >= VRAM_ROWS)
            return;

        int address = VRAM_LO + (y * 8) + (x / 8);
        int bit = 7 - (x % 8);

        if(set)
            //  Для вставновлення "1" ==> операція АБО
            mem.data[address] |= ((Byte)1 << bit);
        else
            //  Для скидання ==> операція І-НЕ
            mem.data[address] &= ~((Byte)1 << bit);
    }

    void plot(Memory& mem, int x, int y) {
        plot_op(mem, x, y, true);
    }

    void unplot(Memory& mem, int x, int y) {
        plot_op(mem, x, y, false);
    }

    void clear(Memory& mem) {
        for (int i = 0; i < VRAM_ROWS*VRAM_COLS / 8; i++) {
            mem.data[VRAM_LO + i] = 0x00;
        }
    }

}