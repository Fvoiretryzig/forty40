#include <os.h>
#include<libc.h>

extern int id;

struct thread	
{
	int id;
	void*  stackaddr;
	size_t  stacksize;
	_RegSet *thread_reg;
};
struct thread_node
{
	struct thread* t;
	struct thread_node* next;
	struct thread_node* prev;
};
struct thread_node* work_head;

struct spinlock
{
	int locked;	//判断锁是否被占用了
	char name[64];
};

struct semaphore
{
	int count;
	int if_sleep;
	int queue[20];
	char name[64];
};

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
	id = 0;
	work_head = NULL;
	//TODO();
}

static int create(thread_t *thread, void (*entry)(void *arg), void *arg)
{
	size_t size = 0x10000;
	void *addr = pmm.alloc(size);
	if(addr == NULL)
		return -1;
	else{
		thread->id = id++;
		thread->stackaddr = addr;
		thread->stacksize = size;
		_Area stack;
		stack.start = addr; stack.end = addr + size;
		thread->thread_reg = _make(&stack, entry, arg);
		
		struct thread_node current;
		current->t->id = thread->id; current->t->stackaddr = addr; current->t->stacksize = size;
		current->t->thread_reg = thread->thread_reg; 
		current->next = work_head; work_head->prev = current; current->prev = NULL;
		work_head = current;
		return 0;
	}
	return -1;
}
static void teardown(thread_t *thread)
{
	void* addr = thread->stackaddr;
	if(addr){

		struct thread_node current;
		while(current->t->id != thread->id)
			current = current->next;
		current->prev->next = current->next; current->next->prev = current->prev;
		current->next = NULL; current->prev = NULL;
		
		pmm.free(addr);
		thread->stackaddr = NULL;
		thread->stacksize = 0;
		thread->thread_reg = NULL;
		return;
	}
	return;
}
static thread_t* schedule()
{
	struct thread_node current = work_head;
	if(current == NULL)
		return NULL;
	while(current->next)
		current = current->next;
	current->prev->next = NULL;
	current->prev = NULL; current->next = work_head;
	work_head = current;	//把处理了的任务放置最前
	return current->t->thread_reg;
}
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
	while(_atomic_xchg(&lk->flag, 1) != 0);
}
static void spin_unlock(spinlock_t *lk)
{
	_atomic_xchg(&lk->flag, 0);
	_intr_write(1);
}
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
