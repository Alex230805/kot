#include <stdlib.h>
#include <stdio.h>

#include <string.h>
#include "ilxer.h"
#include "misc.h"
#include "kot.h"

static Arena_header ah = {0};
static lxer_header lh = {0};
static error_handler eh = {0}; 

#define DEFAULT_BUFFER_SIZE 256
#define DEFAULT_SRC_SIZE 4


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


int main(int argc, char** argv){
	StringBuilder *file = NULL;
	char *buffer = NULL;
	if(argc == 1){
		kot_init_vm(&ah);
		while(true){
			buffer = (char*)arena_alloc(&ah, sizeof(char)*DEFAULT_BUFFER_SIZE);
			fgets(buffer, DEFAULT_BUFFER_SIZE, stdin);
			if(strcmp(buffer, "exit();\n") == 0){
				//kot_get_memory_dump();
				//kot_get_bytecode();
				finish();
			}
			lxer_start_lexing(&lh, buffer);
			if(kot_parse(&ah,&lh, &eh)){
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
						kot_init_vm(&ah);
						kot_parse(&ah, &lh, &eh);
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


