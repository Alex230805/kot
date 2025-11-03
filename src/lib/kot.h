#ifndef KOT_H
#define KOT_H

#include <assert.h>
#include "ilxer.h"
#include "misc.h"

#define GPR_NUM 32

#define DEF_PROGRAM_SIZE 64
#define DEF_MEMORY_SIZE 0xFFFFF

#define DEF_STACK_INIT	0x0FFFF
#define DEF_HEAP_INIT	0x10000

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
	X(IR_RET)\
	X(IR_HALT)

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
	[IR_HALT] = "halt",
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

#define STRT 0
#define COND 1
#define FUNC 2

typedef struct scope{
	int type;
	char** var_def;
	size_t var_def_tracker;
	size_t var_def_size;
	struct scope* master; // scope from which this depend on
	List_header* list; // list of instruction
}scope;

typedef struct{
	int type;
	scope* aster;
	scope* branch_true;
	scope* branch_false;
}scope_branch;

typedef struct{
	uint32_t program_counter;
	int32_t stack_pointer;
	uint32_t gpr[GPR_NUM];
	float fr[GPR_NUM];
	
	inst_slice* bytecode_array;
	size_t bytecode_array_size;
	size_t bytecode_array_tracker;

	uint8_t *memory;
	uint32_t def_memory_size;
	uint32_t memory_tracker;

	scope *main_scope;
	scope *cache_scope;

	char** program_source;
	size_t program_source_size;
	size_t program_source_tracker;
}vkot_machine;


typedef enum{
	KOT_INT, 
	KOT_CHAR, 
	KOT_BOOL,
	KOT_VOID,
	KOT_STR,
	KOT_DOUBLE,
	KOT_FLOAT,
	KOT_UNDEFINED
} KOT_TYPE;


static char* type_table_lh[] = {
	[KOT_INT] = "int", 
	[KOT_CHAR] = "char", 
	[KOT_BOOL] = "bool",
	[KOT_VOID] = "void",
	[KOT_STR] = "str",
	[KOT_DOUBLE] = "double",
	[KOT_FLOAT] = "float",
	[KOT_UNDEFINED] = "undefined type"
};

typedef struct{
	char* name;
	size_t param_len;
	KOT_TYPE* param_type;
}fn_signature;

static vkot_machine kotvm;
static size_t line;

static	char** glob_var_def;
static	size_t glob_var_def_tracker;
static	size_t glob_var_def_size;


static fn_signature* globl_fn_signature;
static size_t globl_fn_signature_tracker;
static size_t globl_fn_signature_size;

void kot_init_vm(Arena_header*ah);
void kot_init_interpreter(Arena_header* ah);

void kot_push_globl_variable_def(Arena_header* ah,char* name);
bool kot_globl_variable_already_present(char* name);

void kot_push_variable_def(Arena_header* ah,char* name);
bool kot_variable_already_present(char* name);

void kot_push_fn_dec(Arena_header* ah, fn_signature fn);
bool kot_fn_already_declared(fn_signature fn);
fn_signature* kot_define_fn(Arena_header* ah,char* name,int param_len, KOT_TYPE* param_type);
inst_slice kot_get_current_inst();
void kot_pc_inc();


int kot_parse(Arena_header* ah,lxer_header* lh, error_handler *eh, bool console);

void kot_run();
bool kot_single_run(inst_slice inst);


int kot_type_processor(Arena_header* ah, lxer_header* lh, error_handler *eh);
int kot_variable_argument_processor(Arena_header * ah, lxer_header* lh, error_handler *eh, char*name, LXR_TOKENS type);
int kot_function_processor(Arena_header* ah, lxer_header* lh, error_handler* eh,char*name, LXR_TOKENS type);
int kot_statement_processor(Arena_header* ah, lxer_header* lh, error_handler *eh);

void kot_set_line(size_t line);

void kot_push_stack(uint8_t* data, int size);
uint8_t* kot_pull_stack(Arena_header* ah,int size);
size_t kot_write_heap(Arena_header* ah,uint8_t* data, int size);

void kot_get_bytecode();
void kot_get_memory_dump();
void kot_push_instruction(Arena_header* ah, kot_ir inst, char* label, uint32_t arg_0, uint32_t arg_1, uint32_t arg_2, bool is_fn);
void kot_get_program_list(FILE* filestream);

#define KOT_R_I int 

#define KOT_FN_DEFINE(name)\
	kot_##name(uint32_t arg_0,uint32_t  arg_2,uint32_t arg_3)

#define KOT_FN_CALL(name)\
	kot_##name(inst.arg_0, inst.arg_1, inst.arg_2)

KOT_R_I KOT_FN_DEFINE(push);
KOT_R_I KOT_FN_DEFINE(pull);



#ifndef KOT_C
#define KOT_C

#endif

#endif // KOT_H
