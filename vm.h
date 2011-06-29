#ifndef H_VM
#define H_VM

#include "lexer.h"
#include "rwx_page.h"

class vm{
public:
	int* stack;
	int size;
	char* page;
	void* page_handle;
	vm(){
		stack = (int*)malloc(sizeof(int)*0x100);
		size = 0;
		page = (char*)rwxalloc(0x1000);
		page_handle = (void*)page;
	}

	bool eval(lexer* l);
};

#endif