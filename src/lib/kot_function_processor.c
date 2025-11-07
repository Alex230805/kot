/*
 *
 * Here functions that perform the parsing for function call or function declaration
 * find they're place.
 *
 *
 * */


int kot_fcall_processor(Arena_header* ah, lxer_header* lh, error_handler *eh){
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
