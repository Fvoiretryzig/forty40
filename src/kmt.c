#include <os.h>
#include<libc.h>

#define STK_SZ 8192
#define FC_SZ 32
#define T_max 20

//static struct thread_node* work_head;

typedef struct thread thread_t;
typedef struct spinlock spinlock_t;
typedef struct semaphore sem_t;
static void kmt_init();
static int create(thread_t *thread, void (*entry)(void *arg), void *arg);
static void teardown(thread_t *thread);
static thread_t* schedule();
static void spin_init(spinlock_t *lk, const char *name);
static void spin_lock(spinlock_t *lk);
static void spin_unlock(spinlock_t *lk);
static void sem_init(sem_t *sem, const char *name, int value);
static void sem_wait(sem_t *sem);
static void sem_signal(sem_t *sem);

extern thread_t work[T_max];
extern int thread_cnt;
extern int current_id;


MOD_DEF(kmt) 
{
	.init = kmt_init,
	.create = create,
	.teardown = teardown,
	.schedule = schedule,
	.spin_init = spin_init,
	.spin_lock = spin_lock,
	.spin_unlock = spin_unlock,
	.sem_init = sem_init,
	.sem_wait = sem_wait,
	.sem_signal = sem_signal,
};

spinlock_t create_lk;
spinlock_t sem_lk;
thread_t work[T_max];
int thread_cnt;
int current_id;

static void kmt_init()
{
	spin_init(&create_lk, "create_lk");
	spin_init(&sem_lk, "sem_lk");
	for(int i = 0; i<T_max; i++)
		work[i].id = -1;
	current_id = 0;
	thread_cnt = 0;
}
  /*===================================*/
 /*==========deal with thread=========*/
/*===================================*/
static int create(thread_t *thread, void (*entry)(void *arg), void *arg)
{
	//spin_lock(&create_lk);
	void *fence1_addr = pmm->alloc(FC_SZ);
	void *addr = pmm->alloc(STK_SZ);
	void *fence2_addr = pmm->alloc(FC_SZ);
	if(addr && fence1_addr && fence2_addr){
		printf("/*=====in kmt.c 58line create()====*/\nfence1:0x%08x addr:0x%08x fence2:0x%08x\n", 
				fence1_addr, addr, fence2_addr);
		thread->id = thread_cnt;
		thread->fence1 = fence1_addr;
		thread->stack = addr;
		thread->fence2 = fence2_addr;
		int id = thread->id;
		for(int i = 0; i<32; i++){
			thread->fence1[i] = id;
			thread->fence2[i] = id;
		}
		_Area stack;
		stack.start = thread->stack; stack.end = thread->stack + STK_SZ;
		thread->thread_reg = _make(stack, entry, arg);
		work[thread_cnt] = *thread;
		printf("/*=====in kmt.c 80line create()====*/\n id:%d work[thread_cnt]:0x%08x ", work[thread_cnt].id, work[thread_cnt]);	
		thread_cnt++;
		//!@#$printf("eip:0x%08x\n", thread->thread_reg->eip);	
		//spin_unlock(&create_lk);
		return 0;
	}
	return -1;
}
static void teardown(thread_t *thread)
{
	void* addr = thread->stack;
	void* fence1_addr = thread->fence1;
	void* fence2_addr = thread->fence2;
	if(addr && fence1_addr && fence2_addr){
		if(current_id == thread->id){
			current_id = (current_id+1)%thread_cnt;
		}
		work[thread->id].id = -1;
		for(int i = thread->id; i<thread_cnt-1; i++)
			work[i] = work[i+1];
		work[thread_cnt].id = -1;
		thread_cnt--;
		pmm->free(addr); pmm->free(fence1_addr); pmm->free(fence2_addr);
	}
	return;
}
static thread_t* schedule()
{		
	printf("\nthis is in schedule!!!!\n");
	int old_id = current_id;
	printf("id:%d ", work[old_id].id);
	//!@#$printf("thread stack:0x%08x ", work[old_id].stack);
	printf("eip 0x%08x:0x%08x\n",&work[old_id].thread_reg->eip, work[old_id].thread_reg->eip);
	current_id = (current_id+1)%thread_cnt;
	
	printf("\n");
	return &work[old_id];
}
  /*===================================*/
 /*=========deal with spinlock========*/
/*===================================*/
static void spin_init(spinlock_t *lk, const char *name)
{
	lk->locked = 0;	//未被占用的锁
	int len = strlen(name);
	strncpy(lk->name, name, len);
	return;
}
static void spin_lock(spinlock_t *lk)
{
	//printf("%s in lock intr:%d\n", lk->name, _intr_read());
	if(_intr_read())
		_intr_write(0);		//关中断
	//printf("%s in lock intr:%d\n",lk->name,  _intr_read());
	while(_atomic_xchg(&lk->locked, 1) != 0);
}
static void spin_unlock(spinlock_t *lk)
{
	_atomic_xchg(&lk->locked, 0);
	_intr_write(1);
	//printf("%s in unlock intr:%d\n", lk->name, _intr_read());
}
  /*===================================*/
 /*========deal with semaphore========*/
/*===================================*/

static void sem_init(sem_t *sem, const char *name, int value)
{
	sem->count = value;
	int len = strlen(name);
	strncpy(sem->name, name ,len);
	//!@#$printf("name:%s\n", sem->name);
	sem->queue = pmm->alloc(sizeof(struct queue_node));
	sem->queue->if_in = 0; sem->queue->next = NULL; sem->queue->prev = NULL;
	return;
}
static void sem_wait(sem_t *sem)
{
	spin_lock(&sem_lk);
	//!@#$printf("\nthis is in %s sem_wait!!!!\n", sem->name);
	sem->count--;
	//printf("/*=====in kmt.c 128line sem_wait()====*/sem->name:%s count:%d\n", sem->name,sem->count);
	if(sem->count < 0){
		//printf("/*=====in kmt.c 128line sem_wait() in if_sleep====*/\nsem->name:%s\n", sem->name);
		sem->count++;
		struct queue_node* current_node = pmm->alloc(sizeof(struct queue_node));
		//int if_vacant = 0;
		current_node = sem->queue;
		//!@#$printf("next is for: ");		
		for(; current_node->next; current_node = current_node->next){
			//!@#$printf("0x%08x ", current_node);
			if(!current_node->if_in){	//找到最前面的那个node
				//if_vacant = 1; 
				break;
			}
		}
		//!@#$printf("\n");
		if(!current_node->if_in){
			current_node->if_in = 1;
		}
		else{
			struct queue_node* add_node = pmm->alloc(sizeof(struct queue_node));
			add_node->prev = NULL; add_node->next = sem->queue; add_node->if_in = 1;
			sem->queue->prev = add_node;
			sem->queue = add_node;		
			//!@#$printf("add node:0x%08x\n", add_node);	
		}	
		struct queue_node* last_node = pmm->alloc(sizeof(struct queue_node));
		last_node = sem->queue;
		while(last_node->next){
			if(last_node->if_in)
				break;
			last_node = last_node->next;
		}
		//!@#$printf("kmt wait 221 last_node:0x%08x last_node->if_in:%d\n",last_node, last_node->if_in);
		spin_unlock(&sem_lk);
		while(last_node->if_in){
			//printf("last_node:0x%08x\n", last_node);
			printf("");
		}
		spin_lock(&sem_lk);
		sem->count--;
		if(last_node->prev || last_node->next){
			if(last_node->prev){
				last_node->prev->next = last_node->next;
				last_node->prev = NULL;
			}
			if(last_node->next){
				last_node->next->prev = last_node->prev;
				last_node->next = NULL;
			}
		}
		pmm->free(last_node);
		//printf("name:%s while(sem->queue[i])\n", sem->name);
	}
	//!@#$printf("/*=====in kmt.c 188line sem_wait()====*/\nsem->name:%s sem->count:%d\n", sem->name, sem->count);
	spin_unlock(&sem_lk);
	
	return;
}
static void sem_signal(sem_t *sem)
{
	spin_lock(&sem_lk);
	//!@#$printf("\nthis is in %s sem_signal!!!!!\n", sem->name);
	sem->count++;
	//printf("name:%s sem->count++;\ncount:%d\n", sem->name, sem->count);
	//printf("/*=====in kmt.c 128line sem_signal()====*/sem->name:%s\n", sem->name);
	struct queue_node* last_node = pmm->alloc(sizeof(struct queue_node));
	int if_occupied = 0;
	last_node = sem->queue;
	for(;last_node; last_node = last_node->next){
		if(last_node->if_in){
			if_occupied = 1;
			break;
		}
	}
	if(if_occupied){
		last_node = sem->queue;
		while(last_node->next){
			if(last_node->if_in)
				break;
			last_node = last_node->next;
		}
		//!@#$printf("kmt signal 253 last_node:0x%08x last_node->if_in:%d\n",last_node, last_node->if_in);
		last_node->if_in = 0;
		//printf("/*=====in kmt.c 128line sem_signal() in if_sleep====*/\nsem->name:%s\n", sem->name);
	}
	
	//!@#$printf("/*=====in kmt.c 203line sem_signal()====*/\nsem->name:%s sem->count:%d\n\n", sem->name, sem->count);
	spin_unlock(&sem_lk);
	return;
}

