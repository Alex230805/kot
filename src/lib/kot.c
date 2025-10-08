#define KOT_C

#include "kot.h"


#define KOT_ERROR_PRECISE(name)\
	error_push_error(eh,name, 0, 2 ,lxer_get_current_pointer(lh),strlen(lxer_get_current_pointer(lh)));\
	status = 1;

#define KOT_ERROR(name)\
	error_push_error(eh,name, 0, 2 ,NULL,0);\
	error_print_error(eh,(print_set){true,true,false,false,true,false, false});\
	eh->tracker = 0;


void kot_init_vm(Arena_header*ah){
	kotvm.program_counter = 0x00;
	memset(kotvm.gpr, 0, sizeof(uint32_t)*GPR_NUM);
	memset(kotvm.fr, 0, sizeof(float)*GPR_NUM);
	kotvm.bytecode_array.program = (inst_slice*)arena_alloc(ah,sizeof(inst_slice)*DEF_PROGRAM_SIZE);
	kotvm.bytecode_array.tracker = 0;
	kotvm.bytecode_array.size = DEF_PROGRAM_SIZE;
	kotvm.memory = (uint32_t*)arena_alloc(ah, sizeof(uint32_t)*DEF_MEMORY_SIZE);
	kotvm.def_memory_size = DEF_MEMORY_SIZE;
	kotvm.stack_pointer = DEF_STACK_INIT;
	
	kotvm.program_source = (char**)arena_alloc(ah, sizeof(char*)*DEF_SOURCE_SIZE);
	kotvm.program_source_size = DEF_SOURCE_SIZE;
	kotvm.program_source_tracker = 0;
}


int kot_parse(Arena_header* ah, lxer_header* lh, error_handler *eh, bool console){
	LXR_TOKENS token; 
	LXR_TOKENS next_token; 
	LXR_TOKENS type;
	int status = 0;
	if(lh->stream_out_len < 2){
		if(!console){
			KOT_ERROR("Empty file!");
			return 0;
		}
	}
	
	if(DEBUG) lxer_get_lxer_content(lh);
	
	while(status == 0 && lxer_get_current_token(lh) != TOKEN_TABLE_END){
		lxer_next_token(lh);
	}
	
	if(status == 0){ // push line only if it is validated and syntactically correct
		dapush(*ah,kotvm.program_source, kotvm.program_source_tracker,kotvm.program_source_size, char*, lh->source);	
	}
	
	return status;
}



void kot_get_program_list(FILE* filestream){
	fprintf(filestream,"\nProgram list: \n\n");
	for(size_t i=0;i<kotvm.program_source_tracker;i++){
		fputs(kotvm.program_source[i],filestream);
	}
}

void kot_get_bytecode(){
	fprintf(stdout, "\nCurrent program list: \n");
	for(size_t i=0;i<kotvm.bytecode_array.tracker;i++){
		if(kotvm.bytecode_array.program[i].fn){
			printf("%zu %s: \n", i, ir_table_lh[kotvm.bytecode_array.program[i].bytecode]);
		}else{
			printf("\t%zu: %s 0x%x, 0x%x, aux 0x%x \t\t| var name: %s\n", i, ir_table_lh[kotvm.bytecode_array.program[i].bytecode],\
																				kotvm.bytecode_array.program[i].arg_0,\
																				kotvm.bytecode_array.program[i].arg_1,\
																				kotvm.bytecode_array.program[i].arg_2,\
																				kotvm.bytecode_array.program[i].label
			);
		}
	}
}


void kot_get_memory_dump(){
	int newline = 0;
	uint32_t i;
	fprintf(stdout,"\nKOTVM memory dump: \n\n");
	for(i=0;i<kotvm.def_memory_size;i++){
		if(newline == 16) {
			fprintf(stdout,"\n");
			newline = 0;
		}
		if(newline == 0){
			fprintf(stdout, "%*x: ", 3,i);
		}
		fprintf(stdout, "0x%x ",kotvm.memory[i]);
		newline++;
	}	
	fprintf(stdout, "\n");
}


void kot_push_instruction(Arena_header* ah, kot_ir inst, char* label, uint32_t arg_0, uint32_t arg_1, uint32_t arg_2, bool tag){
	inst_slice is = {
		.label = label,
		.bytecode = inst,
		.arg_0 = arg_0,
		.arg_1 = arg_1,
		.arg_2 = arg_2,
		.fn = tag
	};
	dapush(*ah, kotvm.bytecode_array.program, kotvm.bytecode_array.tracker, kotvm.bytecode_array.size, inst_slice, is);
}
