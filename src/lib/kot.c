#define KOT_C

#include "kotmacro.c"
#include "kot.h"
#include "kotvm_utils.c"
#include "kot_runner.c"

void kot_init_interpreter(Arena_header* ah){
	glob_var_def = (char**)arena_alloc(ah,sizeof(char*)*DEF_GET_ARR_SIZE);
	glob_var_def_tracker = 0;
	glob_var_def_size = DEF_GET_ARR_SIZE;

	globl_fn_signature = (fn_signature*)arena_alloc(ah, sizeof(fn_signature)*DEF_GET_ARR_SIZE);
	globl_fn_signature_tracker = 0;
	globl_fn_signature_size = DEF_GET_ARR_SIZE;
}

void kot_init_vm(Arena_header*ah){
	kotvm.program_counter = 0x00;
	memset(kotvm.gpr, 0, sizeof(uint32_t)*GPR_NUM);
	memset(kotvm.fr, 0, sizeof(float)*GPR_NUM);
	
	kotvm.bytecode_array = (inst_slice*)arena_alloc(ah,sizeof(inst_slice)*DEF_PROGRAM_SIZE);
	kotvm.bytecode_array_size = DEF_PROGRAM_SIZE;
	kotvm.bytecode_array_tracker = 0;
	
	kotvm.memory = (char*)arena_alloc(ah, sizeof(char)*DEF_MEMORY_SIZE);
	kotvm.def_memory_size = DEF_MEMORY_SIZE;
	kotvm.memory_tracker = 0;

	kotvm.stack_pointer = DEF_STACK_INIT;
	
	kotvm.program_source = (char**)arena_alloc(ah, sizeof(char*)*DEF_SOURCE_SIZE);
	kotvm.program_source_size = DEF_SOURCE_SIZE;
	kotvm.program_source_tracker = 0;

	kotvm.main_scope = (scope*)arena_alloc(ah, sizeof(scope));
	kotvm.main_scope->list = arena_list_alloc(ah);
	kotvm.main_scope->master = NULL; 
	kotvm.main_scope->type = STRT;
	kotvm.cache_scope = kotvm.main_scope;

	kotvm.main_scope->var_def = glob_var_def;
	kotvm.main_scope->var_def_tracker = glob_var_def_tracker;
	kotvm.main_scope->var_def_size = glob_var_def_size;
}


int kot_variable_argument_processor(Arena_header * ah, lxer_header* lh, error_handler *eh, LXR_TOKENS type){
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
					KOT_PUSH_VAR_DEC(IR_PUSH, name, ptr, 4, strlen(string), false);
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
				KOT_PUSH_VAR_DEC(IR_PUSH, name, atoi(args), 4, 0, false);
			}
		}else{
			KOT_ERROR_PRECISE("Not a valid assignment or incomplete syntax");
		}
	}
	return status;
}



int kot_function_processor(Arena_header* ah, lxer_header* lh, error_handler* eh, char* name, LXR_TOKENS type){
	int status = 0;
	int args = 0;
	LXR_TOKENS token, next_token;
	fn_signature* fn = NULL;
	KOT_TYPE* param_type = (KOT_TYPE*)arena_alloc(ah, sizeof(KOT_TYPE)*512);
	KOT_PARSER_REFRESH();
	KOT_PARSER_NEXT();
	if(token == LXR_CLOSE_BRK){
		if(next_token == LXR_OPEN_CRL_BRK){
			fn = kot_define_fn(ah,name,0, NULL);
			if(!kot_fn_already_declared(*fn)){
				kot_push_fn_dec(ah, *fn);
				kot_push_instruction(ah, IR_TAG,fn->name, 0,0,0,true);
				SWITCH_SCOPE(FUNC);	
			}else{
				KOT_ERROR("Function already defined");
			}
		}else{
			KOT_SYNTAX_ERR();
		}
	}else{
		KOT_TYPE arg_type;
		while(lxer_is_type(token) && status == 0){
			if(next_token == LXR_COMMA || next_token == LXR_CLOSE_BRK){
				switch(token){
					case LXR_STRING_TYPE: 
						arg_type = KOT_STR;
						break;
					case LXR_INT_TYPE: 
						arg_type = KOT_INT;
						break;
					case LXR_DOUBLE_TYPE:
						arg_type = KOT_DOUBLE;
					   	break;
					case LXR_FLOAT_TYPE: 
						arg_type = KOT_FLOAT;
						break;
					case LXR_CHAR_TYPE:
						arg_type = KOT_CHAR;	
						break;
					case LXR_VOID_TYPE: 
						arg_type = KOT_VOID;
						break;
					default: 
						KOT_ERROR("Undefined type or currently not supported syntax");
						break;
				}
				if(status == 1) break;
				char* param_name = lxer_get_rh(lh, false, true);
				//printf("parameter name: %s\n", param_name);
				if(param_name != NULL && strlen(param_name) > 0){
					param_type[args] = arg_type; 
					args += 1;
					KOT_PARSER_NEXT();
					KOT_PARSER_NEXT();
				}else{
					KOT_ERROR("Parameter with no name");
				}
			}else{
				KOT_ERROR("Invalid function signature");
				break;
			}
		}

		lxer_increase_tracker(lh,-1);
		KOT_PARSER_REFRESH();
		if(status == 0){
			if(next_token == LXR_OPEN_CRL_BRK){
				fn = kot_define_fn(ah,name,args, param_type);
				if(!kot_fn_already_declared(*fn)){
					kot_push_fn_dec(ah, *fn);
					kot_push_instruction(ah, IR_TAG,fn->name, 0,0,0,true);
					SWITCH_SCOPE(FUNC);	
				}else{
					KOT_ERROR("Function already defined");
				}
			}else{
				KOT_SYNTAX_ERR();
			}
		}
	}
	return status;
}

int kot_statement_processor(Arena_header* ah, lxer_header* lh, error_handler *eh){
	int status = 0;
	KOT_NOT_IMPLEMENTED("Statement instruction processor",NULL);
	return status;
}


int kot_type_processor(Arena_header* ah, lxer_header* lh, error_handler *eh){
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
				status = kot_variable_argument_processor(ah,lh,eh,type);
				break;
			case LXR_OPEN_BRK:
				if(kotvm.cache_scope->type == STRT){
					status = kot_function_processor(ah, lh, eh, name, type);
				}else{
					KOT_ERROR("You cannot declare a function scope inside another function body");
				}
				break;
			case LXR_SEMICOLON:
				KOT_PUSH_VAR_DEC(IR_PUSH, name, 0, 4, 0, false);
				break;
			default: 
				KOT_SYNTAX_ERR();
				break;
		}
	}else{
		if(next_token == TOKEN_TABLE_END){
			KOT_ERROR("Missing semicolon in variable declaration");
		}else{
			KOT_SYNTAX_ERR();
		}
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
			status = kot_type_processor(ah, lh, eh);
		}
		if(lxer_is_statement(token)){
			status = kot_statement_processor(ah, lh, eh);
		}
		if(lxer_is_brk(token)){
			if(token == LXR_CLOSE_CRL_BRK){
				if(kotvm.cache_scope->master == NULL){
					KOT_ERROR("Brachets has no start point or it's out of place");
				}else{
					printf("Going up the hierarchy to %p\n", kotvm.cache_scope->master);
					kotvm.cache_scope = kotvm.cache_scope->master; // going up the hierarchy
				}	
			}
		}
		lxer_next_token(lh);
	}
	if(status == 0){ // check if the parsing process did not return anything different from 0 
		dapush(*ah,kotvm.program_source, kotvm.program_source_tracker,kotvm.program_source_size, char*, lh->source);	
	}
	return status;
}
