#ifndef VM_H
#define VM_H

#include <stdint.h>

#define STACK_MAX 1024
#define LOCALS_MAX 256
#define CALL_DEPTH 129

typedef int32_t Value;  //forces to int using 32bits

typedef enum {
	OP_HALT       =0,
	OP_PUSH       =1,
	OP_POP        =2,
	OP_DUP        =3,
	OP_ADD        =4,
	OP_SUB        =5,
	OP_MUL        =6,
	OP_DIV        =7,
	OP_MOD        =8,
	OP_EQ         =9,
	OP_NE         =10,
	OP_LT         =11,
	OP_GT         =12,
	OP_JMP        =13,
	OP_JZ         =14,
	OP_JNZ        =15,
	OP_CALL       =16,
	OP_RET        =17,
	OP_LOAD       =18,
	OP_STORE      =19,
	OP_PRINT      =20,
	OP_PRINT_CHAR =21,
} Opcode;

typedef struct {
	Value    stack[STACK_MAX];
	int      sp;
	Value    locals[LOCALS_MAX];
	uint8_t  *ip;
	uint8_t  *code;
	int      code_size;
	uint8_t  *call_stack[CALL_DEPTH];
	int      fp;
	
} VM;

void vm_init(VM *vm, uint8_t *code, int code_size);
int vm_exec(VM *vm);
void vm_free(VM *vm);

#endif