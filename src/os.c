#include <os.h>
#include <libc.h>
#include <test.h>

#define T_max 20

thread_t work[T_max];
int thread_cnt;
int current_id;
int last_thread;

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
  //test_file();
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
		//__asm__ __volatile__("int $0x80"); 
	}
  return t->thread_reg;
}
