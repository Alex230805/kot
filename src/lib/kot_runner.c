/*
 * kot_runner.c: here the main execution functions find they're place. 
 * This is responsible to store the function that handle the execution 
 * after the interpretation and the single bytecode-related operation
 * to handle the execution for each IR instruction. 
 *
 *
 * */

void kot_run(){
	for(;;){
		inst_slice inst = kotvm.bytecode_array[kotvm.program_counter];
		if(kot_single_run(inst) && kotvm.program_counter < kotvm.bytecode_array_tracker) break;
		kotvm.program_counter+=1;
	}
}

bool kot_single_run(inst_slice inst){
	bool end = false;
	switch(inst.bytecode){
		case IR_PUSH:
			KOT_FN_CALL(push);
			break;
		case IR_PULL:
			KOT_FN_CALL(pull);
			break;
		case IR_JUMP: 
			KOT_FN_CALL(jump);
			break;
		case IR_RETRN:
			KOT_FN_CALL(retrn);
			break;
		case IR_CALL:
			KOT_FN_CALL(call);
			break;
		case IR_ILLEGAL:
			KOT_EX_NOT_IMPLEMENTED(inst.bytecode, ir_table_lh[inst.bytecode]);
			break;
		case IR_HALT: 
			end = true;
		default: break;
	}
	return end;
}


KOT_R_I KOT_FN_DEFINE(push){
	KOT_R_I status = 0;
	TODO("push instruction", NULL);
	return status;
}

KOT_R_I KOT_FN_DEFINE(pull){
	KOT_R_I status = 0;
	TODO("pull instruction", NULL);
	return status;
}

KOT_R_I KOT_FN_DEFINE(jump){
	KOT_R_I status = 0;
	TODO("jump instruction", NULL);
	return status;
}

KOT_R_I KOT_FN_DEFINE(retrn){
	KOT_R_I status = 0;
	TODO("return instruction", NULL);
	return status;
}

KOT_R_I KOT_FN_DEFINE(call){
	KOT_R_I status = 0;
	TODO("call instruction", NULL);

/*
	scope* fn_scope = kot_fn_get_scope(name);
	if(!fn_scope){
		KOT_ERROR("Error while try to get function scope: unable to find it");
	}else{
		// TODO: here the calling and the argument switching must occur, for now every function does not accept any argument
		switch(fn_scope->type){
			case FFI: 
				__ffi_linker_callback fn_ext_body = fn_scope->fn_pointer;
				fn_ext_body();
				break;
			case FUNC: 

				break;
			default: break;
		}
	}

	*/

	return status;
}
