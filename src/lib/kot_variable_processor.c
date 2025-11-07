/*
 *
 * Here all functions related to variable parsing are contained, specific functions like 
 * argument processing parser or general variable allocation are processed here. 
 *
 *
 *
 * */


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

