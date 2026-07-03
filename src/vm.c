#include "vm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void vm_init(VM *vm, uint8_t *code, int code_size){
	vm->sp = -1;                                 //nothing yet
	vm->fp = 0;                                  //nocalls yet
	vm->code = code;                             //save the *p of the program
	vm->code_size = code_size;                   //size of bytes
	vm->ip = code;                               //pointer for the program
	memset(vm->locals, 0, sizeof(vm->locals));   //all local variables in 0
}

static void push(VM *vm, Value val){
	if(vm->sp >= STACK_MAX - 1) {
		fprintf(stderr, "FATAL: stack fckin overflow\n");
		exit(EXIT_FAILURE);
	}
	vm->stack[++vm->sp] = val;
}

static Value pop(VM *vm){
	if(vm->sp < 0) {
		fprintf(stderr, "FATAL: stack fckin underflow\n");
		exit(EXIT_FAILURE);
	}
	return vm->stack[vm->sp--];
}

//Lectura de enteros con comprobación de límites
//Evita que la VM lea memoria basura si el bytecode se corta de golpe.
static int read_i32(VM *vm) {
    if (vm->ip + 4 > vm->code + vm->code_size) {
        fprintf(stderr, "FATAL: fkcin buffer overflow\n");
        exit(EXIT_FAILURE);
    }
    int v = (int32_t)((uint32_t)vm->ip[0] << 24 |
                      (uint32_t)vm->ip[1] << 16 |
                      (uint32_t)vm->ip[2] << 8  |
                      (uint32_t)vm->ip[3]);
    vm->ip += 4;
    return v;
}

//Helper para saltos seguros
//Evita que un JMP o CALL malicioso o erróneo envíe el IP al hiperespacio.
static void jump_to(VM *vm, int addr) {
    if (addr < 0 || addr >= vm->code_size) {
        fprintf(stderr, "FATAL: jump out of bounds to %d\n", addr);
        exit(EXIT_FAILURE);
    }
    vm->ip = vm->code + addr;
}

int vm_exec(VM *vm) {
    const uint8_t *code_end = vm->code + vm->code_size;

    for (;;) {
        //Comprobación de límites en cada instrucción
        if (vm->ip >= code_end) {
            fprintf(stderr, "FATAL: instruction pointer out of bounds (missing HALT?)\n");
            return 1;
        }

        Opcode op = (Opcode)*vm->ip++;
        
        switch (op) {
            case OP_HALT: return 0;
            case OP_PUSH: push(vm, read_i32(vm)); break;
            case OP_POP:  pop(vm); break;
            case OP_DUP: { Value v = pop(vm); push(vm, v); push(vm, v); break; }
            
            case OP_ADD: { Value b = pop(vm), a = pop(vm); push(vm, a + b); break; }
            case OP_SUB: { Value b = pop(vm), a = pop(vm); push(vm, a - b); break; }
            case OP_MUL: { Value b = pop(vm), a = pop(vm); push(vm, a * b); break; }
            
            //Protección contra División/Módulo por Cero
            case OP_DIV: { 
                Value b = pop(vm), a = pop(vm); 
                if (b == 0) { fprintf(stderr, "FATAL: division by zero, you dumb\n"); exit(EXIT_FAILURE); }
                push(vm, a / b); 
                break; 
            }
            case OP_MOD: { 
                Value b = pop(vm), a = pop(vm); 
                if (b == 0) { fprintf(stderr, "FATAL: modulo by zero\n"); exit(EXIT_FAILURE); }
                push(vm, a % b); 
                break; 
            }
            
            case OP_EQ:  { Value b = pop(vm), a = pop(vm); push(vm, a == b); break; }
            case OP_NE:  { Value b = pop(vm), a = pop(vm); push(vm, a != b); break; }
            case OP_LT:  { Value b = pop(vm), a = pop(vm); push(vm, a < b); break; }
            case OP_GT:  { Value b = pop(vm), a = pop(vm); push(vm, a > b); break; }
            
            //Uso del helper jump_to para seguridad en saltos
            case OP_JMP: { int a = read_i32(vm); jump_to(vm, a); break; }
            case OP_JZ:  { int a = read_i32(vm); if (pop(vm) == 0) jump_to(vm, a); break; }
            case OP_JNZ: { int a = read_i32(vm); if (pop(vm) != 0) jump_to(vm, a); break; }
            
            case OP_CALL: {
                int a = read_i32(vm);
                if (vm->fp >= CALL_DEPTH) { 
                    fprintf(stderr, "FATAL: call stack overflow\n"); 
                    exit(EXIT_FAILURE); 
                }
                vm->call_stack[vm->fp++] = vm->ip;
                jump_to(vm, a);
                break;
            }
            case OP_RET: {
                if (vm->fp == 0) { 
                    fprintf(stderr, "FATAL: ret from main\n"); 
                    exit(EXIT_FAILURE); 
                }
                vm->ip = vm->call_stack[--vm->fp];
                break;
            }
            
            //Acceso a variables locales con límites
            //Evita que un STORE 999 corrompa la memoria de la VM.
            case OP_LOAD:  { 
                int i = read_i32(vm); 
                if (i < 0 || i >= LOCALS_MAX) { 
                    fprintf(stderr, "FATAL: local index out of bounds (%d)\n", i); 
                    exit(EXIT_FAILURE); 
                }
                push(vm, vm->locals[i]); 
                break; 
            }
            case OP_STORE: { 
                int i = read_i32(vm); 
                if (i < 0 || i >= LOCALS_MAX) { 
                    fprintf(stderr, "FATAL: local index out of bounds (%d)\n", i); 
                    exit(EXIT_FAILURE); 
                }
                vm->locals[i] = pop(vm); 
                break; 
            }
            
            case OP_PRINT:      printf("%d", pop(vm)); break;
            case OP_PRINT_CHAR: putchar((char)pop(vm)); break;
            
            default:
                fprintf(stderr, "FATAL: unknown opcode %d at offset %ld\n",
                        op, (long)(vm->ip - vm->code - 1));
                return 1;
        }
    }
}

void vm_free(VM *vm) {
    (void)vm; //No hay memoria dinámica que liberar en esta estructura
}