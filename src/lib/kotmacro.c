/*
 *	
 *	kotmacro.c: here is were the repetitive actions abstracted as macro 
 *	are located. This may include macro related to the parser, the virtual 
 *	machine or the entier runner, it just serve the purpose of having a common
 *	place to store all the possible macro used in the entire kot.
 *
 * */



#define KOT_ERROR_PRECISE(name)\
	error_push_error(eh,name, line, 2 ,lxer_get_current_pointer(lh),strlen(lxer_get_current_pointer(lh)));\
	status = 1;

#define KOT_ERROR(name)\
	error_push_error(eh,name, 0, 2 ,NULL,0);\
	error_print_error(eh,(print_set){true,true,false,false,true,false, false});\
	eh->tracker = 0;\
	status = 1;

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

#define KOT_NOT_IMPLEMENTED(message, ...)\
	TODO(message, __VA_ARGS__);\
	assert(0)

#define SWITCH_SCOPE(ass_type)\
	do{\
		scope *new_scope = (scope*)arena_alloc(ah, sizeof(scope));\
		arena_list_push(ah,kotvm.cache_scope->list,new_scope);\
		new_scope->master = kotvm.cache_scope;\
		new_scope->type = ass_type;\
		new_scope->list = (List_header*)arena_alloc(ah, sizeof(List_header));\
		new_scope->var_def = (char**)arena_alloc(ah,sizeof(char*)*DEF_GET_ARR_SIZE);\
		new_scope->var_def_tracker = 0;\
		new_scope->var_def_size = DEF_GET_ARR_SIZE;\
		kotvm.cache_scope = new_scope;\
	}while(0);


#define KOT_PUSH_VAR_DEC(kot_type, name, arg_0, arg_1,arg_2, is_fn)\
	do{\
		if(!kot_globl_variable_already_present(name)){\
			if(kotvm.cache_scope->type == FUNC){\
				if(!kot_variable_already_present(name)){\
					kot_push_variable_def(ah, name);\
					kot_push_instruction(ah,kot_type, name, arg_0, arg_1,arg_2, is_fn);\
				}else{\
					KOT_ERROR("Variable already defined");\
				}\
			}else{\
				kot_push_globl_variable_def(ah, name);\
				kot_push_instruction(ah, kot_type, name, arg_0, arg_1,arg_2, is_fn);\
			}\
		}else{\
			KOT_ERROR("Variable already defined");\
		}\
	}while(0);

