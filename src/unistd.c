#include<unistd.h>
#include<os.h>

//应该无关物理地址虚拟地址的转换……吧
int brk(void *addr)
{
	if(addr >= program_break && addr < _heap.end){
		program_break = addr;
		return 0;
	}
	return -1;
}
void *sbrk(intptr_t increment)	
{
	if(!increment)
		return program_break;
	intptr_t old_program_break = program_break;
	if(increment + old_program_break < _heap.end){
		program_break += increment;
		return old_program_break;
	}
	return -1;
}
