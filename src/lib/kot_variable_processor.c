/*
 *
 * Here all functions related to variable parsing are contained, specific functions like 
 * argument processing parser or general variable allocation are processed here. 
 *
 *
 *
 * */


int kot_variable_allocator(char** var_list, int var_list_tracker, uint32_t ass_0, uint32_t ass_1, KOT_TYPE type, int status){
	for(int i=0;i<var_list_tracker; i++){
		KOT_INST_VAR(var_list[i],type, kotvm.stack_pointer, ass_0, ass_1);
	}
	return status;
}

int kot_variable_argument_processor(char* name,LXR_TOKENS type){
	int status = 0;
	LXR_TOKENS token, next_token;
	KOT_PARSER_REFRESH();
	char* var_list[512];
	int var_list_tracker = 0;
	while((token != LXR_ASSIGNMENT && token != LXR_SEMICOLON) && status == 0){
		if(token == LXR_WORD){
			char* name = lxer_get_word(lh);
			if(name != NULL){
				var_list[var_list_tracker] = lxer_get_word(lh);
				var_list_tracker+=1;
				if(var_list_tracker > 512){
					KOT_ERROR("Exeeding single line declaration limit");
				}
			}
		}else{
			KOT_ERROR_PRECISE("Missing variable name");
		}
		KOT_PARSER_NEXT();
		if(next_token != TOKEN_TABLE_END && token != LXR_ASSIGNMENT) KOT_PARSER_NEXT();
	}
	if(token == LXR_SEMICOLON){
		return kot_variable_allocator(var_list, var_list_tracker, 0, 0, kot_get_type_from_token(type), status);
	}
	if(token != LXR_ASSIGNMENT){
		KOT_ERROR("Missing semicolon or invalid definition");
	}
	if(type == LXR_STRING_TYPE){
		if(next_token == LXR_DOUBLE_QUOTE){
			KOT_PARSER_NEXT();
			char* string = lxer_get_string(lh, LXR_DOUBLE_QUOTE);
			if(string && strlen(string) > 1){
				KOT_PARSER_REFRESH();
				if(next_token != LXR_SEMICOLON){
					KOT_ERROR_PRECISE("Missing semicolon");
				}else{
					/*	
						TODO: currently there is not syntax decided for differntial variable compount 
						definition, if you declare "int a,b,c = N" all of those will be initialized with N.
						A possible future syntax may be "int a,b,c = {34,35,69}".
					*/
					status = kot_variable_allocator(var_list, var_list_tracker, (size_t)string,strlen(string),kot_get_type_from_token(type), status);
					kot_alloc_stack(strlen(string));
				}
			}else{
				KOT_ERROR("String literal resulted empty, fill it with something");
			}
		}else{
			KOT_ERROR_PRECISE("Not a valid implicit string definition");
		}
		return status;
	}
	if(next_token == LXR_WORD){
		KOT_PARSER_NEXT();
		char* arg =  lxer_get_word(lh);
		float arg_f = 0.00;
		int arg_i = atoi(arg);
		if(type == LXR_FLOAT_TYPE){
			arg_f = kot_process_float_literal();
			arg_i = *(uint32_t*)&arg_f;
		}
		status = kot_variable_allocator(var_list, var_list_tracker, arg_i, 0,kot_get_type_from_token(type), status);
	}else{
		KOT_ERROR_PRECISE("Not a valid assignment or incomplete syntax");
	}
	return status;
}


float kot_process_float_literal(){
	float f;
	LXR_TOKENS token, next_token;
	lxer_increase_tracker(lh, -1);
	KOT_PARSER_REFRESH();
	if(next_token == LXR_WORD){
		char* lit = lxer_get_string(lh, LXR_SEMICOLON);
		f = (float)atof(lit);
	}
	return f;
}

