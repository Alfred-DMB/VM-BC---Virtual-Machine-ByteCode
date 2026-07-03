#include "vm.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <readline/readline.h>
#include <readline/history.h>

#define MAX_LABELS 256
#define MAX_LINE_LEN 256

typedef struct { char name [64]; int addr; } Label;

typedef struct{
	unsigned char *data;
	size_t len;
	size_t cap;
} ByteBuf;

static void buf_init(ByteBuf *b){
	b->data = NULL;
	b->len = 0;
	b->cap = 0;
}

static void buf_ensure(ByteBuf *b, size_t needed){
	if(b->len + needed > b->cap){
		size_t new_cap = b->cap ? b->cap * 2: 256;
		while(new_cap < b->len + needed) new_cap *= 2;

		unsigned char *tmp = realloc(b->data, new_cap);
		if(!tmp){
			fprintf(stderr, "FATAL: OUT OF MEMORYYYY\n");
			exit(EXIT_FAILURE);
		}
		b->data = tmp;
		b->cap = new_cap;
	}
}
static void buf_write(ByteBuf *b, unsigned char byte){
	buf_ensure(b, 1);
	b->data[b->len++] = byte;
}

static void buff_write32(ByteBuf *b, int32_t v){
	buf_ensure(b, 4);
	b->data[b->len++] = (unsigned char)((v >> 24) & 0xFF);
	b->data[b->len++] = (unsigned char)((v >> 16) & 0xFF);
	b->data[b->len++] = (unsigned char)((v >> 8) & 0xFF);
	b->data[b->len++] = (unsigned char)(v & 0xFF);		
}



static int has_immediate(int op) {
    return op == OP_PUSH || op == OP_JMP || op == OP_JZ ||
           op == OP_JNZ || op == OP_CALL || op == OP_LOAD || op == OP_STORE;
}

static const char *op_name(int op) {
    static const char *names[] = {
        "halt","push","pop","dup","add","sub","mul","div","mod",
        "eq","ne","lt","gt","jmp","jz","jnz","call","ret",
        "load","store","print","prc"
    };
    if(op >= 0 && op < (int)(sizeof names / sizeof names[0]))
        return names[op];
    return "???";
}

static int opcode_of(const char *tok){  //easiest part lol
	struct{ const char *name; int op; } map[] = {
		{"halt",OP_HALT},{"push",OP_PUSH},{"pop",OP_POP},{"dup",OP_DUP},
		{"add",OP_ADD},{"sub",OP_SUB},{"mul",OP_MUL},{"div",OP_DIV},{"mod",OP_MOD},
		{"eq",OP_EQ},{"ne",OP_NE},{"lt",OP_LT},{"gt",OP_GT},
		{"jmp",OP_JMP},{"jz",OP_JZ},{"jnz",OP_JNZ},
		{"call",OP_CALL},{"ret",OP_RET},
		{"load",OP_LOAD},{"store",OP_STORE},
		{"print",OP_PRINT},{"prc",OP_PRINT_CHAR},
    };
    for(size_t i = 0; i < sizeof map / sizeof map[0]; i++)
    	if(strcmp(tok, map[i].name) == 0) return map[i].op;
      return -1;
}

static int find_label(Label *labels, int n, const char *name){
	for(int i = 0; i < n; i++)
		if(strcmp(labels[i].name, name) == 0) return labels[i].addr;
	fprintf(stderr, "ERROR: UNDEFINED LABEL joder '%s'\n", name);
	exit(EXIT_FAILURE); // no loop? then I died heh
}

static int parse_int(const char *str, int32_t *out){
	char *endptr;
	errno = 0;
	long val = strtol(str, &endptr, 0);  //uses 0 to detect if the number is hex or oct
	
	if(errno == ERANGE || val > INT32_MAX || val < INT32_MIN){
		fprintf(stderr, "ERROR: integer out of range aaaaa '%s'\n", str);
		return 0;
	}
	
	if(endptr == str || *endptr != '\0'){
		fprintf(stderr, "ERROR: invalid integer format '%s'\n", str);
		return 0;
	}
	*out = (int32_t)val;
	return 1;	//it works? yay
}

static unsigned char *assemble(const char *src, int *out_len){ //translator of texto to machine code
	ByteBuf code;
	buf_init(&code);

	Label labels[MAX_LABELS];
	int nlabels = 0;

    const char *p = src; 
    int addr = 0;
while (*p){  //uses *p to evite calling strlen() 
	while(*p && isspace((unsigned char)*p)) p++;
	if(*p == '\0') break;

	if(*p == '#'){
		while(*p && *p != '\n') p++;
		continue;
	}
	char line[MAX_LINE_LEN];
        int i = 0;
        while (*p && *p != '\n' && *p != '\r' && i < MAX_LINE_LEN - 1) {
            line[i++] = *p++;
        }
        line[i] = '\0';

         while (*p && *p != '\n' && *p != '\r') p++; 
        if (i >= MAX_LINE_LEN - 1 && *p != '\n' && *p != '\r') {
            fprintf(stderr, "ERROR: line too long broda (max %d chars)\n", MAX_LINE_LEN - 1);
            exit(1);
        }
        //parse first token
        char *s = line;
        while(*s && isspace((unsigned char)*s)) s++;
        if(*s== '\0' || *s == '#') continue;

        char tok[64]; int ti = 0;
        while(*s && !isspace((unsigned char)*s) && *s != '#' && ti <64)
        	tok[ti++] = *s++; //*s++ moves the pointer, byte on byte, kinda weird for me now heh
        tok[ti] = '\0';
        if(ti == 0) continue;

    //detection of tickets
    int is_label = 0;
    if(ti > 0 && tok[ti-1] == ':'){
    	tok[ti-1] = '\0';
    	is_label = 1;
    } else if (ti > 0 && tok[0] == ':'){
    	memmove(tok, tok+1, ti);
    	tok[ti - 1] = '\0'; //just to make sure that he will got aagain his \0
    	is_label = 1;
    }
    if(is_label){
    	if(tok[0] == '\0'){
    		fprintf(stderr, "ERROR: empty label name\n");
    		exit(1);
    	}
    	if(nlabels >= MAX_LABELS){
    		fprintf(stderr, "ERRO: too many labels, afolale(max %d)\n", MAX_LABELS);
    		exit(1);
    	}
    	strcpy(labels[nlabels].name, tok);
    	labels[nlabels++].addr = addr;
    } else {
    	int op = opcode_of(tok);
    	if(op < 0){
    		fprintf(stderr, "ERROR: unknow instruccion '%s'\n", tok);
    		exit(1);
    	}
    	addr += has_immediate(op) ? 5 : 1;
    }
}
//SECOND PHASE
p = src;
while(*p){
	while(*p && isspace((unsigned char)*p)) p++;
	if(*p == '\0') break;
	if(*p == '#'){while (*p && *p != '\n') p++; continue; }

	char line[MAX_LINE_LEN];
	int i = 0;
	while(*p && *p != '\n' && *p != '\r' && i < MAX_LINE_LEN - 1){
		line[i++] = *p++;
	}
	line[i] = '\0';
	while (*p && *p != '\n' && *p != '\r') p++;

        char *s = line;
        while (*s && isspace((unsigned char)*s)) s++;
        if (*s == '\0' || *s == '#') continue;

        char tok[64]; int ti = 0;
        while (*s && !isspace((unsigned char)*s) && *s != '#' && ti < 63)
            tok[ti++] = *s++;
        tok[ti] = '\0';
        if (ti == 0) continue;

        //ignore tickets of second phase
        if((ti > 0 && tok[ti-1] == ':') || (ti > 0 && tok[0] == ':')) continue;
        int op = opcode_of(tok); //another code converter yay
        if(op < 0){
        	fprintf(stderr, "ERROR: unknow instruccion '%s'\n", tok);
        	exit(1);
        }
        buf_write(&code, (unsigned char)op);

        if(has_immediate(op)){ 
        	while(*s && isspace((unsigned char)*s)) s++; //for empty spaces
        	if(*s == '\0' || *s == '#') {
        		fprintf(stderr, "ERROR: operand expected for '%s'\n", tok);
        		exit(1);
        	}
        	char arg[64]; int ai = 0;
        	while(*s && !isspace((unsigned char)*s) && *s != '#' && ai < 63)
        		arg[ai++] = *s++;
        	arg[ai] = '\0';

        	if(op == OP_PUSH || op == OP_LOAD || op == OP_STORE) {
        		int32_t val;
        		if(!parse_int(arg, &val)) exit(1);
        		buff_write32(&code, val);
        	} else {
        		//jumps and calls
        		if(arg[0] == ':') memmove(arg, arg + 1, strlen(arg));
        		buff_write32(&code, find_label(labels, nlabels, arg));
        	}
        }
    }
    *out_len = (int)code.len; //saves the code long
    return code.data; //free the caller! 
}
// tools and ejecution of archives, read the bin basically
//search the archive
static char *read_file(const char *path){
   FILE *f = fopen(path, "rb"); //rb reading byte
   if(!f) return NULL;
   fseek(f, 0, SEEK_END);
   long sz = ftell(f);
   rewind(f);

   char *src = malloc((size_t)sz + 1); //give some mem men
   if(!src){ fclose(f); return NULL; }

   size_t r = fread(src, 1, (size_t)sz, f);
   (void)r; //reading
   src[sz] = '\0';
   fclose(f);
   return src;
}

static int run_code(unsigned char *code, int code_size, int quiet){
  if(!quiet) printf("---Execution ---\n");
  VM vm;
  vm_init(&vm, code, code_size);
  int result = vm_exec(&vm);
  vm_free(&vm);
  return result;
}

static void disassemble(const unsigned char *code, int len) {
    printf("--- Disassembly (%d bytes) ---\n", len);
    int i = 0;
    while(i < len) {
        int op = code[i];
        printf("  [%3d]  %s", i, op_name(op));
        i++;
        if(has_immediate(op) && i + 4 <= len) {
            int val = (int32_t)((uint32_t)code[i] << 24 |
                                (uint32_t)code[i+1] << 16 |
                                (uint32_t)code[i+2] << 8 |
                                (uint32_t)code[i+3]);
            printf(" %d", val);
            i += 4;
        }
        printf("\n");
    }
}

static void repl(void) {
    printf("byteVM REPL — enter instructions, empty line to run, 'quit' to exit.\n");
    char block[4096] = {0};
    char *line;
    using_history();
    while((line = readline("bvm> ")) != NULL) {
        if(strcmp(line, "quit") == 0 || strcmp(line, "exit") == 0) {
            free(line);
            break;
        }
        if(line[0] == '\0') {
            if(block[0] == '\0') { free(line); continue; }
            size_t blen = strlen(block);
            char *prog = malloc(blen + 16);
            snprintf(prog, blen + 16, "%s\nhalt\n", block);
            block[0] = '\0';
            int code_len;
            unsigned char *code = assemble(prog, &code_len);
            free(prog);
            disassemble(code, code_len);
            run_code(code, code_len, 0);
            free(code);
            free(line);
            continue;
        }
        add_history(line);
        size_t blen = strlen(block);
        snprintf(block + blen, sizeof block - blen, "%s\n", line);
        free(line);
    }
}

static const char *demo_prog =
    "# Demo: compute 5! (factorial) and print\n"
    "push 1\npush 5\ncall fact\nprint\npush 10\nprc\nhalt\n"
    ":fact\nstore 0\nload 0\npush 1\ngt\njz base\n"
    "load 0\nload 0\npush 1\nsub\ncall fact\nmul\nret\n"
    ":base\npush 1\nret\n";

int main(int argc, char **argv){
  if(argc >= 2 && strcmp(argv[1], "-i") == 0) {
    repl();
    return 0;
  }

  int quiet = (argc >= 3 && strcmp(argv[2], "-q") == 0);
  char *src;
  int free_src = 0;

  if(argc < 2){
    src = (char*)demo_prog;   // sin argumentos: corre la demo
  } else {
    src = read_file(argv[1]);
    if(!src){
      fprintf(stderr, "ERROR: cannot read '%s': %s\n", argv[1], strerror(errno));
      return 1;
    }
    free_src = 1;
  }

  int code_len;
  unsigned char *code = assemble(src, &code_len);
  if(free_src) free(src);

  disassemble(code, code_len);
  int ret = run_code(code, code_len, quiet);
  free(code);
  if(!quiet) printf("\n--- exit code %d ---\n", ret);
  return ret;
}
