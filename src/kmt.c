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
spinlock_t sem_lk;
static void kmt_init()
{
	spin_init(&create_lk, "create_lk");
	spin_init(&sem_lk, "sem_lk");
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
	//spin_lock(&create_lk);
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
			thread->id = work_head->t->id+1;
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
		
		printf("/*=====in kmt.c 78line create()====*/\ntid:%d current:0x%08x current->next:0x%08x current->t->id:%d current->t:0x%08x\n", thread->id, current, current->next, current->t->id,current->t);
		//printf("/*=====in kmt.c 80line create()====*/\nwork_head:0x%08x work_head->next:0x%08x work_head->next id:%d work_head->next->t:0x%08x\n", work_head, work_head->next,work_head->next->t->id, work_head->next->t);		
		//if(current->next)
		//	printf("current->next->t->id:%d\n",current->next->t->id);
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
	printf("this is in schedule!!!!\n");
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
		//printf("/*=====in kmt.c 121line schedule()====*/\ncurrent:0x%08x current->next:0x%08x current->t->id:%d\n", current, current->next, current->t->id);
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
	//printf("current:0x%08x current->t->id:%d\n", current, current->t->id);
	/*if(current->prev){
		current->prev->next = NULL;
		current->prev = NULL;
	}
	pmm->free(current);*/
	//printf("/*=====in kmt.c 128line schedule()====*/\ncurrent:0x%08x current->t:0x%08x\n", current, current->t);	
	//!!!!printf("ktm141: current->id:%d\n", current->t->id);
	printf("\n");
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
	//printf("%s in lock intr:%d\n", lk->name, _intr_read());
	if(_intr_read())
		_intr_write(0);		//关中断
	printf("%s in lock intr:%d\n",lk->name,  _intr_read());
	while(_atomic_xchg(&lk->locked, 1) != 0);
}
static void spin_unlock(spinlock_t *lk)
{
	_atomic_xchg(&lk->locked, 0);
	_intr_write(1);
	printf("%s in unlock intr:%d\n", lk->name, _intr_read());
}
  /*===================================*/
 /*========deal with semaphore========*/
/*===================================*/

static void sem_init(sem_t *sem, const char *name, int value)
{
	sem->count = value;
	int len = strlen(name);
	strncpy(sem->name, name ,len);
	printf("name:%s\n", sem->name);
	sem->queue = pmm->alloc(sizeof(struct queue_node));
	sem->queue->if_in = 0; sem->queue->next = NULL; sem->queue->prev = NULL;
	return;
}
static void sem_wait(sem_t *sem)
{
	spin_lock(&sem_lk);
	printf("this is in sem_wait!!!!\n");
	sem->count--;
	//printf("name:%s sem->count--;\ncount:%d\n", sem->name, sem->count);
	//printf("/*=====in kmt.c 128line sem_wait()====*/sem->name:%s count:%d\n", sem->name,sem->count);
	if(sem->count < 0){
		//printf("/*=====in kmt.c 128line sem_wait() in if_sleep====*/\nsem->name:%s\n", sem->name);
		sem->count++;
		struct queue_node* current_node = pmm->alloc(sizeof(struct queue_node));
		int if_vacant = 0;
		current_node = sem->queue;
		printf("next if for: ");		
		for(; current_node->next; current_node = current_node->next){
			printf("0x%08x ", current_node);
			if(!current_node->if_in){	//找到最前面的那个node
				if_vacant = 1; 
				break;
			}
		}
		printf("\n");
		if(if_vacant){
			current_node->if_in = 1;
		}
		else{
			struct queue_node* add_node = pmm->alloc(sizeof(struct queue_node));
			add_node->prev = NULL; add_node->next = sem->queue; add_node->if_in = 1;
			sem->queue->prev = add_node;
			sem->queue = add_node;		
			printf("add node:0x%08x\n", add_node);	
		}	
		struct queue_node* last_node = pmm->alloc(sizeof(struct queue_node));
		last_node = sem->queue;
		while(last_node->next){
			if(last_node->if_in)
				break;
			last_node = last_node->next;
		}
		printf("kmt wait 221 last_node->if_in:%d\n", last_node->if_in);
		spin_unlock(&sem_lk);
		while(last_node->if_in){
			printf("last_node:0x%08x\n", last_node);
		}	
		spin_lock(&sem_lk);
		//pmm->free(last_node);
		//printf("name:%s while(sem->queue[i])\n", sem->name);
	}
	spin_unlock(&sem_lk);
	printf("/*=====in kmt.c 188line sem_wait()====*/\nsem->name:%s sem->count:%d\n\n", sem->name, sem->count);
	return;
}
static void sem_signal(sem_t *sem)
{
	spin_lock(&sem_lk);
	printf("this is in sem_signal!!!!!\n");
	sem->count++;
	//printf("name:%s sem->count++;\ncount:%d\n", sem->name, sem->count);
	//printf("/*=====in kmt.c 128line sem_signal()====*/sem->name:%s\n", sem->name);
	if(sem->queue->if_in){
		struct queue_node* last_node = pmm->alloc(sizeof(struct queue_node));
		last_node = sem->queue;
		while(last_node->next){
			if(last_node->if_in)
				break;
			last_node = last_node->next;
		}
		printf("kmt signal 253 last_node->if_in:%d\n", last_node->if_in);
		last_node->if_in = 0;
		//printf("/*=====in kmt.c 128line sem_signal() in if_sleep====*/\nsem->name:%s\n", sem->name);
		
	}
	spin_unlock(&sem_lk);
	printf("/*=====in kmt.c 203line sem_signal()====*/\nsem->name:%s sem->count:%d\n\n", sem->name, sem->count);
	return;
}

