#include <Windows.h>

#ifndef RWX_PAGE
#define RWX_PAGE

void* rwxalloc(size_t bytes);
bool rwxfree(void*);

inline unsigned get_pagesize() {
	SYSTEM_INFO info;
	GetSystemInfo(&info);
	return static_cast<unsigned>(info.dwPageSize);
}

#endif