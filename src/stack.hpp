#ifndef _STACK_HPP_
#define _STACK_HPP_

#include "memory.hpp"

using namespace std;

struct Stack {
    Memory* mem = nullptr;
    U16 sp = STACK_HI;          // 0xFFF - стек порожній
};


void stack_init(Stack& s, Memory& mem);
bool stack_peek(const Stack& s, size_t depth, Byte& out);

bool push(Stack& s, Byte v);
bool pop(Stack& s, Byte& out);

U16 stack_sp(const Stack& s);


#endif