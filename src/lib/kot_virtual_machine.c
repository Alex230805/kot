/*
 *
 * Here you can find functions related to the virtual machine initialization and 
 * management
 *
 * */


void kot_init_interpreter(Arena_header* arena, lxer_header* lxer, error_handler* error){
	ah = arena;
	lh = lxer;
	eh = error;
	glob_var_def = (var_cell*)arena_alloc(ah,sizeof(var_cell)*DEF_GET_ARR_SIZE);
	glob_var_def_tracker = 0;
	glob_var_def_size = DEF_GET_ARR_SIZE;

	globl_fn_signature = (fn_signature*)arena_alloc(ah, sizeof(fn_signature)*DEF_GET_ARR_SIZE);
	globl_fn_signature_tracker = 0;
	globl_fn_signature_size = DEF_GET_ARR_SIZE;
}

void kot_init_vm(){
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


void kot_get_bytecode(){
	// kot_get_bytecode will print the content from the bytecode of the VM, not the internal scope tree constructer during 
	// compilation
	fprintf(stdout, "\nCurrent program list: \n");
	for(size_t i=0;i<kotvm.bytecode_array_tracker;i++){
		printf("\t\t%zu: %s 0x%x, 0x%x, aux 0x%x \n", i, ir_table_lh[kotvm.bytecode_array[i].bytecode],\
				kotvm.bytecode_array[i].arg_0,\
				kotvm.bytecode_array[i].arg_1,\
				kotvm.bytecode_array[i].arg_2\
			  );
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

void kot_push_instruction(kot_ir inst, uint32_t arg_0, uint32_t arg_1, uint32_t arg_2){
	inst_slice *is = (inst_slice*)arena_alloc(ah, sizeof(inst_slice));
	is->type = INST;
	is->bytecode = inst;
	is->arg_0 = arg_0;
	is->arg_1 = arg_1;
	is->arg_2 = arg_2;
	arena_list_push(ah, kotvm.cache_scope->list, is);
	if(kotvm.cache_scope->type == STRT){
		if(kotvm.bytecode_array_tracker + 1 > kotvm.bytecode_array_size){
			inst_slice* old_arr = kotvm.bytecode_array;
			kotvm.bytecode_array = (inst_slice*)arena_alloc(ah, sizeof(inst_slice)*kotvm.bytecode_array_size*2);
			kotvm.bytecode_array_size = kotvm.bytecode_array_size*2;
			memcpy(kotvm.bytecode_array, old_arr, sizeof(inst_slice)*kotvm.bytecode_array_tracker);
		}
		kotvm.bytecode_array[kotvm.bytecode_array_tracker] = *is;
		kotvm.bytecode_array_tracker += 1;
	}
}


inst_slice kot_get_current_inst(){
	return kotvm.bytecode_array[kotvm.program_counter];
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

uint8_t* kot_pull_stack(int size){
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

uint8_t* kot_get_stack(int size){
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


size_t kot_write_heap(uint8_t* data, int size){
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
