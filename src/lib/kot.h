#ifndef KOT_H
#define KOT_H

#include <assert.h>
#include "ilxer.h"
#include "misc.h"
#include <math.h>

#define GPR_NUM 32

#define DEF_PROGRAM_SIZE 64

#define DEF_MEMORY_SIZE 0xFFFFF


#define DEF_STK_CALL_INIT	0x00000
#define DEF_STK_CALL_LIMIT	0x00FFF
#define DEF_STACK_INIT		0x01000
#define DEF_STACK_LIMIT		0x0FFFF
#define DEF_HEAP_INIT		0x10000

#define DEF_SOURCE_SIZE  256 // intended as line of code, not individual character
#define DEF_GET_ARR_SIZE 256


typedef enum{
	IR_PUSH,
	IR_PULL,
	IR_JUMP,
	IR_CALL,
	IR_RETRN,
	IR_HALT,
	IR_ILLEGAL,
	IR_TABLE_END
}kot_ir;

static kot_ir ir_table[] = {
	// push(arg_0 = mem dest, arg_1 = ptr | data, arg_3 ? len of array : 0) NOTE: length of type is automatically known in the global/local var declaration table
	IR_PUSH,
	// pull(arg_0 = reg_dest, arg_1 = stack_pos, arg_3 = offset in byte) NOTE: lenght of type is automatically known in the global/local var declaration table
	IR_PULL,

	IR_JUMP,
	
	//  call(arg_0 = fn_name pointer, arg_1 = arg count, arg_3 = 0)
	IR_CALL,
	IR_RETRN,
	IR_HALT,
	IR_ILLEGAL,
	IR_TABLE_END
};

static char* ir_table_lh[] = {
	[IR_PUSH] = "push",
	[IR_PULL] = "pull",
	[IR_JUMP] = "jump",
	[IR_CALL] = "call",
	[IR_RETRN] = "ret",
	[IR_HALT] = "halt",
	[IR_ILLEGAL] = "illegal"
};

typedef struct{
	kot_ir bytecode;
	uint32_t arg_0;
	uint32_t arg_1;
	uint32_t arg_2;
}inst_slice;

typedef enum{
	KOT_INT, 
	KOT_CHAR, 
	KOT_BOOL,
	KOT_VOID,
	KOT_STR,
	KOT_FLOAT,
	KOT_UNDEFINED
}KOT_TYPE;


static char* type_table_lh[] = {
	[KOT_INT] = "int", 
	[KOT_CHAR] = "char", 
	[KOT_BOOL] = "bool",
	[KOT_VOID] = "void",
	[KOT_STR] = "str",
	[KOT_FLOAT] = "float",
	[KOT_UNDEFINED] = "undefined type"
};

static int  type_table_size[] = {
	[KOT_INT] = 4, 
	[KOT_CHAR] = 1, 
	[KOT_BOOL] = 1,
	[KOT_VOID] = 4,
	[KOT_STR] = 4,
	[KOT_FLOAT] = 4,
	[KOT_UNDEFINED] = 0
};


typedef struct{
	KOT_TYPE type;
	char* name;
	int32_t adr;
}var_cell;

#define STRT 0
#define COND 1
#define FUNC 2
#define FFI  3

typedef int(*__ffi_linker_callback)(void);

typedef struct scope{
	int type;
	
	var_cell* var_def;
	size_t var_def_tracker;
	size_t var_def_size;
	
	struct scope* master; // scope from which this depend on
	__ffi_linker_callback fn_pointer;

	List_header* list; // list of instruction
}scope;

typedef struct{
	int type;
	scope* master;
	
	scope* branch_true;
	scope* branch_false;
}scope_branch;


typedef struct{
	char* name;
	size_t param_len;
	KOT_TYPE* param_type;
	scope* fn_scope;
}fn_signature;


typedef struct{
	// gpr call convention: above x20 ( from x21 to x31 ) those are reserved as function argument
	uint32_t gpr[GPR_NUM];
	float fr[GPR_NUM];
	
	inst_slice* bytecode_array;
	size_t bytecode_array_size;
	size_t bytecode_array_tracker;

	uint8_t *memory;
	uint32_t def_memory_size;
	uint32_t memory_tracker;
	uint32_t program_counter;
	int32_t	 stack_pointer;
	int32_t  call_stack_pointer;

	scope *main_scope;
	scope *cache_scope;

	char** program_source;
	size_t program_source_size;
	size_t program_source_tracker;

}vkot_machine;


static vkot_machine kotvm;
static size_t line;

static	var_cell* glob_var_def;
static	size_t glob_var_def_tracker;
static	size_t glob_var_def_size;


static fn_signature* globl_fn_signature;
static size_t globl_fn_signature_tracker;
static size_t globl_fn_signature_size;

void kot_init_vm(Arena_header*ah);
void kot_init_interpreter(Arena_header* ah);

void kot_push_globl_variable_def(Arena_header* ah,char* name, KOT_TYPE type, int32_t pos);
bool kot_globl_variable_already_present(char* name);

void kot_push_variable_def(Arena_header* ah,char* name, KOT_TYPE type, int32_t pos);
bool kot_variable_already_present(char* name);
KOT_TYPE kot_globl_var_get_type(char* name);
KOT_TYPE kot_var_get_type(char* name);
fn_signature* kot_fn_get_signature(char* name);

void kot_push_fn_dec(Arena_header* ah, fn_signature fn);
bool kot_fn_already_declared(char* name);
fn_signature* kot_define_fn(Arena_header* ah,char* name,int param_len, KOT_TYPE* param_type, scope* fn_scope);
inst_slice kot_get_current_inst();
void kot_pc_inc();
bool kot_link_function(Arena_header* ah, fn_signature *fn, __ffi_linker_callback fn_call);
scope* kot_fn_get_scope(char* name);

int kot_parse(Arena_header* ah,lxer_header* lh, error_handler *eh, bool console);

void kot_run();
bool kot_single_run(inst_slice inst);


int kot_type_processor(Arena_header* ah, lxer_header* lh, error_handler *eh);
int kot_variable_argument_processor(Arena_header * ah, lxer_header* lh, error_handler *eh, char*name, LXR_TOKENS type);
int kot_function_processor(Arena_header* ah, lxer_header* lh, error_handler* eh,char*name, LXR_TOKENS type);
int kot_statement_processor(Arena_header* ah, lxer_header* lh, error_handler *eh);
int kot_word_processor(Arena_header* ah, lxer_header* lh, error_handler *eh);

	
void kot_set_line(size_t line);
KOT_TYPE kot_get_type_from_token(LXR_TOKENS token);
int kot_get_size_from_type(KOT_TYPE type);

float kot_process_float_literal(lxer_header* lh);

void kot_push_stack(uint8_t* data, int size);
uint8_t* kot_pull_stack(Arena_header* ah,int size);
uint8_t* kot_get_stack(Arena_header* ah, int size);
size_t kot_write_heap(Arena_header* ah,uint8_t* data, int size);

// note about kot_alloc_stack(int size)
/*
 * The role of this function is to allocate a reserved space inside the stack for variable allocation, specifically for the 
 * instruction "PUSH" to know a position for storing information inside the data stack.
 * To define an instruction to push a variable during execution time you need to use the macro KOT_PUSH_VAR(...) and then 
 * offset the stack pointer by the size of the allocated variable. This means that if you want to allocate 4 byte in the 
 * stack for an integer you first declare the instruction PUSH to write 4 bite with value N in the current stack position, and 
 * then you need to offset the stack pointer for new allocation.
 *
 * So this is used only during the "compilation" process to define the position for each variable in the data stack, then during the 
 * execution those PUSH instruction will write in the allocated position the required data. 
 *
 *
 * */
void kot_alloc_stack(int size);

void kot_push_return_ptr(uint32_t ptr);
uint32_t kot_pull_return_ptr();


void kot_get_bytecode();
void kot_get_memory_dump();
void kot_push_instruction(Arena_header* ah, kot_ir inst, uint32_t arg_0, uint32_t arg_1, uint32_t arg_2);
void kot_get_program_list(FILE* filestream);

#define KOT_R_I int 

#define KOT_FN_DEFINE(name)\
	kot_##name(uint32_t arg_0,uint32_t  arg_2,uint32_t arg_3)

#define KOT_FN_CALL(name)\
	kot_##name(inst.arg_0, inst.arg_1, inst.arg_2)

KOT_R_I KOT_FN_DEFINE(push);
KOT_R_I KOT_FN_DEFINE(pull);
KOT_R_I KOT_FN_DEFINE(jump);
KOT_R_I KOT_FN_DEFINE(retrn);
KOT_R_I KOT_FN_DEFINE(call);



#ifndef KOT_C
#define KOT_C

#endif

#endif // KOT_H
