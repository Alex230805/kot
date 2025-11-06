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

void kot_get_bytecode(){
	// kot_get_bytecode will print the content from the bytecode of the VM, not the internal scope tree constructer during 
	// compilation
	fprintf(stdout, "\nCurrent program list: \n");
	for(size_t i=0;i<kotvm.bytecode_array_tracker;i++){
		printf("\t\t%zu: %s -1x%x, 0x%x, aux 0x%x \t\t| ", i, ir_table_lh[kotvm.bytecode_array[i].bytecode],\
				kotvm.bytecode_array[i].arg_0,\
				kotvm.bytecode_array[i].arg_1,\
				kotvm.bytecode_array[i].arg_2\
			  );
		printf("\n");
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

void kot_push_instruction(Arena_header* ah, kot_ir inst, uint32_t arg_0, uint32_t arg_1, uint32_t arg_2){
	inst_slice *is = (inst_slice*)arena_alloc(ah, sizeof(inst_slice));
	is->bytecode = inst;
	is->arg_0 = arg_0;
	is->arg_1 = arg_1;
	is->arg_2 = arg_2;
	arena_list_push(ah, kotvm.cache_scope->list, is);
	//if(!is_fn) printf("Pushing new instruction inside scope %p\n", kotvm.cache_scope);
}


inst_slice kot_get_current_inst(){
	return kotvm.bytecode_array[kotvm.program_counter];
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
	strcpy(fn->name, name);
	fn->param_len = param_len;
	fn->param_type = param_type;
	fn->fn_scope = fn_scope;
	return fn;
}


void kot_set_line(size_t cline){
	line = cline;
}

void kot_pc_inc(){
	kotvm.program_counter += 1;
}

void kot_push_stack(uint8_t* data, int size){
	for(size_t i=0; i < size; i++){
		kotvm.memory[kotvm.stack_pointer] = data[i];
		if(kotvm.stack_pointer + 1 >= DEF_STACK_LIMIT){
			kotvm.stack_pointer = 0;
		}else{
			kotvm.stack_pointer += 1;
		}
	}
}

void kot_alloc_stack(int size){
	//printf("Requiring %d reserved stack space\n", size);
	for(size_t i=0; i < size; i++){
		if(kotvm.stack_pointer + 1 >= DEF_STACK_LIMIT){
			kotvm.stack_pointer = 0;
		}else{
			kotvm.stack_pointer += 1;
		}
	}
}

uint8_t* kot_pull_stack(Arena_header* ah, int size){
	uint8_t* buffer = (uint8_t*)arena_alloc(ah, sizeof(uint8_t)*size);
	for(size_t i=0; i < size; i++){
		buffer[i] = kotvm.memory[kotvm.stack_pointer];
		if(kotvm.stack_pointer - 1 < DEF_STACK_INIT){
			kotvm.stack_pointer = DEF_STACK_LIMIT;
		}else{
			kotvm.stack_pointer -= 1;
		}
	}
	return buffer;
}

uint8_t* kot_get_stack(Arena_header* ah, int size){
	uint32_t og_ptr = kotvm.stack_pointer;
	uint8_t* buffer = (uint8_t*)arena_alloc(ah, sizeof(uint8_t)*size);
	for(size_t i=0; i < size; i++){
		buffer[i] = kotvm.memory[kotvm.stack_pointer];
		if(kotvm.stack_pointer - 1 < DEF_STACK_INIT){
			kotvm.stack_pointer = DEF_STACK_LIMIT;
		}else{
			kotvm.stack_pointer -= 1;
		}
	}
	kotvm.stack_pointer = og_ptr;
	return buffer;
}

size_t kot_write_heap(Arena_header* ah,uint8_t* data, int size){
	if(kotvm.memory_tracker + size >= kotvm.def_memory_size){
		uint8_t *old_mem = kotvm.memory;
		kotvm.memory = (uint8_t*)arena_alloc(ah, sizeof(uint8_t)*kotvm.def_memory_size*2);
		kotvm.def_memory_size *= 2;
		memcpy(kotvm.memory, old_mem,kotvm.memory_tracker);
	}
	size_t ptr = kotvm.memory_tracker;
	memcpy(kotvm.memory+ptr, data, size);
	kotvm.memory[ptr+size] = '\0';
	kotvm.memory_tracker += size+1;
	return ptr;
}


void kot_push_return_ptr(uint32_t ptr){
	for(size_t i=0; i < 4; i++){
		kotvm.memory[kotvm.call_stack_pointer] = ptr >> i*8;
		if(kotvm.call_stack_pointer + 1 >= DEF_STK_CALL_LIMIT){
			kotvm.call_stack_pointer = 0;
		}else{
			kotvm.call_stack_pointer += 1;
		}
	}
}

uint32_t kot_pull_return_ptr(){	
	uint32_t kot_cache_fn_pointer = 0;
	for(size_t i=0; i < 4; i++){
		kot_cache_fn_pointer = kot_cache_fn_pointer |  kotvm.memory[kotvm.call_stack_pointer] << i*8;
		if(kotvm.call_stack_pointer - 1 < DEF_STK_CALL_INIT){
			kotvm.call_stack_pointer = DEF_STK_CALL_LIMIT;
		}else{
			kotvm.call_stack_pointer -= 1;
		}
	}
	return kot_cache_fn_pointer; 
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
