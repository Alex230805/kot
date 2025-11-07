/*
 *	
 *	kotmacro.c: here is were the repetitive actions abstracted as macro 
 *	are located. This may include macro related to the parser, the virtual 
 *	machine or the entier runner, it just serve the purpose of having a common
 *	place to store all the possible macro used in the entire kot.
 *
 * */



#define KOT_ERROR_PRECISE(name)\
	if(lxer_get_current_token(lh) == LXR_WORD){\
		error_push_error(eh,name, lxer_get_current_line(lh), 2,lxer_get_current_pointer(lh),strlen(lxer_get_word(lh)));\
	}else{\
		error_push_error(eh,name, lxer_get_current_line(lh), 2,lxer_get_current_pointer(lh),strlen(lxer_get_string_representation(lxer_get_current_token(lh))));\
	}\
	status = 1;\
	return status

#define KOT_ERROR(name)\
	error_push_error(eh,name, lxer_get_current_line(lh), 2 ,NULL,0);\
	error_print_error(eh,(print_set){true,true,false,false,true,false, false});\
	eh->tracker = 0;\
	status = 1;\
	return status;

#define KOT_SYNTAX_ERR()\
	KOT_ERROR_PRECISE("Syntax error");

#define KOT_PARSER_NEXT()\
	lxer_next_token(lh);\
	token = lxer_get_current_token(lh);\
	next_token = lxer_get_next_token(lh);

#define KOT_PARSER_REFRESH()\
	token = lxer_get_current_token(lh);\
	next_token = lxer_get_next_token(lh);

#define KOT_GET_REF()\
	printf("Current token: %s\n", token_table_lh[token]);\
	printf("Following token: %s\n", token_table_lh[next_token]);


#define KOT_EX_NOT_IMPLEMENTED(bytecode, string)\
	printf("Parsing of instruction '[%d] = %s' and behaviour is under implementation", bytecode, string);\
	assert(0);

#define KOT_NOT_IMPLEMENTED(message)\
	TODO(message, NULL);\
	arena_free(&eh->ah);\
	arena_free(ah);\
	arena_free(&lh->lxer_ah);\
	exit(69);

#define SWITCH_SCOPE(ass_type)\
	do{\
		arena_list_push(ah,kotvm.cache_scope->list,new_scope);\
		new_scope->master = kotvm.cache_scope;\
		new_scope->type = ass_type;\
		new_scope->list = (List_header*)arena_alloc(ah, sizeof(List_header));\
		new_scope->var_def = (var_cell*)arena_alloc(ah,sizeof(var_cell)*DEF_GET_ARR_SIZE);\
		new_scope->var_def_tracker = 0;\
		new_scope->var_def_size = DEF_GET_ARR_SIZE;\
		kotvm.cache_scope = new_scope;\
	}while(0);

#define KOT_INST_VAR(name, kot_type, arg_0, arg_1, arg_2)\
	do{\
		if(!kot_globl_variable_already_present(name)){\
			if(kotvm.cache_scope->type == FUNC){\
				if(!kot_variable_already_present(name)){\
					kot_push_variable_def(ah, name, kot_type, arg_0);\
					kot_push_instruction(ah,IR_PUSH, arg_0, arg_1,arg_2);\
					if(arg_2 == 0) kot_alloc_stack(kot_get_size_from_type(kot_type));\
				}else{\
					KOT_ERROR("Variable already defined");\
				}\
			}else{\
				kot_push_globl_variable_def(ah, name, kot_type, arg_0);\
				kot_push_instruction(ah, IR_PUSH, arg_0, arg_1,arg_2);\
				if(arg_2 == 0) kot_alloc_stack(kot_get_size_from_type(kot_type));\
			}\
		}else{\
			KOT_ERROR("Variable already defined");\
		}\
	}while(0);
