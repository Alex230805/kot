#define KOT_C

#include "kot.h"


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
}


int kot_parse(Arena_header* ah, lxer_header* lh, error_handler *eh){
	//lxer_get_lxer_content(lh);
	LXR_TOKENS token; 
	LXR_TOKENS type;
	int status = 0; 
	do{
		token = lxer_get_current_token(lh);
		if(token != TOKEN_TABLE_END) status = 1;
		
		if(lxer_is_type(token)){
			char* buffer = lxer_get_rh(lh,false);
			if(buffer == NULL || strlen(buffer) < 1) {
				error_push_error(eh, "missing name", 0, 1,NULL,0);
				status = 1;
				goto ret;
			}

			type = token;
			lxer_next_token(lh);
			token = lxer_get_current_token(lh);
			if(token == LXR_OPEN_BRK){
				printf("Function definition detected\n");

			}else if(token == LXR_ASSIGNMENT){
				printf("variable definition detected: '%s'\n", buffer);
			}else if(token == LXR_SEMICOLON){
				//printf("variable declaration detected: '%s'\n", buffer);
				kot_push_instruction(ah, IR_PUSH, buffer, 0, 0);
			}else{
				error_push_error(eh, "syntax error, incomplete phrase", 0, 1,NULL,0);
				status = 1;
			}
		
		}else{
			error_push_error(eh, "syntax error", 0, 1,NULL,0);
			status = 1;
		}
	}while(lxer_next_token(lh));
ret:
	return status;
}



void kot_get_bytecode(){
	fprintf(stdout, "\nCurrent program list: \n");
	for(size_t i=0;i<kotvm.bytecode_array.tracker;i++){
		if(kotvm.bytecode_array.program[i].fn){
			printf("%zu %s: \n", i, ir_table_lh[kotvm.bytecode_array.program[i].bytecode]);
		}else{
			printf("\t%zu: %s 0x%x, 0x%x \t\t| var name: %s\n", i, ir_table_lh[kotvm.bytecode_array.program[i].bytecode],\
																				kotvm.bytecode_array.program[i].arg_0,\
																				kotvm.bytecode_array.program[i].arg_1,\
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


void kot_push_instruction(Arena_header* ah, kot_ir inst, char* label, uint32_t arg_0, uint32_t arg_1){
	inst_slice is = {
		.label = label,
		.bytecode = inst,
		.arg_0 = arg_0,
		.arg_1 = arg_1
	};
	dapush(*ah, kotvm.bytecode_array.program, kotvm.bytecode_array.tracker, kotvm.bytecode_array.size, inst_slice, is);
}
