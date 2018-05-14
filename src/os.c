#include <os.h>
#include <libc.h>

sem_t empty, fill;
thread_t t1, t2;
extern spinlock_t lk;
#define BUF_SIZE 1

static void producer() {
	while (1) {
		printf("point p1\n");
		kmt->sem_wait(&empty);
		printf("point p2\n");
		printf("(");
		printf("point p3\n");
		kmt->sem_signal(&fill);
		printf("point p4\n");
	}
}
static void consumer() {
	while (1) {
		printf("point c1\n");
		kmt->sem_wait(&fill);
		printf("point c2\n");
		printf(")");
		printf("point c3\n");
		kmt->sem_signal(&empty);
		printf("point c4\n");
	}
}
//thread_t t1; thread_t t2;
static void test_run() {
	kmt->spin_init(&lk, "sem_lk");
	kmt->sem_init(&empty, "empty", BUF_SIZE);
	kmt->sem_init(&fill, "fill", 0);
  	kmt->create(&t1, &producer, NULL);
  	kmt->create(&t2, &consumer, NULL);
  	//printf("t1:0x%08x t2:0x%08x\n", t1.stack, t2.stack);
  // create producers and consumers
}

static void os_init();
static void os_run();
static _RegSet *os_interrupt(_Event ev, _RegSet *regs);

MOD_DEF(os) 
{
	.init = os_init,
	.run = os_run,
	.interrupt = os_interrupt,
};

static void os_init() 
{
  printf("Hello, OS World!\n");
  printf("heap start:0x%08x heap end:0x%08x\n", _heap.start, _heap.end);
}

static void os_run() {
  _intr_write(1); // enable interrupt
  test_run();
  while (1) ; // should never return
}
spinlock_t lk;
static _RegSet *os_interrupt(_Event ev, _RegSet *regs) {
	if(ev.event == _EVENT_IRQ_TIMER){
		//printf("this is irq_timer\n");
		thread_t* t = kmt->schedule();
		//printf("t:%d\n", t->id);
		regs = t->thread_reg;
	}//时钟中断???????????；
	if(ev.event == _EVENT_IRQ_IODEV){
		printf("this is _EVENT_IRQ_IODEV\n");
		thread_t* t = kmt->schedule();
		//printf("t:%d\n", t->id);
		regs = t->thread_reg;		
	}//设备中断；
	if(ev.event == _EVENT_ERROR) {
		printf("this is _EVENT_ERROR\n");
		_halt(1);
	}//其他异常；
	if(ev.event == _EVENT_PAGEFAULT){
		printf("this is pagefault...\n");
		//TODO();
	}
		
	if(ev.event == _EVENT_YIELD){
	kmt->spin_lock(&lk);
		printf("request trap into kernal...\n");
		thread_t* t = kmt->schedule();
		printf("hahaha\n");
		//printf("t:%d\n", t->id);
		regs = t->thread_reg;		
		//__asm__ __volatile__("int $0x81"); 
	kmt->spin_unlock(&lk);
	}
		
	if(ev.event == _EVENT_SYSCALL){
		printf("call system...\n");
		__asm__ __volatile__("int $0x80"); 
	}
		
	
/*_EVENT_PAGEFAULT: 缺页异常，其中ev.cause存放异常发生的原因(_PROT_NONE: 页未被映射;_PROT_READ: 读取时出错; _PROT_WRITE: 写入时出错; _PROT_EXEC: 执行时出错)，ev.ref存放访问的地址。*/

  //return NULL; // this is allowed by AM
  return regs;
}
