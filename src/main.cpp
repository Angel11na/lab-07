#include <exception>
#include <iostream>
#include <sstream>
#include <string>

using namespace std;

#include "cpu.hpp"
#include "dump.hpp"
#include "memory.hpp"
#include "ember.hpp"
#include "display.hpp"
#include "stack.hpp"


static void print_help() {
    cout << "commands:\n"
         << "  dump                                         print all " << MEM_SIZE << " bytes\n"
         << "  get <addr>                                   show one byte four ways\n"
         << "  get16 <addr>                                 show two bytes from addr\n"
         << "  set <addr> <val>                             write one byte (dec or 0x hex)\n"
         << "  set16 <addr> <val>                           write two bytes (dec or 0x hex)\n"
         << "  find <lo> <hi> <byte> <?deepth=8>            find <byte> at memory, -1 for non-zero search\n"
         << "  find_first <lo> <hi> <byte> <?deepth=8>      find only first <byte> at memory, -1 for non-zero search"
         << "  inc <addr>                                   increment value and overwrite\n"
         << "  reg <a|b> <0..255>                           set CPU register <a|b> value\n"
         << "  regs                                         show current CPU state\n"
         << "  step                                         perform one CPU instruction\n"
         << "  run                                          perform steps until HALT\n"
         << "  vram_show                                    draw VRAM state to the console\n"
         << "  draw_frame                                   draw frame in VRAM\n"
         << "  sprite <i> <x> <y> <vx> <vy>                 set one sprite in VRAM\n"
         << "  tick                                         show one frame sprites animation\n"
         << "  tick_loop <c>                                tick <c> times in loop\n"
         << "  vram_clear                                   clear all VRAM\n"
         << "  show_sizes                                   show sizeof() alignof() offsetof() CPU\n"
         << "  stack                                        show SP and top bytes of the stack\n"
         << "  trace <on|off>                               print a line for every CALL/RET\n"
         << "  help                                         this list\n"
         << "  quit                                         leave\n";
}


int main() {
    Memory mem; // 4096 bytes
    CPU cpu;
    Sprite sprites[8];

    //  --------------  TEST AREA   ----------------------------------------
    

    //  Test 1: Memory leak
    //new int{1};

    // // Test 2: Read after delete
    // int* p = new int{42};
    // cout << *p << '\n';
    // delete p;
    // cout << *p << '\n';   // dangling

    // // Test 3: Double delete
    // int *p = new int{42};
    // cout << *p << "\n";
    // delete p;
    // delete p;

    //  ---------------------------------------------------------------------


    cpu.mem = &mem; //  прив'язка процесора до пам'яті, & значить - "працює з пам'яттю напряму"
    stack_init(cpu.stack, mem);


    cout << "ember 0.1 - 4096 bytes of memory you can see. Type `help`.\n";

    string line;
    while (true) {
        cout << "\nember> ";

        // getline reads whole line. It returns false at end of input (Ctrl-D)
        if (!getline(cin, line)) {
            cout << '\n';
            break;
        }

        // Split the line into words
        istringstream words(line);
        string cmd;

        words >> cmd;

        if (cmd.empty()) {
            continue; // the user just pressed Enter
        } else if (cmd == "quit" || cmd == "exit") {
            break;
        } else if (cmd == "help") {
            print_help();
        } else if (cmd == "dump") {
            dump(mem);
        } else if (cmd == "get") {
            cmd_get(mem, words);
        } else if (cmd == "set") {
            cmd_set(mem, words);
        } else if (cmd == "inc") {
            cmd_inc(mem, words);
        } else if (cmd == "reg") {
            cmd_reg(cpu, words);
        } else if (cmd == "regs") {
            dump_regs(cpu);
        } else if(cmd == "step") {
            step(cpu);
        } else if (cmd == "run") {
            cmd_run(cpu);
        }
        
        
        else if (cmd == "get16") {
            cmd_get16(mem, words);
        } else if (cmd == "set16") {
            cmd_set16(mem, words);   
        }

        else if (cmd == "find") {
            cmd_find(mem, words);
        }
        else if (cmd == "find_first") {
            cmd_find(mem, words, true);
        }

        else if(cmd == "draw_frame") {
            cmd_draw_frame(mem);
        }
        else if(cmd == "sprite") {
            cmd_sprite(sprites, words);
        }
        else if (cmd == "vram_show") {
            display::show(mem);
        }
        else if(cmd == "tick") {
            cmd_tick(sprites, mem);
        }
        else if(cmd == "tick_loop") {
            cmd_tick_loop(sprites, mem, words);
        }
        else if (cmd == "vram_clear") {
            display::clear(mem);
        }
        else if(cmd == "show_sizes") {
            cmd_show_sizes();
        }

        else if (cmd == "stack") {
            cmd_stack(cpu);
        }

        else if (cmd == "trace") {
            cmd_trace(cpu, words);
        }

        else {
            cout << "unknown command: " << cmd << " (try `help`)\n";
        }
    }

    return 0; // 0 means "success" to the operating system
}



