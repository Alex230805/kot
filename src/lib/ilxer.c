#define ILXER_IMPLEMENTATION

#include "ilxer.h"


void lxer_start_lexing(lxer_header* lh, char * source){
	if(DEBUG) DINFO("Start lexing, file size: %lu", strlen(source));
	lh->source = source;
	lh->source_len = strlen(source);
	size_t array_size = 12;
	size_t array_tracker = 0;
	token_slice *cache_mem = (token_slice*)arena_alloc(&lh->lxer_ah,sizeof(token_slice)*array_size);
	char * buffer = (char*)arena_alloc(&lh->lxer_ah,sizeof(char)*32);
	bool ignore_lex = true;
	
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

	for(size_t i=0;i<lh->source_len;i++){
		char* tracker = &lh->source[i];
		for(size_t j=0;j<TOKEN_TABLE_END; j++){
			LXR_TOKENS token = token_array[j];
			switch(token){
				case TAG_MATH_END:
				case TAG_TYPE_END:
				case TAG_COMMENT_END:
				case TAG_SEP_END:
				case TAG_BRK_END:
				case TAG_STATEMENT_END:
				case TAG_MISC_END:
					ignore_lex = true;
					break;
				default:
					ignore_lex = false;
					break;
			}

			//////////////////////////////////////////
			if(!ignore_lex){
				size_t ws = strlen(token_table_lh[token]);
				bool isolated = false;
				buffer[0] = '\0';
				strcpy(buffer, tracker);
				buffer[ws] = '\0';

				if((tracker+ws < (lh->source+lh->source_len)) && lh->source[i+ws] == ' '){
					isolated = true;
				}

				if(strcmp(buffer,token_table_lh[token]) == 0 && strlen(buffer) > 0){
					switch(token){
						// token syntax variation: if a token require 
						// a space to be validated you can manually 
						// insert it in the fallthrough down here
						case LXR_CONST_DECLARATION:
						case LXR_VAR_DECLARATION:
						case LXR_STRING_TYPE:
						case LXR_INT_TYPE:
						case LXR_DOUBLE_TYPE:
						case LXR_FLOAT_TYPE:
						case LXR_CHAR_TYPE:
						case LXR_VOID_TYPE:
						case LXR_BOOL_TYPE:
							if(isolated){
								cache_mem[array_tracker].token = token;
								cache_mem[array_tracker].byte_pointer = tracker;
								array_tracker+=1;
							}
							break;
						default: 
							cache_mem[array_tracker].token = token;
							cache_mem[array_tracker].byte_pointer = tracker;
							array_tracker+=1;
							break;
					}

					if(array_tracker >= array_size){
						size_t old_size = array_size;
						array_size *= 2;
						token_slice* n_cache_mem = (token_slice*)arena_alloc(&lh->lxer_ah,sizeof(token_slice)*array_size);
						for(size_t z=0;z<old_size;z++){
							n_cache_mem[z] = cache_mem[z];
						}
						cache_mem = n_cache_mem;
					}
				}
			}
			////////////////////////////////////////
		}
	}
	lh->stream_out = cache_mem;
	lh->stream_out_len = array_tracker;
	return;
}


void lxer_get_lxer_content(lxer_header*lh){
	NOTY("ILXER", "Stream out length: %zu", lh->stream_out_len);
	NOTY("ILXER","Tokenzer output: ", NULL);
	for(size_t i=0;i<lh->stream_out_len;i++){
		LXR_TOKENS tok = lh->stream_out[i].token;
		char* pointer = lh->stream_out[i].byte_pointer;
		printf("\ttoken found at byte_stream[%ld]: token_tablep[%d] -> %s\n", pointer-lh->source, tok,token_table_lh[tok]);
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


char* lxer_get_rh(lxer_header* lh, bool reverse, bool strict){
	char* buffer = (char*)arena_alloc(&lh->lxer_ah, sizeof(char)*256);

	size_t tracker = lh->lxer_tracker;

	if(tracker+1 >= lh->stream_out_len){
		buffer = NULL;	
		return buffer;
	}
	if(reverse){
		if(tracker > 0){
			tracker-=1;
		}
		else{
			buffer = NULL;	
			return buffer;
		}
	}

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
