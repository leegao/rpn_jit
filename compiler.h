#ifndef H_COMPILER
#define H_COMPILER

#include "vm.h"
#include "lexer.h"
#include "rwx_page.h"
#include <vector>
#include <map>

typedef int (*jitter) (vm*);

class codegen{
public:
	vm* runtime;
	static map<string, int> m;
	codegen(vm* runtime) : runtime(runtime) {}
	int get_pops(vector<int>*, int*);
	jitter compile(vector<int>*);
};

#endif