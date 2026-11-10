#include <iomanip>
#include <iostream>
using namespace std;

#include "alu.hpp"
#include "display.hpp"
#include "cpu.hpp"
#include "dump.hpp"
#include "stack.hpp"


void dump_regs(const CPU& cpu) {
    cout << "PC = " << "0x" << hex << cpu.pc << dec << "\n";
    cout << "H = " << "0x" << hex << setw(2) << setfill('0') << cpu.H << dec << "  "
         << to_bin(cpu.H, 16, 4) << "\n";
    cout << "A = ";
    show_byte(cpu.a);
    cout << "B = ";
    show_byte(cpu.b);
    cout << "Z: " << cpu.f.z << "\n" << "N: " << cpu.f.n << "\n" << "C: " << cpu.f.c << "\n";
    cout << "SP = " << "0x" << hex << stack_sp(cpu.stack) << dec << "\n";
}

inline const char* bit(bool b) { return b ? "1" : "0"; }
string flags_state(const CPU& cpu) {
    return "   |   Z: " + string(bit(cpu.f.z)) + "  " + "N: " + string(bit(cpu.f.n)) + "  " +
           "C: " + string(bit(cpu.f.c));
}



static bool is_mem_end(CPU &cpu) {
    size_t idx = static_cast<size_t>(cpu.pc);
    if(idx >= MEM_SIZE){
        cout << "cpu.pc >= " << MEM_SIZE << ", value was " << cpu.pc << ". CPU was halted.\n";
        cpu.halted = true;
        return true;
    }
    return false;
}

// універсальний обробник невідомого опкоду для всіх default:
static void unknown_opcode(Byte opcode, CPU &cpu){
    cerr << "Unknown opcode 0x" << hex << static_cast<int>(opcode)
        << " at PC=0x" << cpu.pc << dec << "\n";
    cpu.halted = true;
}

// помилки стеку
static void stack_overflow(CPU& cpu) {
    cerr << "Stack overflow: SP=0x" << hex << stack_sp(cpu.stack)
         << " is below STACK_LO=0x" << STACK_LO
         << " (PC=0x" << cpu.pc << ")" << dec << ". CPU halted.\n";
    cpu.halted = true;
}

static void stack_underflow(CPU& cpu) {
    cerr << "Stack underflow: stack is empty, SP=0x" << hex << stack_sp(cpu.stack)
         << " (PC=0x" << cpu.pc << ")" << dec << ". CPU halted.\n";
    cpu.halted = true;
}
// ----------------------------------------------------------------

// Трасування (виведення)
static void trace_line(const CPU& cpu, const char* name, U16 from) {
    if (!cpu.trace) return;
    cout << name << "  PC 0x" << hex << from << " -> 0x" << cpu.pc
         << "  SP=0x" << stack_sp(cpu.stack) << dec
         << "  A=" << static_cast<int>(cpu.a) << "\n";
}

//  загальна ф-я для обробки стрибків за умови
static void JMPX(bool flag, CPU &cpu){
    if(!flag){   //  якщо флаг НЕ виставлений - пропустити цю команду
        cpu.pc +=3;
        return;
    }
    //  інакше, флаг ВИСТАВЛЕНИЙ
    cpu.pc = get16(*cpu.mem, cpu.pc+1);
}





void step(CPU& cpu) {
    if (cpu.halted) {
        cout << "halted\n";
        return;
    }
    //  -- cpu.mem - це сам вказівник
    //  ~ cpu.mem = ...                                         | переміщення
    //  ~ cpu.mem++ / cpu.mem-- / cpu.mem += ... / cpu.mem -=   | переміщення через арифметику
    //  ~ *cpu.mem                                              | взяти значення
    Byte op = mem_get(*cpu.mem, cpu.pc);

    switch ((Op)op) {
        //  ------------------------------------------------------------------------------------
        // -- IO
        case Op::HALT:
            cpu.halted = true;
            cpu.pc++; // розмір HALT з таблиці — 1
            break;
        case Op::NOP:
            cpu.pc++;
            break;
        case Op::OUT:
            cout << to_character(cpu.a) << "\n";
            cpu.pc++;
            break;
        case Op::OUTN:
            cout << static_cast<int>(cpu.a) << " ";
            cpu.pc++;
            break;
        case Op::OUTS: {
            cpu.pc++;
            U16 addr = get16(*cpu.mem, cpu.pc);
            cpu.pc+=2;

            for(int i = 0; i < 256; i++){
                if(cpu.mem->data[addr + i] == '\0')
                    break;
                
                cout << static_cast<char>(cpu.mem->data[addr+i]);
            }

            break;
        }

        //  ------------------------------------------------------------------------------------
        // -- ALU
        case Op::ADD: {
            cout << "\n"
                 << "ADD(" << static_cast<int>(cpu.a) << ", " << static_cast<int>(cpu.b)
                 << ") = " << static_cast<int>(ADD(cpu.a, cpu.b, cpu.f)) << flags_state(cpu)
                 << "\n\n";
            cpu.pc++;
            break;
        }

        case Op::SUB: {
            cout << "\n"
                 << "SUB(" << static_cast<int>(cpu.a) << ", " << static_cast<int>(cpu.b)
                 << ") = " << static_cast<int>(SUB(cpu.a, cpu.b, cpu.f)) << flags_state(cpu)
                 << "\n\n";
            cpu.pc++;
            break;
        }

        case Op::AND: {
            cout << "\n"
                 << "AND = " << to_bin(AND(cpu.a, cpu.b, cpu.f)) << flags_state(cpu) << "\n\n";
            cpu.pc++;
            break;
        }

        case Op::OR: {
            cout << "\n"
                 << "OR = " << to_bin(OR(cpu.a, cpu.b, cpu.f)) << flags_state(cpu) << "\n\n";
            cpu.pc++;
            break;
        }

        case Op::XOR: {
            cout << "\n"
                 << "XOR = " << to_bin(XOR(cpu.a, cpu.b, cpu.f)) << flags_state(cpu) << "\n\n";
            cpu.pc++;
            break;
        }

        case Op::NOT: {
            cout << "\n"
                 << "NOT(a) = " << to_bin(NOT(cpu.a, cpu.f)) << flags_state(cpu) << "\n"
                 << "NOT(b) = " << to_bin(NOT(cpu.b, cpu.f)) << flags_state(cpu) << "\n\n";
            cpu.pc++;
            break;
        }

        case Op::SHL: {
            cout << "\n"
                 << "SHL(a) = " << to_bin(SHL(cpu.a, cpu.f)) << flags_state(cpu) << "\n"
                 << "SHL(b) = " << to_bin(SHL(cpu.b, cpu.f)) << flags_state(cpu) << "\n\n";
            cpu.pc++;
            break;
        }

        case Op::SHR: {
            cout << "\n"
                 << "SHR(a) = " << to_bin(SHR(cpu.a, cpu.f)) << flags_state(cpu) << "\n"
                 << "SHR(b) = " << to_bin(SHR(cpu.b, cpu.f)) << flags_state(cpu) << "\n\n";
            cpu.pc++;
            break;
        }

        case Op::INC: {
            cpu.a = INC(cpu.a, cpu.f);
            cpu.pc++;
            break;
        }

        case Op::DEC: {
            cpu.a = DEC(cpu.a, cpu.f);
            cpu.pc++;
            break;
        }

        case Op::CMP: {
            (void)SUB(cpu.a, cpu.b, cpu.f);
            cpu.pc++;
            break;
        }

        case Op::MUL:{
            cpu.a = MUL(cpu.a, cpu.b, cpu.f);
            cpu.pc++;
            break;
        }

        //  ------------------------------------------------------------------------------------
        // -- DATA
        case Op::LOADI_A: {
            cpu.pc++;                           //  наступна комірка = значення, imm8
            if(is_mem_end(cpu)) return;
            cpu.a = mem_get(*cpu.mem, cpu.pc);  //  взяти значення з пам'яті
            cpu.pc++;                           //  наступна інструкція
            break;
        }
        case Op::LOADI_B: {
            cpu.pc++;                           //  наступна комірка = значення, imm8
            if(is_mem_end(cpu)) return;
            cpu.b = mem_get(*cpu.mem, cpu.pc);  //  взяти значення з пам'яті
            cpu.pc++;                           //  наступна інструкція
            break;
        }
        case Op::LOAD_A: {
            cpu.pc++;
            if(is_mem_end(cpu)) return;
            U16 addr = get16(*cpu.mem, cpu.pc);
            cpu.a = mem_get(*cpu.mem, addr);        //  у регістр А
            cpu.pc+=2;
            break;
        }
        case Op::LOAD_B: {
            cpu.pc++;
            if(is_mem_end(cpu)) return;
            U16 addr = get16(*cpu.mem, cpu.pc);
            cpu.b = (Byte)mem_get(*cpu.mem, addr);  //  у регістр B
            cpu.pc+=2;
            break;
        }
        case Op::STORE_A: {
            cpu.pc++;
            if(is_mem_end(cpu)) return;
            U16 addr = get16(*cpu.mem, cpu.pc);
            mem_set(*cpu.mem, addr, cpu.a);
            cpu.pc+=2;
            break;
        }
        case Op::STORE_B: {
            cpu.pc++;
            if(is_mem_end(cpu)) return;
            U16 addr = get16(*cpu.mem, cpu.pc);
            mem_set(*cpu.mem, addr, cpu.b);
            cpu.pc+=2;
            break;
        }
        case Op::MOV_AB: {
            cpu.a = cpu.b;
            cpu.pc++;
            break;
        }
        case Op::MOV_BA: {
            cpu.b = cpu.a;
            cpu.pc++;
            break;
        }
        case Op::LOADH: {
            cpu.pc++;
            if(is_mem_end(cpu)) return;
            U16 addr = get16(*cpu.mem, cpu.pc);
            cpu.H = addr;
            cpu.pc+=2;
            break;
        }
        case Op::LOAD_AH: {
            cpu.a = mem_get(*cpu.mem, cpu.H);
            cpu.pc++;
            break;
        }
        case Op::STORE_HA: {
            mem_set(*cpu.mem, cpu.H, cpu.a);
            cpu.pc++;
            break;
        }
        case Op::INC_H: {
            cpu.H++;
            cpu.pc++;
            break;
        }
        case Op::DEC_H: {
            cpu.H--;
            cpu.pc++;
            break;
        }
        case Op::H_LOW: {
            /* NOT THIS LAB */
            break;
        }

        //  ------------------------------------------------------------------------------------
        // -- JUMP
        case Op::JMP:
            cpu.pc = get16(*cpu.mem, cpu.pc+1);
            break;

        case Op::JZ:
            JMPX(cpu.f.z, cpu);
            break;

        case Op::JNZ:
            JMPX(!cpu.f.z, cpu);
            break;

        case Op::JC:
            JMPX(cpu.f.c, cpu);
            break;

        case Op::JNC:
            JMPX(!cpu.f.c, cpu);
            break;

        //  ------------------------------------------------------------------------------------
        // -- DISPLAY
        case Op::SHOW:
            display::show(*cpu.mem);
            cpu.pc++;
            break;

        case Op::PLOT:
            display::plot(*cpu.mem, cpu.a, cpu.b);
            cpu.pc++;
            break;

        case Op::UNPLOT:
            display::unplot(*cpu.mem, cpu.a, cpu.b);
            cpu.pc++;
            break;

        case Op::CLS:
            display::clear(*cpu.mem);
            cpu.pc++;
            break;

        //  ------------------------------------------------------------------------------------
        // -- STACK
        case Op::PUSH_A:
            if (!push(cpu.stack, cpu.a)) { stack_overflow(cpu); return; }
            cpu.pc++;
            break;
        case Op::POP_A:
            if (!pop(cpu.stack, cpu.a)) { stack_underflow(cpu); return; }
            cpu.pc++;
            break;
        case Op::PUSH_B:
            if (!push(cpu.stack, cpu.b)) { stack_overflow(cpu); return; }
            cpu.pc++;
            break;
        case Op::POP_B:
            if (!pop(cpu.stack, cpu.b)) { stack_underflow(cpu); return; }
            cpu.pc++;
            break;
        
        case Op::CALL: {
            U16 target = get16(*cpu.mem, cpu.pc + 1);                 // адреса підпрограми (2 байти після опкоду)
            U16 ret    = static_cast<U16>(cpu.pc + 3);                // CALL займає 3 байти => наступна інструкція

            if (!push(cpu.stack, static_cast<Byte>(ret & 0xFF))) {    // 1) молодший байт
                stack_overflow(cpu); return;
            }
            if (!push(cpu.stack, static_cast<Byte>(ret >> 8))) {      // 2) старший байт
                stack_overflow(cpu); return;
            }
            U16 from = cpu.pc;
            cpu.pc = target;
            trace_line(cpu, "CALL", from);
            break;
        }
        case Op::RET: {
            Byte high = 0, low = 0;

            if (!pop(cpu.stack, high)) {                              // 1) спершу старший
                stack_underflow(cpu); return;
            }
            if (!pop(cpu.stack, low)) {                               // 2) потім молодший
                stack_underflow(cpu); return;
            }
            U16 from = cpu.pc;
            cpu.pc = static_cast<U16>(low | (high << 8));
            trace_line(cpu, "RET", from);
            break;
        }

        //  ------------------------------------------------------------------------------------
        default:
            unknown_opcode(op,cpu);
            break;

    }
}