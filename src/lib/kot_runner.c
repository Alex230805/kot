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
		if(kot_single_run(inst)) break;
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
		case IR_BEQ:	
		case IR_BGE:		
		case IR_BLT:		
		case IR_BNE:		
		case IR_RET:		
		case IR_MOV:
		case IR_LB:		
		case IR_LH:		
		case IR_LW:	
		case IR_SB:		
		case IR_SH:		
		case IR_SW:		
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
	//TODO("push instruction", NULL);
	return status;
}

KOT_R_I KOT_FN_DEFINE(pull){
	KOT_R_I status = 0;
	//TODO("pull instruction", NULL);
	return status;
}
