#ifndef _MEMORY_HPP_
#define _MEMORY_HPP_

#include <iostream>
#include <cstddef>
#include <cstdint>

using namespace std;

// --- ALIASES
using Byte = uint8_t;
using U16 = uint16_t;

// `const` means: this name will not be used to change these bits.
const size_t MEM_SIZE = 4096;

// Межі пам'яті стека
const size_t STACK_LO = 0xF00;
const size_t STACK_HI = 0xFFF;



struct Memory {
    Byte data[MEM_SIZE]{};
};

Byte mem_get(const Memory& mem, size_t addr);
bool mem_set(Memory& mem, size_t addr, Byte value);

U16 get16(const Memory& mem, size_t addr);
bool set16(Memory& mem, size_t addr, U16 value);

//  Get byte, increment and overwrite
bool inc(Memory& mem, size_t addr);

#endif