#ifndef KOT_H
#define KOT_H


#include "ilxer.h"
#include "misc.h"

#define GPR_NUM 32
#define DEF_PROGRAM_SIZE 64
#define DEF_MEMORY_SIZE 0xFFFFF
#define DEF_STACK_INIT	0x01000

#define IR()\
	X(IR_PUSH)\
	X(IR_PULL)\
	X(IR_CALL)\
	X(IR_RET)


#define X(name) name,

typedef enum{
	IR()
	IR_ILLEGAL,
	IR_TABLE_END
}kot_ir;


static kot_ir ir_table[] = {
	IR()	
};
#undef X

static char* ir_table_lh[] = {
	[IR_PUSH] = "push",
	[IR_PULL] = "pull",
	[IR_CALL] = "call",
	[IR_RET] = "ret",
	[IR_ILLEGAL] = "illegal"
};

typedef struct{
	char* label;
	bool fn;
	kot_ir bytecode;
	uint32_t arg_0;
	uint32_t arg_1;
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
	uint32_t *memory;
	uint32_t def_memory_size;
}vkot_machine;

static vkot_machine kotvm;

void kot_init_vm(Arena_header*ah);
int kot_parse(Arena_header* ah,lxer_header* lh, error_handler *eh);
void kot_get_bytecode();
void kot_get_memory_dump();
void kot_push_instruction(Arena_header* ah, kot_ir inst, char* label, uint32_t arg_0, uint32_t arg_1);


#ifndef KOT_C
#define KOT_C

#endif

#endif // KOT_H
