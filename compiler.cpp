#include "compiler.h"

int codegen::get_pops(vector<int>* il, vector<struct procedure*>* procedures, int* pushes){
	// get the number of pops made off of the original stack (this never decreases)
	int pops = 0, current = 0;
	vector<int>::iterator op;
	for (op = il->begin(); op < il->end(); op++){
		// only when the current drops below 0, we reset current on each push
		int d = *op;
		if ((d & OP) && (d & VAL) < CALL_PROC){
			// binary operator - pop2 and push1
			if (current == 1) {
				pops++;
			} else if (current < 1){
				pops += 2;
				(*pushes)++;
			} else {
				// no pops made, decrease pushes and current
				current--;
				(*pushes)--;
			}
		} else if (d & OP) {
			// theoretically, this case is impossible
			//int pushes_, pops_;
			//pops_ = get_pops(procedures->at((d & VAL) - CALL_PROC)->il, procedures, &pushes_);
			cerr << "Compile Error: Cannot compile unflattened code" << endl;
			return -1;
		} else {
			// load operator, increments pushes
			current = (current < 0) ? 1 : current + 1;
			(*pushes)++;
		}
	}
	return pops;
}

int lol(vm* eng){
	int size = eng->size;
	eng->size+=6;
	eng->stack[eng->size] = 32;
	return size;
}

int main(){
	// self modifying code test

	

	lexer* l = new lexer("add: +");
	l->lex("add_1: 1 add");
	l->lex("test: + + + +");
	l->lex("test2: 3 1 2 3 4 + + + -");
	l->lex("1 2 * add_1 add_1");

	vm* engine = new vm();
	engine->eval(l);
	engine->eval(l);
	engine->eval(l);

	//lol(engine);

	codegen* compiler = new codegen(engine);
	int pushes = 0;
	int pops = compiler->get_pops(l->procedures->at(3)->il, l->procedures, &pushes);

	int stack = offsetof(vm, stack), size = offsetof(vm, size);
	unsigned char start[] = {
		// eax is pushes, ecx is pops, edx is engine
		// we'll use eax and ebx and leave edx
		0x55,  // push ebp
		0x8b, 0xec, //mov ebp, esp
		0x81, 0xec, 0xcc, 0x00, 0x00, 0x00, // sub esp, 0xcc
		0x53, // push ebx
		0x56, // push esi
		0x57, // push edi
		0x8d, 0xbd, 0x34, 0xff, 0xff, 0xff, // lea edi, [ebp-0xcc]

		0xf3, 0xab// rep STOS dword ptr es:[edi]
	
	}; 

	unsigned char ret[] = {
		// Return block here
		//0x8b, 0x45, 0x100 - 8*(n), // move eax, prt[ebp - n]
		
		0x5f, // pop edi
		0x5e, // pop esi
		0x5b, // pop ebx
		0x8b, 0xe5, // mov esp, ebp
		0x5d, // pop ebp
		0xc3
	};

	char* page = (char*)rwxalloc(1024);
	memcpy(page, start, sizeof(start));
	char* fun = page;
	page += sizeof(start);

	// Program logic starts here.

	memcpy(page, ret, sizeof(ret));


	int eax = ((int(*)(void*))fun)((void*)engine);
	cout << eax;

	string lol;
	cin >> lol;
	return 0;
}

/*int main(){
	lexer* l = new lexer("add: +");
	l->lex("add_1: 1 add");
	l->lex("test: + + + +");
	l->lex("test2: 3 1 2 3 4 + + + -");
	l->lex("1 2 * add_1 add_1");

	vm* engine = new vm();
	engine->eval(l);
	engine->eval(l);
	engine->eval(l);

	int i;
	for (i = 0; i < engine->size; i++){
		printf("%x\n", engine->stack[i]);
	}

	vector<int>::iterator it;
	for (it = l->il->begin(); it < l->il->end(); it++){
		printf("%x ", *it);
	}

	codegen* compiler = new codegen(engine);
	int j = 0;
	int pops = compiler->get_pops(l->procedures->at(3)->il, l->procedures, &j);

	cout << "\n" << pops << " " << j << endl;

	string lol;
	cin >> lol;
	return 0;
}*/