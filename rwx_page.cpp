#include "rwx_page.h"

void* rwxalloc(size_t bytes){
	if (!bytes) return 0;
	static const size_t pagesize = get_pagesize();
	size_t numpages = (bytes+pagesize-1)/pagesize;

	void* block = VirtualAlloc(NULL, numpages*pagesize, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	return block;
}

bool rwxfree(void* block){
	if (!block) return false;
	return VirtualFree(block, 0, MEM_RELEASE);
}