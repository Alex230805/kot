/*
 *	kotvm_utils.c: here is were the functions related to the peripherals 
 *	functionality of the vm are located. This does not directly contain 
 *	parser-related operation but more likely parser related functions 
 *	that are used inside the parser to interact with the vm during the 
 *	interpretation process. This means that for example here you find 
 *	functions related to the creation of  function signature/definition 
 *	or variable declaration/definition, but you can also find more general
 *	functions like kot_get_bytecode() which will print the final bytecode 
 *	of the vm after the interpretation.
 *
 * */


void kot_get_program_list(FILE* filestream){
	fprintf(filestream,"\n");
	for(size_t i=0;i<kotvm.program_source_tracker;i++){
		fputs(kotvm.program_source[i],filestream);
	}
}

void kot_push_globl_variable_def(Arena_header* ah,char* name, KOT_TYPE type, int32_t pos){
	//NOTY("GLOBAL ACTION", "Pushing global variable '%s'", name);
	if(glob_var_def_tracker += 1 >= glob_var_def_size){
		var_cell* old_arr = glob_var_def;
		glob_var_def = (var_cell*)arena_alloc(ah,sizeof(var_cell)*glob_var_def_size*2);
		glob_var_def_size*=2;
		memcpy(glob_var_def, old_arr, sizeof(var_cell)*glob_var_def_tracker);
		kotvm.main_scope->var_def = glob_var_def;
		kotvm.main_scope->var_def_size = glob_var_def_size;
	}
	glob_var_def[glob_var_def_tracker].name = name;
	glob_var_def[glob_var_def_tracker].type = type;
	glob_var_def[glob_var_def_tracker].adr = pos;
	glob_var_def_tracker += 1;
	kotvm.main_scope->var_def_tracker = glob_var_def_tracker;
}

bool kot_globl_variable_already_present(char* name){
	for(size_t i=0;i<glob_var_def_tracker; i++){
		//printf("Comparisong between '%s' and '%s'\n", name, glob_var_def[i].name);
		if(strcmp(name, glob_var_def[i].name)==0){
			return true;
		}
	}
	return false;
}
void kot_push_variable_def(Arena_header* ah,char* name, KOT_TYPE type, int32_t pos){
	//NOTY("LOCAL ACTION", "Pushing local variable '%s'", name);
	if(kotvm.cache_scope->var_def_tracker += 1 >= kotvm.cache_scope->var_def_size){
		var_cell* old_arr = kotvm.cache_scope->var_def;
		kotvm.cache_scope->var_def = (var_cell*)arena_alloc(ah,sizeof(var_cell)*kotvm.cache_scope->var_def_size*2);
		kotvm.cache_scope->var_def_size*=2;
		memcpy(kotvm.cache_scope->var_def, old_arr, sizeof(var_cell)*kotvm.cache_scope->var_def_tracker);
	}
	kotvm.cache_scope->var_def[kotvm.cache_scope->var_def_tracker].name = name;
	kotvm.cache_scope->var_def[kotvm.cache_scope->var_def_tracker].type = type;
	kotvm.cache_scope->var_def[kotvm.cache_scope->var_def_tracker].adr = pos;
	kotvm.cache_scope->var_def_tracker += 1;
}

bool kot_variable_already_present(char* name){
	for(size_t i=0;i<kotvm.cache_scope->var_def_tracker; i++){
		if(strcmp(name, kotvm.cache_scope->var_def[i].name)==0){
			return true;
		}
	}
	return false;
}

int32_t kot_var_get_adr(char* name){
	for(size_t i=0;i<kotvm.cache_scope->var_def_tracker; i++){
		if(strcmp(name, kotvm.cache_scope->var_def[i].name)==0){
			return kotvm.cache_scope->var_def[i].adr;
		}
	}
	return 0;
}

int32_t kot_globl_var_get_adr(char* name){
	for(size_t i=0;i<glob_var_def_tracker; i++){
		if(strcmp(name, glob_var_def[i].name)==0){
			return glob_var_def[i].adr; 
		}
	}
	return 0;
}



void kot_push_fn_dec(Arena_header* ah, fn_signature fn){
	if(globl_fn_signature_tracker += 1 >= globl_fn_signature_size){
		fn_signature* old_arr = globl_fn_signature;
		globl_fn_signature = (fn_signature*)arena_alloc(ah,sizeof(fn_signature)*globl_fn_signature_size*2);
		globl_fn_signature_size*=2;
		memcpy(globl_fn_signature, old_arr, sizeof(fn_signature)*globl_fn_signature_tracker);
	}
	globl_fn_signature[globl_fn_signature_tracker] = fn;
	globl_fn_signature_tracker += 1;
}


bool kot_fn_already_declared(char* name){
	bool status = false;
	for(size_t i=0;i<globl_fn_signature_tracker && !status; i++){
		if(globl_fn_signature[i].name != NULL){
			if(strcmp(globl_fn_signature[i].name, name) == 0){
				status = true;
			}
		}
	}
	return status;
}

scope* kot_fn_get_scope(char* name){
	bool status = false;
	scope* fn_scope = NULL;
	for(size_t i=0;i<globl_fn_signature_tracker && !status; i++){
		if(globl_fn_signature[i].name != NULL){
			if(strcmp(globl_fn_signature[i].name, name) == 0){
				status = true;
				fn_scope = globl_fn_signature[i].fn_scope;
			}
		}
	}
	return fn_scope;
}

fn_signature* kot_fn_get_signature(char* name){
	for(size_t i=0;i<globl_fn_signature_tracker; i++){
		if(globl_fn_signature[i].name != NULL){
			if(strcmp(globl_fn_signature[i].name, name) == 0){
				return &globl_fn_signature[i];
			}
		}
	}
	return NULL;
}

KOT_TYPE kot_globl_var_get_type(char* name){
	for(size_t i=0;i<glob_var_def_tracker; i++){
		if(glob_var_def[i].name != NULL){
			if(strcmp(name, glob_var_def[i].name)==0){
				return glob_var_def[i].type;
			}
		}
	}
	return KOT_UNDEFINED;
}

KOT_TYPE kot_var_get_type(char* name){
	for(size_t i=0;i<kotvm.cache_scope->var_def_tracker; i++){
		if(kotvm.cache_scope->var_def[i].name != NULL){
			if(strcmp(name, kotvm.cache_scope->var_def[i].name)==0){
				return kotvm.cache_scope->var_def[i].type;
			}
		}
	}
	return KOT_UNDEFINED;
}

fn_signature* kot_define_fn(Arena_header* ah,char* name,int param_len, KOT_TYPE* param_type, scope* fn_scope){
	fn_signature* fn = (fn_signature*)arena_alloc(ah, sizeof(fn_signature));
	fn->name = (char*)arena_alloc(ah, sizeof(char)*strlen(name));
	fn->position = kotvm.bytecode_array_tracker;
	strcpy(fn->name, name);
	fn->param_len = param_len;
	fn->param_type = param_type;
	fn->fn_scope = fn_scope;
	return fn;
}


void kot_set_line(size_t cline){
	line = cline;
}


bool kot_link_function(Arena_header* ah, fn_signature *fn, __ffi_linker_callback fn_call){
	if(!kot_fn_already_declared(fn->name)){
		scope *new_scope = (scope*)arena_alloc(ah, sizeof(scope));
		new_scope->type = FFI;
		new_scope->var_def = NULL;
		new_scope->var_def_tracker = 0;
		new_scope->var_def_size = 0;
		new_scope->master = kotvm.main_scope;
		new_scope->fn_pointer = fn_call;
		new_scope->list = NULL;	
		fn->fn_scope = new_scope;
		kot_push_fn_dec(ah, *fn);
		return true;
	}
	return false;
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
