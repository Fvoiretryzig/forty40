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

static void kmt_init()
{
	work_head = pmm->alloc(12);
	if(work_head == NULL)
		_halt(1);
	//TODO();
}
  /*===================================*/
 /*==========deal with thread=========*/
/*===================================*/
static int create(thread_t *thread, void (*entry)(void *arg), void *arg)
{
	//printf("thread:0x%08x\n", thread);
	void *fence1_addr = pmm->alloc(FC_SZ);
	void *addr = pmm->alloc(STK_SZ);
	void *fence2_addr = pmm->alloc(FC_SZ);
	if(addr && fence1_addr && fence2_addr){
		printf("fence1:0x%08x addr:0x%08x fence2:0x%08x\n", fence1_addr, addr, fence2_addr);
		struct thread_node* current = pmm->alloc(sizeof(struct thread_node));
		if(current){
			thread->id = ++current->t->id;
		}
		else thread->id = 1;
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
		//printf("current:0x%08x thread in create: 0x%08x\n",current, current->t);
		//printf("thread:0x%08x current->t:0x%08x\n", thread, current->t);
		current->next = work_head; work_head->prev = current; current->prev = NULL;
		work_head = current;
		printf("in kmt.c 75lines: work_head:0x%08x\n", work_head);
		//printf("current:0x%08x\n", current);
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
	struct thread_node* current = pmm->alloc(sizeof(struct thread_node));
	current = work_head;

	if(current == NULL)
		return NULL;
	int i = 0;
	while(current->next){
		
		current = current->next;
		printf("current->t:0x%08x i:%d\n", (struct thread_t*)current->t, i);
		i++;
	}
	printf("i:%d\n", i);
		
	current->prev->next = NULL;
	current->prev = NULL; current->next = work_head;
	work_head = current;	//把处理了的任务放置最前
	printf("this is schedule ");
	printf("current->t:0x%08x work_head:0x%08x\n", current->t,work_head->t);
	struct thead_t* temp;
	temp = (struct thead_t*)work_head->t;
	printf("temp:0x%08x\n", temp);
	pmm->free(current->t);
	printf("temp after free:0x%08x\n", temp);
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
static void sem_init(sem_t *sem, const char *name, int value)
{
	sem->count = value;
	int len = strlen(sem->name);
	strncpy(sem->name, name ,len);
	for(int i = 0; i<20; i++)
		sem->queue[i] = 0;
	sem->if_sleep = 0;
	return;
}
static void sem_wait(sem_t *sem)
{
	sem->count--;
	if(sem->count < 0){
		sem->if_sleep = 1;
		int i = 0;
		while(sem->queue[i])
			i++;
		sem->queue[i] = 1;
		while(sem->queue[i]);
	}
	return;
}
static void sem_signal(sem_t *sem)
{
	sem->count++;
	if(sem->if_sleep){
		int i = 0;
		while(sem->queue[i+1])
			i++;
		sem->queue[i] = 0;
	}
	return;
}
/*#include <os.h>

MOD_DEF(kmt) 
{
};*/
