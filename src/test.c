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
	char *buf = pmm->alloc(1024); int size = 0;
	/*========================random========================*/
	int random_fd = vfs->open("/dev/random", O_RDONLY);
	printf("random fd:%d\n", random_fd);
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
	printf("null fd:%d\n", null_fd);
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
	printf("after read /dev/null buf:%d\n", *buf);
	vfs->close(null_fd);
	/*========================zero========================*/
	int zero_fd = vfs->open("/dev/zero", O_RDONLY);
	printf("zero fd:%d\n", zero_fd);
	strcpy(buf, "40404040");
	printf("this is buf in zero before read buf:%s\n", buf);
	size = vfs->read(zero_fd, buf, 0);
	if(size < 0){
		printf("error read /dev/zero\n");
		vfs->close(zero_fd);
		pmm->free(buf);
		return;
	}	
	printf("after read /dev/zero buf:%d\n", *buf);
	vfs->close(zero_fd);
	vfs->close(random_fd);
	pmm->free(buf);
	printf("this is checkpoint\n");
	return;
}
void proc_test()
{
	return;
}
void kv_test()
{
	char *buf = pmm->alloc(1024); int size = 0;
	char *name = pmm->alloc(128);
	strcpy(name, "/forty/40c");
	int fd = vfs->open(name, O_CREATE|O_RDWR);
	printf("fd for %s:%d\n", name, fd);
	if(fd < 0){
		printf("open %s error!!\n", name);
		return;
	}
	strcpy(buf, "forty-forty\nthis is a test for kvdb\n40404040\n");
	size = vfs->write(fd, buf, strlen(buf));
	if(size < 0){
		printf("write %s error!!\n", name);
		vfs->close(fd);
		pmm->free(buf); pmm->free(name);
		return;
	}
	printf("write %s size:%d\n", name, size);
	vfs->close(fd);
	fd = vfs->open(name, O_RDWR);
	printf("fd for %s:%d\n", name, fd);
	if(fd < 0){
		printf("open %s error!!\n", name);
		return;
	}	
	strcpy(buf, " ");
	size = vfs->read(fd, buf, 128);
	if(size < 0){
		printf("read %s error!!\n", name);
		vfs->close(fd);
		pmm->free(buf); pmm->free(name);		
		return;
	}
	printf("read %s size:%d\nread content:\n%s", name, size, buf);
	
	return;
}

void test_file()
{
	//kmt->spin_init(&lk,"test_file_lk");
	//kmt->spin_lock(&lk);
	
	kmt->create(&t1, &dev_test, NULL);kmt->create(&t2, &kv_test, NULL);
	//kmt->create(&t3, &proc_test, NULL);
	
	
	//kmt->spin_unlock(&lk);
	return;
}

