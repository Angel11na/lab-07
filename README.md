


Структура стеку у stack.hpp задана як звичайний тип даних, що не відповідає умові зберігання стеку у ділянці пам'яті 0xF00-0xFFF. Вона замінена.

Подані у лабі сигнатури функцій у stack.hpp не відповідають _ISA.uk.md_. В сигнатурах uint16_t, тобто 2 байти, а у ISA операції мають на увазі один байт. Робота виконувалась з логікою для одного байту. 








# M1 — SP, PUSH, POP і Stack як тип. 

`SP` стартує з `0xFFF`; 
регіон стека — `0xF00–0xFFF`. 

```cpp
// memory.hpp
const size_t STACK_LO = 0xF00;
const size_t STACK_HI = 0xFFF;

// stack.hpp
struct Stack {
    Memory* mem = nullptr;
    U16 sp = STACK_HI;          // 0xFFF - стек порожній
};
```

### Реалізуйте `PUSH A`, `POP A`, `PUSH B`, `POP B`:
```cpp
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
```

### Додайте в `ember` команду `stack`, яка друкує `SP` і кілька верхніх байтів, щоб стек було видно.
```cpp
// ember.cpp
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
```

### Перевірка:

- **Послідовність команд:**
  - `LOADI_A 42;`   | Покласти 42 у регістр А
  - `PUSH_A;`       | Покласти з регістра А у стек
  - `LOADI_A 0;`    | Занулити регістр А
  - `POP_A;`        | Дістати зі стеку у регістр А
  - `OUTN;`         | Вивести з регістру А
  - `HALT`          | Зупинка
- **Команди для `ember`**
    ```
    set 0 0x20
    set 1 42
    set 2 0x50
    set 3 0x20
    set 4 0
    set 5 0x51
    set 6 0x03
    set 7 0x00
    step
    step
    stack <- у стеку 42
    step
    step
    step
    stack <- стек вже порожній
    ```
- **Виведення**
    ```
    ember> set 0 0x20
    set 1 42
    set 2 0x50
    set 3 0x20
    set 4 0
    set 5 0x51
    set 6 0x03
    set 7 0x00
    ember> 

    ember> step

    ember> step

    ember> stack
    SP = 0xffe  (stack region 0xf00..0xfff)
    0xfff: 42  0x2a  00101010  '*'

    ember> step

    ember> step

    ember> step
    42 
    ember> stack
    SP = 0xfff  (stack region 0xf00..0xfff)
    (stack is empty)

    ember> 
    ```





# M2 — CALL / RET. 

Додано у `cpu.cpp`:
```cpp
case Op::CALL: {
    U16 target = get16(*cpu.mem, cpu.pc + 1);                 // адреса підпрограми (2 байти після опкоду)
    U16 ret    = static_cast<U16>(cpu.pc + 3);                // CALL займає 3 байти => наступна інструкція

    if (!push(cpu.stack, static_cast<Byte>(ret & 0xFF))) {    // 1) молодший байт
        stack_overflow(cpu); return;
    }
    if (!push(cpu.stack, static_cast<Byte>(ret >> 8))) {      // 2) старший байт
        stack_overflow(cpu); return;
    }
    cpu.pc = target;
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
    cpu.pc = static_cast<U16>(low | (high << 8));
    break;
}
```



### Програма: 
- CALL printA, HALT; 
- printA: OUT, RET. 
- **Послідовність команд:**
    ```
    set 0 0x54      # CALL
    set 1 4         # адрес printA, молодший байт
    set 2 0         # адрес printA, старший байт
    set 3 0x00      # HALT
    set 4 0x02      # printA: OUT
    set 5 0x55      # RET
    reg a 65        # A = 'A', щоб OUT було що друкувати
    regs
    stack
    step
    stack
    step
    step
    stack
    step
    ```
- **Для копіювання у `ember`:**
    ```
    set 0 0x54
    set 1 4
    set 2 0
    set 3 0x00
    set 4 0x02
    set 5 0x55
    reg a 65
    ```
- **Виведення:**
    ```
    ember> set 0 0x54
    set 1 4
    set 2 0
    set 3 0x00
    set 4 0x02
    set 5 0x55
    reg a 65
    ember> 

    ember> regs
    PC = 0x0
    H = 0x00  0000_0000_0000_0000
    A = 65  0x41  01000001  'A'
    B = 0  0x0  00000000  '.'
    Z: 0
    N: 0
    C: 0
    SP = 0xfff

    ember> stack
    SP = 0xfff  (stack region 0xf00..0xfff)
    (stack is empty)

    ember> step

    ember> stack
    SP = 0xffd  (stack region 0xf00..0xfff)
    0xffe: 0  0x0  00000000  '.'
    0xfff: 3  0x3  00000011  '.'

    ember> step
    A

    ember> step

    ember> stack
    SP = 0xfff  (stack region 0xf00..0xfff)
    (stack is empty)

    ember> step

    ember> step
    halted
    ```


# M3 — рекурсія: fact

## MUL

Нова команда `MUL`: `A = A * B`. У `enum Op` додано `MUL = 0x1b`, у ALU нова
функція `MUL(a, b, flags)`, у `cpu.cpp` додано `case Op::MUL`, який записує результат у `A`

```cpp
Byte MUL(Byte a, Byte b, Flags &flags){
    unsigned p = static_cast<unsigned>(a) * b;
    Byte result = static_cast<Byte>(p);   // лишаються тільки молодші 8 біт
    flags.c = p > 0xFF;                   // C = 1, якщо добуток не влізло в байт
    flags.z = (result == 0);
    flags.n = (result & 0x80) != 0;
    return result;
}
```

Для зручності трасування додано команду `trace on|off`. Коли вона увімкнена,
кожен `CALL` і `RET` друкує рядок із `PC`, `SP` і `A`

## Лістинг fact (з ISA.uk.md §6, 30 байтів)

```
адр.  байти        текст
0000  20 05        LOADI A, 5        ; n = 5
0002  54 07 00     CALL  fact
0005  03           OUTN              ; надрукує 120
0006  00           HALT

0007  21 00        fact:  LOADI B, 0
0009  1A                  CMP   A, B       ; n == 0 ?
000A  31 1B 00            JZ    base
000D  21 01               LOADI B, 1
000F  1A                  CMP   A, B       ; n == 1 ?
0010  31 1B 00            JZ    base
0013  50                  PUSH  A          ; зберегти n — виклик знищить A
0014  19                  DEC   A
0015  54 07 00            CALL  fact       ; A = fact(n-1)
0018  53                  POP   B          ; B = збережене n
0019  1B                  MUL   A, B       ; A = fact(n-1) * n
001A  55                  RET
001B  20 01        base:  LOADI A, 1
001D  55                  RET
```

Завантаження в `ember`:
```
set 0 0x20
set 1 5
set 2 0x54
set 3 0x07
set 4 0x00
set 5 0x03
set 6 0x00
set 7 0x21
set 8 0x00
set 9 0x1a
set 10 0x31
set 11 0x1b
set 12 0x00
set 13 0x21
set 14 0x01
set 15 0x1a
set 16 0x31
set 17 0x1b
set 18 0x00
set 19 0x50
set 20 0x19
set 21 0x54
set 22 0x07
set 23 0x00
set 24 0x53
set 25 0x1b
set 26 0x55
set 27 0x20
set 28 0x01
set 29 0x55
trace on
run
```

## Конвенція виклику

- Аргумент передається в `A`, результат повертається в `A`
- `B` - тимчасовий регістр: підпрограма може його змінювати
- Прапорці після виклику не гарантовані
- Значення, яке має пережити рекурсивний виклик (`n`), підпрограма кладе
  на стек (`PUSH A`) і знімає після повернення (`POP B`)
- Адреса повернення (`PC + 3`, 2 байти) лежить на стеку: молодший байт глибше,
  старший зверху
- `PUSH` і `POP` мають бути парними, інакше `SP` не повернеться в `0xFFF`

## Результати

```
CALL  PC 0x2 -> 0x7  SP=0xffd  A=5
CALL  PC 0x15 -> 0x7  SP=0xffa  A=4
CALL  PC 0x15 -> 0x7  SP=0xff7  A=3
CALL  PC 0x15 -> 0x7  SP=0xff4  A=2
CALL  PC 0x15 -> 0x7  SP=0xff1  A=1
RET  PC 0x1d -> 0x18  SP=0xff3  A=1
RET  PC 0x1a -> 0x18  SP=0xff6  A=2
RET  PC 0x1a -> 0x18  SP=0xff9  A=6
RET  PC 0x1a -> 0x18  SP=0xffc  A=24
RET  PC 0x1a -> 0x5  SP=0xfff  A=120
120 
```

Після останнього `RET` керування повертається на `OUTN` (0x0005), який друкує `120`.

**Наскільки глибоко воно провалюється:** 5 вкладених `CALL` (`fact(5)` => ... => `fact(1)`)
Кожен рівень, який рекурсує, займає 3 байти стека: 2 байти адреси повернення + 1 байт
збереженого `n` (`PUSH A`). Найглибший рівень (`n = 1`) іде в `base` ще до `PUSH A`,
тож займає лише 2 байти. Разом `SP` опускається з `0xFFF` до `0xFF1`, тобто
використано 14 байтів із 256 регіону `0xF00–0xFFF`

Рядок 6 показує `A = 1`, бо це `RET` із `base`; на рядку 7 `A = 2`, бо після `POP B`
і `MUL` (1 * 2) повертається `fact(2)`, і так далі по ланцюжку: 2 => 6 => 24 => 120.

## fact(6)

Змінюємо першу пару байтів на `20 06` (`set 1 6`). Програма друкує **208**
за 72 кроки.

Пояснення: Регістр `A` має 8 біт і вміщує значення 0..255. Добуток
`120 * 6 = 720 = 2*256 + 208 = 0b10_1101_0000`, а в байт потрапляють тільки
молодші 8 біт (`0b1101_0000 = 208`), старші відкидаються. Прапорець `C` після
цього `1`. ALU знає, що добуток не вмістився.

Для беззнакових типів переповнення не є помилкою, це
загортання за модулем 256.

## Навмисне переповнення стека

Прибираємо базовий випадок: обидва `JZ base` (байти `0x000A–0x000C` і
`0x0010–0x0012`) замінюємо на `NOP` (`0x01`):

```
set 10 1
set 11 1
set 12 1
set 16 1
set 17 1
set 18 1
run
```

Тепер `base` недосяжний, рекурсія не зупиняється: `A` проходить через 0 і
загортається в 255, 254, ..., а кожен `CALL` і `PUSH A` з'їдає ще 3 байти стека,
поки він не закінчується. Результат:

```
Stack overflow: SP=0xeff is below STACK_LO=0xf00 (PC=0x15). CPU halted.
```




# M4 — розділіть бінарник. 

Бінарник з самого початку розділений по модулям:
- `main.cpp`
- `ember.cpp`
- `memory.cpp`
- `dump.cpp`
- `display.cpp`
- `cpu.cpp`
- `alu.cpp`
- `stack.cpp`

Код розподілений за логікою модулів.