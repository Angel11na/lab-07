#include "stack.hpp"


void stack_init(Stack& s, Memory& mem) {
    s.mem = &mem;
    s.sp = STACK_HI;    // вища адреса = дно стека
}



bool push(Stack& s, Byte v) {
    if (s.sp < STACK_LO) return false;   // нижче 0xF00
    
    mem_set(*s.mem, s.sp, v);            // запис у пам'ять
    s.sp--;                              // зменшення адреси
    return true;
}

bool pop(Stack& s, Byte& out) {
    if (s.sp >= STACK_HI) return false;  // пустий стек

    s.sp++;                              // збільшення адреси
    out = mem_get(*s.mem, s.sp);         // читання з адреси
    return true;
}



U16 stack_sp(const Stack& s) {
    return s.sp;
}

bool stack_peek(const Stack& s, size_t depth, Byte& out) {
    size_t used = STACK_HI - s.sp;              // скільки байтів зараз у стеку
    if (depth >= used) return false;

    out = mem_get(*s.mem, s.sp + 1 + depth);    // верхівка = sp+1
    return true;
}