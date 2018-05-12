#include <os.h>
#include <libc.h>

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
  //for (const char *p = "Hello, OS World!\n"; *p; p++) {
  //  _putc(*p);
  //}
  printf("Hello, OS World!\n");
  printf("heap start:0x%08x heap end:0x%08x\n", _heap.start, _heap.end);
}

static void os_run() {
  _intr_write(1); // enable interrupt
  while (1) ; // should never return
}

static _RegSet *os_interrupt(_Event ev, _RegSet *regs) {
	if(ev.event == _EVENT_IRQ_TIMER){
		//printf("this is irq_timer\n");
		 _asye_init((ev, regs));
	}//时钟中断；
	if(ev.event == _EVENT_IRQ_IODEV) 
		_putc('I');	//设备中断；
	if(ev.event == _EVENT_ERROR) {
		_putc('x');
		_halt(1);
	}//其他异常；
	if(ev.event == _EVENT_PAGEFAULT)
		printf("this is pagefault...\n");
	if(ev.event == _EVENT_YIELD)
		printf("request trap into kernal...\n");
	if(ev.event == _EVENT_SYSCALL)
		printf("call system???????\n");
	
/*_EVENT_PAGEFAULT: 缺页异常，其中ev.cause存放异常发生的原因(_PROT_NONE: 页未被映射;_PROT_READ: 读取时出错; _PROT_WRITE: 写入时出错; _PROT_EXEC: 执行时出错)，ev.ref存放访问的地址。*/

  //return NULL; // this is allowed by AM
  return regs;
}
