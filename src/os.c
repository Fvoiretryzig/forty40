#include <os.h>
#include <libc.h>
#include <test.h>
//sem_t empty, fill;
//thread_t t1, t2,t3,t4,t5,t6,t7, t8 ,t9, t10,t11,t12,t13,t14,t15,t16;
//#define BUF_SIZE 4
#define T_max 20

thread_t work[T_max];
int thread_cnt;
int current_id;
int last_thread;
/*static void producer() {
	while (1) {
		kmt->sem_wait(&empty);
		printf("(");
		kmt->sem_signal(&fill);
	}
}
static void consumer() {
	while (1) {
		kmt->sem_wait(&fill);
		printf(")");
		kmt->sem_signal(&empty);
	}
}
spinlock_t lk;
static void test_run() {
	kmt->spin_init(&lk,"intestrun");
	kmt->spin_lock(&lk);
	kmt->sem_init(&empty, "empty", BUF_SIZE);
	kmt->sem_init(&fill, "fill", 0);
	
  	kmt->create(&t1, &producer, NULL);
  	kmt->create(&t2, &consumer, NULL);
  	kmt->create(&t3, &producer, NULL);
  	kmt->create(&t4, &consumer, NULL);	
  	kmt->create(&t5, &producer, NULL);
  	kmt->create(&t6, &consumer, NULL);
  	kmt->create(&t7, &consumer, NULL);
  	kmt->create(&t8, &producer, NULL);
  	kmt->create(&t9, &consumer, NULL);
  	kmt->create(&t10, &consumer, NULL);
	kmt->create(&t11, &producer, NULL);
  	kmt->create(&t12, &consumer, NULL);
  	kmt->create(&t13, &consumer, NULL);
  	kmt->create(&t14, &producer, NULL);
  	kmt->create(&t15, &consumer, NULL);
  	kmt->create(&t16, &consumer, NULL);
  	kmt->spin_unlock(&lk);
}*/

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
  last_thread = -1;

}

static void os_run() {

  _intr_write(1); // enable interrupt
  test_run();
  while (1) ; // should never return
}

static _RegSet *os_interrupt(_Event ev, _RegSet *regs) {
	if(last_thread != -1){
		work[last_thread].thread_reg = regs;
	}
	thread_t* t = kmt->schedule();
	last_thread = t->id;
	if(ev.event == _EVENT_IRQ_TIMER){
		//!@#$printf("this is irq_timer\n");
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
		printf("call system...\n");
		__asm__ __volatile__("int $0x80"); 
	}
  return t->thread_reg;
}
