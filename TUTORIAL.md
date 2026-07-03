# byteVM Tutorial

## How it works

The VM has a **stack**. Almost everything happens through it:

```
push 5    → stack: [5]
push 3    → stack: [5, 3]
add       → pops 3 and 5, adds, pushes result → stack: [8]
print     → pops 8 and prints it → stack: []
```

---

## Instructions one by one

### halt
Stops execution. Always at the end.

```asm
halt
```

### push N
Pushes number N onto the stack.

```asm
push 42    → stack: [42]
```
N can be decimal (`42`) or hexadecimal (`0x2A`).

### pop
Pops the top of the stack and discards it.

```asm
push 10
push 20
pop        → discards 20, stack: [10]
```

### dup
Duplicates the top of the stack.

```asm
push 5
dup        → stack: [5, 5]
```
Useful before `print` if you want to keep the value.

### add / sub / mul / div / mod
Arithmetic operations. Pop two values, operate, push the result.

```asm
push 10
push 3
add        → stack: [13]

push 10
push 3
sub        → 10 - 3 = 7

push 10
push 3
mul        → 10 * 3 = 30

push 10
push 3
div        → 10 / 3 = 3 (integer)

push 10
push 3
mod        → 10 % 3 = 1
```

**Important**: `sub` and `div` do `first - second` and `first / second`.
The first `pop` is the second operand:

```asm
push 20    # first value (a)
push 5     # second value (b)
sub        # a - b = 20 - 5 = 15
```

### eq / ne / lt / gt
Comparisons. Pop two values, push 1 (true) or 0 (false).

```asm
push 5
push 5
eq         → 1 (true)

push 5
push 3
gt         → 5 > 3 = 1

push 5
push 3
lt         → 5 < 3 = 0
```

### jmp / jz / jnz — Jumps
- **jmp label**: always jumps
- **jz label**: jumps if top of stack is 0
- **jnz label**: jumps if top of stack is NOT 0

```asm
push 5
:loop
print
push 1
sub
dup
push 0
gt
jnz loop    # jumps if gt gave 1 (true)
pop
halt
```
Prints: `54321`

### call label / ret
Subroutine call. `call` saves the return address, jumps to the label. `ret` returns.

```asm
push 5
call double
print
halt

:double
dup
add
ret
```
Prints: `10`

**How it works**:
1. `call double` saves "come back here" and jumps to `:double`
2. Executes `dup`, `add` (double and add = 5+5 = 10)
3. `ret` returns to where it was saved
4. `print` prints 10

### load N / store N
Local variables. There are 256 (0 to 255).
- **store N**: pops from stack and stores in local N
- **load N**: pushes the value of local N onto the stack

```asm
push 42
store 0       # local[0] = 42
push 10
store 1       # local[1] = 10
load 0        # stack: [42]
load 1        # stack: [42, 10]
add           # 42 + 10 = 52
print
halt
```

### print
Pops the top of the stack and prints it as an integer. **No newline**.

```asm
push 42
print         # shows "42"
push 10
prc           # newline (ASCII 10 = \n)
```

### prc
Pops the top of the stack and prints it as a **character** (ASCII).

```asm
push 65
prc           # shows 'A'
push 10
prc           # newline
```

---

## Common patterns

### Counter loop
```asm
push 5
:loop
dup
print
push 1
sub
dup
push 0
gt
jnz loop
pop
push 10
prc
halt
```

### If-then
```asm
push 5
push 3
gt            # 5 > 3 → 1
jz false
push 1
print
push 10
prc
halt
:false
push 0
print
push 10
prc
halt
```

### Recursive factorial
```asm
push 5
call fact
print
push 10
prc
halt

:fact
store 0
load 0
push 1
gt
jz base
load 0
load 0
push 1
sub
call fact
mul
ret

:base
push 1
ret
```

---

## Tips

- Use `dup` before `print` if you need the value afterwards
- `10` in `prc` = newline (`\n`)
- Comments start with `#`
- Labels can be `:name` at the start or `name:` at the end
- `ret` without a previous `call` = fatal error
- If you see `FATAL: stack underflow`, you missed a `push` before the operation

---

## Common errors

```
ERROR: unknow instruccion
```
You misspelled an instruction.

```
FATAL: stack underflow
```
You're performing an operation that needs values on the stack and there aren't enough.

```
FATAL: stack overflow
```
You pushed more than 1024 values onto the stack (almost always an infinite loop without `halt`).

```
FATAL: ret from main
```
You used `ret` without having done `call` first.
