#ifndef H_VM
#define H_VM

#include "lexer.h"

class vm{
public:
	int* stack;
	int size;
	vm(){
		stack = (int*)malloc(sizeof(int)*0x100);
		size = 0;
	}

	bool eval(lexer* l);
};

#endif