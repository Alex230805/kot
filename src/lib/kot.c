#define KOT_C

#include "kotmacro.c"
#include "kot.h"
#include "kotvm_utils.c"
#include "kot_runner.c"

void kot_init_interpreter(Arena_header* ah){
	glob_var_def = (var_cell*)arena_alloc(ah,sizeof(var_cell)*DEF_GET_ARR_SIZE);
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
	
	kotvm.memory = (uint8_t*)arena_alloc(ah, sizeof(uint8_t)*DEF_MEMORY_SIZE);
	kotvm.def_memory_size = DEF_MEMORY_SIZE;
	kotvm.memory_tracker = DEF_HEAP_INIT;
	kotvm.stack_pointer = DEF_STACK_INIT;
	kotvm.call_stack_pointer = DEF_STK_CALL_INIT;

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


int kot_variable_argument_processor(Arena_header * ah, lxer_header* lh, error_handler *eh, char* name,LXR_TOKENS type){
	int status = 0;
	LXR_TOKENS token, next_token;
	KOT_PARSER_REFRESH();
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
					KOT_INST_VAR(name,kot_get_type_from_token(type),kotvm.stack_pointer, (size_t)string, strlen(string));
					kot_alloc_stack(strlen(string));
				}
			}
		}else{
			KOT_ERROR_PRECISE("Not a valid implicit string definition");
		}
	}else{
		if(next_token == LXR_WORD){
			KOT_PARSER_NEXT();
			char* arg =  lxer_get_word(lh);
			float arg_f = 0.00;
			int arg_i = atoi(arg);
			if(type == LXR_FLOAT_TYPE){
				arg_f = kot_process_float_literal(lh);
				arg_i = *(uint32_t*)&arg_f;
			}
			KOT_INST_VAR(name,kot_get_type_from_token(type), kotvm.stack_pointer, arg_i, 0);
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
			if(!kot_fn_already_declared(name)){
				scope *new_scope = (scope*)arena_alloc(ah, sizeof(scope));
				//printf("Switching to new scope %p from %p\n", new_scope, kotvm.cache_scope);
				SWITCH_SCOPE(FUNC);
				fn = kot_define_fn(ah,name,0, NULL, new_scope);
				kot_push_fn_dec(ah, *fn);
			}else{
				KOT_ERROR("Function already defined");
			}
		}else{
			KOT_SYNTAX_ERR();
		}
	}else{
		KOT_TYPE arg_type;
		while(lxer_is_type(token) && status == 0){
			if(token == LXR_WORD){
				KOT_ERROR_PRECISE("Custom type definition not implemented yet");
			}else{
				arg_type = kot_get_type_from_token(token);
			}
			if(next_token == LXR_WORD){
				KOT_PARSER_NEXT();
				char* name = lxer_get_word(lh);	
				KOT_PARSER_REFRESH();
				if(next_token == LXR_COMMA || next_token == LXR_CLOSE_BRK){
					param_type[args] = arg_type; 
					args += 1;
					KOT_PARSER_NEXT();
					KOT_PARSER_NEXT();
				}else{
					KOT_ERROR("Invalid function signature");
					break;
				}
			}else{
				KOT_ERROR("Parameter has no name");
			}
		}
		lxer_increase_tracker(lh,-1);
		KOT_PARSER_REFRESH();
		if(next_token == LXR_OPEN_CRL_BRK){
			if(!kot_fn_already_declared(name)){
				scope *new_scope = (scope*)arena_alloc(ah, sizeof(scope));
				//printf("Switching to new scope %p from %p\n", new_scope, kotvm.cache_scope);
				SWITCH_SCOPE(FUNC);	
				fn = kot_define_fn(ah,name,args, param_type, new_scope);
				kot_push_fn_dec(ah, *fn);
			}else{
				KOT_ERROR("Function already defined");
			}
		}else{
			KOT_SYNTAX_ERR();
		}
	}
	return status;
}


int kot_word_processor(Arena_header* ah, lxer_header* lh, error_handler *eh){
	int status = 0;
	LXR_TOKENS token, next_token;
	KOT_PARSER_REFRESH();
	char* name = lxer_get_word(lh);
	if(lxer_is_brk(next_token)){
		if(next_token == LXR_OPEN_BRK){
			KOT_PARSER_NEXT();
			KOT_PARSER_NEXT();
			if(token == LXR_CLOSE_BRK || token == LXR_WORD){
				//printf("Function call identified: '%s'\n", name);
				if(kot_fn_already_declared(name)){
					fn_signature* local_sign = kot_fn_get_signature(name);
					if(local_sign == NULL){
						KOT_ERROR("Unable to get function signature");
					}

					int argc = 0;
					int exp_argc = local_sign->param_len;
					KOT_TYPE* exp_argt = local_sign->param_type;
					
					// TODO: hard coded address position array for each variable used as argument during the request, this is limited at 64 for now
					int32_t var_adr[64] = {0};
					int var_adr_tracker = 0;
					int var_adr_size = 64;

					char* var_name = NULL; 

					while(token != LXR_CLOSE_BRK && status == 0 && argc < exp_argc){
						var_name = lxer_get_word(lh);
						KOT_TYPE type = KOT_UNDEFINED;

						if(kot_globl_variable_already_present(var_name)){
							type = kot_globl_var_get_type(var_name);
							var_adr[var_adr_tracker] = kot_globl_var_get_adr(var_name);
							if(var_adr_tracker + 1 > var_adr_size){
								KOT_ERROR("Exceeding argument limit array");
							}else{
								var_adr_tracker++;
							}
						}else if(kot_variable_already_present(var_name)){
							type = kot_var_get_type(var_name);
							var_adr[var_adr_tracker] = kot_var_get_adr(var_name);
							if(var_adr_tracker + 1 > var_adr_size){
								KOT_ERROR("Exceeding argument limit array");
							}else{
								var_adr_tracker++;
							}
						}else{
							//printf("Total globl variable: %zu\n", glob_var_def_tracker);
							KOT_ERROR_PRECISE("Unable to locate variable, did you define it??");
						}

						if(type != KOT_UNDEFINED && type == exp_argt[argc]){
							//printf("Global variable '%s' match the expected type '%s'\n", var_name, type_table_lh[exp_argt[argc]]);
							argc++;
							KOT_PARSER_NEXT();
							if(token == LXR_COMMA) {
								KOT_PARSER_NEXT();
							}
						}else{
							char* buffer = (char*)arena_alloc(ah, sizeof(char)*256);
							char* temp = (char*)temp_alloc(sizeof(char)*32);
							strcat(buffer, "Type miscmatch, variable '");
							sprintf(temp, "%s",  var_name);
							strcat(buffer, var_name);
							strcat(buffer, "' has type '");
							sprintf(temp, "%s", type_table_lh[type]);
							strcat(buffer, temp);
							strcat(buffer, "', but arg ");
							sprintf(temp, "%d", argc);
							strcat(buffer, temp);
							strcat(buffer, " expect type '");
							sprintf(temp, "%s", type_table_lh[exp_argt[argc]]);
							strcat(buffer, temp);
							strcat(buffer, "'");
							KOT_ERROR_PRECISE(buffer);
						}
					}

					if(argc > 10){
						char* buffer = (char*)arena_alloc(ah, sizeof(char)*256);
						char* temp = (char*)temp_alloc(sizeof(char)*32);
						strcat(buffer, "Too many function argument, currently the maximum supported is limited to the register number for the machine, the function '");
						sprintf(temp, "%s", name);
						strcat(buffer, temp);
						strcat(buffer, "' require ");
						sprintf(temp, "%d", argc);
						strcat(buffer, temp);
						strcat(buffer, " arguments, but the max supported is 10 for now");
						KOT_ERROR(buffer);
					
					}
					{
						if(status == 0){
							if(argc == exp_argc){
								for(int i=0;i<argc;i++){
									//printf("Adding pull instruction for variable located at 0x%x, load destination is register %d\n", var_adr[i], 21+i);
									kot_push_instruction(ah, IR_PULL, 21+i, var_adr[i], 0);
								}
								kot_push_instruction(ah, IR_CALL, *(uint32_t*)name, argc, 0);
							}else{
								char* buffer = (char*)arena_alloc(ah, sizeof(char)*256);
								char* temp = (char*)temp_alloc(sizeof(char)*32);
								strcat(buffer, "Unable to call '");
								sprintf(temp, "%s", name);
								strcat(buffer, temp);
								strcat(buffer, "', ");
								strcat(buffer, "not enought arguments, expecting ");
								sprintf(temp, "%d", exp_argc);
								strcat(buffer, temp);
								strcat(buffer, " but got ");
								sprintf(temp, "%d", argc);
								strcat(buffer, temp);
								KOT_ERROR_PRECISE(buffer);
							}
						}else{
							KOT_ERROR("Unable to perform function call");
						}
					}

				}else{
					KOT_ERROR_PRECISE("No function found with such name");
				}
			}else{
				KOT_SYNTAX_ERR();
			}
		}else{
			KOT_ERROR("Not a valid function call");
		}
	}else{
		KOT_SYNTAX_ERR();
	}	
	return status;
}


int kot_statement_processor(Arena_header* ah, lxer_header* lh, error_handler *eh){
	int status = 0;
	LXR_TOKENS token, next_token;
	KOT_PARSER_REFRESH();
	if(kotvm.cache_scope->type == STRT){
		KOT_ERROR_PRECISE("Cannot have a statement outside a function body");
	}
	KOT_NOT_IMPLEMENTED("Statement instruction processor");
	switch(token){
		case LXR_IF_STATEMENT: 
			break; 
		case LXR_WHILE_STATEMENT: 
			break; 
		case LXR_SWITCH_STATEMENT: 
			break; 
		case LXR_DO_STATEMENT: 
			break; 
		case LXR_FOR_STATEMENT: 
			break; 
		case LXR_RET_STATEMENT: 
			break;
		default: 
			KOT_ERROR_PRECISE("Unrecognized statement");
			break;
	}
	return status;
}


int kot_type_processor(Arena_header* ah, lxer_header* lh, error_handler *eh){
	int status = 0;
	LXR_TOKENS token, next_token, type, cache_token; 
	KOT_PARSER_REFRESH();
	char* name;
	type = token;
	KOT_PARSER_NEXT();
	if(token == LXR_WORD){
		name = lxer_get_word(lh);
	}else{
		KOT_ERROR("No name provided");
	}
	if(lxer_is_statement(next_token) || lxer_is_sep(next_token) || lxer_is_brk(next_token)){
		KOT_PARSER_NEXT();
		switch(token){
			case LXR_ASSIGNMENT:
				status = kot_variable_argument_processor(ah,lh,eh, name,type);
				break;
			case LXR_OPEN_BRK:
				if(kotvm.cache_scope->type == STRT){
					status = kot_function_processor(ah, lh, eh, name, type);
				}else{
					KOT_ERROR("You cannot declare a function scope inside another function body");
				}
				break;
			case LXR_SEMICOLON:
				KOT_INST_VAR(name,kot_get_type_from_token(type), kotvm.stack_pointer, 0, 0);
				break;
			default: 
				KOT_SYNTAX_ERR();
				break;
		}
	}else{
		KOT_ERROR("Missing semicolon or syntax error");
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
		if(token == LXR_WORD){
			status = kot_word_processor(ah, lh, eh);
		}
		if(lxer_is_brk(token)){
			if(token == LXR_CLOSE_CRL_BRK){
				if(kotvm.cache_scope->master == NULL){
					KOT_ERROR("Brachets has no start point or it's out of place");
				}else{
					//printf("Going up the hierarchy to %p\n", kotvm.cache_scope->master);
					if(kotvm.cache_scope->type == FUNC){
						inst_slice* i = list_get_at(kotvm.cache_scope->list, kotvm.cache_scope->list->count-1);
						if(i != NULL){
							if(i->bytecode != IR_RETRN){
								kot_push_instruction(ah, IR_RETRN, 0, 0, 0);
							}
						}
					}
					kotvm.cache_scope = kotvm.cache_scope->master;
					if(kotvm.cache_scope->type == STRT){
						scope* s = list_get_at(kotvm.cache_scope->list, kotvm.cache_scope->list->count-1);
						if(s->type == FUNC){
							kot_scope_writedown(ah, s);
						}
					}
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


KOT_TYPE kot_get_type_from_token(LXR_TOKENS token){
	KOT_TYPE arg_type = KOT_UNDEFINED;
	switch(token){
		case LXR_STRING_TYPE: 
			arg_type = KOT_STR;
			break;
		case LXR_INT_TYPE: 
			arg_type = KOT_INT;
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
		default: break;
	}
	return arg_type;
}

int kot_get_size_from_type(KOT_TYPE type){
	return type_table_size[type];
}


float kot_process_float_literal(lxer_header* lh){
	float f;
	LXR_TOKENS token, next_token;
	lxer_increase_tracker(lh, -1);
	KOT_PARSER_REFRESH();
	if(next_token == LXR_WORD){
		char* lit = lxer_get_string(lh, LXR_SEMICOLON);
		char* format = strchr(lit, 'f');
		if(format != NULL){
			lit++;
		}
		f = (float)atof(lit);
	}
	return f;
}

void kot_scope_writedown(Arena_header* ah, scope* s){
	size_t* offset = (size_t*)temp_alloc(sizeof(size_t));
	kot_scope_list_writedown(ah, s->list,offset);
	//printf("Previous execution target: %d\n", kotvm.program_counter);
	kotvm.program_counter += *offset;
	//printf("New program execution target: %d\n",kotvm.program_counter);
	return;
}

void kot_scope_list_writedown(Arena_header* ah, List_header* lh, size_t* offset){
	for(size_t i=0;lh->count; i++){
		inst_slice * d = list_get_at(lh, i);
		if(d == NULL) break;
		if(d->type != INST){
			if(d->type == FUNC){
				scope* sc = list_get_at(lh, i);
				kot_scope_list_writedown(ah, sc->list, offset);
			}else if(d->type == COND){
				scope_branch* sb = list_get_at(lh, i);
				kot_scope_list_writedown(ah, sb->branch_true->list, offset);
				if(sb->branch_false != NULL){
					kot_scope_list_writedown(ah, sb->branch_false->list, offset);
				}
			}
		}else if(d->type == INST){
			if(kotvm.bytecode_array_tracker + 1 > kotvm.bytecode_array_size){
				inst_slice* old_arr = kotvm.bytecode_array;
				kotvm.bytecode_array = (inst_slice*)arena_alloc(ah, sizeof(inst_slice)*kotvm.bytecode_array_size*2);
				kotvm.bytecode_array_size = kotvm.bytecode_array_size*2;
				memcpy(kotvm.bytecode_array, old_arr, sizeof(inst_slice)*kotvm.bytecode_array_tracker);
			}
			kotvm.bytecode_array[kotvm.bytecode_array_tracker] = *d;
			kotvm.bytecode_array_tracker += 1;
			//printf("Instruction found\n");
			*offset += 1;
		}
	}
	return;
}
