#ifndef __TEST_H__
#define __TEST_H__

#include <kernel.h>

sem_t empty, fill;
thread_t t1, t2,t3,t4,t5,t6,t7, t8 ,t9, t10,t11,t12,t13,t14,t15,t16;
#define BUF_SIZE 4

thread_t work[T_max];
int thread_cnt;
int current_id;
int last_thread;

void producer();
void consumer();
spinlock_t lk;
void test_run();
 
