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
void finish(){
	arena_free(&eh.ah);
	arena_free(&ah);
	arena_free(&lh.lxer_ah);
	exit(0);
}

int kot_parse_expression();

int main(int argc, char** argv){
	StringBuilder *file = NULL;
	char *buffer = NULL;
	if(argc == 1){
		while(true){
			buffer = (char*)arena_alloc(&ah, sizeof(char)*DEFAULT_BUFFER_SIZE);
			fgets(buffer, DEFAULT_BUFFER_SIZE, stdin);
			if(strcmp(buffer, "exit();\n") == 0) finish();
			lxer_start_lexing(&lh, buffer);
			kot_parse_expression();
		}
	}else{
		switch(argc){
			case 2: 
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
	lxer_get_lxer_content(&lh);
	return 0;
}
