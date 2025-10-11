#ifndef KOT_H
#define KOT_H

#include <assert.h>
#include "ilxer.h"
#include "misc.h"

#define GPR_NUM 32
#define DEF_PROGRAM_SIZE 64
#define DEF_MEMORY_SIZE 0xFF
#define DEF_STACK_INIT	0x01000
#define DEF_SOURCE_SIZE  256 // intended as line of code, not individual character
#define DEF_GET_ARR_SIZE 256

#define IR()\
	X(IR_PUSH)\
	X(IR_PULL)\
	X(IR_JUMP)\
	X(IR_BEQ)\
	X(IR_BGE)\
	X(IR_BLT)\
	X(IR_BNE)\
	X(IR_MOV)\
	X(IR_LB)\
	X(IR_LH)\
	X(IR_LW)\
	X(IR_SB)\
	X(IR_SH)\
	X(IR_SW)\
	X(IR_RET)


#define X(name) name,

typedef enum{
	IR()
	IR_TAG,
	IR_ILLEGAL,
	IR_TABLE_END
}kot_ir;


static kot_ir ir_table[] = {
	IR()
	IR_TAG,
	IR_ILLEGAL,
	IR_TABLE_END
};
#undef X

static char* ir_table_lh[] = {
	[IR_PUSH] = "push",
	[IR_PULL] = "pull",
	[IR_JUMP] = "jump",
	[IR_BEQ] =	"beq",
	[IR_BGE] =	"bge",
	[IR_BLT] =	"blt",
	[IR_BNE] =	"bne" ,
	[IR_RET] =	"ret",
	[IR_MOV] =  "mov",
	[IR_LB] =	"lb",
	[IR_LH] =	"lh",
	[IR_LW] =	"lw",
	[IR_SB] =	"sb",
	[IR_SH] =	"sh",
	[IR_SW] =	"sw",
	[IR_TAG] =  "tag/fn-placeholder",
	[IR_ILLEGAL] = "illegal"
};

typedef struct{
	char* label;
	bool fn;
	kot_ir bytecode;
	uint32_t arg_0;
	uint32_t arg_1;
	uint32_t arg_2;
}inst_slice;

typedef struct{
	inst_slice* program;
	size_t tracker;
	size_t size;
}binary_rp;


typedef struct{
	uint32_t program_counter;
	uint32_t stack_pointer;
	uint32_t gpr[GPR_NUM];
	float fr[GPR_NUM];
	
	binary_rp bytecode_array;
	
	char *memory;
	uint32_t def_memory_size;
	uint32_t memory_tracker;

	char** program_source;
	size_t program_source_size;
	size_t program_source_tracker;
}vkot_machine;

static vkot_machine kotvm;
static size_t line;

static char** globl_variable;
static size_t globl_variable_tracker;
static size_t globl_variable_size;

void kot_init_vm(Arena_header*ah);
void kot_init_interpreter(Arena_header* ah);

void kot_push_globl_variable_def(Arena_header* ah,char* name);
bool kot_globl_variable_already_present(char* name);

int kot_parse(Arena_header* ah,lxer_header* lh, error_handler *eh, bool console);
void kot_get_bytecode();
void kot_get_memory_dump();
void kot_push_instruction(Arena_header* ah, kot_ir inst, char* label, uint32_t arg_0, uint32_t arg_1, uint32_t arg_2, bool is_fn);
void kot_get_program_list(FILE* filestream);
int kot_argument_processor(Arena_header * ah, lxer_header* lh, error_handler *eh, LXR_TOKENS type);
int kot_process_type(Arena_header* ah, lxer_header* lh, error_handler *eh);
void kot_set_line(size_t line);
size_t kot_write_mem(Arena_header* ah,char* string, int size);


#ifndef KOT_C
#define KOT_C

#endif

#endif // KOT_H
