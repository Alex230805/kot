#include <stdlib.h>
#include <stdio.h>

#include <string.h>
#include "ilxer.h"
#include "misc.h"
#include "kot.h"

Arena_header ah = {0};
lxer_header lh = {0};
error_handler eh = {0}; 

#define DEFAULT_BUFFER_SIZE 256
#define DEFAULT_SRC_SIZE 4

// =================================================================

// example of external linking between C function and internal Kot machine

static int custom_hello(){ // signature of int __ffi_linker_callback(void);
	printf("Hello World\n");
	return 0;
}

void push_custon_fn(){
	fn_signature* fn = kot_define_fn(&ah,"hello",0, NULL, NULL);
	if(!kot_link_function(&ah, fn, &custom_hello)){
		printf("Unable to link custom function\n");
	}
}

// =================================================================

void abort(){
	error_print_error(&eh,(print_set){false,true,false,false,true,false,false});
	arena_free(&eh.ah);
	arena_free(&ah);
	arena_free(&lh.lxer_ah);
	exit(1);
}

void kot_report(){
	error_print_error(&eh,(print_set){true,true,true,true,true,true,true});
}


void finish(){
	arena_free(&eh.ah);
	arena_free(&ah);
	arena_free(&lh.lxer_ah);
	exit(0);
}


void kot_console_helper(){
	fprintf(stdout, "\nCommand helper in kot console mode: \n");
	fprintf(stdout, "h: get help \n");
	fprintf(stdout, "l: get current program listate\n");
	fprintf(stdout, "q: quit kot console mode and go back to execution mode\n\n");
}

void kot_helper(){
	fprintf(stdout,"\n===========================================================\n\n");
	fprintf(stdout, "KOT: C-like console parser and interpreter:\n");
	fprintf(stdout, "\tusage: kot [param]\n");
	fprintf(stdout, "\t\t-h/--help: print help\n\n");
	fprintf(stdout, "\t\t-ir/--intermediate-representation: get intermediate\n\t\trepresentation in form of bytecode used by the \n\t\tkot vm\n\n");
	fprintf(stdout, "\t\t-md/--memory-dump: get kot vm's memory dump after\n\t\tthe execution has ended\n\n");
	fprintf(stdout, "\t\t-pl/--program-list: get program list printed on the\n\t\tstandard output after the console execution has\n\t\tended without errors, only the validated lines\n\t\twill appear in the final stream\n\n");
	fprintf(stdout, "\tusage: kot {file.kot}\n");
	fprintf(stdout, "\t\tresult: kot interpreter will execute the .kot file.\n\
		A .kot can be anything that include a working C code\n\
		implementation, this means that it will not accept\n\
		library header or strange preprocessing things like\n\
		the classic X macro technique.\n\
		C programs with working algorithm and the proper\n\
		implementation should work just fine, remember that\n\
		any type of function or variable, data structure or\n\
		enum DECLARATION ( not implementation ) will be excluded\n\
		as part of what can be parser, it will have no effect on\n\
		the program. For example any function implementation\n\
		will automatically declare it and pushit in a general\n\
		namespace, ready to be reached from all point in the program\n");
	fprintf(stdout,"\n===========================================================\n");
}


int main(int argc, char** argv){
	StringBuilder *file = NULL;
	char *buffer = NULL;
	bool ir_out = false;
	bool md_out = false;
	bool pl_out = false;
	bool file_provided = false;
	int file_pos_ptr = 0;
	for(int i=1;i<argc;i++){
		if(strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0){
			kot_helper();
			return 0;		
		}else if(strcmp(argv[i], "-ir") == 0 || strcmp(argv[i], "--intermediate-representation") == 0){
			ir_out = true;
		}else if(strcmp(argv[i], "-md") == 0 || strcmp(argv[i], "--memory-dump") == 0){
			md_out = true;
		}else if(strcmp(argv[i], "-pl") == 0 || strcmp(argv[i], "-program-list") == 0){
			pl_out = true;
		}else if(strstr(argv[i],".kot") != NULL){
			file_provided = true;
			file_pos_ptr = i;
		}
	}
	kot_init_interpreter(&ah);
	kot_init_vm(&ah);
	push_custon_fn();
	if(!file_provided){
		fputs("Starting kot in console mode\n\n", stdout);
		size_t line = 0;
		bool kot_console = false;
		while(true){
			buffer = (char*)arena_alloc(&ah, sizeof(char)*DEFAULT_BUFFER_SIZE);
			if(kot_console){
				// kot console mode
				fprintf(stdout, "kot:> ");
				fgets(buffer, DEFAULT_BUFFER_SIZE, stdin);
				if(strstr(buffer, "q") == buffer){
					kot_console = false;
				}else if(strstr(buffer, "l") == buffer){
					kot_get_program_list(stdout);
				}else if(strstr(buffer, "h") == buffer || strstr(buffer, "help") == buffer){
					kot_console_helper();
				}else{
					fprintf(stdout, "Not a valid command\n");
					kot_console_helper();
				}
			}else{
				// kot execution mode
				fprintf(stdout, "%zu: ", line);
				fgets(buffer, DEFAULT_BUFFER_SIZE, stdin);
				if(strstr(buffer, "kot") == buffer && (*(buffer+3) == ' ' || *(buffer+3) == '\n')){
					kot_console = true;
				}else{
					if(strcmp(buffer, "exit();\n") == 0){
						if(md_out) kot_get_memory_dump();
						if(ir_out) kot_get_bytecode();
						if(pl_out) kot_get_program_list(stdout);
						finish();
					}
					kot_set_line(line);
					lxer_start_lexing(&lh, buffer);
					if(kot_parse(&ah,&lh, &eh, true)){
						lh.lxer_tracker = 0;
						kot_report();
						eh.tracker = 0;
					}else{
						line++;
						lh.lxer_tracker = 0;
						//kot_single_run(kot_get_current_inst());
						kot_pc_inc();
					}
				}
			}
		}
	}else{
		// file execution mode
		file = read_file_no_error(&ah,argv[file_pos_ptr]);
		if(!file->string){
			error_push_error(&eh, "Unable to find the specified file", 0, 1, NULL, 0);
			abort();
		}
		kot_init_interpreter(&ah);
		kot_init_vm(&ah);
		kot_parse(&ah, &lh, &eh, false);
		kot_run();
		if(md_out) kot_get_memory_dump();
		if(ir_out) kot_get_bytecode();
	}
	finish();
	return 0;
}


