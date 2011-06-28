#include "compiler.h"

int codegen::get_pops(vector<int>* il, int* pushes){
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
			return -1;
		} else {
			// load operator, increments pushes
			current = (current < 0) ? 1 : current + 1;
			(*pushes)++;
		}
	}
	return pops;
}


/*void print(int i){ // debug only
	printf("%x\n", i);
}*/

double _pow(double a, int b){
	return (int)pow(a,b);
}

jitter codegen::compile(vector<int>* il){

	int pushes = 0;
	int pops = get_pops(il, &pushes);

	if (pops < 0) {
		cerr << "JIT Error: Cannot compile unflattened code" << endl;
		return 0;
	}

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

		//0xf3, 0xab, // rep STOS dword ptr es:[edi]

		/* Set the vm->stack to the correct position
		00EA30BE 8B 45 08             mov         eax,dword ptr [eng]  
		00EA30C1 8B 48 04             mov         ecx,dword ptr [eax+4]  
		00EA30C4 83 E9 pops           sub         ecx,pops  
		00EA30C7 8B 55 08             mov         edx,dword ptr [eng]  
		00EA30CA 89 4A 04             mov         dword ptr [edx+4],ecx  
		*/

		0x8b, 0x45, 0x08,
		0x8b, 0x48, 0x04,
		0x83, 0xe9, pops,
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

	int local = 0;
	// Program logic starts here.
#define w(ch) *(page++) = ch
#define i(n) {int j = (n); w((((char*)(&j))[0])); w((((char*)(&j))[1])); w((((char*)(&j))[2])); w((((char*)(&j))[3]));}
#define var (0x100 - 8 - (local*4))

	int pre;
	for (pre=0; pre<pops; pre++){
		/*    46: 	int a = eng->stack[eng->size + 1];
		011030BE 8B 45 08             mov         eax,dword ptr [eng]  
		011030C1 8B 48 04             mov         ecx,dword ptr [eax+4]  
		011030C4 8B 55 08             mov         edx,dword ptr [eng]  
		011030C7 8B 02                mov         eax,dword ptr [edx]  
		NOT THIS 011030C9 8B 4C 88 pre*4       mov         ecx,dword ptr [eax+ecx*4+pre*4]  
		8B 8C 88 00 04 00 00
		011030CD 89 4D var             mov         dword ptr [var],ecx  
		*/
		w(0x8b); w(0x45); w(0x08);
		w(0x8b); w(0x48); w(0x04);
		w(0x8b); w(0x55); w(0x08);
		w(0x8b); w(0x02);
		w(0x8b); w(0x4c); w(0x88); w((char)(pre*4));
		w(0x89); w(0x4d); w(var);
		local++;

		/* // debug 
		w(0x8b); w(0x45); w(var+4);
		w(0x50);
		w(0xb8); i((int)&print);
		w(0xff); w(0xd0); // call eax
		w(0x83); w(0xC4); w(0x04); // add 4 to the ESP */
	}

	vector<int>::iterator iptr;
	for (iptr = il->begin(); iptr < il->end(); iptr++){
		int op = *iptr;
		if (op & OP){
			// Binary operator
			// check that local > 1 
			if (local < 2) {
				cerr << "JIT Error: insufficient stack space." << endl; 
				return 0;
			}

			switch(op & VAL){
			case PLUS:
				// mov eax, local-2
				w(0x8b); w(0x45); w(var + 8);
				// add eax, local-1
				w(0x03); w(0x45); w(var + 4);
				local--;
				// overwrite local-1 (what used to be local-2)
				w(0x89); w(0x45); w(var + 4);
				break;
			case MINUS: 
				// mov eax, local-2
				w(0x8b); w(0x45); w(var + 8);
				// sub eax, local-1
				w(0x2b); w(0x45); w(var + 4);
				local--;
				// overwrite local-1 (what used to be local-2)
				w(0x89); w(0x45); w(var + 4);
				break;
			case MUL:
				// mov eax, local-2
				w(0x8b); w(0x45); w(var + 8);
				// imul eax, local-1
				w(0x0f); w(0xaf); w(0x45); w(var + 4);
				local--;
				// overwrite local-1 (what used to be local-2)
				w(0x89); w(0x45); w(var + 4);
				break;
			case DIV:
				// mov eax, local-2
				w(0x8b); w(0x45); w(var + 8);
				// cdq; idiv eax, local-1
				w(0x99); w(0xf7); w(0x7d); w(var + 4);
				local--;
				// overwrite local-1 (what used to be local-2)
				w(0x89); w(0x45); w(var + 4);
				break;
			case MOD:
				// mov eax, local-2
				w(0x8b); w(0x45); w(var + 8);
				// cdq; idiv eax, local-1
				w(0x99); w(0xf7); w(0x7d); w(var + 4);
				local--;
				// overwrite local-1 (what used to be local-2)
				w(0x89); w(0x55); w(var + 4);
				break;
			case EXP:
				// mov eax, local-1
				w(0x8b); w(0x45); w(var + 4);
				// push eax
				w(0x50);
				// to float local-2
				w(0xdb); w(0x45); w(var + 8);
				// push float (ie: sub esp by 8 and store)
				w(0x83); w(0xec); w(0x08);
				w(0xdd); w(0x1c); w(0x24);
				
				// call pow
				w(0xb8); i((int)&_pow);
				w(0xff); w(0xd0); // call eax
				// advance esp by 4 + 8 (4 -> int, 8 -> double)
				w(0x83); w(0xc4); w(0x0c);
				// hack, we need to use intel's internal func, __ftol2_sse to cast float into int
				// w(0xb8); i((int)&__ftol2_sse); // inlined into _pow
				local--;
				// overwrite local-1 (what used to be local-2)
				w(0x89); w(0x45); w(var + 4);
				break;

			default:
				cerr << "JIT Error: unknown operator" << endl; 
				return 0;
			}

			
		} else { // Value
			// C7 45 F8 01 00 00 00		mov         dword ptr [i],1
			w(0xc7); w(0x45); w(var); i(op & VAL);
			// local ++ (each push does this)
			local++;
		}
	}

	// iterate through pushes and add back into the stack
	int push;
	for (push = 0; push < pushes; push++){
		/*    
		47: 	eng->stack[eng->size++] = a;
		002730C5 8B 45 08             mov         eax,dword ptr [eng]  
		002730C8 8B 48 04             mov         ecx,dword ptr [eax+4]  
		002730CB 8B 55 08             mov         edx,dword ptr [eng]  
		002730CE 8B 02                mov         eax,dword ptr [edx]  
		002730D0 8B 55 (0x100-8-(p*4))mov         edx,dword ptr [a]  
		002730D3 89 14 88             mov         dword ptr [eax+ecx*4],edx  
		002730D6 8B 45 08             mov         eax,dword ptr [eng]  
		002730D9 8B 48 04             mov         ecx,dword ptr [eax+4]  
		002730DC 83 C1 01             add         ecx,1  
		002730DF 8B 55 08             mov         edx,dword ptr [eng]  
		002730E2 89 4A 04             mov         dword ptr [edx+4],ecx  
		*/

		w(0x8b); w(0x45); w(0x08);
		w(0x8b); w(0x48); w(0x04);
		w(0x8b); w(0x55); w(0x08);
		w(0x8b); w(0x02);
		w(0x8b); w(0x55); w(0x100-8-(push*4));
		w(0x89); w(0x14); w(0x88);
		w(0x8b); w(0x45); w(0x08);
		w(0x8b); w(0x48); w(0x04);
		w(0x83); w(0xc1); w(0x01);
		w(0x8b); w(0x55); w(0x08);
		w(0x89); w(0x4a); w(0x04);
	}


#undef var
#undef i
#undef w

	memcpy(page, ret, sizeof(ret));
	return (jitter)fun;
}

int main(){
	lexer* l = new lexer("add: +");
	l->lex("add_1: 1 add");
	l->lex("1 2 5 add add");

	vm* engine = new vm();
	engine->eval(l);
	engine->eval(l);

	codegen* compiler = new codegen(engine);
	jitter fun = compiler->compile(l->il);

	if (fun) fun(engine);

	int i;
	for (i = 0; i < engine->size; i++){
		printf("%x\n", engine->stack[i]);
	}

	string lol;
	cin >> lol;

	return 0;
}