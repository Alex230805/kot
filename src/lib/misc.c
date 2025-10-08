#define MISC_IMP

#include "misc.h"

#ifdef GC_IMP

void gc_init(tb_gc *gc){
  gc->address = (void**)malloc(sizeof(void*)*POOL_SIZE);
  gc->size = POOL_SIZE;
  gc->pointer = 0;
}

void gc_free(tb_gc*gc){
  for(int i=0;i<gc->pointer;i++){
    free(gc->address[i]);
    gc->address[i] = NULL;
  }
}

void gc_push(tb_gc*gc, void* address){
  if(gc->pointer+1 >= POOL_SIZE){
    fprintf(stderr, "garbage collector full of trash man, check your code, this is unacceptable!\n");
    exit(1);
  }else{
    gc->address[gc->pointer] = address;
    gc->pointer += 1;
  }
}

#endif

StringBuilder* read_file(Arena_header* ah, char*path){
  if(DEBUG) DINFO("Reading file", NULL); 
  StringBuilder *sb; 
  sb = (StringBuilder*)arena_alloc(ah,sizeof(StringBuilder));
  FILE * fp;
  fp = fopen(path, "r");
  if(fp == NULL){
    fprintf(stderr, "Unable to open instruction file: %s\n", strerror(errno));
    exit(errno);
  }
  fseek(fp, 0, SEEK_END);
  sb->len = ftell(fp);
  rewind(fp);
  sb->string = (char*)arena_alloc(ah,sizeof(char)*sb->len);
  sb->size = sb->len;
  fread(sb->string, sizeof(char), sb->len,fp);
  sb->string[sb->len] = '\0';
  fclose(fp);
  return sb;
}

StringBuilder* read_file_no_error(Arena_header* ah, char*path){
  if(DEBUG) DINFO("Reading file", NULL); 
  StringBuilder *sb; 
  sb = (StringBuilder*)arena_alloc(ah,sizeof(StringBuilder));
  FILE * fp;
  fp = fopen(path, "r");
  if(fp == NULL){
	sb->string = NULL;
	return sb;	
  }
  fseek(fp, 0, SEEK_END);
  sb->len = ftell(fp);
  rewind(fp);
  sb->string = (char*)arena_alloc(ah,sizeof(char)*sb->len);
  sb->size = sb->len;
  fread(sb->string, sizeof(char), sb->len,fp);
  sb->string[sb->len] = '\0';
  fclose(fp);
  return sb;
}



void write_file(StringBuilder *sb, char *path){
  if(DEBUG) DINFO("Writing file", NULL); 
  FILE * fp;
  fp = fopen(path, "w");
  if(fp == NULL){
    fprintf(stderr, "Unable to open instruction file: %s\n", strerror(errno));
    exit(errno);
  }
  fwrite(sb->string, 1, sb->len, fp);
  fclose(fp);
}


u8t hexDigitConverter(char s){
    if(isdigit(s)){
        return s - '0';
    }else{
         return toupper(s) - 'A' + 10;
    }
}

u8t hexStringConverter(char string[]){
    uint8_t HexString = 0;
    int len = strlen(string);
    for(int i=0;i<len;i++){
        if(!isxdigit(string[i]))
            return -1;
       int cache = hexDigitConverter(string[i]);
       HexString = (HexString << 4) | cache; 
    }
    return HexString;
} 


void arena_create(Arena_header* arenah, int page_size, int page_count){
  
  Arena_alloc* arena = (Arena_alloc*)malloc(sizeof(Arena_alloc));
  arena->obj = 0;
  arena->pages = page_count;
  arena->free_pages = page_count;
  arena->arena_start_ptr = malloc(sizeof(size_t)*page_size*page_count);
  arena->page_size = page_size;
  arena->pages_pointers = malloc(sizeof(size_t*)*page_count);
  arena->allocated_page = malloc(sizeof(bool)*page_count);
  arena->next = NULL;

  for(int i=0;i<page_count;i++){
    arena->allocated_page[i] = false;
  }
  for(int i=0;i<page_count;i++){
    arena->pages_pointers[i] = &arena->arena_start_ptr[(i*page_size)];
  }

  arena->cursor = 0; 
  arenah->swap = arenah->first_arena;
  
  if(arenah->arena_count == 0 && arenah->swap != NULL){
    while ( arenah->swap->next != NULL) arenah->swap = arenah->swap->next;
    arenah->swap->next = arena;
    arenah->swap = NULL;
    arenah->arena_count += 1;
    arenah->cursor = arena;
  }else{
    arenah->first_arena = arena;
    arenah->swap = NULL;
    arenah->arena_count = 1;
    arenah->cursor = arena;
  }
}

StringBuilder* sb_alloc(Arena_header* arenah){
  StringBuilder *sb = (StringBuilder*)arena_alloc(arenah, sizeof(StringBuilder));
  sb->string = (char*)arena_alloc(arenah, sizeof(char)*DEF_SB_SIZE);
  sb->size = DEF_SB_SIZE;
  sb->len = 0;
  return sb;
}

void sb_append(Arena_header* arenah, StringBuilder* sb, char* string){
  int mult_factor = 1;
  if(strlen(string) > sb->size - sb->len){ 
    while((sb->size*mult_factor - sb->len) <= strlen(string) ){ mult_factor += 1; }
    char* buffer = (char*)arena_alloc(arenah, sizeof(char)*sb->size*mult_factor);
    strcpy(buffer, sb->string);
    sb->size *= 2;
    sb->string = buffer;
  }
  strcat(sb->string, string);
  sb->len = strlen(sb->string);
}

void* arena_alloc(Arena_header* arenah, size_t size){
  if(arenah->arena_count < 1){
    arena_create(arenah,PAGE_SIZE, PAGE_NUMBER);
  }
  Arena_alloc* arena = arenah->cursor;
  // number of required pages after normalization
  int page_required = (int) (size/ (arena->page_size));
  int size_required = (int) (size/ (arena->page_size*arena->pages));
  if(page_required > arena->free_pages){
    if(size_required >= 1){
      int new_size = arena->pages;
      int new_page_size = arena->page_size;
      while( (int)size/(1+(new_size*new_page_size)) >= 1){
        new_size*=2;
      }
      arena_create(arenah,new_page_size, new_size);
    }else{
      arena_create(arenah,arena->page_size, arena->pages);
    }
    arena = arenah->cursor;
  }
 
  void* pointer = NULL;
  page_required+=1;
  int i = 0;
  for(i=0; i < page_required;i++){
    arena->allocated_page[arena->cursor+i] = true;
  }
  
  pointer = (void*)arena->pages_pointers[arena->cursor];
  arena->cursor = (arena->cursor + i) + 1;
  arena->free_pages -= (i+1);

  return pointer;
}
void arena_free_area(Arena_alloc* arena){
  free(arena->allocated_page);
  arena->allocated_page = NULL;
  free(arena->pages_pointers);
  arena->pages_pointers = NULL;
  free(arena->arena_start_ptr);
  arena->pages_pointers = NULL;
  free(arena);
  return;
}


void arena_free(Arena_header *arenah){
  if(arenah->arena_count < 1) return;
  Arena_alloc* a = arenah->first_arena;
  if(arenah->first_arena->next != NULL){
    arenah->first_arena = arenah->first_arena->next;
    arena_free(arenah);
  }
  arena_free_area(a);
  a = NULL;
  return;
}


void error_push_error(error_handler *eh, char* error_string, int line_position, size_t error_code, char*source_ptr, int source_line_len){
	if(eh->size < DEFAULT_ERROR_ARRAY_SIZE){
		eh->size = DEFAULT_ERROR_ARRAY_SIZE;
		eh->tracker = 0;
		eh->error_array = (error_slice**)arena_alloc(&eh->ah,sizeof(error_slice*)*eh->size);
	}
	error_slice * es = (error_slice*)arena_alloc(&eh->ah,sizeof(error_slice));
	es->error_code = error_code;
	es->line = line_position;
	es->error = (char*)arena_alloc(&eh->ah, sizeof(char)*strlen(error_string));
	es->source_ptr = source_ptr;
	es->source_line_len = source_line_len;

	strcpy(es->error, error_string);
	dapush(eh->ah, eh->error_array, eh->tracker, eh->size, error_slice*, es);
}


void error_print_error(error_handler *eh, const print_set pp){
	char *buffer = NULL;
	for(int i=0; i<eh->tracker;i++){
		if(pp.pretty_color) fprintf(stderr, "\e[31;49m");
		error_slice * es = eh->error_array[i];
		
		if(pp.error_prefix) fprintf(stderr,"[ERROR] ");
		if(pp.include_error_code){
			fprintf(stderr,"return status %d", es->error_code);
		}

		if(pp.include_error_line){
			if(pp.include_error_code) fprintf(stderr, " ");
			fprintf(stderr, "in line %d", es->line);
		}

		if(	pp.include_error_code == true ||\
			pp.include_error_line == true ||\
			pp.pretty_indentation == true ||\
			pp.include_reference_line == true ||\
			pp.include_reference_decoration == true){
			fprintf(stderr, ": ");
		}
		
		if(es->error != NULL) fprintf(stderr, "%s", es->error);	
		if(pp.include_reference_line && es->source_ptr != NULL){
			buffer = (char*)arena_alloc(&eh->ah,sizeof(char)*es->source_line_len);
			memcpy(buffer, es->source_ptr, es->source_line_len+1);
			buffer[es->source_line_len+1] = '\0';		
			if(pp.pretty_indentation) fprintf(stderr,"\n\t");
			fprintf(stderr, "%s", buffer);
			if(pp.include_reference_decoration){
				fprintf(stderr, "\n\t");
				for(int i=0;i<es->source_line_len;i++){
					fprintf(stderr, "^");
				}
				fprintf(stderr, "\n");
			}
		}
		if(pp.pretty_color) fprintf(stderr, "\e[0m");
		fprintf(stderr, "\n");
	}
}






