#include "compiler.h"

int codegen::get_pops(vector<int>* il, vector<struct procedure*>* procedures, int* pushes){
	// get the number of pops made off of the original stack (this never decreases)
	int pops = 0, current = 0;
	vector<int>::iterator op;
	for (op = il->begin(); op < il->end(); op++){
		// only when the current drops below 0, we reset current on each push
		int d = *op;
		if ((d & OP) && (d&VAL) < CALL_PROC){
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
			// we need to get_pops from it too.
			// theoretically, this case is impossible
		} else {
			// load operator, increments pushes
			current = (current < 0) ? 1 : current + 1;
			(*pushes)++;
		}
	}
	return pops;
}