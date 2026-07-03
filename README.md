# byteVM

A toy stack-based bytecode VM written in C.
Built because I wanted to understand how virtual machines work from the inside.

## How it works

```
.basm  ──→  assembler  ──→  bytecode  ──→  VM
```

The assembler works in two passes:
1. Scans labels and calculates addresses
2. Emits bytecode

The VM executes bytecode using a stack, local variables, and call support.

## Instruction set

| Op | Name   | Description |
|----|--------|-------------|
| 0  | halt   | Stop execution |
| 1  | push N | Push N onto stack |
| 2  | pop    | Pop from stack |
| 3  | dup    | Duplicate top of stack |
| 4  | add    | Addition |
| 5  | sub    | Subtraction |
| 6  | mul    | Multiplication |
| 7  | div    | Division |
| 8  | mod    | Modulo |
| 9  | eq     | Equality |
| 10 | ne     | Not equal |
| 11 | lt     | Less than |
| 12 | gt     | Greater than |
| 13 | jmp    | Unconditional jump |
| 14 | jz     | Jump if zero |
| 15 | jnz    | Jump if not zero |
| 16 | call   | Call subroutine |
| 17 | ret    | Return from subroutine |
| 18 | load N | Load local variable N |
| 19 | store N| Store in local variable N |
| 20 | print  | Print integer |
| 21 | prc    | Print character |

## Build

```sh
make
```

## Usage

```sh
./vm program.basm
```

## Example

```asm
push 1
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

## References

- Crafting Interpreters (Robert Nystrom) — the book that got me into this
