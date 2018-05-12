#include <os.h>
#include <libc.h>

sem_t *empty; sem_t* fill;
thread_t* t1; thread_t* t2;

#define BUF_SIZE 3

static void producer() {
	while (1) {
		kmt->sem_wait(empty);
		printf("(");
		kmt->sem_signal(fill);
	}
}
static void consumer() {
	while (1) {
		kmt->sem_wait(fill);
		printf(")");
		kmt->sem_signal(empty);
	}
}
//thread_t t1; thread_t t2;
static void test_run() {
	kmt->sem_init(empty, "empty", BUF_SIZE);
	kmt->sem_init(fill, "fill", 0);
  	kmt->create(t1, &producer, NULL);
  	kmt->create(t2, &consumer, NULL);
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

static _RegSet *os_interrupt(_Event ev, _RegSet *regs) {
	if(ev.event == _EVENT_IRQ_TIMER){
		//printf("this is irq_timer\n");
		thread_t* t = kmt->schedule();
		printf("haha\n");
		regs = t->thread_reg;
	}//时钟中断???????????；
	if(ev.event == _EVENT_IRQ_IODEV){
		printf("this is _EVENT_IRQ_IODEV\n");
	}//设备中断；
	if(ev.event == _EVENT_ERROR) {
		printf("this is _EVENT_ERROR\n");
		_halt(1);
	}//其他异常；
	if(ev.event == _EVENT_PAGEFAULT){
		printf("this is pagefault...\n");
	}
		
	if(ev.event == _EVENT_YIELD){
		printf("request trap into kernal...\n");
	}
		
	if(ev.event == _EVENT_SYSCALL){
		printf("call system???????\n");
	}
		
	
/*_EVENT_PAGEFAULT: 缺页异常，其中ev.cause存放异常发生的原因(_PROT_NONE: 页未被映射;_PROT_READ: 读取时出错; _PROT_WRITE: 写入时出错; _PROT_EXEC: 执行时出错)，ev.ref存放访问的地址。*/

  //return NULL; // this is allowed by AM
  return regs;
}
