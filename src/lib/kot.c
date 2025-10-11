#define KOT_C

#include "kot.h"


#define KOT_ERROR_PRECISE(name)\
	error_push_error(eh,name, line, 2 ,lxer_get_current_pointer(lh),strlen(lxer_get_current_pointer(lh)));\
	status = 1;

#define KOT_ERROR(name)\
	error_push_error(eh,name, 0, 2 ,NULL,0);\
	error_print_error(eh,(print_set){true,true,false,false,true,false, false});\
	eh->tracker = 0;\
	status = 1;

#define KOT_SYNTAX_ERR()\
	KOT_ERROR_PRECISE("Syntax error");

#define KOT_PARSER_NEXT()\
	lxer_next_token(lh);\
	token = lxer_get_current_token(lh);\
	next_token = lxer_get_next_token(lh);

#define KOT_PARSER_REFRESH()\
	token = lxer_get_current_token(lh);\
	next_token = lxer_get_next_token(lh);

#define KOT_GET_REF()\
	printf("Current token: %s\n", token_table_lh[token]);\
	printf("Following token: %s\n", token_table_lh[next_token]);


void kot_init_interpreter(Arena_header* ah){
	globl_variable = (char**)arena_alloc(ah,sizeof(char*)*DEF_GET_ARR_SIZE);
	globl_variable_tracker = 0;
	globl_variable_size = DEF_GET_ARR_SIZE;
}

void kot_init_vm(Arena_header*ah){
	kotvm.program_counter = 0x00;
	memset(kotvm.gpr, 0, sizeof(uint32_t)*GPR_NUM);
	memset(kotvm.fr, 0, sizeof(float)*GPR_NUM);
	kotvm.bytecode_array.program = (inst_slice*)arena_alloc(ah,sizeof(inst_slice)*DEF_PROGRAM_SIZE);
	kotvm.bytecode_array.tracker = 0;
	kotvm.bytecode_array.size = DEF_PROGRAM_SIZE;
	
	kotvm.memory = (char*)arena_alloc(ah, sizeof(char)*DEF_MEMORY_SIZE);
	kotvm.def_memory_size = DEF_MEMORY_SIZE;
	kotvm.memory_tracker = 0;

	kotvm.stack_pointer = DEF_STACK_INIT;
	
	kotvm.program_source = (char**)arena_alloc(ah, sizeof(char*)*DEF_SOURCE_SIZE);
	kotvm.program_source_size = DEF_SOURCE_SIZE;
	kotvm.program_source_tracker = 0;
}


int kot_argument_processor(Arena_header * ah, lxer_header* lh, error_handler *eh, LXR_TOKENS type){
	int status = 0;
	LXR_TOKENS token, next_token;
	KOT_PARSER_REFRESH();
	char* name = lxer_get_rh(lh, true, true);
	if(type == LXR_STRING_TYPE){
		if(next_token == LXR_DOUBLE_QUOTE){
			KOT_PARSER_NEXT();
			char* string = lxer_get_string(lh, LXR_DOUBLE_QUOTE);
			if(!string || strlen(string) < 1){
				KOT_ERROR_PRECISE("This is an empty string, fill with with something");
			}else{
				KOT_PARSER_REFRESH();
				if(next_token != LXR_SEMICOLON){
					KOT_ERROR_PRECISE("Missing semicolon");
				}else{
					size_t ptr = kot_write_mem(ah,string, strlen(string));
					//printf("String assigned with '%s'\n", string);
					if(!kot_globl_variable_already_present(name)){
						kot_push_globl_variable_def(ah, name);
						kot_push_instruction(ah, IR_PUSH, name, ptr, 4,strlen(string),false);
					}else{
						KOT_ERROR("Variable already defined");
					}
				}
			}
		}else{
			KOT_ERROR_PRECISE("Not a valid string implicit definition");
		}
	}else{
		if(next_token == LXR_SEMICOLON){
			char* args = lxer_get_rh(lh,false, true);
			if(!args || strchr(args, ' ') != NULL){
				KOT_ERROR_PRECISE("Not a valid variable declaration, the following value is not an integer");
			}else{
				//printf("Variable '%s' assigned with '%d'\n", name,atoi(args));
				if(!kot_globl_variable_already_present(name)){
					kot_push_globl_variable_def(ah, name);
					kot_push_instruction(ah, IR_PUSH, name, atoi(args), 4, 0, false);
				}else{
					KOT_ERROR("Variable already defined");
				}
			}
		}else{
			KOT_ERROR_PRECISE("Not a valid assignment or incomplete syntax");
		}
	}
	return status;
}

/* PROCESSING EXPRESSION THAT START WITH A TYPE */

int kot_process_type(Arena_header* ah, lxer_header* lh, error_handler *eh){
	int status = 0;
	LXR_TOKENS token, next_token, type, cache_token; 
	KOT_PARSER_REFRESH();

	if(lxer_is_statement(next_token) || lxer_is_sep(next_token) || lxer_is_brk(next_token)){
		char* name = lxer_get_rh(lh, false, true);
		if(!name){
			KOT_ERROR_PRECISE("No name provided");
		}
		type =  token;
		KOT_PARSER_NEXT();
		switch(token){
			case LXR_ASSIGNMENT:
				status = kot_argument_processor(ah,lh,eh,type);
				break;
			case LXR_OPEN_BRK:
				break;
			case LXR_SEMICOLON:
				if(!kot_globl_variable_already_present(name)){
					kot_push_globl_variable_def(ah, name);
					kot_push_instruction(ah, IR_PUSH, name, 0, 4, 0, false);
				}else{
					KOT_ERROR("Variable already defined");
				}
				break;
			default: 
				KOT_SYNTAX_ERR();
				break;
		}
	}else{
		KOT_SYNTAX_ERR();
	}	

	return status;
}



/* MAIN PARSER LOOP */

int kot_parse(Arena_header* ah, lxer_header* lh, error_handler *eh, bool console){
	LXR_TOKENS token, next_token, cache_token;
	int status = 0;
	if(lh->stream_out_len < 2){
		if(!console){
			KOT_ERROR("Empty file!");
			return 0;
		}
	}
	
	if(DEBUG) lxer_get_lxer_content(lh);
	while(status == 0 && lxer_get_current_token(lh) != TOKEN_TABLE_END){
		token = lxer_get_current_token(lh);
		next_token = lxer_get_next_token(lh);
		if(lxer_is_type(token)){
			status = kot_process_type(ah, lh, eh);
		}
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
			printf("\t%zu: %s 0x%x, 0x%x, aux 0x%x \t\t| ", i, ir_table_lh[kotvm.bytecode_array.program[i].bytecode],\
																				kotvm.bytecode_array.program[i].arg_0,\
																				kotvm.bytecode_array.program[i].arg_1,\
																				kotvm.bytecode_array.program[i].arg_2\
			);
			if(kotvm.bytecode_array.program[i].label != NULL){
				printf("label %s\n", kotvm.bytecode_array.program[i].label);
			}else{
				printf("\n");
			}
		}
	}
}


void kot_get_memory_dump(){
	int newline = 0;
	uint32_t i;
	fprintf(stdout,"\nKOTVM memory dump: \n\n");
	for(i=0;i<kotvm.def_memory_size;i++){
		if(newline == 4) {
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

void kot_push_instruction(Arena_header* ah, kot_ir inst, char* label, uint32_t arg_0, uint32_t arg_1, uint32_t arg_2, bool is_fn){
	inst_slice is = {
		.label = label,
		.bytecode = inst,
		.arg_0 = arg_0,
		.arg_1 = arg_1,
		.arg_2 = arg_2,
		.fn = is_fn
	};
	dapush(*ah, kotvm.bytecode_array.program, kotvm.bytecode_array.tracker, kotvm.bytecode_array.size, inst_slice, is);
}

void kot_push_globl_variable_def(Arena_header* ah,char* name){
	if(globl_variable_tracker += 1 >= globl_variable_tracker){
		char** old_arr = globl_variable;
		globl_variable = (char**)arena_alloc(ah,sizeof(char*)*globl_variable_size*2);
		globl_variable_size*=2;
		memcpy(globl_variable, old_arr, sizeof(char*)*globl_variable_tracker);
	}
	globl_variable[globl_variable_tracker] = name;
	globl_variable_tracker += 1;
}

bool kot_globl_variable_already_present(char* name){
	for(size_t i=0;i<globl_variable_tracker; i++){
		if(globl_variable[i] != NULL){
			if(strcmp(name, globl_variable[i])==0){
				return true;
			}
		}
	}
	return false;
}

void kot_set_line(size_t cline){
	line = cline;
}


size_t kot_write_mem(Arena_header* ah,char* string, int size){
	if(kotvm.memory_tracker + size >= kotvm.def_memory_size){
		char *old_mem = kotvm.memory;
		kotvm.memory = (char*)arena_alloc(ah, sizeof(char)*kotvm.def_memory_size*2);
		kotvm.def_memory_size *= 2;
		memcpy(kotvm.memory, old_mem,kotvm.memory_tracker);	
	}
	size_t ptr = kotvm.memory_tracker;
	memcpy(kotvm.memory, string, size);
	kotvm.memory_tracker += size;
	return ptr;
}
