#define ILXER_IMPLEMENTATION

#include "ilxer.h"

#define TOK_RESIZE()\
	do{																											\
		if(array_tracker >= array_size){																		\
			size_t old_size = array_size;																		\
			array_size *= 2;																					\
			token_slice* n_cache_mem = (token_slice*)arena_alloc(&lh->lxer_ah,sizeof(token_slice)*array_size);	\
			for(size_t z=0;z<old_size;z++){																		\
				n_cache_mem[z] = cache_mem[z];																	\
			}																									\
			cache_mem = n_cache_mem;																			\
		}																										\
	}while(0)																									\


#define TOK_RESIZE_PRE()\
	if(pre_array_tracker+5 >= pre_array_size){																		\
		size_t old_size = pre_array_size;																			\
		pre_array_size *= 2;																						\
		token_slice* pre_n_cache_mem = (token_slice*)arena_alloc(&lh->lxer_ah,sizeof(token_slice)*pre_array_size);	\
		for(size_t z=0;z<old_size;z++){																				\
			pre_n_cache_mem[z] = pre_cache_mem[z];																	\
		}																											\
		pre_cache_mem = pre_n_cache_mem;																			\
	}


void lxer_start_lexing(lxer_header* lh, char * source){
	lh->source = source;
	lh->source_len = strlen(source);

	size_t array_size = 12;
	size_t array_tracker = 0;
	token_slice *cache_mem = (token_slice*)arena_alloc(&lh->lxer_ah,sizeof(token_slice)*array_size);

	char * buffer = (char*)arena_alloc(&lh->lxer_ah,sizeof(char)*32);
	bool ignore_lex = true;
	size_t line_tracker = 1;	
	for(size_t i=0;i<lh->source_len && ignore_lex;i++){
		if(lh->source[i] > 0x20){
			ignore_lex = false;
		}
	}
	if(ignore_lex){
		cache_mem[array_tracker].token = TOKEN_TABLE_END;
		cache_mem[array_tracker].byte_pointer = lh->source;
		lh->stream_out = cache_mem;
		lh->stream_out_len = array_tracker;
		return;
	}

#ifndef LXER_DISABLE_MODE_SWITCHER
	bool ignore_string = false;
	bool jump_hover = false;
#endif
	for(size_t i=0;i<lh->source_len;i++){
		char* tracker = &lh->source[i];
		for(size_t j=0;j<TOKEN_TABLE_END; j++){
			LXR_TOKENS token = token_array[j];
			switch(token){
#define X(name)\
	case name:
				TOKEN_SEPARATOR()
					ignore_lex = true;
					break;
				default:
					ignore_lex = false;
					break;
			}
#undef X

			//////////////////////////////////////////
			if(!ignore_lex){
				bool isolated = false;
				bool abort = true;
				bool nl = false;
				if(token == LXR_NEW_LINE){
					if(*tracker == '\n') {
						abort = false;
						nl = true;
					}
				}else{
					size_t ws = strlen(token_table_lh[token]);
					buffer[0] = '\0';
					memcpy(buffer, tracker, sizeof(char)*32);
					buffer[ws] = '\0';

					if(!nl &&(tracker+ws < (lh->source+lh->source_len)) && lh->source[i+ws] == ' ')	isolated = true;
					if(!nl && strcmp(buffer,token_table_lh[token]) == 0 && strlen(buffer) > 0){
#ifndef LXER_DISABLE_MODE_SWITCHER
						if(token == LXR_DOUBLE_QUOTE){
							if(!ignore_string){
								ignore_string = true;
							}else{
								ignore_string = false;
								jump_hover = false;
							}
						}
#endif
						abort = false;
					}
				}
#ifndef LXER_DISABLE_MODE_SWITCHER
				if(!abort && !jump_hover){
#else
				if(!abort){
#endif
					if(token == LXR_NEW_LINE){
						line_tracker += 1;
					}
					switch(token){
						// token syntax variation: if a token require 
						// a space to be validated you can manually 
						// insert it in the fallthrough down here
#define X(name)\
						case name:
							TOKEN_ISOLATED()
								if(isolated){
									cache_mem[array_tracker].token = token;
									cache_mem[array_tracker].byte_pointer = tracker;
									cache_mem[array_tracker].line = line_tracker;
									array_tracker+=1;
								}
							break;
#undef X
						default:
							cache_mem[array_tracker].token = token;
							cache_mem[array_tracker].byte_pointer = tracker;
							cache_mem[array_tracker].line = line_tracker;
							array_tracker+=1;
#ifndef LXER_DISABLE_MODE_SWITCHER
							if(ignore_string){
								jump_hover = true;
							}
#endif
							break;
					}
					TOK_RESIZE();
				}
			}
			////////////////////////////////////////
		}
	}
	
	lh->stream_out = cache_mem;
	lh->stream_out_len = array_tracker;
#ifdef ILXER_PRECISE_MODE

	size_t pre_array_size = 12;
	size_t pre_array_tracker = 0;
	token_slice *pre_cache_mem = (token_slice*)arena_alloc(&lh->lxer_ah,sizeof(token_slice)*pre_array_size);
	
	char * start = lh->source;
	char* end = NULL;
	
	for(size_t i=0;i<lh->stream_out_len; i++){
		size_t word_size = 0;
		end = lh->stream_out[i].byte_pointer;
		word_size = end - start;
		char* word_buffer= (char*)arena_alloc(&lh->lxer_ah, sizeof(char)*512);
		memcpy(word_buffer, start, sizeof(char)*word_size);
		word_buffer[word_size] = '\0';

		bool valid_word = false;
		for(size_t j=0;j<word_size;j++){
			if(word_buffer[j] >= '0'){
				valid_word = true;
			}
		}
		if(valid_word){
			bool cl_end = false;
			char* cleaned_word = (char*)arena_alloc(&lh->lxer_ah, sizeof(char)*512);
			size_t tr_copy_tracker = 0;
			for(size_t tr = 0; tr < word_size && !cl_end; tr++){
				if(word_buffer[tr] >= '0'){
					cleaned_word[tr_copy_tracker] = word_buffer[tr];
					tr_copy_tracker += 1;
				}
			}
			cleaned_word[tr_copy_tracker] = '\0';
			pre_cache_mem[pre_array_tracker].token = LXR_WORD;
			pre_cache_mem[pre_array_tracker].byte_pointer = cleaned_word;
			pre_cache_mem[pre_array_tracker].line = lh->stream_out[i].line;
			pre_array_tracker += 1;
		}
		
		if(lh->stream_out[i].token != LXR_SPACE && lh->stream_out[i].token != LXR_NEW_LINE){
			pre_cache_mem[pre_array_tracker] = lh->stream_out[i];
			pre_array_tracker += 1;
			TOK_RESIZE_PRE();
		}
		if(lh->stream_out[i].token == LXR_NEW_LINE){
			start = lh->stream_out[i].byte_pointer + 1;		
		}else{
			start = lh->stream_out[i].byte_pointer + strlen(token_table_lh[lh->stream_out[i].token]);
		}
	}
	lh->stream_out = pre_cache_mem;
	lh->stream_out_len = pre_array_tracker;

#endif
	return;
}

size_t lxer_get_current_line(lxer_header*lh){
	return lh->stream_out[lh->lxer_tracker].line;
}

void lxer_get_lxer_content(lxer_header*lh){
	NOTY("ILXER", "Stream out length: %zu", lh->stream_out_len);
	NOTY("ILXER","Tokenzer output: ", NULL);
	for(size_t i=0;i<lh->stream_out_len;i++){
		LXR_TOKENS tok = lh->stream_out[i].token;
		char* pointer = lh->stream_out[i].byte_pointer;
		if(lh->stream_out[i].token == LXR_WORD){
			printf("\ttoken word found at line %zu -> %s\n", lh->stream_out[i].line,lh->stream_out[i].byte_pointer);
		}else{
			printf("\ttoken found at line %zu: token_table[%d] -> %s\n", lh->stream_out[i].line, tok,token_table_lh[tok]);
		}
	}
}

bool lxer_next_token(lxer_header*lh){
	lh->lxer_tracker+=1;
	if(lh->lxer_tracker >= lh->stream_out_len) {
		return false;
	}
	return true;
}


LXR_TOKENS lxer_get_current_token(lxer_header*lh){
	if(lh->lxer_tracker >= lh->stream_out_len) {
		return TOKEN_TABLE_END;
	}
	return lh->stream_out[lh->lxer_tracker].token;
}


LXR_TOKENS lxer_get_next_token(lxer_header*lh){
	if(lh->lxer_tracker+1 >= lh->stream_out_len) {
		return TOKEN_TABLE_END;
	}
	return lh->stream_out[lh->lxer_tracker+1].token;
}

void lxer_set_new_target(lxer_header* lh, char* new_line){
	for(size_t i=lh->lxer_tracker;i<lh->stream_out_len; i++){
		if(lh->stream_out[i].byte_pointer > new_line){
			lh->lxer_tracker = i;
			break;
		}
	}
}

void lxer_increase_tracker(lxer_header*lh,int tracker){
	if(lh->lxer_tracker+tracker < lh->stream_out_len){
		lh->lxer_tracker += tracker;
	}
}



void lxer_set_new_tracker(lxer_header*lh,int tracker){
	if(tracker < lh->stream_out_len){
		lh->lxer_tracker = tracker;
	}
}

char* lxer_get_string_representation(LXR_TOKENS tok){
	return token_table_lh[tok];
}

char* lxer_get_word(lxer_header*lh){
	if(lh->stream_out[lh->lxer_tracker].token == LXR_WORD){
		return lh->stream_out[lh->lxer_tracker].byte_pointer;
	}
	return NULL;
}

char* lxer_get_current_pointer(lxer_header*lh){
	return lh->stream_out[lh->lxer_tracker].byte_pointer;
}

bool lxer_is_math(LXR_TOKENS token){
	if(token < TAG_MATH_END && token >= 0) return true;
	return false;
}
bool lxer_is_comment(LXR_TOKENS token){
	if(token < TAG_COMMENT_END && token > TAG_MATH_END) return true;
	return false;
}

bool lxer_is_type(LXR_TOKENS token){
	if(token > TAG_COMMENT_END && token < TAG_TYPE_END) return true;
	return false;
}

bool lxer_is_sep(LXR_TOKENS token){
	if(token > TAG_TYPE_END && token < TAG_SEP_END) return true;
	return false;
}
bool lxer_is_brk(LXR_TOKENS token){
	if(token > TAG_SEP_END && token < TAG_BRK_END) return true;
	return false;
}

bool lxer_is_statement(LXR_TOKENS token){
	if(token > TAG_BRK_END && token < TAG_STATEMENT_END) return true;
	return false;
}

bool lxer_is_misc(LXR_TOKENS token){
	if(token < TAG_MISC_END && token > TAG_STATEMENT_END) return true;
	return false;
}

bool lxer_is_pp(LXR_TOKENS token){
	if(token < TOKEN_TABLE_END && token > TAG_MISC_END) return true;
	return false;
}

bool lxer_math_expect_math(lxer_header*lh){
	if( 
			lxer_is_math(lh->stream_out[lh->lxer_tracker].token) && 
			lxer_is_math(lh->stream_out[lh->lxer_tracker+1].token)
	  ){
		return true;
	}
	return false; 
}

bool lxer_math_expect_comment(lxer_header*lh){ 
	if( 
			lxer_is_math(lh->stream_out[lh->lxer_tracker].token) && 
			lxer_is_comment(lh->stream_out[lh->lxer_tracker+1].token)
	  ){
		return true;
	}
	return false;
}

bool lxer_math_expect_type(lxer_header*lh){
	if( 
			lxer_is_math(lh->stream_out[lh->lxer_tracker].token) && 
			lxer_is_type(lh->stream_out[lh->lxer_tracker+1].token)
	  ){
		return true;
	}
	return false; 
}


bool lxer_math_expect_sep(lxer_header*lh){ 
	if( 
			lxer_is_math(lh->stream_out[lh->lxer_tracker].token) && 
			lxer_is_sep(lh->stream_out[lh->lxer_tracker+1].token)
	  ){
		return true;
	}
	return false; 
}

bool lxer_math_expect_brk(lxer_header*lh){
	if( 
			lxer_is_math(lh->stream_out[lh->lxer_tracker].token) && 
			lxer_is_brk(lh->stream_out[lh->lxer_tracker+1].token)
	  ){
		return true;
	}
	return false; 
}

bool lxer_math_expect_statement(lxer_header*lh){ 
	if( 
			lxer_is_math(lh->stream_out[lh->lxer_tracker].token) && 
			lxer_is_statement(lh->stream_out[lh->lxer_tracker+1].token)
	  ){
		return true;
	}
	return false; 
}

bool lxer_math_expect_misc(lxer_header*lh){
	if( 
			lxer_is_math(lh->stream_out[lh->lxer_tracker].token) && 
			lxer_is_misc(lh->stream_out[lh->lxer_tracker+1].token)
	  ){
		return true;
	}
	return false; 
}

bool lxer_comment_expect_math(lxer_header*lh){
	if( 
			lxer_is_comment(lh->stream_out[lh->lxer_tracker].token) && 
			lxer_is_math(lh->stream_out[lh->lxer_tracker+1].token)
	  ){
		return true;
	}
	return false; 
}

bool lxer_comment_expect_comment(lxer_header*lh){
	if( 
			lxer_is_comment(lh->stream_out[lh->lxer_tracker].token) && 
			lxer_is_comment(lh->stream_out[lh->lxer_tracker+1].token)
	  ){
		return true;
	}
	return false; 
}

bool lxer_comment_expect_type(lxer_header*lh){
	if( 
			lxer_is_comment(lh->stream_out[lh->lxer_tracker].token) && 
			lxer_is_type(lh->stream_out[lh->lxer_tracker+1].token)
	  ){
		return true;
	}
	return false; 
}

bool lxer_comment_expect_sep(lxer_header*lh){
	if( 
			lxer_is_comment(lh->stream_out[lh->lxer_tracker].token) && 
			lxer_is_sep(lh->stream_out[lh->lxer_tracker+1].token)
	  ){
		return true;
	} return false; 
}

bool lxer_comment_expect_brk(lxer_header*lh){
	if( 
			lxer_is_comment(lh->stream_out[lh->lxer_tracker].token) && 
			lxer_is_brk(lh->stream_out[lh->lxer_tracker+1].token)
	  ){
		return true;
	} return false; 
}

bool lxer_comment_expect_statement(lxer_header*lh){
	if( 
			lxer_is_comment(lh->stream_out[lh->lxer_tracker].token) && 
			lxer_is_statement(lh->stream_out[lh->lxer_tracker+1].token)
	  ){
		return true;
	}
	return false; 
}

bool lxer_comment_expect_misc(lxer_header*lh){
	if( 
			lxer_is_comment(lh->stream_out[lh->lxer_tracker].token) && 
			lxer_is_misc(lh->stream_out[lh->lxer_tracker+1].token)
	  ){
		return true;
	}
	return false; 
}

bool lxer_type_expect_math(lxer_header*lh){
	if( 
			lxer_is_type(lh->stream_out[lh->lxer_tracker].token) && 
			lxer_is_math(lh->stream_out[lh->lxer_tracker+1].token)
	  ){
		return true;
	}
	return false; 
}

bool lxer_type_expect_comment(lxer_header*lh){
	if( 
			lxer_is_type(lh->stream_out[lh->lxer_tracker].token) && 
			lxer_is_comment(lh->stream_out[lh->lxer_tracker+1].token)
	  ){
		return true;
	}
	return false; 
}

bool lxer_type_expect_type(lxer_header*lh){
	if( 
			lxer_is_type(lh->stream_out[lh->lxer_tracker].token) && 
			lxer_is_type(lh->stream_out[lh->lxer_tracker+1].token)
	  ){
		return true;
	}
	return false; 
}

bool lxer_type_expect_sep(lxer_header*lh){
	if( 
			lxer_is_type(lh->stream_out[lh->lxer_tracker].token) && 
			lxer_is_sep(lh->stream_out[lh->lxer_tracker+1].token)
	  ){
		return true;
	}
	return false; 
}

bool lxer_type_expect_brk(lxer_header*lh){
	if( 
			lxer_is_type(lh->stream_out[lh->lxer_tracker].token) && 
			lxer_is_brk(lh->stream_out[lh->lxer_tracker+1].token)
	  ){
		return true;
	}return false; 
}

bool lxer_type_expect_statement(lxer_header*lh){
	if( 
			lxer_is_type(lh->stream_out[lh->lxer_tracker].token) && 
			lxer_is_statement(lh->stream_out[lh->lxer_tracker+1].token)
	  ){
		return true;
	}return false; 
}

bool lxer_type_expect_misc(lxer_header*lh){
	if( 
			lxer_is_type(lh->stream_out[lh->lxer_tracker].token) && 
			lxer_is_misc(lh->stream_out[lh->lxer_tracker+1].token)
	  ){
		return true;
	}
	return false; 
}

bool lxer_sep_expect_math(lxer_header*lh){ 
	if( 
			lxer_is_sep(lh->stream_out[lh->lxer_tracker].token) && 
			lxer_is_math(lh->stream_out[lh->lxer_tracker+1].token)
	  ){
		return true;
	}
	return false; 
}


bool lxer_sep_expect_comment(lxer_header*lh){
	if( 
			lxer_is_sep(lh->stream_out[lh->lxer_tracker].token) && 
			lxer_is_comment(lh->stream_out[lh->lxer_tracker+1].token)
	  ){
		return true;
	}
	return false; 
}

bool lxer_sep_expect_type(lxer_header*lh){ 
	if( 
			lxer_is_sep(lh->stream_out[lh->lxer_tracker].token) && 
			lxer_is_type(lh->stream_out[lh->lxer_tracker+1].token)
	  ){
		return true;
	}
	return false; 
}
bool lxer_sep_expect_sep(lxer_header*lh){ 
	if( 
			lxer_is_sep(lh->stream_out[lh->lxer_tracker].token) && 
			lxer_is_sep(lh->stream_out[lh->lxer_tracker+1].token)
	  ){
		return true;
	} 
	return false; 
}


bool lxer_sep_expect_brk(lxer_header*lh){ 
	if( 
			lxer_is_sep(lh->stream_out[lh->lxer_tracker].token) && 
			lxer_is_brk(lh->stream_out[lh->lxer_tracker+1].token)
	  ){
		return true;
	}
	return false; 
}


bool lxer_sep_expect_statement(lxer_header*lh){
	if( 
			lxer_is_sep(lh->stream_out[lh->lxer_tracker].token) && 
			lxer_is_statement(lh->stream_out[lh->lxer_tracker+1].token)
	  ){
		return true;
	}
	return false; 
}

bool lxer_sep_expect_misc(lxer_header*lh){ 
	if( 
			lxer_is_sep(lh->stream_out[lh->lxer_tracker].token) && 
			lxer_is_misc(lh->stream_out[lh->lxer_tracker+1].token)
	  ){
		return true;
	}
	return false;
}



bool lxer_brk_expect_math(lxer_header*lh){
	if( 
			lxer_is_brk(lh->stream_out[lh->lxer_tracker].token) && 
			lxer_is_math(lh->stream_out[lh->lxer_tracker+1].token)
	  ){
		return true;
	}
	return false; 
}


bool lxer_brk_expect_comment(lxer_header*lh){ 
	if( 
			lxer_is_brk(lh->stream_out[lh->lxer_tracker].token) && 
			lxer_is_comment(lh->stream_out[lh->lxer_tracker+1].token)
	  ){
		return true;
	}
	return false; 
}


bool lxer_brk_expect_type(lxer_header*lh){
	if( 
			lxer_is_brk(lh->stream_out[lh->lxer_tracker].token) && 
			lxer_is_type(lh->stream_out[lh->lxer_tracker+1].token)
	  ){
		return true;
	}
	return false;
}


bool lxer_brk_expect_sep(lxer_header*lh){
	if( 
			lxer_is_brk(lh->stream_out[lh->lxer_tracker].token) && 
			lxer_is_sep(lh->stream_out[lh->lxer_tracker+1].token)
	  ){
		return true;
	}
	return false; 
}

bool lxer_brk_expect_brk(lxer_header*lh){ 
	if( 
			lxer_is_brk(lh->stream_out[lh->lxer_tracker].token) && 
			lxer_is_brk(lh->stream_out[lh->lxer_tracker+1].token)
	  ){
		return true;
	}
	return false; 
}

bool lxer_brk_expect_statement(lxer_header*lh){
	if( 
			lxer_is_brk(lh->stream_out[lh->lxer_tracker].token) && 
			lxer_is_statement(lh->stream_out[lh->lxer_tracker+1].token)
	  ){
		return true;
	}
	return false; 
}

bool lxer_brk_expect_misc(lxer_header*lh){ 
	if( 
			lxer_is_brk(lh->stream_out[lh->lxer_tracker].token) && 
			lxer_is_misc(lh->stream_out[lh->lxer_tracker+1].token)
	  ){
		return true;
	}
	return false; 
}



bool lxer_statement_expect_math(lxer_header*lh){
	if( 
			lxer_is_statement(lh->stream_out[lh->lxer_tracker].token) && 
			lxer_is_math(lh->stream_out[lh->lxer_tracker+1].token)
	  ){
		return true;
	}
	return false; 
}

bool lxer_statement_expect_comment(lxer_header*lh){
	if( 
			lxer_is_statement(lh->stream_out[lh->lxer_tracker].token) && 
			lxer_is_comment(lh->stream_out[lh->lxer_tracker+1].token)
	  ){
		return true;
	}
	return false;
}

bool lxer_statement_expect_type(lxer_header*lh){
	if( 
			lxer_is_statement(lh->stream_out[lh->lxer_tracker].token) && 
			lxer_is_type(lh->stream_out[lh->lxer_tracker+1].token)
	  ){
		return true;
	}
	return false; 
}
bool lxer_statement_expect_sep(lxer_header*lh){
	if( 
			lxer_is_statement(lh->stream_out[lh->lxer_tracker].token) && 
			lxer_is_sep(lh->stream_out[lh->lxer_tracker+1].token)
	  ){
		return true;
	}
	return false; 
}
bool lxer_statement_expect_brk(lxer_header*lh){
	if( 
			lxer_is_statement(lh->stream_out[lh->lxer_tracker].token) && 
			lxer_is_brk(lh->stream_out[lh->lxer_tracker+1].token)
	  ){
		return true;
	}
	return false; 
}
bool lxer_statement_expect_statement(lxer_header*lh){
	if( 
			lxer_is_statement(lh->stream_out[lh->lxer_tracker].token) && 
			lxer_is_statement(lh->stream_out[lh->lxer_tracker+1].token)
	  ){
		return true;
	}
	return false; 
}


bool lxer_statement_expect_misc(lxer_header*lh){ 
	if( 
			lxer_is_statement(lh->stream_out[lh->lxer_tracker].token) && 
			lxer_is_misc(lh->stream_out[lh->lxer_tracker+1].token)
	  ){
		return true;
	}
	return false; 
}



bool lxer_misc_expect_math(lxer_header*lh){ 
	if( 
			lxer_is_misc(lh->stream_out[lh->lxer_tracker].token) && 
			lxer_is_math(lh->stream_out[lh->lxer_tracker+1].token)
	  ){
		return true;
	} 
	return false; 
}

bool lxer_misc_expect_comment(lxer_header*lh){
	if( 
			lxer_is_misc(lh->stream_out[lh->lxer_tracker].token) && 
			lxer_is_comment(lh->stream_out[lh->lxer_tracker+1].token)
	  ){
		return true;
	}  
	return false; 
}

bool lxer_misc_expect_type(lxer_header*lh){ 
	if( 
			lxer_is_misc(lh->stream_out[lh->lxer_tracker].token) && 
			lxer_is_type(lh->stream_out[lh->lxer_tracker+1].token)
	  ){
		return true;
	} 
	return false; 
}

bool lxer_misc_expect_sep(lxer_header*lh){
	if( 
			lxer_is_misc(lh->stream_out[lh->lxer_tracker].token) && 
			lxer_is_sep(lh->stream_out[lh->lxer_tracker+1].token)
	  ){
		return true;
	} 
	return false; 
}

bool lxer_misc_expect_brk(lxer_header*lh){ 
	if( 
			lxer_is_misc(lh->stream_out[lh->lxer_tracker].token) && 
			lxer_is_brk(lh->stream_out[lh->lxer_tracker+1].token)
	  ){
		return true;
	}  
	return false; 
}

bool lxer_misc_expect_statement(lxer_header*lh){ 
	if( 
			lxer_is_misc(lh->stream_out[lh->lxer_tracker].token) && 
			lxer_is_statement(lh->stream_out[lh->lxer_tracker+1].token)
	  ){
		return true;
	} 
	return false; 
}

bool lxer_misc_expect_misc(lxer_header*lh){ 
	if( 
			lxer_is_misc(lh->stream_out[lh->lxer_tracker].token) && 
			lxer_is_misc(lh->stream_out[lh->lxer_tracker+1].token)
	  ){
		return true;
	} 
	return false; 
}



void lxer_get_compounds(){
	size_t i=0;
	while(compound_exp[i]->len != 0){
		printf("Compound in line %zu: \n", i);
		printf("Sequence: ");
		for(size_t j=0;j<compound_exp[i]->len; j++){
			printf("[%u] = '%s', ", compound_exp[i]->token_chain[j],token_table_lh[compound_exp[i]->token_chain[j]]);
		}
		printf(" | ");
		printf("final expression: '");
		for(size_t j=0;j<compound_exp[i]->len; j++){
			printf("%s", token_table_lh[compound_exp[i]->token_chain[j]]);
		}
		printf("'\n");
		i+=1;	
	}
}


CINDEX lxer_expect_compound(lxer_header *lh){
	size_t i = 0;
	size_t lxer_tracker = lh->lxer_tracker;
	bool invalid = false;
	CINDEX c = CINDEX_NOT_FOUND;
	int c_len = 0;
	while(compound_exp[i]->len != 0){
		for(size_t j=0;j<compound_exp[i]->len && !invalid; j++){
			if(lxer_get_current_token(lh) != compound_exp[i]->token_chain[j]){
				invalid = true;
			}else{
				lxer_next_token(lh);
			}
		}
		if(!invalid){
			if(compound_exp[i]->len > c_len){
				c = (CINDEX)i;
				c_len = compound_exp[i]->len;
			}
		}
		i+=1;
		lh->lxer_tracker = lxer_tracker; 
		invalid = false;
	}
	lh->lxer_tracker = lxer_tracker;
	return c;
}

int lxer_get_compound_length(CINDEX c){
	if(c < CINDEX_END){
		return compound_exp[c]->len;
	}
	return -1;
}



char* lxer_get_compound_lh(CINDEX c){
	size_t len;
	if(c < CINDEX_END){
		for(size_t i=0;i<compound_exp[c]->len; i++){
			len += strlen(token_table_lh[compound_exp[c]->token_chain[i]]);
		}
		char* buff = (char*)temp_alloc(sizeof(char)*len);
		buff[0] = '\0';
		for(size_t i=0;i<compound_exp[c]->len; i++){
			strcat(buff, token_table_lh[compound_exp[c]->token_chain[i]]);
		}
		return buff;
	}
	return NULL;
}

char* lxer_get_rh(lxer_header* lh, bool reverse, bool strict){

	char* buffer = NULL;
	size_t tracker = lh->lxer_tracker;

	if(tracker+1 >= lh->stream_out_len){
		return buffer;
	}
	if(reverse){
		if(tracker > 0){
			tracker-=1;
		}
		else{
			return buffer;
		}
	}
	if(lh->stream_out[tracker].token != LXR_WORD && lh->stream_out[tracker+1].token != LXR_WORD){
		buffer = (char*)arena_alloc(&lh->lxer_ah, sizeof(char)*256);

		char*pointer = lh->stream_out[tracker].byte_pointer + strlen(token_table_lh[lh->stream_out[tracker].token]);
		char*end_ptr = NULL;
		if(tracker < lh->stream_out_len){
			end_ptr = lh->stream_out[tracker+1].byte_pointer;
		}else{
			end_ptr = &lh->source[lh->source_len-1];
		}
		while(*pointer < '0') pointer+=1;
		size_t word_len = 0;
		word_len = end_ptr-pointer;
		memcpy(&buffer[0],pointer, word_len);
		buffer[word_len] = '\0';
		char* space = strchr(buffer, ' ');
		if(space != NULL && strict){
			buffer[space - buffer] = '\0';
		}
	}
	return buffer;
}
char** lxer_get_rh_lh(lxer_header*lh){
	char** buffer_array = (char**)arena_alloc(&lh->lxer_ah, sizeof(char*)*2);
	buffer_array[0] = lxer_get_rh(lh, false, false); 
	buffer_array[1] = lxer_get_rh(lh, true, false);
	return buffer_array;
}

char*   lxer_get_string(lxer_header*lh, LXR_TOKENS target){
	char* buffer = NULL;
	size_t tracker = lh->lxer_tracker;
	
	if(tracker >= lh->stream_out_len) return NULL;
	char* current_ptr = lh->stream_out[tracker].byte_pointer;
	char* end_ptr = NULL;
	bool found = false;
	size_t size = 0;

	current_ptr += strlen(token_table_lh[lh->stream_out[tracker].token]);
	tracker += 1;
	while(tracker < lh->stream_out_len && !found){
		if(lh->stream_out[tracker].token == target){
			found = true;
		}else{
			tracker += 1;
		}
	}
	if(!found || tracker >= lh->stream_out_len) return NULL;
	end_ptr = lh->stream_out[tracker].byte_pointer;
	size = end_ptr - current_ptr;
	buffer = (char*)arena_alloc(&lh->lxer_ah, sizeof(char)*size);
	memcpy(buffer,current_ptr, size);
	buffer[size] = '\0';
	lh->lxer_tracker = tracker;
	return buffer;
}
