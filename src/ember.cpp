#include <iostream>
#include <sstream>
#include <unistd.h>
#include <cstdlib>

using namespace std;

#include "ember.hpp"
#include "display.hpp"
#include "dump.hpp"


const int TICK_DELAY = 100; //  delay in sprites animation drawing (ms)


// Turn a word into a number. Accepts decimal (65) and hex (0x41).
// Returns false if the word is not a number at all.
static bool parse_number(const string& word, long& out) {
    try {
        size_t used = 0;
        // base 0 means: look at the prefix. "0x41" is hex, "65" is decimal.
        out = stol(word, &used, 0);
        return used == word.size(); // reject things like "12abc"
    } catch (...) {
        return false;
    }
}


//  =============================================================================
//  ==============  COMMANDS
//
void cmd_get(Memory& mem, istringstream& words) {
    string a;
    long addr = 0;
    if (!(words >> a) || !parse_number(a, addr)) {
        cout << "usage: get <addr>\n";
        return;
    }
    if (addr < 0) {
        cout << "address must not be negative\n";
        return;
    }
    
    show_byte(mem_get(mem, static_cast<size_t>(addr)));
}

void cmd_set(Memory& mem, istringstream& words) {
    string a, v;
    long addr = 0, value = 0;
    if (!(words >> a) || !(words >> v) || !parse_number(a, addr) || !parse_number(v, value)) {
        cout << "usage: set <addr> <value>\n";
        return;
    }
    if (addr < 0) {
        cout << "address must not be negative\n";
        return;
    } 
    if (value < 0 || value > 255) {
        // A cell holds ONE byte. 256 does not fit. Lab 1, theory 3.
        cout << "a byte is 0..255, got " << value << '\n';
        return;
    }
    if (!mem_set(mem, static_cast<size_t>(addr), static_cast<Byte>(value))) {
        cout << "address " << addr << " is outside 0.." << MEM_SIZE - 1 << '\n';
        return;
    }
}

void cmd_inc(Memory& mem, istringstream& words) {
    string a;
    long addr = 0;

    if (!(words >> a) || !parse_number(a, addr)) {
        cout << "usage: inc <addr>\n";
    } else if (addr < 0) {
        cout << "address must not be negative\n";
    } else {
        bool result = inc(mem, addr);

        if (!result) {
            cout << "Error during cmd: inc <addr>, addr = " << addr << "\n";
        }
    }
}

void cmd_reg(CPU& cpu, istringstream& words) {
    string r, v;
    long value = 0;
    
    if (
        !(words >> r) || !(words >> v) ||   //  чи достатньо елементів у команді?
        !(r == "a" || r == "b") ||          //  r є "а" або "b"?
        !parse_number(v, value) ||          //  чи v є цілим числом?
        value < 0 || value > 255            //  чи v в межах 0-255
    ) {
        cout << "usage: reg <a|b> <0..255>\n";
        return;
    }
    
    cpu.a = (r == "a") ? (Byte)value : cpu.a;
    cpu.b = (r == "b") ? (Byte)value : cpu.b;
}


void cmd_get16(Memory& mem, istringstream& words){
    string a;
    long addr = 0;
    if (!(words >> a) || !parse_number(a, addr)) {
        cout << "usage: get16 <addr>\n";
        return;
    }
    if (addr < 0) {
        cout << "address must not be negative\n";
        return;
    }
    U16 result = get16(mem, addr);

    show_two_bytes(result);
}


void cmd_set16(Memory& mem, istringstream& words){
    string a, v;
    long addr = 0, value = 0;
    if (!(words >> a) || !(words >> v) || !parse_number(a, addr) || !parse_number(v, value)) {
        cout << "usage: set16 <addr> <value>\n";
        return;
    }
    if (addr < 0) {
        cout << "address must not be negative\n";
        return;
    } 
    if (value < 0 || value > 65'535) {
        // A cell holds TWO byte. Value less than 65536 (2^16) 
        cout << "a byte is 0..65535, got " << value << '\n';
        return;
    }

    if(!set16(mem, addr, static_cast<U16>(value))){
        cout << "address " << addr << " is outside 0.." << MEM_SIZE - 1 << '\n';
    }
}





void cmd_run(CPU& cpu) {
    size_t steps = 0;

    if (cpu.halted) {
        cerr << "Error: CPU halted\n";
        return;
    }

    while (!cpu.halted) {

        if(steps >= MAX_STEPS){
            cout << "MAX_STEPS (" << MAX_STEPS << ") limit reached, run stopped\n";
            break;
        }

        step(cpu);
        steps++;
    }   
}

//  find <lo> <hi> <byte> <?deepth=8>
//  <lo> <hi> - це адреса, з якої почати пошук (глибиною 8 байт)
//  <byte> - яке значення шукаємо?
void cmd_find(Memory& mem, istringstream &words, bool first_only){
    string l, h, b, d;
    long low, high, search_byte;
    int deepth = 8;

    if( //  зчитування слів, перетворення у числа
        !(words >> l) || !(words >> h) || !(words >> b) ||
        !parse_number(l, low) || !parse_number(h, high) || !parse_number(b, search_byte)
    ) {
        cout << "usage: find <lo> <hi> <byte> <?deepth=8>\n";
        return;
    }

    long ldeepth = 0;       //  зчитування глибини пошуку (опціонально)
    if(words >> d) {
        parse_number(d, ldeepth);       //  перетворення у число

        if(ldeepth > 0 && (size_t)ldeepth < MEM_SIZE){
            //  якщо більше 0 та менше за розмір пам'яті - прийняти значення
            deepth = ldeepth;
        }
    }

    //  збірка 16-адреси з двох байтів (8)
    U16 addr = 0;
    addr = addr | low;
    addr = addr | (high << 8);

    //  перетворення шуканого байту у число
    long byte_value = static_cast<long>(search_byte);
    if(byte_value < -1 || byte_value > 255) {    //  перевірка меж
        cout << "byte must not be 0-255 of -1 (non-zero search), got " << byte_value << "\n";
        return;
    }

    bool finded = false;    //  чи знайдено хоча б один збіг?

    //  цикл пошуку: для addr та наступних deepth адрес
    for(int i = 0; i < deepth; i++){    //  до глибини пошуку
        if((size_t)(addr + i) >= MEM_SIZE)        //  чи кінець пам'яті?
            break;

        Byte mem_value = mem.data[addr + i];    //  взяти значення

        //  якщо співпало
        if(
            (static_cast<long>(mem_value) == search_byte) ||            //  збіг
            (search_byte == -1 && static_cast<long>(mem_value) != 0)    //  ненульове значення, якщо пошук за -1
        
        ) {
            cout << "Value 0x" << hex << static_cast<long>(mem_value) << " was finded at address 0x" << (addr+i) << dec << "\n";
            finded = true;

            if(first_only)
                return;
        }
    }

    //  якщо жодного збігу
    if(!finded) {
        cout << "Value 0x" << hex << search_byte << " was NOT finded at 0x" << addr << " - 0x" << addr+deepth << dec << "\n";
    }
}

void cmd_draw_frame(Memory &mem){
    display::clear(mem);
    for(int i = 0; i < display::VRAM_COLS; i++)
        display::plot(mem, i, 0);
    //  нижня
    for(int i = 0; i < display::VRAM_COLS; i++)
        display::plot(mem, i, display::VRAM_ROWS-1);
    //  ліва
    for(int i = 0; i < display::VRAM_ROWS; i++)
        display::plot(mem, 0, i);
    //  права
    for(int i = 0; i < display::VRAM_ROWS; i++)
        display::plot(mem, display::VRAM_COLS-1, i);
}


void cmd_sprite(Sprite (&sprites)[8], istringstream& words){
    string i, x, y, vx, vy;
    
    //  check words
    if(
        !(words >> i) || !(words >> x) || !(words >> y) ||
        !(words >> vx) || !(words >> vy)
    ) {
        cout << "usage: sprite <i> <x> <y> <vx> <vy>\n";
        return;
    }

    //  strings => long
    long li, lx, ly, lvx, lvy;
    bool ic = parse_number(i, li);
    bool xc = parse_number(x, lx);
    bool yc = parse_number(y, ly);
    bool vxc = parse_number(vx, lvx);
    bool vyc = parse_number(vy, lvy);

    //  Validation
    if(!ic || li < 0 || li > 7){
        cout << "<i> must be 0..7\n"; return;
    }
    if(!xc || lx < 0 || lx > display::VRAM_COLS) {
        cout << "<x> must be 0.." << display::VRAM_COLS << "\n"; return;
    }
    if(!yc || ly < 0 || ly > display::VRAM_ROWS) {
        cout << "<y> must be 0.." << display::VRAM_ROWS << "\n"; return;
    }
    if(!vxc || lvx < -1 || lvx > 1){
        cout << "<vx> must be -1..1\n"; return;
    }
    if(!vyc || lvy < -1 || lvy > 1){
        cout << "<vy> must be -1..1\n"; return;
    }

    // Create sprite
    Sprite s;
    s.alive = true;
    s.x = (uint8_t)lx;
    s.y = (uint8_t)ly;
    s.vx = (int8_t)lvx;
    s.vy = (int8_t)lvy;

    //  Put into array
    sprites[(int)li] = s;
}

void cmd_tick(Sprite (&sprites)[8], Memory& mem){

    //  Clear display buffer
    display::clear(mem);

    for(int i = 0; i < 8; i++){

        if(
            sprites[i].x >= display::VRAM_COLS || sprites[i].y >= display::VRAM_ROWS
        ){
            continue;
        }

        //  calculate memory address
        int address = display::VRAM_LO + (sprites[i].y * 8) + (sprites[i].x / 8);
        int bit = 7 - (sprites[i].x % 8);

        //  set sprite to memory
        mem.data[address] |= ((Byte)1 << bit);

        uint8_t nextX = sprites[i].x + sprites[i].vx;
        uint8_t nextY = sprites[i].y + sprites[i].vy;

        //  sprite moving
        if(nextX < display::VRAM_COLS) sprites[i].x = nextX;
        if(nextY < display::VRAM_ROWS) sprites[i].y = nextY;

        // check display edges collision
        if(
            (sprites[i].x >= display::VRAM_COLS-1 && sprites[i].vx > 0) ||
            (sprites[i].x <= 0 && sprites[i].vx < 0)
        )
            sprites[i].vx *= -1;

        if(
            (sprites[i].y >= display::VRAM_ROWS-1 && sprites[i].vy > 0) ||
            (sprites[i].y <= 0 && sprites[i].vy < 0)
        )
            sprites[i].vy *= -1;
        
    }

    //  Show display
    display::show(mem);
}
void cmd_tick_loop(Sprite (&sprites)[8], Memory& mem, istringstream& words){
    string c;
    long lc;
    if(!(words >> c) || !parse_number(c, lc)){
        cout << "usage: tick_loop <c>, where c > 0\n";
        return;
    }
    
    for(int i = 0; i < (int)lc; i++){
        system("clear");
        cmd_tick(sprites, mem);
        usleep(TICK_DELAY * 1000);
    }
}



void cmd_show_sizes(){
    cout << "\n";
    cout << "sizeof(CPU)    = " << sizeof(CPU)  << '\n';
    cout << "alignof(CPU)   = " << alignof(CPU) << '\n';
    cout << "sizeof(Flags)  = " << sizeof(Flags)<< '\n';
    cout << "alignof(Flags) = "<< alignof(Flags)<< '\n';

    cout << "\n";

    cout << "sizeof(CPU) => {\n";
    cout << "  Memory* mem  |   sizeof(Memory*)  = " << sizeof(Memory*) << "\n";
    cout << "  U16 pc       |   sizeof(uint16_t) = " << sizeof(U16) << "\n";
    cout << "  U16 H        |   sizeof(uint16_t) = " << sizeof(U16) << "\n";
    cout << "  Byte a       |   sizeof(uint8_t)  = " << sizeof(Byte) << "\n";
    cout << "  Byte b       |   sizeof(uint8_t)  = " << sizeof(Byte) << "\n";
    cout << "  Flags f      |   sizeof(Flags)    = " << sizeof(Flags) << "\n";
    cout << "  bool halted  |   sizeof(bool)     = " << sizeof(bool) << "\n";
    cout << "}\n\n";

    cout << "Offsets: \n";
    cout << "   mem    @" << offsetof(CPU, mem)    << '\n';
    cout << "   pc     @" << offsetof(CPU, pc)     << '\n';
    cout << "   H      @" << offsetof(CPU, H)      << '\n';
    cout << "   a      @" << offsetof(CPU, a)      << '\n';
    cout << "   b      @" << offsetof(CPU, b)      << '\n';
    cout << "   f      @" << offsetof(CPU, f)      << '\n';
    cout << "   halted @" << offsetof(CPU, halted) << '\n';
}


void cmd_stack(const CPU& cpu) {
    const size_t SHOW = 8;      // скільки верхніх байтів показати
    Byte v = 0;

    cout << "SP = 0x" << hex << stack_sp(cpu.stack) << dec
         << "  (stack region 0x" << hex << STACK_LO << "..0x" << STACK_HI << dec << ")\n";

    bool any = false;
    for (size_t d = 0; d < SHOW; d++) {
        if (!stack_peek(cpu.stack, d, v)) break;
        any = true;
        cout << "  0x" << hex << (stack_sp(cpu.stack) + 1 + d) << dec << ": ";
        show_byte(v);
    }
    if (!any)
        cout << "  (stack is empty)\n";
}

void cmd_trace(CPU& cpu, istringstream& words) {
    string m;
    if (!(words >> m) || (m != "on" && m != "off")) {
        cout << "usage: trace <on|off>\n";
        return;
    }
    cpu.trace = (m == "on");
}