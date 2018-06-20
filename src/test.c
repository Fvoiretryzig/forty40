#include<test.h>

  /*=======================================================*/
 /*====================test for thread====================*/
/*=======================================================*/
void producer()
{
	while (1) {
		kmt->sem_wait(&empty);
		printf("(");
		kmt->sem_signal(&fill);
	}
}
void consumer()
{
	while (1) {
		kmt->sem_wait(&fill);
		printf(")");
		kmt->sem_signal(&empty);
	}
}
void test_run()
{
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
}
  /*=======================================================*/
 /*=====================test for file=====================*/
/*=======================================================*/
void dev_test()
{
	/*========================random========================*/
	int random_fd = vfs->open("/dev/random", O_RDONLY);
	char *buf = pmm->alloc(1024);
	int size = 0;
	size = vfs->read(random_fd, buf, 0);
	if(size < 0){
		printf("error read /dev/random in dev_test\n");
		vfs->close(random_fd);
		pmm->free(buf);
		return;
	}
	printf("this is the random number return by /dev/random:%s size:%d\n", buf, size);
	size = vfs->read(random_fd, buf, 0);
	if(size < 0){
		printf("error read /dev/random in dev_test\n");
		vfs->close(random_fd);
		pmm->free(buf);
		return;
	}
	printf("this is the random number return by /dev/random:%s size:%d\n", buf, size);	
	/*========================null========================*/
	int null_fd = vfs->open("/dev/null", O_RDWR);
	strcpy(buf, "40404040");
	size = vfs->write(null_fd, buf, strlen(buf));
	if(size < 0){
		printf("error write /dev/null\n");
		vfs->close(null_fd);
		pmm->free(buf);
		return;
	}
	printf("this is the writing /dev/null operation return value:%d\n", size);
	size = vfs->read(null_fd, buf, 0);
	if(size < 0){
		printf("error read /dev/null\n");
		vfs->close(null_fd);
		pmm->free(buf);
		return;
	}
	printf("after read /dev/null buf:%d\n", (int)*buf);
	vfs->close(null_fd);
	/*========================null========================*/
	int zero_fd = vfs->open("/dev/zero", O_RDONLY);
	strcpy(buf, "40404040");
	size = vfs->read(zero_fd, buf, 0);
	if(size < 0){
		printf("error read /dev/zero\n");
		vfs->close(zero_fd);
		pmm->free(buf);
		return;
	}	
	printf("after read /dev/zero: %d\n", (int)*buf);
	vfs->close(zero_fd);
	pmm->free(buf);
	return;
}
void proc_test()
{
	return;
}
void kv_test()
{
	return;
}

void test_file()
{
	//kmt->spin_init(&lk,"test_file_lk");
	//kmt->spin_lock(&lk);
	kmt->create(&t1, &dev_test, NULL);
	//kmt->create(&t2, &proc_test, NULL);
	//kmt->create(&t3, &kv_test, NULL);
	
	//kmt->spin_unlock(&lk);
	return;
}

