#include <os.h>

#include <unistd.h>
#include <stdint.h>
#include <libc.h>

#define block_size 20
int* program_break;
spinlock_t lk;

struct block 
{
    size_t size;
    struct block *prev; 
    struct block *next; 
    int if_free;     
    void *ptr;    
    char data[1];  
};
static struct block *head = NULL, *tail = NULL;

size_t pmm_align(size_t size)
{
	return ((size>>1) + 1)<<1;
} 
struct block* find(size_t size)
{
	struct block* current = head;
	while(current && (!current->if_free || current->size < size))
		current = current->next;
	return current;
}
struct block* add_block(size_t size)
{
	struct block* current = NULL;
	current = sbrk(0);
	if(sbrk(block_size + size) == (void *)-1)
		return NULL;
	current->next = NULL; current->prev = NULL;
	current->size = size; current->if_free = 0; current->ptr = current->data;
	if(tail){
		current->prev = tail;
		tail->next = current;
		tail = current;		
	}
	return current;
}
void split(struct block* current, size_t size, size_t remain_size)
{
	struct block* remain = current->ptr + size;
	remain->size = remain_size; remain->if_free = 1;
	remain->prev = current; remain->next = current->next; remain->ptr = remain->data;
	current->size = size;
	current->next = remain;
	return;
}
void *malloc_unsafe(size_t size)
{
	struct block *current;
	size_t align_size = pmm_align(size);
	if(!head){	
		current = add_block(align_size);
		if(current){
			head = current;
			tail = current;
		}
		else
			return NULL;	
	}
	else{
		current = find(align_size);
		if(!current){	//if there is no available block
			current = add_block(align_size);
			if(!current)
				return NULL;
		}
		else{
			current->if_free = 0;
			size_t remain_size = current->size - block_size - size;
			if(remain_size > 0)
				split(current, align_size, remain_size);
		}
	}
	return current->ptr;
}

struct block* get_block(void *ptr) 
{
    char *temp;  
    temp = ptr;
    ptr = temp - block_size;
    return ptr;
}
int valid_addr(void *ptr) 
{
    if(head){
        if((uint32_t)ptr > (uint32_t)head && ptr < sbrk(0)){
            return ptr == (get_block(ptr))->ptr;
        }
    }
    return 0;
}
void merge(struct block* current)
{
	struct block* next = current->next;
	current->size += next->size + block_size;
	current->next = next->next;
	//printf("this is merge\n");
}
void free_unsafe(void *ptr)
{
	struct block* current = NULL;
	if(valid_addr(ptr)){
		current = get_block(ptr);
		current->if_free = 1; int if_merge = 0;
		if(current->next){
			if(current->next->if_free){
				merge(current);
				if_merge = 1;
			}
				
		}
		else if(current->prev){
			if(current->prev->if_free){
				merge(current->prev);
				if_merge = 1;
			}
				
		}
		if(!if_merge){
			if(current->next == NULL){//this is the last block
				if(current->prev){
					tail = current->prev;
					current->prev->next = NULL;
					current->prev = NULL;
					int decrement = 0-current->size;
					if(sbrk(decrement) == -1){
						printf("sbrk(decrement) == -1\n");
						return;
					}
				}				
				else{
					head = NULL;
					tail = NULL;
					if(sbrk(decrement) == -1){
						printf("sbrk(decrement) == -1\n");
						return;
					}
				}
			}
			else if(current->prev == NULL){
				if(current->next){
					head = current->nex;
					current->next->prev = NULL;
					current->next = NULL;
					int decrement = 0-current->size;
					if(sbrk(decrement) == -1){
						printf("sbrk(decrement) == -1\n");
						return;
					}
				}
			}
		}
	}
	return;
}
spinlock_t pmm_lk;
static void pmm_init();
static void* pmm_alloc(size_t size);
static void pmm_free(void *ptr); 
MOD_DEF(pmm){
	.init = pmm_init,
	.alloc = pmm_alloc,
	.free = pmm_free,
};
static void pmm_init()
{
	program_break = _heap.start;
	kmt->spin_init(&pmm_lk, "pmmlock");
}
static void* pmm_alloc(size_t size)	//TODO():thread unsafe
{
	kmt->spin_lock(&pmm_lk);
	void* ret = malloc_unsafe(size);
	printf("in pmm alloc:ret address:0x%08x\n", ret);
	kmt->spin_unlock(&pmm_lk);
	return ret;
}
//static void* pmm_free(void *ptr);
static void pmm_free(void *ptr)	//TODO():thread unsafe
{	
	kmt->spin_lock(&pmm_lk);
	free_unsafe(ptr);
	kmt->spin_unlock(&pmm_lk);
	return; 
}


