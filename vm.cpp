
#include <cstdio>
#include <cmath>
#include "lexer.h"
#include "vm.h"
#include "compiler.h"

struct inline_meta{int a; int b;};
bool vm::eval(lexer* l){
	// Naive implementation

	vector<struct inline_meta*> v;
	int offset = 0, lptr = 0;

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
					size_t index = val - CALL_PROC;
					// call the procedure
					if (index < 0 || index >= l->procedures->size()){
						cerr << "Runtime Error: Procedure does not exist." << endl;
						return false;
					}
					if (l->procedures->at(index)->status > NORMAL)
						cerr << "Debug: Inlined function encountered." << endl;
					
					// check if jitted
					jitter fun;
					if ((fun = l->procedures->at(index)->jit)){
						fun(this);
					} else {
						// evaluate the current chunk of code with context to the global lexer (pass in reference too)
						lexer* proxy = new lexer(l, l->procedures->at(index)->il);
						if (!eval(proxy)) return false;

						(l->procedures->at(index)->calls)++;

						// add in the metadata for procedure inlining
						// once a procedure is flattened, track its call number to determine if it's a candidate for jitting
						if (l->procedures->at(index)->calls > 0){
							struct inline_meta* meta = new (struct inline_meta)();
							meta->a = lptr;
							meta->b = index;
							v.push_back(meta);
						}

						if (l->procedures->at(index)->calls > 1){
							// jit compile this.
							int _;
							codegen* compiler;
							if ((compiler = new codegen(this))->get_pops(l->procedures->at(index)->il, &_) >= 0){
								// okay to jit the procedure
								jitter func = compiler->compile(l->procedures->at(index)->il);
								l->procedures->at(index)->jit = func;
							}
						}
					}
				}
			}
		} else {
			// load a number onto stack
			stack[size++] = val;
		}
		lptr++;
	}

	// inline the procedure to trade time complexity for size, helps with jitting considerably
	vector<struct inline_meta*>::iterator procs;

	for (procs = v.begin(); procs < v.end(); procs++){
		struct inline_meta* meta = *procs;
		procedure* proc = l->procedures->at(meta->b);
		vector<int>* to_inline = l->procedures->at(meta->b)->il;

		// mark this procedure
		proc->status = (proc->status < INLINED) ? INLINED : proc->status;

		// start at meta->a + offset, for to_inline->size()
		vector<int>::iterator index = l->il->begin() + meta->a + offset;

		// inline the procedure into this current chunk
		l->il->insert(index, to_inline->begin(), to_inline->end());
		offset += to_inline->size() - 1;
		l->il->erase( l->il->begin() + meta->a + offset + 1);
	}

	return true;
}

