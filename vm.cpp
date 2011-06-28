#include "lexer.h"
#include "vm.h"
#include <cstdio>
#include <cmath>

bool vm::eval(lexer* l){
	// Naive implementation

	vector<int>::iterator iptr;
	for(iptr = l->il->begin(); iptr < l->il->end(); iptr++){
		int d = *iptr;
		int val = VAL & d;
		if (d & OP){
			// operator
			switch (val){
			case PLUS:
				if (size < 2){ 
					cerr << "Runtime Error: Too few elements on the stack." << endl;
					return false;
				}
				val = stack[size - 2] + stack[size - 1];
				size--;
				stack[size-1] = val;
				break;
			case MINUS:
				if (size < 2){ 
					cerr << "Runtime Error: Too few elements on the stack." << endl;
					return false;
				}
				val = stack[size - 2] - stack[size - 1];
				size--;
				stack[size-1] = val;
				break;
			case MUL:
				if (size < 2){ 
					cerr << "Runtime Error: Too few elements on the stack." << endl;
					return false;
				}
				val = stack[size - 2] * stack[size - 1];
				size--;
				stack[size-1] = val;
				break;
			case DIV:
				if (size < 2){ 
					cerr << "Runtime Error: Too few elements on the stack." << endl;
					return false;
				}
				if (!stack[size-1]){
					cerr << "Runtime Error: Division by zero." << endl;
					return false;
				}
				val = stack[size - 2] / stack[size - 1];
				size--;
				stack[size-1] = val;
				break;
			case EXP:
				if (size < 2){ 
					cerr << "Runtime Error: Too few elements on the stack." << endl;
					return false;
				}
				val = (int)pow((double)stack[size - 2], stack[size - 1]);
				size--;
				stack[size-1] = val;
				break;
			case MOD:
				if (size < 2){ 
					cerr << "Runtime Error: Too few elements on the stack." << endl;
					return false;
				}
				val = stack[size - 2] % stack[size - 1];
				size--;
				stack[size-1] = val;
				break;
			default:
				{
					int index = val - CALL_PROC;
					// call the procedure
					if (index < 0 || index >= l->procedures->size()){
						cerr << "Runtime Error: Procedure does not exist." << endl;
						return false;
					}

					lexer* proxy = new lexer(l, l->procedures->at(index)->il);
					if (!eval(proxy)) return false;
				}
			}
		} else {
			// load a number onto stack
			stack[size++] = val;
		}
	}

	return true;
}

int main(){
	lexer* l = new lexer("add: +");
	l->lex("add_1: 1 +");
	l->lex("1 2 add add_1");

	vm* engine = new vm();
	engine->eval(l);


	int i;
	for (i = 0; i < engine->size; i++){
		printf("%x\n", engine->stack[i]);
	}

	string lol;
	cin >> lol;
	return 0;
}