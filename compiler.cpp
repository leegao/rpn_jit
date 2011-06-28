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
	int i = 1;
	int size = eng->size;
	eng->size+=6;
	eng->stack[eng->size] = 32;
	return size;
}

int lol2(vm* eng){
	eng->size -= 3;
	return 1;
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

	//lol2(engine);

	codegen* compiler = new codegen(engine);
	int pushes = 0;
	int pops = compiler->get_pops(l->procedures->at(3)->il, l->procedures, &pushes);

	pushes = 1; pops = 1;

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

		0xf3, 0xab, // rep STOS dword ptr es:[edi]

		/* Set the vm->stack to the correct position
		00EA30BE 8B 45 08             mov         eax,dword ptr [eng]  
		00EA30C1 8B 48 04             mov         ecx,dword ptr [eax+4]  
		00EA30C4 83 E9 pops           sub         ecx,pops  
		00EA30C7 8B 55 08             mov         edx,dword ptr [eng]  
		00EA30CA 89 4A 04             mov         dword ptr [edx+4],ecx  
		*/

		0x8b, 0x45, 0x08,
		0x8b, 0x48, 0x04,
		0x83, 0xe3, pops,
		0x8b, 0x55, 0x08,
		0x89, 0x4a, 0x04
	
	}; 

	unsigned char ret[] = {
		// Return block here
		//0x8b, 0x45, 0x100 - 8*(n), // move eax, prt[ebp - n]
#define i(j) (((char*)(&j))[0]), (((char*)(&j))[1]), (((char*)(&j))[2]), (((char*)(&j))[3]),
		0xb8, i(pushes)
#undef i
		
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

	int local = 1;
	// Program logic starts here.
#define w(ch) *(page++) = ch
#define i(n) {int j = n; w((((char*)(&j))[0])); w((((char*)(&j))[1])); w((((char*)(&j))[2])); w((((char*)(&j))[3]));}
#define var (0x100 - (local*8))

	// simulate one_plus_one: 1 1 + +
	// C7 45 F8 01 00 00 00		mov         dword ptr [i],1
	w(0xc7); w(0x45); w(var); i(1);
	// local ++ (each push does this)
	local++;

	// C7 45 F8 01 00 00 00		mov         dword ptr [i],1
	w(0xc7); w(0x45); w(var); i(1);
	// local ++ (each push does this)
	local++;

	// check that local > 2 (strictly), as local = 1 implies no locals
	if (local < 3) {cerr << "JIT Compilation: Cannot compile code, insufficient stack space." << endl; return 0;}
	// mov eax, local-2
	w(0x8b); w(0x45); w(var + 16);
	// add eax, local-1
	w(0x03); w(0x45); w(var + 8);
	local--;
	// overwrite local-1 (what used to be local-2)
	w(0x89); w(0x45); w(var + 8);

	// check that local > 2 (strictly), as local = 1 implies no locals
	if (local < 3) {cerr << "JIT Compilation: Cannot compile code, insufficient stack space." << endl; return 0;}
	// mov eax, local-2
	w(0x8b); w(0x45); w(var + 16);
	// add eax, local-1
	w(0x03); w(0x45); w(var + 8);
	local--;
	// overwrite local-1 (what used to be local-2)
	w(0x89); w(0x45); w(var + 8);

#undef var
#undef i
#undef w

	memcpy(page, ret, sizeof(ret));


	int eax = ((int(*)(void*))fun)((void*)engine);
	cout << eax << " " << pushes << endl;

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