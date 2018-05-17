#include <os.h>
#include <libc.h>

sem_t empty, fill;
thread_t t1, t2;//, t3,t4,t5, t6, t7, t8 ,t9, t10,t11,t12,t13,t14,t15,t16;
#define BUF_SIZE 4

static void producer() {
	while (1) {
		//printf("before p1 t1 id:%d eip:0x%08x\n", t1.id, t1.thread_reg->eip);
		printf("point p1\n");
		kmt->sem_wait(&empty);
		//printf("before p2 t1 id:%d eip:0x%08x\n", t1.id, t1.thread_reg->eip);
		printf("point p2\n");
		printf("(");
		printf("point p3\n");
		kmt->sem_signal(&fill);
	}
}
static void consumer() {
	while (1) {
		printf("before c1 t2 id:%d eip:0x%08x\n", t2.id, t2.thread_reg->eip);
		printf("point c1\n");
		kmt->sem_wait(&fill);
		printf("before c2 t2 id:%d eip:0x%08x\n", t2.id, t2.thread_reg->eip);
		printf("point c2\n");
		printf(")");
		printf("point c3\n");
		kmt->sem_signal(&empty);
	}
}
//thread_t t1; thread_t t2;
spinlock_t lk;
static void test_run() {
	kmt->spin_init(&lk,"intestrun");
	kmt->spin_lock(&lk);
	kmt->sem_init(&empty, "empty", BUF_SIZE);
	kmt->sem_init(&fill, "fill", 0);
	
  	//printf("before create t1\n");
  	kmt->create(&t1, &producer, NULL);
  	kmt->create(&t2, &consumer, NULL);
  	//printf("before create t2\n");
  	//kmt->create(&t3, &producer, NULL);
  	//printf("before create t3\n");
  	//kmt->create(&t4, &consumer, NULL);	
  	//kmt->create(&t5, &producer, NULL);
  	//kmt->create(&t6, &consumer, NULL);
  	//kmt->create(&t7, &consumer, NULL);
  	//kmt->create(&t8, &producer, NULL);
  	//kmt->create(&t9, &consumer, NULL);
  	//kmt->create(&t10, &consumer, NULL);
	//kmt->create(&t11, &producer, NULL);
  	//kmt->create(&t12, &consumer, NULL);
  	//kmt->create(&t13, &consumer, NULL);
  	//kmt->create(&t14, &producer, NULL);
  	//kmt->create(&t15, &consumer, NULL);
  	//kmt->create(&t16, &consumer, NULL);
  	kmt->spin_unlock(&lk);
  	//printf("in test run _intr_read():%d\n",_intr_read());
/*	printf("before create t1\n");
  	kmt->create(&t1, &producer, NULL);
  	printf("before create t2\n");
  	kmt->create(&t2, &producer, NULL);
  	printf("before create t3\n");
  	kmt->create(&t3, &consumer, NULL);*/
//  	kmt->create(&t4, &consumer, NULL);
//  	kmt->create(&t5, &producer, NULL);
//  	kmt->create(&t6, &producer, NULL);
//  	kmt->create(&t7, &producer, NULL);
//  	kmt->create(&t8, &consumer, NULL);
//  	kmt->create(&t9, &consumer, NULL);
//  	kmt->create(&t10, &consumer, NULL);
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
int point, thread_num;
thread_t* work[T_max];
static _RegSet *os_interrupt(_Event ev, _RegSet *regs) {
	if(point == 0){
		work[thread_num-1]->thread_reg = regs;
	}
	else work[point-1]->thread_reg = regs;
	
	if(ev.event == _EVENT_IRQ_TIMER){
		//printf("this is irq_timer\n");
		//printf("in os_interrupt _intr_read():%d\n",_intr_read());
		//if(_intr_read()){
		thread_t* t = kmt->schedule();
		//printf("t:%d\n", t->id);
		regs = t->thread_reg;		
		//}

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
		//	printf("request trap into kernal...\n");
		thread_t* t = kmt->schedule();
		//printf("hahaha\n");
		//printf("t:%d\n", t->id);
		regs = t->thread_reg;		
		//__asm__ __volatile__("int $0x81"); 
	}
		
	if(ev.event == _EVENT_SYSCALL){
		printf("call system...\n");
		__asm__ __volatile__("int $0x80"); 
	}
		
	
/*_EVENT_PAGEFAULT: 缺页异常，其中ev.cause存放异常发生的原因(_PROT_NONE: 页未被映射;_PROT_READ: 读取时出错; _PROT_WRITE: 写入时出错; _PROT_EXEC: 执行时出错)，ev.ref存放访问的地址。*/

  //return NULL; // this is allowed by AM
  return regs;
}
