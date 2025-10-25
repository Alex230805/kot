#ifndef ILXER_H
#define ILXER_H 


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <misc.h>



// ilxer's main macros to allow easy modification

#define MAX_COMPOUND 32

#define TOKEN_TABLE_LH_SIGN\
	static char* token_table_lh[] = 

#define DECLARE_TOKEN_STRING(token, string)\
	[token] = string

#define DECLARE_TOKEN_TABLE_LH(...)\
	TOKEN_TABLE_LH_SIGN {\
		__VA_ARGS__\
	};

#define TOKEN_COMPOUND_SIGN\
	static ilxer_compound* compound_exp[] = 

#define COMPOUND_DEFINITION\
	struct ilxer_compound{\
		size_t len;\
		LXR_TOKENS token_chain[MAX_COMPOUND];\
	};

#define DECLARE_COMPOUND(index, length, ...)\
	[index] = &((ilxer_compound){			\
			.len = length,					\
			{								\
				__VA_ARGS__					\
			}								\
		}									\
	)

#define DECLARE_COMPOUND_TABLE(...)\
	COMPOUND_DEFINITION\
	TOKEN_COMPOUND_SIGN {\
		__VA_ARGS__,\
		/* terminator for compound expression array, this is required to estimate the size of the compound expressions array*/\
		DECLARE_COMPOUND(CINDEX_END, 0, TOKEN_TABLE_END)\
	};


#define TOKEN_TABLE_SIGN\
	static const LXR_TOKENS token_array[] = 

#define DEFINE_TOKEN_TABLE(...)\
	TOKEN_TABLE_SIGN {\
		__VA_ARGS__,\
		TOKEN_TABLE_END,\
		NOT_A_TOKEN\
	};


#define DEFINE_TOKEN_TAG(...)\
	typedef enum {\
		__VA_ARGS__,\
		TOKEN_TABLE_END,\
		NOT_A_TOKEN\
	}LXR_TOKENS;

#define DEFINE_TOKEN(...)\
	DEFINE_TOKEN_TAG(__VA_ARGS__)\
	DEFINE_TOKEN_TABLE(__VA_ARGS__)


#define DECLARE_COMPOUND_TOKEN(...)\
	typedef enum{\
		__VA_ARGS__,\
		CINDEX_END,\
		CINDEX_NOT_FOUND\
	}CINDEX;


#define DECLARE_MATH(...)\
	__VA_ARGS__,\
	TAG_MATH_END


#define DECLARE_COMMENT(...)\
	__VA_ARGS__,\
	TAG_COMMENT_END\

#define DECLARE_TYPE(...)\
	__VA_ARGS__,\
	TAG_TYPE_END

#define DECLARE_SEP(...)\
	__VA_ARGS__,\
	TAG_SEP_END


#define DECLARE_BRK(...)\
	__VA_ARGS__,\
	TAG_BRK_END


#define DECLARE_STATEMENT(...)\
	__VA_ARGS__,\
	TAG_STATEMENT_END


#define DECLARE_MISC(...)\
	__VA_ARGS__,\
	TAG_MISC_END

#define DECLARE_PP(...)\
	__VA_ARGS__,\
	TAG_PP_END

#define TOKEN_SEPARATOR()\
	X(TAG_MATH_END)\
	X(TAG_TYPE_END)\
	X(TAG_COMMENT_END)\
	X(TAG_SEP_END)\
	X(TAG_BRK_END)\
	X(TAG_STATEMENT_END)\
	X(TAG_MISC_END)\
	X(TAG_PP_END)


// forward declaration of the main struct

typedef struct token_slice token_slice;
typedef struct lxer_header lxer_header;
typedef struct ilxer_compound ilxer_compound;


// default configuration for lxer 

#ifndef STRIP_DEFAULT_IMPLEMENTATION

#include "ilxer_default_config.h"

#endif

// token slice and lxer_header are used inside the parsing section 
// to form the outpu stream of tokens, were for each token the char*
// pointer from the source code is tinght to the matching token. 
//
// lxer_header keep track of the current pointed token, store the source 
// code and the output stream, it include also a reserved Arena_header 
// to allocate memory as needed by using the arena allocator shipped with 
// the misc.h library ( namely cbox )

struct token_slice{
	LXR_TOKENS token;
	char* byte_pointer;
};

struct lxer_header{
	char*   source;
	size_t  source_len;
	size_t  lxer_tracker;

	struct token_slice* stream_out;
	size_t       stream_out_len;
	Arena_header lxer_ah;
};


// length of the defined token table
static size_t token_table_length = TOKEN_TABLE_END;

// lexing functions
void lxer_start_lexing(lxer_header* lh, char* source_file);
void lxer_get_lxer_content(lxer_header*lh);

// increment the local tracker by one to index the following token
bool lxer_next_token(lxer_header*lh);

void lxer_set_new_target(lxer_header* lh, char* new_line);
void lxer_set_new_tracker(lxer_header*lh,int tracker);
void lxer_increase_tracker(lxer_header*lh,int tracker);

// get token associated with the current pointer
LXR_TOKENS lxer_get_current_token(lxer_header*lh);
// get token associated with the following pointer
LXR_TOKENS lxer_get_next_token(lxer_header*lh);

// get current pointer (char*) of a token from the source code 
char* lxer_get_current_pointer(lxer_header*lh);
char* lxer_get_string_representation(LXR_TOKENS tok);


// lxer check functions: check if the token passed as parameter is math, comment, type, sep, brk, statement or misc
bool lxer_is_math(LXR_TOKENS token);
bool lxer_is_comment(LXR_TOKENS token);
bool lxer_is_type(LXR_TOKENS token);
bool lxer_is_sep(LXR_TOKENS token);
bool lxer_is_brk(LXR_TOKENS token);
bool lxer_is_statement(LXR_TOKENS token);
bool lxer_is_misc(LXR_TOKENS token);



// lxer expect functions, usefull to decide the parsing process based on the type of the following token
bool lxer_math_expect_math(lxer_header*lh);
bool lxer_math_expect_comment(lxer_header*lh);
bool lxer_math_expect_type(lxer_header*lh);
bool lxer_math_expect_sep(lxer_header*lh);
bool lxer_math_expect_brk(lxer_header*lh);
bool lxer_math_expect_statement(lxer_header*lh);
bool lxer_math_expect_misc(lxer_header*lh);

bool lxer_comment_expect_math(lxer_header*lh);
bool lxer_comment_expect_comment(lxer_header*lh);
bool lxer_comment_expect_type(lxer_header*lh);
bool lxer_comment_expect_sep(lxer_header*lh);
bool lxer_comment_expect_brk(lxer_header*lh);
bool lxer_comment_expect_statement(lxer_header*lh);
bool lxer_comment_expect_misc(lxer_header*lh);

bool lxer_type_expect_math(lxer_header*lh);
bool lxer_type_expect_comment(lxer_header*lh);
bool lxer_type_expect_type(lxer_header*lh);
bool lxer_type_expect_sep(lxer_header*lh);
bool lxer_type_expect_brk(lxer_header*lh);
bool lxer_type_expect_statement(lxer_header*lh);
bool lxer_type_expect_misc(lxer_header*lh);

bool lxer_sep_expect_math(lxer_header*lh);
bool lxer_sep_expect_comment(lxer_header*lh);
bool lxer_sep_expect_type(lxer_header*lh);
bool lxer_sep_expect_sep(lxer_header*lh);
bool lxer_sep_expect_brk(lxer_header*lh);
bool lxer_sep_expect_statement(lxer_header*lh);
bool lxer_sep_expect_misc(lxer_header*lh);


bool lxer_brk_expect_math(lxer_header*lh);
bool lxer_brk_expect_comment(lxer_header*lh);
bool lxer_brk_expect_type(lxer_header*lh);
bool lxer_brk_expect_sep(lxer_header*lh);
bool lxer_brk_expect_brk(lxer_header*lh);
bool lxer_brk_expect_statement(lxer_header*lh);
bool lxer_brk_expect_misc(lxer_header*lh);


bool lxer_statement_expect_math(lxer_header*lh);
bool lxer_statement_expect_comment(lxer_header*lh);
bool lxer_statement_expect_type(lxer_header*lh);
bool lxer_statement_expect_sep(lxer_header*lh);
bool lxer_statement_expect_brk(lxer_header*lh);
bool lxer_statement_expect_statement(lxer_header*lh);
bool lxer_statement_expect_misc(lxer_header*lh);

bool lxer_misc_expect_math(lxer_header*lh);
bool lxer_misc_expect_comment(lxer_header*lh);
bool lxer_misc_expect_type(lxer_header*lh);
bool lxer_misc_expect_sep(lxer_header*lh);
bool lxer_misc_expect_brk(lxer_header*lh);
bool lxer_misc_expect_statement(lxer_header*lh);
bool lxer_misc_expect_misc(lxer_header*lh);

// compound related functions 

void lxer_get_compounds();
CINDEX lxer_expect_compound(lxer_header* lh);
char* lxer_get_compound_lh(CINDEX c);
int lxer_get_compound_length(CINDEX c);


// lxer get right hand, left hand or both functions, those are usefull to grep ONLY the 
// text string inside the source file that follow the current token or it's behind it.
// This work only if the previous/next word is not a token:
//
//	get left hand:
//					\
//					|
//					+
//	.... [token-i] word [token] [token+i] .....
//
//	If there is no word before the current token, then the function will return a NULL
//	pointer ( same behaviour apply to get both and get reverse )
//
//
char*   lxer_get_rh(lxer_header*lh, bool reverse, bool strict);
char**  lxer_get_rh_lh(lxer_header*lh);
char*   lxer_get_string(lxer_header*lh, LXR_TOKENS target);

#ifndef ILXER_IMPLEMENTATION
#define ILXER_IMPLEMENTATION

#endif
#endif // lxer
