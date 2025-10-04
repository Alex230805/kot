#ifndef ILXER_H
#define ILXER_H 


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <misc.h>


#define TAG_MATH()\
	X(LXR_SUM_SYMB)\
	X(LXR_SUB_SYMB)\
	X(LXR_MLT_SYMB)\
	X(LXR_DIV_SYMB)\
	X(LXR_GRT_SYBM)\
	X(LXR_LST_SYBM)\
	X(LXR_EQL_SYBM)

#define TAG_COMMENT()\
	X(LXR_LINE_COMMENT)\
	X(LXR_OPEN_COMMENT)\
	X(LXR_CLOSE_COMMENT)


#define TAG_TYPE()\
	X(LXR_STRING_TYPE)\
	X(LXR_INT_TYPE)\
	X(LXR_DOUBLE_TYPE)\
	X(LXR_FLOAT_TYPE)\
	X(LXR_CHAR_TYPE)\
	X(LXR_POINTER_TYPE)\
	X(LXR_VOID_TYPE)


#define TAG_SEP()\
	X(LXR_COMMA)\
	X(LXR_SEMICOLON)\
	X(LXR_DOUBLE_DOTS)\
	X(LXR_DOT)\
	X(LXR_QUOTE)\
	X(LXR_DOUBLE_QUOTE)\


#define TAG_BRK()\
	X(LXR_OPEN_BRK)\
	X(LXR_CLOSE_BRK)\
	X(LXR_OPEN_CRL_BRK)\
	X(LXR_CLOSE_CRL_BRK)\
	X(LXR_OPEN_SQR_BRK)\
	X(LXR_CLOSE_SQR_BRK)

#define TAG_STATEMENT()\
	X(LXR_IF_STATEMENT)\
	X(LXR_WHILE_STATEMENT)\
	X(LXR_RET_STATEMENT)\
	X(LXR_ASSIGNMENT)\

#define TAG_MISC()\
	X(LXR_CONST_DECLARATION)\
	X(LXR_VAR_DECLARATION)\
	X(LXR_FN_DECLARATION)\
	X(INVALID_POINTER)
// Token organization table: this is used to generate the token enumerator and the token array. This architecture define the structure and the sepatration between tokens of different kinds

#define TOKEN_DISPOSE()\
	TAG_MATH()\
	TAG_MATH_END,\
	TAG_COMMENT()\
	TAG_COMMENT_END,\
	TAG_TYPE()\
	TAG_TYPE_END,\
	TAG_SEP()\
	TAG_SEP_END,\
	TAG_BRK()\
	TAG_BRK_END,\
	TAG_STATEMENT()\
	TAG_STATEMENT_END,\
	TAG_MISC()\
	TOKEN_TABLE_END,\
	NOT_A_TOKEN

// lexer tokenizer
#define X(name) name,

typedef enum {
	TOKEN_DISPOSE()
}LXR_TOKENS;

#undef X

#define X(name) name,

static const LXR_TOKENS token_array[] = {
	TOKEN_DISPOSE()
};

#undef X

// data structure that hold 
// the token associated with a specific 
// section of the code with a pointer and 
// the length of the element.

typedef struct{
	LXR_TOKENS token;
	char* byte_pointer;
}token_slice;

// lxer data structure

// To use the lxer you must define a const table with the 
// string associated with the desired token, at least for now.
// you can create the table and load it inside the lxer structure.


typedef struct{
	char*   source;
	size_t  source_len;
	size_t  lxer_tracker;

	token_slice* stream_out;
	size_t       stream_out_len;
	Arena_header lxer_ah;
}lxer_head;

// some predefined tokens, those are here only for 
// example, you can add or remove anything you want 
// as long you respect how the structure is defined 
// in the TOKEN_DISPOSE macro 

static char* token_table_lh[] = {

	// math token 
	[LXR_SUM_SYMB] = "+",
	[LXR_SUB_SYMB] = "-",
	[LXR_MLT_SYMB] = "*",
	[LXR_DIV_SYMB] = "/",
	[LXR_GRT_SYBM] = ">",
	[LXR_LST_SYBM] = "<",
	[LXR_EQL_SYBM] = "==",


	// comment 
	[LXR_LINE_COMMENT] = "",
	[LXR_OPEN_COMMENT] = "",
	[LXR_CLOSE_COMMENT] = "",


	// type 
	[LXR_STRING_TYPE] = "",
	[LXR_INT_TYPE] = "",
	[LXR_DOUBLE_TYPE] = "",
	[LXR_FLOAT_TYPE] = "",
	[LXR_CHAR_TYPE] = "",
	[LXR_POINTER_TYPE] = "",
	[LXR_VOID_TYPE] = "",


	// sep 
	[LXR_COMMA] = ",",
	[LXR_SEMICOLON] = ";",
	[LXR_DOUBLE_DOTS] = ":",
	[LXR_DOT] = ".",
	[LXR_QUOTE] = "'",
	[LXR_DOUBLE_QUOTE] = "\"",


	// brackets
	[LXR_OPEN_BRK] = "(",
	[LXR_CLOSE_BRK] = ")",
	[LXR_OPEN_CRL_BRK] = "{",
	[LXR_CLOSE_CRL_BRK] = "}",
	[LXR_OPEN_SQR_BRK] = "[",
	[LXR_CLOSE_SQR_BRK] = "]",


	// statement 
	[LXR_IF_STATEMENT] = "",
	[LXR_WHILE_STATEMENT] = "",
	[LXR_RET_STATEMENT] = "",
	[LXR_ASSIGNMENT] = "=",


	// misc
	[LXR_CONST_DECLARATION] = "const",
	[LXR_VAR_DECLARATION] = "var",
	[LXR_FN_DECLARATION] = "fn",
	[NOT_A_TOKEN] = "NOT_A_TOKEN",
	[INVALID_POINTER] = "INVALID_POINTER"
};

// length of the defined token table
static size_t token_table_length = TOKEN_TABLE_END;


// lexing functions
void lxer_start_lexing(lxer_head* lh, char* source_file);
void lxer_get_lxer_content(lxer_head*lh);

// increment the local tracker by one to index the following token
bool lxer_next_token(lxer_head*lh);

void lxer_set_new_target(lxer_head* lh, char* new_line);

// get token associated with the current pointer
LXR_TOKENS lxer_get_current_token(lxer_head*lh);
// get token associated with the following pointer
LXR_TOKENS lxer_get_next_token(lxer_head*lh);

// get current pointer (char*) of a token from the source code 
char* lxer_get_current_pointer(lxer_head*lh);

// lxer check functions: check if the token passed as parameter is math, comment, type, sep, brk, statement or misc
bool lxer_is_math(LXR_TOKENS token);
bool lxer_is_comment(LXR_TOKENS token);
bool lxer_is_type(LXR_TOKENS token);
bool lxer_is_sep(LXR_TOKENS token);
bool lxer_is_brk(LXR_TOKENS token);
bool lxer_is_statement(LXR_TOKENS token);
bool lxer_is_misc(LXR_TOKENS token);



// lxer expect functions, usefull to decide the parsing process based on the type of the following token
bool lxer_math_expect_math(lxer_head*lh);
bool lxer_math_expect_comment(lxer_head*lh);
bool lxer_math_expect_type(lxer_head*lh);
bool lxer_math_expect_sep(lxer_head*lh);
bool lxer_math_expect_brk(lxer_head*lh);
bool lxer_math_expect_statement(lxer_head*lh);
bool lxer_math_expect_misc(lxer_head*lh);

bool lxer_comment_expect_math(lxer_head*lh);
bool lxer_comment_expect_comment(lxer_head*lh);
bool lxer_comment_expect_type(lxer_head*lh);
bool lxer_comment_expect_sep(lxer_head*lh);
bool lxer_comment_expect_brk(lxer_head*lh);
bool lxer_comment_expect_statement(lxer_head*lh);
bool lxer_comment_expect_misc(lxer_head*lh);

bool lxer_type_expect_math(lxer_head*lh);
bool lxer_type_expect_comment(lxer_head*lh);
bool lxer_type_expect_type(lxer_head*lh);
bool lxer_type_expect_sep(lxer_head*lh);
bool lxer_type_expect_brk(lxer_head*lh);
bool lxer_type_expect_statement(lxer_head*lh);
bool lxer_type_expect_misc(lxer_head*lh);

bool lxer_sep_expect_math(lxer_head*lh);
bool lxer_sep_expect_comment(lxer_head*lh);
bool lxer_sep_expect_type(lxer_head*lh);
bool lxer_sep_expect_sep(lxer_head*lh);
bool lxer_sep_expect_brk(lxer_head*lh);
bool lxer_sep_expect_statement(lxer_head*lh);
bool lxer_sep_expect_misc(lxer_head*lh);


bool lxer_brk_expect_math(lxer_head*lh);
bool lxer_brk_expect_comment(lxer_head*lh);
bool lxer_brk_expect_type(lxer_head*lh);
bool lxer_brk_expect_sep(lxer_head*lh);
bool lxer_brk_expect_brk(lxer_head*lh);
bool lxer_brk_expect_statement(lxer_head*lh);
bool lxer_brk_expect_misc(lxer_head*lh);


bool lxer_statement_expect_math(lxer_head*lh);
bool lxer_statement_expect_comment(lxer_head*lh);
bool lxer_statement_expect_type(lxer_head*lh);
bool lxer_statement_expect_sep(lxer_head*lh);
bool lxer_statement_expect_brk(lxer_head*lh);
bool lxer_statement_expect_statement(lxer_head*lh);
bool lxer_statement_expect_misc(lxer_head*lh);

bool lxer_misc_expect_math(lxer_head*lh);
bool lxer_misc_expect_comment(lxer_head*lh);
bool lxer_misc_expect_type(lxer_head*lh);
bool lxer_misc_expect_sep(lxer_head*lh);
bool lxer_misc_expect_brk(lxer_head*lh);
bool lxer_misc_expect_statement(lxer_head*lh);
bool lxer_misc_expect_misc(lxer_head*lh);


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
char*   lxer_get_rh(lxer_head*lh, bool reverse);
char**  lxer_get_rh_lh(lxer_head*lh);


#ifndef ILXER_IMPLEMENTATION
#define ILXER_IMPLEMENTATION

#endif
#endif // lxer
