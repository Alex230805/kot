/*
 *
 * Here the main functions related to the parsing of the source code found they're place
 *
 * */


int kot_statement_dispatch(Arena_header* ah, lxer_header* lh, error_handler *eh){
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


int kot_type_dispatch(Arena_header* ah, lxer_header* lh, error_handler *eh){
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
			status = kot_type_dispatch(ah, lh, eh);
		}
		if(lxer_is_statement(token)){
			status = kot_statement_dispatch(ah, lh, eh);
		}
		if(token == LXR_WORD){
			status = kot_fcall_processor(ah, lh, eh);
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
