#include<test.h>

  /*=======================================================*/
 /*====================test for thread====================*/
/*=======================================================*/
extern spinlock_t lk;
spinlock_t lk;
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
	while(1){
		kmt->spin_lock(&lk);
		/*========================random========================*/
		int random_fd = vfs->open("/dev/random", O_RDONLY);
		printf("dev:random fd:%d\n", random_fd);
		size = vfs->read(random_fd, buf, 0);
		if(size < 0){
			printf("dev:error read /dev/random in dev_test\n");
			vfs->close(random_fd);
			continue;
		}
		printf("dev:this is the random number return by /dev/random:%s size:%d\n", buf, size);
		size = vfs->read(random_fd, buf, 0);
			if(size < 0){
			printf("dev:error read /dev/random in dev_test\n");
			vfs->close(random_fd);
			continue;
		}
		printf("dev:this is the random number return by /dev/random:%s size:%d\n\n", buf, size);
		/*========================null========================*/
		int null_fd = vfs->open("/dev/null", O_RDWR);
			printf("dev:null fd:%d\n", null_fd);
		strcpy(buf, "40404040");
		size = vfs->write(null_fd, buf, strlen(buf));
		if(size < 0){
			printf("dev:error write /dev/null\n");
			vfs->close(null_fd);
			continue;
		}
		printf("dev:this is the writing /dev/null operation return value:%d\n", size);
		size = vfs->read(null_fd, buf, 0);
		if(size < 0){
			printf("dev:error read /dev/null\n");
			vfs->close(null_fd);;
			continue;
		}
		printf("dev:after read /dev/null buf:%d\n\n", *buf);
		vfs->close(null_fd);
		/*========================zero========================*/
		int zero_fd = vfs->open("/dev/zero", O_RDONLY);
		printf("dev:zero fd:%d\n", zero_fd);
		strcpy(buf, "40404040");
		printf("dev:this is buf in zero before read buf:%s\n", buf);
		size = vfs->read(zero_fd, buf, 0);
		if(size < 0){
			printf("dev:error read /dev/zero\n");
			vfs->close(zero_fd);
			continue;
		}	
		printf("dev:after read /dev/zero buf:%d\n\n", *buf);
		vfs->close(zero_fd);
		vfs->close(random_fd);
		kmt->spin_unlock(&lk);
	}		
	pmm->free(buf);
	return;
}
void proc_test()
{
	char *buf = pmm->alloc(1024); int size = 0;
	while(1){
		kmt->spin_lock(&lk);
	/*========================cpuinfo========================*/
		int cpu_fd = vfs->open("/proc/cpuinfo", O_RDONLY);
		if(cpu_fd < 0){
			printf("proc:open cpuinfo error!\n");
			continue;
		}
		printf("proc: the cpu fd is %d\n", cpu_fd);
		size = vfs->read(cpu_fd, buf, 1024);
		if(size < 0){
			printf("proc:read error while read cpuinfo\n");
			vfs->close(cpu_fd);
			continue;
		}
		printf("proc:the read result:\nsize:%d\ncontent:\n%s\n\n", size, buf);
		vfs->close(cpu_fd);
	/*========================meminfo========================*/
		int mem_fd = vfs->open("/proc/meminfo", O_RDONLY);
		if(mem_fd < 0){
			printf("proc:open meminfo error!\n");
			continue;
		}
		printf("proc: the mem fd is %d\n", mem_fd);
		size = vfs->read(mem_fd, buf, 1024);
		if(size < 0){
			printf("proc:read error while read meminfo\n");
			vfs->close(mem_fd);
			continue;
		}
		printf("proc:size:%d\ncontent:\n%s\n\n", size, buf);
		vfs->close(mem_fd);
		
	/*========================procinfo========================*/
		int proc_fd = vfs->open("/proc/0", O_RDONLY);
		if(proc_fd < 0){
			printf("proc:open /proc/0 error!\n");
			continue;
		}
		printf("proc: the process fd is %d\n", proc_fd);
		size = vfs->read(proc_fd, buf, 1024);
		if(size < 0){
			printf("proc:read error while read process\n");
			vfs->close(proc_fd);
			continue;
		}
		printf("proc:size:%d\ncontent:\n%s\n\n", size, buf);
		vfs->close(proc_fd);
		kmt->spin_unlock(&lk);
	}
	return;
}
void kv_test()
{		
	char *buf = pmm->alloc(1024); int size = 0;
	char *name = pmm->alloc(64);		
	strcpy(name, "/forty/40c");
	int fd = vfs->open(name, O_CREATE|O_RDWR);
	vfs->close(fd);
	while(1){
		kmt->spin_lock(&lk);
		fd = vfs->open(name, O_RDWR);
		printf("kv:fd for %s:%d\n", name, fd);
		if(fd < 0){
			printf("kv:open %s error!!\n", name);
			continue;
		}
		strcpy(buf, "forty-forty\nthis is a test for kvdb\n40404040\n");
		size = vfs->write(fd, buf, strlen(buf));
		if(size < 0){
			printf("kv:write %s error!!\n", name);
			vfs->close(fd);
			continue;
		}
		printf("kv:write %s size:%d\n", name, size);
		vfs->lseek(fd, 0, SEEK_SET);
		//fd = vfs->open(name, O_RDWR);
		printf("kv:fd for %s:%d\n", name, fd);
		if(fd < 0){
			printf("kv:open %s error!!\n", name);
			continue;
		}	
		strcpy(buf, " ");
		size = vfs->read(fd, buf, 64);
		if(size < 0){
			printf("kv:read %s error!!\n", name);
			vfs->close(fd);	
			continue;
		}
		printf("kv:read %s size:%d\ncontent:\n%s\n\n", name, size, buf);
		strcpy(buf, "");
		vfs->close(fd);
		kmt->spin_unlock(&lk);	
	}	
	pmm->free(buf); pmm->free(name);
	return;
}
void single_thread_test()
{
	kmt->create(&t1, &dev_test, NULL);
	kmt->create(&t2, &kv_test, NULL);
	kmt->create(&t3, &proc_test, NULL);	
}
void file2()
{
	//kmt->spin_lock(&lk);
	printf("this is file2\n");
	char* buf = pmm->alloc(1024); char* name = pmm->alloc(64);
	int size = 0; int fd = 0; 
	strcpy(name, "/home/forty/4040");
	if(vfs->access(name, F_OK) < 0){
		fd = vfs->open(name, O_CREATE|O_RDWR);
		vfs->close(fd);
	}	
	//kmt->spin_unlock(&lk);
	while(1){
		printf("this is file2 begin");
		kmt->spin_lock(&lk);
		fd = vfs->open(name, O_RDWR);
		int offset = 0;
		printf("file2:fd:%d\n", fd);
		if(fd < 0){
			printf("file2:open %s error!!\n", name);
			continue;
		}		
		strcpy(buf, "this is /home/forty/4040 in file2\n");
		size = vfs->write(fd, buf, strlen(buf));
		if(size < 0){
			printf("file2:write %s error!!\n", name);
			vfs->close(fd);
			continue;
		}		
		printf("file2:first write size:%d\n", size);
		size = vfs->write(fd, buf, strlen(buf));	//写两遍
		if(size < 0){
			printf("file2:write %s error!!\n", name);
			vfs->close(fd);
			continue;
		}
		printf("file2:second write size:%d\n", size);		
		offset = vfs->lseek(fd, 0, SEEK_SET);
		if(offset < 0){
			printf("file2:lseek %s error!!\n", name);
			vfs->close(fd);
			continue;
		}
		printf("file2:first lseek offset:%d\n", offset);
		size = vfs->read(fd, buf, strlen(buf));
		if(size < 0){
			printf("file2:read %s error!!\n", name);
			vfs->close(fd);
			continue;
		}		
		printf("file2:read size:%d\n", size); printf("content:\n%s", buf);
		strcpy(buf, "");	
		offset = vfs->lseek(fd, 0, SEEK_END);
		if(offset < 0){
			printf("file2:lseek %s error!!\n", name);
			vfs->close(fd);
			continue;
		}
		printf("file2:second lseek offset:%d\n", offset);		
		strcpy(buf, "hello world from file2\n");
		size = vfs->write(fd, buf, strlen(buf));	//写两遍
		if(size < 0){
			printf("file2:write %s error!!\n", name);
			vfs->close(fd);
			continue;
		}
		printf("file2:secon write size:%d\n", size);
		offset = vfs->lseek(fd, 0, SEEK_SET);
		if(offset < 0){
			printf("file2:lseek %s error!!\n", name);
			vfs->close(fd);
			continue;
		} 	
		size = vfs->read(fd, buf, 1024);
		if(size < 0){
			printf("file2:read %s error!!\n", name);
			vfs->close(fd);
			continue;
		}		
		printf("file2:read size:%d\n", size); printf("content:\n%s\n", buf);
		strcpy(buf, "");			
		vfs->close(fd);
		kmt->spin_unlock(&lk);
		printf("this is file2 end\n");
	}
	pmm->free(buf); pmm->free(name);
	return;
}
mountpath_t* kvfs_p;
void file1()
{
while(1){
		kmt->spin_lock(&lk);
		printf("this is file1\n");
		char buf[1024]; char name[1024];
		int size = 0; int fd = -1;
		strcpy(name, "/home/forty/4040");
		//kmt->spin_lock(&lk);
		if(vfs->access(name, F_OK) < 0){
			fd = vfs->open(name, O_CREATE|O_RDWR);
			//vfs->close(fd);
		}
		else{
			fd = vfs->open(name, O_RDWR);
		}
		char *s0 = kvfs_p->fs->inode[0]->name;
		char *s = kvfs_p->fs->inode[1]->name;
		printf("file1:buf address:0x%08x inode[0] address:0x%08x inode[1] address:0x%08x\n", buf, s0,s);
		printf("heiheihei\n");
		//printf("file1:before_intr_read():%d\n",_intr_read());
		//printf("file1:this is before yield\n");
		//_yield();
		//printf("file1:after_intr_read():%d\n",_intr_read());
		//printf("file1:this is after yield\n");
	
		//kmt->spin_lock(&lk);
		int offset = 0;
		//fd = vfs->open(name, O_RDWR);
		
		printf("file1:fd:%d\n", fd);
		if(fd < 0){
			printf("file1:open %s error!!\n", name);
			continue;
		}	
		//printf("file1:kvfs_p->fs->inode[0]:%s\n", kvfs_p->fs->inode[0]->name);	
		strcpy(buf, "this is /home/forty/4040\n");
		size = vfs->write(fd, buf, strlen(buf));
		//printf("file1: size:%d\n", size);
		if(size < 0){
			printf("file1:write %s error!!\n", name);
			vfs->close(fd);
			continue;
		}		
		printf("file1:first write size:%d\n", size);
		/*size = vfs->write(fd, buf, strlen(buf));	//写两遍
		if(size < 0){
			printf("file1:write %s error!!\n", name);
			vfs->close(fd);
			continue;
		}
		printf("file1:second write size:%d\n", size);*/		
		printf("file1:_intr_read():%d\n",_intr_read());
		offset = vfs->lseek(fd, 0, SEEK_SET);
		if(offset < 0){
			printf("file1:lseek %s error!!\n", name);
			vfs->close(fd);
			continue;
		}
		size = vfs->read(fd, buf, strlen(buf));
		if(size < 0){
			printf("file1:read %s error!!\n", name);
			vfs->close(fd);
			continue;
		}		
		printf("file1:read size:%d\n", size); printf("content:\n%s", buf);
		/*offset = vfs->lseek(fd, 0, SEEK_SET);
		if(offset < 0){
			printf("file1:lseek %s error!!\n", name);
			vfs->close(fd);
			continue;
		}
		size = vfs->read(fd, buf, strlen(buf)*2);
		if(size < 0){
			printf("file1:read %s error!!\n", name);
			vfs->close(fd);
			continue;
		}
		printf("file1:read size:%d\n", size); printf("content:\n%s\n", buf);*/
		strcpy(buf, "");
		vfs->close(fd);
		printf("file1 end\n\n");
		kmt->spin_unlock(&lk);
		_yield();
	}
	
	return;
}
void file22()
{	
while(1){
		kmt->spin_lock(&lk);
		printf("file22:this is file11\n");
		char buf[1024]; char name[64];
		int size = 0; int fd = -1;
		strcpy(name, "/home/4040");
		if(vfs->access(name, F_OK) < 0){
			printf("file22:hahah create!!!\n");
			fd = vfs->open(name, O_CREATE|O_RDWR);
		}
		else{
			printf("file22:xixi not create!!\n");
			fd = vfs->open(name, O_RDWR);
		}
		
		int offset = 0;
		printf("file22:fd:%d\n", fd);
		if(fd < 0){
			printf("file22:open %s error!!\n", name);
			continue;
		}		
		strcpy(buf, "this is /home/4040\n");
		size = vfs->write(fd, buf, strlen(buf));
		printf("file22:size:%d\n", size);
		if(size < 0){
			printf("file22:write %s error!!\n", name);
			vfs->close(fd);
			continue;
		}		
		printf("file22:first write size:%d\n", size);
		offset = vfs->lseek(fd, 0, SEEK_SET);
		if(offset < 0){
			printf("file22:lseek %s error!!\n", name);
			vfs->close(fd);
			continue;
		}
		size = vfs->read(fd, buf, strlen(buf));
		if(size < 0){
			printf("file22:read %s error!!\n", name);
			vfs->close(fd);
			continue;
		}		
		printf("file22:read size:%d\n", size); printf("content:\n%s", buf);
		strcpy(buf, "");
		vfs->close(fd);
		printf("file22:end\n\n");pmm->free(buf); pmm->free(name);
		kmt->spin_unlock(&lk);
		_yield();
	}
	
	return;
}
void file11()
{	
while(1){
		kmt->spin_lock(&lk);
		printf("file11:this is file11\n");
		char buf[1024]; char name[64];
		int size = 0; int fd = -1;
		strcpy(name, "/home/vier");
		if(vfs->access(name, F_OK) < 0){
			printf("file11:hahah create!!!\n");
			fd = vfs->open(name, O_CREATE|O_RDWR);
		}
		else{
			printf("file11:xixi not create!!\n");
			fd = vfs->open(name, O_RDWR);
		}
		
		int offset = 0;
		printf("file11:fd:%d\n", fd);
		if(fd < 0){
			printf("file11:open %s error!!\n", name);
			continue;
		}		
		strcpy(buf, "this is /home/vier\n");
		size = vfs->write(fd, buf, strlen(buf));
		printf("file11:size:%d\n", size);
		if(size < 0){
			printf("file11:write %s error!!\n", name);
			vfs->close(fd);
			continue;
		}		
		printf("file11:first write size:%d\n", size);
		offset = vfs->lseek(fd, 0, SEEK_SET);
		if(offset < 0){
			printf("file11:lseek %s error!!\n", name);
			vfs->close(fd);
			continue;
		}
		size = vfs->read(fd, buf, strlen(buf));
		if(size < 0){
			printf("file11:read %s error!!\n", name);
			vfs->close(fd);
			continue;
		}		
		printf("file11:read size:%d\n", size); printf("content:\n%s", buf);
		strcpy(buf, "");
		vfs->close(fd);
		printf("file11:end\n\n");pmm->free(buf); pmm->free(name);
		kmt->spin_unlock(&lk);
		_yield();
	}
	return;
}
void file11fuben()
{	
while(1){
		kmt->spin_lock(&lk);
		printf("file11fuben:this is file11\n");
		char buf[1024]; char name[64];
		int size = 0; int fd = -1;
		strcpy(name, "/home/vier");
		if(vfs->access(name, F_OK) < 0){
			printf("file11fuben:hahah create!!!\n");
			fd = vfs->open(name, O_CREATE|O_RDWR);
		}
		else{
			printf("file11fuben:xixi not create!!\n");
			fd = vfs->open(name, O_RDWR);
		}
		
		int offset = 0;
		printf("file11fuben:fd:%d\n", fd);
		if(fd < 0){
			printf("file11fuben:open %s error!!\n", name);
			continue;
		}		
		strcpy(buf, "this is /home/vier\n");
		size = vfs->write(fd, buf, strlen(buf));
		printf("file11fuben:size:%d\n", size);
		if(size < 0){
			printf("file11fuben:write %s error!!\n", name);
			vfs->close(fd);
			continue;
		}		
		printf("file11fuben:first write size:%d\n", size);
		offset = vfs->lseek(fd, 0, SEEK_SET);
		if(offset < 0){
			printf("file11fuben:lseek %s error!!\n", name);
			vfs->close(fd);
			continue;
		}
		size = vfs->read(fd, buf, strlen(buf));
		if(size < 0){
			printf("file11fuben:read %s error!!\n", name);
			vfs->close(fd);
			continue;
		}		
		printf("file11fuben:read size:%d\n", size); printf("content:\n%s", buf);
		strcpy(buf, "");
		vfs->close(fd);
		printf("file11fuben:end\n\n");pmm->free(buf); pmm->free(name);
		kmt->spin_unlock(&lk);
		_yield();
	}
	return;
}
spinlock_t lk_multhread;
void multi_thread_test()
{
	kmt->spin_lock(&lk_multhread);
	kmt->create(&t4, &file22, NULL);
	kmt->create(&t5, &file11, NULL);
	kmt->create(&t6, &file11fuben, NULL);
	kmt->spin_unlock(&lk_multhread);
}
void test_file()
{
	kmt->spin_init(&lk, "filetest_lk");
	kmt->spin_init(&lk, "multithread_lk");
	//single_thread_test();
	multi_thread_test();
	return;
}


