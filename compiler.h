#ifndef H_COMPILER
#define H_COMPILER

#include "vm.h"
#include "lexer.h"
#include <vector>
#include <map>

class codegen{
public:
	vm* runtime;
	static map<string, int> m;
	codegen(vm* runtime) : runtime(runtime) {}
	int get_pops(vector<int>*, vector<struct procedure*>*, int*);
};

#endif