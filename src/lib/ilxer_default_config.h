
	
/*
 *
 *	DEFINE_TOKEN(...) Allow the definition of the tokenizer's main token to use during the lexing 
 *	process. Each macro inside it like DECLARE_MATH_TOKEN(...) introduces after the provided arguments
 *	a special separator that will be used during the construction of the token table to define boundaries
 *	that will be used to identify the type of each token. This will create effectively a token list 
 *	that will look like this: 
 *
 *	typedef enum{
 *		LXR_TOKEN_00,
 *		LXR_TOKEN_01,
 *		LXR_TOKEN_02,
 *		LXR_TOKEN_0_END,
 *		LXR_TOKEN_10,
 *		LXR_TOKEN_11,
 *		LXR_TOKEN_12,
 *		LXR_TOKEN_1_END,
 *		....
 *		LXR_TOKEN_TABLE_END
 *	}TOKENS
 *
 * The confrontation with the current pointed token is basically a condition check to find out if the 
 * token is between a predefined range:
 *
 *		if(token > LXR_TOKEN_0_END && LXR_TOKEN_1_END){ ... } // this will find out if the token is contained in the second group
 *
 * This means that all the functions similar to "lxer_is_token(...)" depends pretty much on how this table is constructed and where
 * this separator are placed, for such the correct order of declaration must be:
 *
 *		DECLARE_MATH(...)
 *		DECLARE_COMMENT(...)
 *		DECLARE_TYPE(...)
 *		DECLARE_SEP(...)
 *		DECLARE_BRK(...)
 *		DECLARE_STATEMENT(...)
 *		DECLARE_MISC(...)
 *		DECLARE_PP(...)
 *
 * Note on those macros: the separator is introduced for each class by the DECLARE macro up there, this means that if you don't want 
 * to define a comment token for example you still need to include the DECLARE_COMMENT() macro, just leave it empty.
 *
 */	
	
DEFINE_TOKEN(
	DECLARE_MATH(LXR_SUM_SYMB, LXR_SUB_SYMB, LXR_MLT_SYMB, LXR_DIV_SYMB, LXR_GRT_SYBM, LXR_LST_SYBM),
	DECLARE_COMMENT(LXR_LINE_COMMENT, LXR_OPEN_COMMENT, LXR_CLOSE_COMMENT),
	DECLARE_TYPE(LXR_STRING_TYPE, LXR_INT_TYPE, LXR_DOUBLE_TYPE, LXR_FLOAT_TYPE, LXR_CHAR_TYPE, LXR_VOID_TYPE),
	DECLARE_SEP(LXR_COMMA, LXR_SEMICOLON, LXR_DOUBLE_DOTS, LXR_DOT, LXR_QUOTE, LXR_DOUBLE_QUOTE, LXR_QUESTION_MARK, LXR_BAR),
	DECLARE_BRK(LXR_OPEN_BRK, LXR_CLOSE_BRK, LXR_OPEN_CRL_BRK, LXR_CLOSE_CRL_BRK, LXR_OPEN_SQR_BRK, LXR_CLOSE_SQR_BRK),
	DECLARE_STATEMENT(LXR_IF_STATEMENT, LXR_WHILE_STATEMENT, LXR_SWITCH_STATEMENT, LXR_DO_STATEMENT, LXR_FOR_STATEMENT, LXR_RET_STATEMENT,LXR_ASSIGNMENT),
	DECLARE_MISC(LXR_CONST_DECLARATION,LXR_VAR_DECLARATION,LXR_ENUM_DECLARATION,LXR_STRUCT_DECLARATION,INVALID_POINTER),
	DECLARE_PP(LXR_DEFINE,LXR_UNDEF,LXR_MACRO_OPEN,LXR_MACRO_CLOSE)
)


/*
 * DECLARE_TOKEN_TABLE_LH(...) is needed to map the existing defined token to a string equivalent. 
 * This is used inside the main lexing function to matching the source code's content and assing 
 * a specific token based on what is analized. 
 * This is a pretty simple architecture that does not support a sort of "mode switching" functionality, 
 * this means that string literals for example are not tokenized with a single tag, this must be done 
 * on the level of the parser by using the proper defined functions of ilxer to extract the string literals 
 * from the source code. This decision was made to maintain the simplicity ilxer currently have.
 *
 * */




DECLARE_TOKEN_TABLE_LH(
	// logic 
	DECLARE_TOKEN_STRING(LXR_SUM_SYMB,"+"),
	DECLARE_TOKEN_STRING(LXR_SUB_SYMB,"-"),
	DECLARE_TOKEN_STRING(LXR_MLT_SYMB,"*"),
	DECLARE_TOKEN_STRING(LXR_DIV_SYMB,"/"),
	DECLARE_TOKEN_STRING(LXR_GRT_SYBM,">"),
	DECLARE_TOKEN_STRING(LXR_LST_SYBM,"<"),

	// comment 
	DECLARE_TOKEN_STRING(LXR_LINE_COMMENT,""),
	DECLARE_TOKEN_STRING(LXR_OPEN_COMMENT,""),
	DECLARE_TOKEN_STRING(LXR_CLOSE_COMMENT,""),

	// type 
	DECLARE_TOKEN_STRING(LXR_STRING_TYPE,"str"),
	DECLARE_TOKEN_STRING(LXR_INT_TYPE,"int"),
	DECLARE_TOKEN_STRING(LXR_DOUBLE_TYPE,"double"),
	DECLARE_TOKEN_STRING(LXR_FLOAT_TYPE,"float"),
	DECLARE_TOKEN_STRING(LXR_CHAR_TYPE,"char"),
	DECLARE_TOKEN_STRING(LXR_VOID_TYPE,"void"),


	// sep 
	DECLARE_TOKEN_STRING(LXR_COMMA,","),
	DECLARE_TOKEN_STRING(LXR_SEMICOLON,";"),
	DECLARE_TOKEN_STRING(LXR_DOUBLE_DOTS,":"),
	DECLARE_TOKEN_STRING(LXR_DOT,"."),
	DECLARE_TOKEN_STRING(LXR_QUOTE,"'"),
	DECLARE_TOKEN_STRING(LXR_DOUBLE_QUOTE,"\""),
	DECLARE_TOKEN_STRING(LXR_QUESTION_MARK,"?"),
	DECLARE_TOKEN_STRING(LXR_BAR,"|"),


	// brackets
	DECLARE_TOKEN_STRING(LXR_OPEN_BRK,"("),
	DECLARE_TOKEN_STRING(LXR_CLOSE_BRK,")"),
	DECLARE_TOKEN_STRING(LXR_OPEN_CRL_BRK,"{"),
	DECLARE_TOKEN_STRING(LXR_CLOSE_CRL_BRK,"}"),
	DECLARE_TOKEN_STRING(LXR_OPEN_SQR_BRK,"["),
	DECLARE_TOKEN_STRING(LXR_CLOSE_SQR_BRK,"]"),


	// statement 
	DECLARE_TOKEN_STRING(LXR_IF_STATEMENT,"if"),
	DECLARE_TOKEN_STRING(LXR_WHILE_STATEMENT,"while"),
	DECLARE_TOKEN_STRING(LXR_SWITCH_STATEMENT,"switch"),
	DECLARE_TOKEN_STRING(LXR_DO_STATEMENT,"do"),
	DECLARE_TOKEN_STRING(LXR_FOR_STATEMENT,"for"),
	DECLARE_TOKEN_STRING(LXR_RET_STATEMENT,"return"),
	DECLARE_TOKEN_STRING(LXR_ASSIGNMENT,"="),

	// misc
	DECLARE_TOKEN_STRING(LXR_CONST_DECLARATION,"const"),
	DECLARE_TOKEN_STRING(LXR_VAR_DECLARATION,"var"),
	DECLARE_TOKEN_STRING(LXR_ENUM_DECLARATION,"enum"),
	DECLARE_TOKEN_STRING(LXR_STRUCT_DECLARATION,"enum"),
	DECLARE_TOKEN_STRING(INVALID_POINTER,"NULL"),

	// pre processor
	DECLARE_TOKEN_STRING(LXR_DEFINE,"#define"),
	DECLARE_TOKEN_STRING(LXR_UNDEF,"#undef"),
	DECLARE_TOKEN_STRING(LXR_MACRO_OPEN,"#macro"),
	DECLARE_TOKEN_STRING(LXR_MACRO_CLOSE,"#endmacro")
)


/*
 * The compound tokens are group of tokens that need to be recognised easily during the parsing section. 
 * To remove repetitive group of tokens-matching the compound extension allow the creation of a compound 
 * token that is made of multiple already defined tokens by the standard lxer lexer to allow fast 
 * check for pattern matching parser. By using the the DECLARE_COMPOUND_TABLE(...) macro you can define 
 * a list of compound expression with the use of DECLARE_COMPOUND(...) macro which accept the 
 * new compound token ( used for identification ), the length of the compound expression and then the 
 * list of tokens.
 *
 * To declare a new compound token you can use DECLARE_COMPOUND_TOKEN(...) macro to create easily a list 
 * of tokens used only for the compound expression, those are not the same tokens used by ilxer for 
 * analizyng the code.
 *
 *
 *
 * */

DECLARE_COMPOUND_TOKEN(
	CINDEX_DOUBLE_EQ,
	CINDEX_GRT_EQ, 
	CINDEX_LST_EQ,
	CINDEX_LST,
	CINDEX_GRT,
	CINDEX_INLINE_EQ,
	CINDEX_INLINE_ELSE
)

DECLARE_COMPOUND_TABLE(
		DECLARE_COMPOUND(CINDEX_DOUBLE_EQ,	2, LXR_ASSIGNMENT, LXR_ASSIGNMENT),		// '==' symbol
		DECLARE_COMPOUND(CINDEX_GRT_EQ,		2, LXR_GRT_SYBM, LXR_ASSIGNMENT),		// '>=' symbol
		DECLARE_COMPOUND(CINDEX_LST_EQ,		2, LXR_LST_SYBM, LXR_ASSIGNMENT),		// '<=' symbol
		DECLARE_COMPOUND(CINDEX_LST,		1, LXR_LST_SYBM),						// '<' symbol
		DECLARE_COMPOUND(CINDEX_GRT,		1, LXR_GRT_SYBM),						// '>' symbol
		DECLARE_COMPOUND(CINDEX_INLINE_EQ,   2, LXR_SUB_SYMB, LXR_GRT_SYBM),		// '->' symbol 
		DECLARE_COMPOUND(CINDEX_INLINE_ELSE, 2, LXR_BAR, LXR_GRT_SYBM)				// '|>' symbol 
)

