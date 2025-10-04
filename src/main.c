#include <stdlib.h>
#include <stdio.h>

#include <string.h>
#include "ilxer.h"
#include "misc.h"

Arena_header ah = {0};
lxer_head lh = {0};
error_handler eh = {0}; 

#define DEFAULT_BUFFER_SIZE 256
#define DEFAULT_SRC_SIZE 4

//void error_push_error(error_handler *eh, char* error_string, int line_position, size_t error_code, char*source_ptr, int source_line_len);


void abort(){
	error_print_error(&eh,(print_set){false,true,false,false,true,false,false});
	arena_free(&eh.ah);
	arena_free(&ah);
	arena_free(&lh.lxer_ah);
	exit(1);
}

void kot_report(){
	error_print_error(&eh,(print_set){true,true,false,true,true,true,false});
}


void finish(){
	arena_free(&eh.ah);
	arena_free(&ah);
	arena_free(&lh.lxer_ah);
	exit(0);
}


void kot_helper(){
	fprintf(stdout, "KOT: C-like console parser:\n");
	fprintf(stdout, "\tusage: kot [param]\n");
	fprintf(stdout, "\t\t-h/--help: print help\n\n");
	fprintf(stdout, "\tusage: kot {file.kot}\n");
	fprintf(stdout, "\t\tresult: kot interpreter will execute the .kot file\n");
}


int kot_parse_expression();

int main(int argc, char** argv){
	StringBuilder *file = NULL;
	char *buffer = NULL;
	if(argc == 1){
		while(true){
			buffer = (char*)arena_alloc(&ah, sizeof(char)*DEFAULT_BUFFER_SIZE);
			fgets(buffer, DEFAULT_BUFFER_SIZE, stdin);
			if(strcmp(buffer, "kot_exit();\n") == 0) finish();
			lxer_start_lexing(&lh, buffer);
			if(kot_parse_expression()){
				kot_report();
				eh.tracker = 0;
			}
			lh.lxer_tracker = 0;
		}
	}else{
		switch(argc){
			case 2: 
				if(strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0){
					kot_helper();
				}else{
					char *correct_source = strstr(argv[1],".kot");
					if(correct_source){
						file = read_file_no_error(&ah,argv[1]);
						if(!file->string){
							error_push_error(&eh, "Unable to find the specified file", 0, 1, NULL, 0);
							abort();
						}
					}else{
						error_push_error(&eh, "No source file, be sure to provide a '.kot' file!", 0, 2, NULL, 0);
						abort();
					}
				}
				break;
			default: 
				error_push_error(&eh, "Too many arguments", 0, 420, NULL, 0);
				abort();
				break;
		}	
	
	}
	return 0;
}


int kot_parse_expression(){
	//lxer_get_lxer_content(&lh);
	LXR_TOKENS token; 
	LXR_TOKENS type;
	int status = 0; 
	do{
		token = lxer_get_current_token(&lh);
		if(token != TOKEN_TABLE_END) status = 1;
		
		if(lxer_is_type(token)){
			type = token;
			lxer_next_token(&lh);
			token = lxer_get_current_token(&lh);
			if(token == LXR_OPEN_BRK){
				printf("Function definition detected\n");

			}else if(token == LXR_ASSIGNMENT){
				printf("variable definition detected\n");
			
			}else if(token == LXR_SEMICOLON){
				printf("variable declaration detected\n");
			
			}else{
				error_push_error(&eh, "syntax error, incomplete phrase", 0, 1,NULL,0);
				status = 1;
			}
		}else{
			error_push_error(&eh, "syntax error", 0, 1,NULL,0);
			status = 1;
		}
	}while(lxer_next_token(&lh));
	return status;
}
