using namespace std;

#include "dump.hpp"
#include "memory.hpp"


const int SIZE = 4095;

Byte mem_get(const Memory& mem, size_t addr) {
    
    if(addr > SIZE)
        return 0;

    return mem.data[addr];
}
U16 get16(const Memory& mem, size_t addr) {
    Byte low = mem_get(mem, addr);
    Byte high = mem_get(mem, addr + 1);
    return (U16)(low | (high << 8));
}


bool mem_set(Memory& mem, size_t addr, Byte value) {
    
    if(addr > SIZE)
        return false;

    mem.data[addr] = value;
    return true;
}
bool set16(Memory& mem, size_t addr, U16 value){

    if(addr >= SIZE)
        return false;

    Byte low = value;           //  автоматично відріже 8 молодших біт (усічення)
    Byte high = value >> 8;

    mem.data[addr] = low;
    mem.data[addr + 1] = high;

    return true;
}


bool inc(Memory& mem, size_t addr){

    if(addr > SIZE)
        return false;
    
    Byte b = mem_get(mem, addr);
    b++;
    return mem_set(mem, addr, b);
}
 