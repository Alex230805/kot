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
	bool ir_out = false;
	bool md_out = false;
	bool file_provided = false;
	int file_pos_ptr = 0;
	for(int i=1;i<argc;i++){
		if(strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0){
			kot_helper();		
		}else if(strcmp(argv[i], "-ir") == 0 || strcmp(argv[i], "--intermediate-representation") == 0){
			ir_out = true;
		}else if(strcmp(argv[i], "-md") == 0 || strcmp(argv[i], "--memory-dump") == 0){
			md_out = true;
		}else if(strstr(argv[i],".kot") != NULL){
			file_provided = true;
			file_pos_ptr = i;
		}
	}
	kot_init_vm(&ah);
	if(!file_provided){
		while(true){
			buffer = (char*)arena_alloc(&ah, sizeof(char)*DEFAULT_BUFFER_SIZE);
			fgets(buffer, DEFAULT_BUFFER_SIZE, stdin);
			if(strcmp(buffer, "exit();\n") == 0){
				if(md_out) kot_get_memory_dump();
				if(ir_out) kot_get_bytecode();
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
		file = read_file_no_error(&ah,argv[file_pos_ptr]);
		if(!file->string){
			error_push_error(&eh, "Unable to find the specified file", 0, 1, NULL, 0);
			abort();
		}
		kot_init_vm(&ah);
		kot_parse(&ah, &lh, &eh);
		// file execution 
	
		if(md_out) kot_get_memory_dump();
		if(ir_out) kot_get_bytecode();
	}
	return 0;
}


