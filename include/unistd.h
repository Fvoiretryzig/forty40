#ifndef __UNISTD_H__
#define __UNISTD_H__

#include <os.h>
typedef int *intptr_t;

extern intptr_t program_break;

int brk(void *addr);
void *sbrk(intptr_t increment);

#endif
