#include <os.h>
#include<libc.h>

#define STK_SZ 0x10000
#define FC_SZ 32

static struct thread_node* work_head;

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
static void kmt_init()
{
	/*work_head = pmm->alloc(12);
	if(work_head == NULL)
		_halt(1);
	work_head->t = NULL; work_head->next = NULL; work_head->prev = NULL;*/
	spin_init(&create_lk, "create_lk");
	work_head = NULL;
	//TODO();
}
  /*===================================*/
 /*==========deal with thread=========*/
/*===================================*/
static int create(thread_t *thread, void (*entry)(void *arg), void *arg)
{
	//printf("/*=====in kmt.c 51line create()====*/\nwork_head:0x%08x work_head->next:0x%08x\n",
			//work_head, work_head->next);
	spin_lock(&create_lk);
	void *fence1_addr = pmm->alloc(FC_SZ);
	void *addr = pmm->alloc(STK_SZ);
	void *fence2_addr = pmm->alloc(FC_SZ);
	if(addr && fence1_addr && fence2_addr){
		printf("/*=====in kmt.c 58line create()====*/\nfence1:0x%08x addr:0x%08x fence2:0x%08x\n", 
				fence1_addr, addr, fence2_addr);
		struct thread_node* current = pmm->alloc(sizeof(struct thread_node));
		if(!work_head){
			thread->id = 1;
		}
		else{
			thread->id = ++work_head->t->id;
		}
		//printf("/*=====in kmt.c 68line create()====*/\nwork_head:0x%08x work_head->next:0x%08x ",
		//	work_head, work_head->next);
		//printf("current:0x%08x current->t:0x%08x\n",current, current->t);
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
		current->t = thread;
		current->next = work_head; work_head->prev = current; current->prev = NULL;
		work_head = current;
		
		printf("/*=====in kmt.c 78line create()====*/\ntid:%d current:0x%08x current->next:0x%08x current->t->id:0x%d\n\n", thread->id, current, current->next, current->t->id);
		//printf("/*=====in kmt.c 80line create()====*/\nwork_head:0x%08x work_head->next:0x%08x\n",
		//	work_head, work_head->next);		
		spin_unlock(&create_lk);
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
		struct thread_node* current = work_head;
		while(current->t->id != thread->id)
			current = current->next;
		current->prev->next = current->next; current->next->prev = current->prev;
		current->next = NULL; current->prev = NULL;
		pmm->free(current);
		pmm->free(fence2_addr);
		pmm->free(addr);
		pmm->free(fence1_addr);
		thread->stack = NULL; thread->fence1 = NULL; thread->fence2 = NULL;
		thread->thread_reg = NULL;
		return;
	}
	return;
}
static thread_t* schedule()
{		
	//printf("kmt113\n");
	struct thread_node* current = pmm->alloc(sizeof(struct thread_node));
	//printf("kmt115\n");
	current = work_head;
	//printf("kmt117\n");
	if(current == NULL){
		return NULL;
		//printf("kmt120\n");
	}
	printf("current:0x%08x current->t->id:%d\n", current, current->t->id);	
	while(current->next){
		current = current->next;
		//printf("kmt126\n");
		printf("/*=====in kmt.c 121line schedule()====*/\ncurrent:0x%08x current->next:0x%08x current->t->id:%d\n", current, current->next, current->t->id);
	}
	//printf("ktm130: work_head:0x%08x current:0x%08x\n", work_head, current);
	 //thread_t* ret = current->t;
	if(current->prev){
		//printf("kmt132 current->t:0x%08x current->prev->t:0x%08x\n",current->t, current->prev->t);
		current->prev->next = NULL; work_head->prev = current;
		current->prev = NULL; current->next = work_head;
		work_head = current;	//把处理了的任务放置最前
		//printf("work_head:0x%08x work_head->next:0x%08x\n", work_head, work_head->next);
	
	}
	printf("current:0x%08x current->t->id:%d\n", current, current->t->id);
	/*if(current->prev){
		current->prev->next = NULL;
		current->prev = NULL;
	}
	pmm->free(current);*/
	//printf("/*=====in kmt.c 128line schedule()====*/\ncurrent:0x%08x current->t:0x%08x\n", current, current->t);	
	printf("ktm141: current->id:%d\n", current->t->id);
	return current->t;
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
	if(_intr_read())
		_intr_write(0);		//关中断
	while(_atomic_xchg(&lk->locked, 1) != 0);
}
static void spin_unlock(spinlock_t *lk)
{
	_atomic_xchg(&lk->locked, 0);
	_intr_write(1);
}
  /*===================================*/
 /*========deal with semaphore========*/
/*===================================*/
//spinlock_t lk;
static void sem_init(sem_t *sem, const char *name, int value)
{
	sem->count = value;
	int len = strlen(name);
	strncpy(sem->name, name ,len);
	printf("name:%s\n", sem->name);
	for(int i = 0; i<100; i++)
		sem->queue[i] = 0;
	return;
}
static void sem_wait(sem_t *sem)
{
	//spin_lock(&lk);
	sem->count--;
	//printf("name:%s sem->count--;\ncount:%d\n", sem->name, sem->count);
	//printf("/*=====in kmt.c 128line sem_wait()====*/sem->name:%s\n", sem->name);
	if(sem->count < 0){
		//printf("/*=====in kmt.c 128line sem_wait() in if_sleep====*/\nsem->name:%s\n", sem->name);
		sem->count++;
		//printf("name:%s sem->count++;\ncount:%d\n", sem->name, sem->count);
		int i = 0;
		while(sem->queue[i]){
			i++;
			//printf("in the sem_wait while sem->name:%s i:%d\n", sem->name, i);
		}	
		sem->queue[i] = 1;
		//printf("name:%s sem->queue[i] = 1 i:%d\n", sem->name,i);
		//printf("sem->name:%s queue: 0:%d 1:%d count:%d\n", sem->name, sem->queue[0], sem->queue[1],sem->count);
		while(sem->queue[i]){
			//printf("name:%s this in while queue[%d]:%d\n", sem->name, i, sem->queue[i]);
			//if(work_head->next)
			//	_yield();
		}
		//printf("name:%s while(sem->queue[i])\n", sem->name);
	}
	//spin_unlock(&lk);
	//printf("/*=====in kmt.c 188line sem_wait()====*/\nsem->name:%s sem->count:%d\n", sem->name, sem->count);
	return;
}
static void sem_signal(sem_t *sem)
{
	//spin_lock(&lk);
	sem->count++;
	//printf("name:%s sem->count++;\ncount:%d\n", sem->name, sem->count);
	//printf("/*=====in kmt.c 128line sem_signal()====*/sem->name:%s\n", sem->name);
	if(sem->queue[0]){
		//printf("/*=====in kmt.c 128line sem_signal() in if_sleep====*/\nsem->name:%s\n", sem->name);
		int i = 0;
		while(sem->queue[i+1]){
			i++;
			//printf("in the sem_signal while\nsem->name:%s i:%d\n", sem->name, i);
		}
		//printf("in signal 200 sem->name:%s queue: 0:%d 1:%d count:%d\n", sem->name, sem->queue[0], sem->queue[1],sem->count);
		sem->queue[i] = 0;
		//if(work_head->next)
		//	_yield();
		//printf("in signal 202 sem->name:%s queue: 0:%d 1:%d count:%d\n", sem->name, sem->queue[0], sem->queue[1],sem->count);
		//printf("name:%s sem->queue[i] = 0 i:%d\n", sem->name,i);
	}
	//spin_unlock(&lk);
	//printf("/*=====in kmt.c 203line sem_signal()====*/\nsem->name:%s sem->count:%d\n", sem->name, sem->count);
	return;
}

