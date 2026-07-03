# byteVM Assembly Exercises

A series of exercises from least to most complex to practice programming in byteVM's toy ASM.

## Before you start

Quick reminder:

| Instruction | What it does |
|-------------|--------------|
| `push N` | pushes N onto the stack |
| `pop` | pops and discards the top |
| `dup` | duplicates the top |
| `add`, `sub`, `mul`, `div`, `mod` | a OP b → result (pops two, pushes one) |
| `eq`, `ne`, `lt`, `gt` | a OP b → 1 or 0 (pops two, pushes one) |
| `jmp lbl` | always jumps |
| `jz lbl` | jumps if top == 0 |
| `jnz lbl` | jumps if top != 0 |
| `call lbl` | calls subroutine |
| `ret` | returns from subroutine |
| `load N` | pushes local[N] onto stack |
| `store N` | stores top in local[N] |
| `print` | prints integer |
| `prc` | prints ASCII character |

**Arithmetic**: `sub` and `div` do `a - b` and `a / b` where `a` is lower on the stack (pushed first).

```
push 20      # a
push 5       # b
sub          # 20 - 5 = 15
```

---

## Exercise 1 — Hello world

**Goal**: Print `"Hello"` using ASCII numbers.

Hint: `H=72`, `e=101`, `l=108`, `l=108`, `o=111`. Use `prc` for each letter and `push 10` + `prc` for the newline.

<details>
<summary>Solution</summary>

```asm
push 72
prc
push 101
prc
push 108
prc
push 108
prc
push 111
prc
push 10
prc
halt
```
</details>

---

## Exercise 2 — Area of a rectangle

**Goal**: Calculate `width × height` and print it. Try `width=7`, `height=3`.

<details>
<summary>Solution</summary>

```asm
push 7
push 3
mul
print
push 10
prc
halt
```
</details>

---

## Exercise 3 — Temperature: Celsius to Fahrenheit

**Goal**: Convert `°C` to `°F` using the formula `F = C × 9/5 + 32`. Try `C=100` → should give `212`.

Hint: first `C × 9`, then divide by `5`, then add `32`. Be careful with the order of `div`.

<details>
<summary>Solution</summary>

```asm
push 100
push 9
mul
push 5
div
push 32
add
print
halt
```
</details>

---

## Exercise 4 — Even or odd

**Goal**: Given a number, print `1` if it's even, `0` if it's odd.

Hint: `N % 2` → if it gives `0`, it's even. Try `N=7` and `N=42`.

<details>
<summary>Solution</summary>

```asm
push 7
push 2
mod
push 0
eq
print
push 10
prc
halt
```
</details>

---

## Exercise 5 — Countdown with loop

**Goal**: Print the numbers from 5 down to 1, each on its own line.

Hint: use `dup`, `print`, `push 1 sub`, a conditional jump, and a final `pop` to clean the stack.

Expected output:
```
5
4
3
2
1
```

<details>
<summary>Solution</summary>

```asm
push 5
:loop
dup
print
push 10
prc
push 1
sub
dup
push 0
gt
jnz loop
pop
halt
```
</details>

---

## Exercise 6 — Sum from 1 to N

**Goal**: Given `N` (e.g., `N=10`), calculate `1+2+...+N` and print it.

Result for 10: `55`.

Hint: you need two accumulators — one for the counter (i) and one for the sum. Use `store`/`load`.

<details>
<summary>Solution</summary>

```asm
push 10
store 0        # local[0] = N (limit)
push 0
store 1        # local[1] = running sum
push 1
store 2        # local[2] = counter i

:loop
load 2
load 1
add
store 1        # sum += i
load 2
push 1
add
store 2        # i++
load 2
load 0
gt             # i > N ?
jz loop        # if not, continue

load 1         # final result
print
push 10
prc
halt
```
</details>

---

## Exercise 7 — Iterative factorial

**Goal**: Calculate `N!` with a loop (no recursion). Try `N=6` → `720`.

<details>
<summary>Solution</summary>

```asm
push 6
store 0        # N
push 1
store 1        # result = 1
push 1
store 2        # i = 1

:loop
load 2
load 1
mul
store 1        # result *= i
load 2
push 1
add
store 2        # i++
load 2
load 0
gt             # i > N ?
jz loop        # if not, continue

load 1
print
push 10
prc
halt
```
</details>

---

## Exercise 8 — Fibonacci (iterative)

**Goal**: Calculate the `N`-th Fibonacci number. `F(0)=0, F(1)=1, F(n)=F(n-1)+F(n-2)`. Try `N=10` → `55`.

<details>
<summary>Solution</summary>

```asm
push 10
store 0        # N
push 0
store 1        # a = 0
push 1
store 2        # b = 1
push 2
store 3        # i = 2

:loop
load 1
load 2
add
store 4        # c = a + b
load 2
store 1        # a = b
load 4
store 2        # b = c
load 3
push 1
add
store 3        # i++
load 3
load 0
gt
jnz done
jmp loop

:done
load 2
print
push 10
prc
halt
```
</details>

---

## Exercise 9 — Maximum of three numbers

**Goal**: Given three numbers in locals, determine the maximum and print it. Try `A=12, B=45, C=7` → `45`.

<details>
<summary>Solution</summary>

```asm
push 12
store 0
push 45
store 1
push 7
store 2

load 0
load 1
gt
jz b_bigger
load 0
load 2
gt
jz c_bigger_a
load 0
jmp end

:b_bigger
load 1
load 2
gt
jz c_bigger_b
load 1
jmp end

:c_bigger_a
:c_bigger_b
load 2

:end
print
push 10
prc
halt
```
</details>

---

## Exercise 10 — Print a multiplication table

**Goal**: Print the 7 times table: `"7 x 1 = 7"`, etc. from 1 to 10. Use `print` and `prc` for formatting.

Expected output (unformatted, just numbers):
```
7
14
21
...
70
```

<details>
<summary>Solution</summary>

```asm
push 1
store 0        # i = 1

:loop
push 7
load 0
mul
print
push 10
prc
load 0
push 1
add
store 0
load 0
push 10
gt
jz loop
halt
```
</details>

---

## Exercise 11 — Prime numbers (detection)

**Goal**: Given `N`, determine if it's prime: print `1` if prime, `0` if not. Try `N=17` → `1`, `N=15` → `0`.

Hint: test divisors from 2 to `N-1`. If `N % d == 0`, it's not prime.

<details>
<summary>Solution</summary>

```asm
push 17
store 0        # N
push 2
store 1        # d = 2

:loop
load 0
load 1
mod
push 0
eq
jnz not_prime   # if N % d == 0, not prime
load 1
push 1
add
store 1        # d++
load 1
load 0
lt
jnz loop       # if d < N, continue

push 1
print
push 10
prc
halt

:not_prime
push 0
print
push 10
prc
halt
```
</details>

---

## Exercise 12 — Subroutine: square

**Goal**: Make a `square` subroutine that receives a value through the stack, duplicates and multiplies it, leaving the result on the stack. Try `7` → `49`.

<details>
<summary>Solution</summary>

```asm
push 7
call square
print
push 10
prc
halt

:square
dup
mul
ret
```
</details>

---

## Exercise 13 — Subroutine: recursive factorial

**Goal**: Reimplement factorial recursively (using `call`/`ret`). This is byteVM's classic example.

<details>
<summary>Solution</summary>

```asm
push 1
push 6
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
</details>

---

## Exercise 14 — Recursive power

**Goal**: `pow` subroutine that receives `base` and `exponent` (in that order on the stack) and leaves `base^exp` on the stack. Try `2^10 = 1024`.

Hint: base case `exp == 0` → result `1`. Otherwise, `base * pow(base, exp-1)`.

<details>
<summary>Solution</summary>

```asm
push 2
push 10
call pow
print
push 10
prc
halt

:pow
store 0        # exp
store 1        # base
load 0
push 0
eq
jz rec
push 1
ret

:rec
load 1
load 0
push 1
sub
call pow
mul
ret
```
</details>

---

## Exercise 15 — GCD (Euclidean algorithm) recursive

**Goal**: Calculate the greatest common divisor of `a` and `b` using Euclid's algorithm: `GCD(a, b) = a if b=0, else GCD(b, a%b)`. Try `48` and `18` → `6`.

<details>
<summary>Solution</summary>

```asm
push 48
push 18
call gcd
print
push 10
prc
halt

:gcd
store 0        # b
store 1        # a
load 0
push 0
eq
jz go_on
load 1
ret

:go_on
load 0
load 1
load 0
mod
call gcd
ret
```
</details>

---

## Exercise 16 — Print byte in binary

**Goal**: Given a number (0-255), print its 8 bits as `0`s and `1`s. Try `170` (`0xAA`) → `10101010`.

Hint: loop 8 iterations, shift bits (and with 1, then divide by 2).

<details>
<summary>Solution</summary>

```asm
push 170
store 0        # n
push 8
store 1        # counter

:loop
load 0
push 1
and
print          # prints 0 or 1
load 0
push 2
div
store 0        # n = n / 2
load 1
push 1
sub
store 1
load 1
push 0
gt
jnz loop
push 10
prc
halt
```
Prints bits in reverse (LSB first). To print in order, you would save them in locals first.

</details>

---

## Exercise 17 — Collatz conjecture

**Goal**: Given `N`, print the Collatz sequence until reaching 1. If `N` is even → `N/2`, if odd → `3N+1`. Try `N=6`:

```
6
3
10
5
16
8
4
2
1
```

<details>
<summary>Solution</summary>

```asm
push 6
store 0

:loop
load 0
print
push 10
prc
load 0
push 1
eq
jnz end
load 0
push 2
mod
push 1
eq
jz even

# odd: 3n + 1
load 0
push 3
mul
push 1
add
store 0
jmp loop

:even
load 0
push 2
div
store 0
jmp loop

:end
halt
```
</details>

---

## Exercise 18 — Print string with inline data

**Goal**: Print `"HI"` by storing characters as constants in the code and using a subroutine.

There's no data in memory, only constants in instructions. But you can chain `prc`s.

<details>
<summary>Solution</summary>

```asm
call print_hi
halt

:print_hi
push 72
prc
push 73
prc
push 10
prc
ret
```
</details>

---

## Exercise 19 — Average calculator

**Goal**: Given 5 numbers stored in locals, calculate the integer average (sum / 5) and print it. Try `10, 20, 30, 40, 50` → `30`.

<details>
<summary>Solution</summary>

```asm
push 10
store 0
push 20
store 1
push 30
store 2
push 40
store 3
push 50
store 4

load 0
load 1
add
load 2
add
load 3
add
load 4
add
push 5
div
print
push 10
prc
halt
```
</details>

---

## Exercise 20 — Minimal REPL interpreter

**Goal** (no code): Using `./vm -i`, write a small program line by line in the REPL. For example, calculate `(3 + 5) * 2`:

```
bvm> push 3
bvm> push 5
bvm> add
bvm> push 2
bvm> mul
bvm> print
bvm> push 10
bvm> prc
bvm>
```

The REPL assembles and executes when you leave a blank line.

---

## Bonus — Extra challenges

1. **Leap year**: Given a year, print `1` if it's a leap year, `0` if not (rule: divisible by 4, but not by 100, unless also divisible by 400).
2. **Reverse digits**: Given `1234`, print `4321`.
3. **Tower of Hanoi**: Simulate the moves (count steps) for N disks.
4. **State machine**: A counter that prints `"Fizz"` if multiple of 3, `"Buzz"` if multiple of 5, `"FizzBuzz"` if both.

---

> Any questions or would you like me to add more exercises on a particular topic?
