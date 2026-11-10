//#pragma once
#ifndef _CPU_HPP_
#define _CPU_HPP_

#include <iostream>
using namespace std;

#include "memory.hpp"
#include "stack.hpp"


enum class Op : Byte {
    // --- IO
    HALT      = 0x00,  //  Stop VM (exit)   0
    NOP       = 0x01,  //  No operation     1
    OUT       = 0x02,  //  Print A as symbol
    OUTN      = 0x03,  //  Print A as DEC number +space 
    OUTS      = 0x04, //  Print bytes from Addr while not '\0'

    // --- ALU
    ADD       = 0x10,  //  +    16
    SUB       = 0x11,  //  -    17
    AND       = 0x12,  //  &&   18
    OR        = 0x13,  //  ||   19
    XOR       = 0x14,  //  %2   20
    NOT       = 0x15,  //  !    21
    SHL       = 0x16,  //  <<   22
    SHR       = 0x17,  //  >>   23
    INC       = 0x18,  //  ++   24
    DEC       = 0x19,  //  --   25
    CMP       = 0x1a,  //  A-B, set Z/N/C
    MUL       = 0x1b,  //  A = A * B (молодші 8 біт), C = добуток не вліз в байт

    // --- DATA
    LOADI_A   = 0x20, //  Load 8 bits to 
    LOADI_B   = 0x21, //  Load 8 bits to 
    LOAD_A    = 0x22, //  Load 8 bits to 
    LOAD_B    = 0x23, //  Load 8 bits to 
    STORE_A   = 0x24, //  Put A to mem[ad
    STORE_B   = 0x25, //  Put B to mem[ad
    MOV_AB    = 0x26, //  A = B
    MOV_BA    = 0x27, //  B = A
    LOADH     = 0x28, //  Load 16 bit to 
    LOAD_AH   = 0x29, //  A = mem[H]
    STORE_HA  = 0x2a, //  mem[H] = A
    INC_H     = 0x2b, //  H +1
    DEC_H     = 0x2c, //  H -1
    H_LOW     = 0x2d, //  low byte from H

    // --- JUMP
    JMP       = 0x30,  // jump anyway
    JZ        = 0x31,  // jump, if Z=1
    JNZ       = 0x32,  // jump, if Z=0
    JC        = 0x33,  // jump, if C=1
    JNC       = 0x34,  // jump, if C=0

    // --- DISPLAY
    CLS       = 0x40,  //  clear VRAM
    PLOT      = 0x41,  //  draw one pixel
    UNPLOT    = 0x42,  //  clear one pixel
    SHOW      = 0x43,  //  print VRAM in console

    // --- STACK
    PUSH_A    = 0x50,  // mem[SP] = A; SP = SP - 1
    POP_A     = 0x51,  // SP = SP + 1; A = mem[SP]
    PUSH_B    = 0x52,  // ...same
    POP_B     = 0x53,  // ...as for A
    CALL      = 0x54,  // покласти адресу повернення (2 байти), потім PC = addr
    RET       = 0x55,  // зняти 2 байти в PC
    PUSH_H    = 0x56,  // opt покласти H (2 байти: молодший, потім старший)
    POP_H     = 0x57,  // opt зняти 2 байти в H
};



//  Types
//
struct Flags {
    bool z = false;   // результат нульовий
    bool n = false;   // старший біт результату — одиниця
    bool c = false;   // біт, який виїхав за межу восьми
};

struct CPU {
    Memory* mem = nullptr;   // вказівник на пам'ять
    U16 pc = 0;     //  адреса поточної інструкції для step
    U16 H = 0;      //  пам'ять для адреси
    Byte a = 0;
    Byte b = 0;
    Flags f;
    Stack stack;
    bool halted = false;     // після HALT наступні step відмовляють
    bool trace = false;      // друкувати рядок на кожен CALL/RET
};
//  -----------------------------------------------------------------


//  Functions
//  
void step(CPU& cpu);
void dump_regs(const CPU& cpu);
string flags_state(const CPU& cpu);
//  ----------------------------------------

#endif