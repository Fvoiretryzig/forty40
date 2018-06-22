#include <os.h>
#include <vfs.h>
#include<libc.h>


//typedef struct filesystem filesystem_t;
//typedef struct inode inode_t;
//typedef struct file file_t;
static void vfs_init();
static int access(const char *path, int mode);
static int mount(const char *path);
static int unmount(const char *path);
static int open(const char *path, int flags);
static ssize_t read(int fd, void *buf, size_t nbyte);
static ssize_t write(int fd, void *buf, size_t nbyte);
static off_t lseek(int fd, off_t offset, int whence);
static int close(int fd);

static int inode_num_proc;// = 0;
static int inode_num_dev;
static int inode_num_kv;
mountpath_t procfs_p;
mountpath_t devfs_p;
extern mountpath_t kvfs_p;
mountpath_t kvfs_p;
int fd[fd_cnt];
file_t file_table[file_cnt];

filesystem_t fs[3];	//fs[0]:procfs fs[1]:devfs fs[2]:kvfs
int seed;

spinlock_t vfs_lk;
MOD_DEF(vfs)
{
	.init = vfs_init,
	.access = access,
	.mount = mount,
	.unmount = unmount,
	.open = open,
	.read = read,
	.write = write,
	.lseek = lseek,
	.close = close,
};

  /*====================================================================*/
 /*==============================vfs init==============================*/
/*====================================================================*/
static fsops_t procfs_op;
static fsops_t devfs_op;
static fsops_t kvfs_op;
void procfs_init(inode_t *dev)
{
	for(int i = 0; i<inode_cnt; i++){
		fs[0].inode[i].if_write = 0; fs[0].inode[i].if_read = 0; fs[0].inode[i].if_exist = 0;
		fs[0].inode[i].thread_cnt = 0; fs[0].inode[i].size = 0;
	}
	/*================cpuinfo================*/
	if(inode_num_proc == inode_cnt){
		printf("the inode is full in procfs\n");
		return;
	}
	inode_t cpuinfo;
	cpuinfo.if_exist = 1;
	cpuinfo.if_write = 0; cpuinfo.if_read = 1;
	strcpy(cpuinfo.name, "/proc");
	strcat(cpuinfo.name, "/cpuinfo");
	char* c_info = "My cpuinfo:remain to be done";
	strcpy(cpuinfo.content, c_info);
	cpuinfo.size = strlen(c_info);
	fs[0].inode[inode_num_proc++] = cpuinfo;
	/*================meminfo================*/
	if(inode_num_proc == inode_cnt){
		printf("the inode is full in procfs\n");
		return;
	}		
	inode_t meminfo;
	meminfo.if_exist = 1;
	meminfo.if_write = 0; meminfo.if_read = 1;
	strcpy(meminfo.name, "/proc");
	strcat(meminfo.name, "/meminfo");
	char* m_info = "My meminfo:remain to be done";
	strcpy(meminfo.content, m_info);
	meminfo.size = strlen(m_info);
	fs[0].inode[inode_num_proc++] = meminfo;	
	return;
}
void devfs_init(inode_t *dev)
{
	for(int i = 0; i<inode_cnt; i++){
		fs[1].inode[i].if_write = 0; fs[1].inode[i].if_read = 0; fs[1].inode[i].if_exist = 0;
		fs[1].inode[i].thread_cnt = 0; fs[1].inode[i].size = 0;
	}
	/*================null================*/	
	if(inode_num_dev == inode_cnt){
		printf("the inode is full in procfs\n");
		return;
	}
	inode_t null;
	null.if_exist = 1;
	null.if_write = 1; null.if_read = 1; null.size = 0;
	strcpy(null.name, "/dev");
	strcat(null.name, "/null");
	fs[1].inode[inode_num_dev++] = null;
	/*================zero================*/	
	if(inode_num_dev == inode_cnt){
		printf("the inode is full in procfs\n");
		return;
	}	
	inode_t zero;
	zero.if_exist = 1;
	zero.if_write = 1; zero.if_read = 1; zero.size = 0;
	strcpy(zero.name, "/dev");
	strcat(zero.name, "/zero");
	fs[1].inode[inode_num_dev++] = zero;
	/*================random================*/		
	if(inode_num_dev == inode_cnt){
		printf("the inode is full in procfs\n");
		return;
	}	
	inode_t random;
	random.if_exist = 1; 
	random.if_write = 1; random.if_read = 1; random.size = 0;
	strcpy(random.name,"/dev");
	strcat(random.name, "/random");
	fs[1].inode[inode_num_dev++] = random;
	return;		
}
void kvfs_init(inode_t *dev)
{
	//TODO();目前不知道这里kvfs如何初始化
	for(int i = 0; i<inode_cnt; i++){
		fs[2].inode[i].if_write = 0; fs[2].inode[i].if_read = 0; fs[2].inode[i].if_exist = 0;
		fs[2].inode[i].thread_cnt = 0; fs[2].inode[i].size = 0;
	}	
	return;
}
void fs_init(const char *name, inode_t *dev)	//dev的作用
{
	if(!strcmp(name, "procfs")){
		procfs_init(dev);		
	}
	else if(!strcmp(name, "devfs")){
		devfs_init(dev);
	}
	else if(!strcmp(name, "kvfs")){
		kvfs_init(dev);
	}
}
int lookup(filesystem_t fs, const char *path, int flag)
{	
	printf("lookup:path:%s\n", path);
	int ret = -1;
	int index = -1; int if_find = 0;
	while(index < inode_cnt){
		if(fs.inode[index].if_exist){
			if(!strcmp(path, fs.inode[index].name)){
				if_find = 1;
				break;
			}			
		}
		index++;
	}
	if(if_find && index < inode_cnt){
		ret = index;	
	}
	//printf("lookup:if_find:%d\n", if_find);
	return ret;
}
int fs_close(inode_t inode)
{
	//TODO()目前不知道这里要干什么
	return 0;
}
/*void fsop_init()
{
	procfs_op.init = fs_init;
	procfs_op.lookup = &lookup;
	procfs_op.close = &fs_close;
	
	devfs_op.init = &fs_init;
	devfs_op.lookup = &lookup;
	devfs_op.close = &fs_close;	
	
	kvfs_op.init = &fs_init;
	kvfs_op.lookup = &lookup;
	kvfs_op.close = &fs_close;		
	
	return;		
}*/
void create_procfs() 
{
	strcpy(fs[0].name, "procfs");
	//if (!fs) panic("procfs allocation failed");
	//fs[0].ops = procfs_op; // 你为procfs定义的fsops_t，包含函数的实现
	fs_init("procfs", NULL);
	
	return;
}
void create_devfs() 
{
	strcpy(fs[1].name, "devfs"); 
	//if (!fs) panic("devfs allocation failed");
	//fs[1].ops = devfs_op; // 你为procfs定义的fsops_t，包含函数的实现
	fs_init("devfs", NULL);
	
	return;
}
void create_kvfs() 
{
	strcpy(fs[2].name, "kvfs"); 
	//if (!fs) panic("fs allocation failed");
	//fs[2].ops = kvfs_op; // 你为procfs定义的fsops_t，包含函数的实现
	fs_init("kvfs", NULL);
	return;
}
int mount(const char *path)
{
	if(!strcmp(path, "/proc")){
		strcpy(procfs_p.p, path);
		//procfs_p.fs = fs[0];
		fs[0].path = procfs_p;
	}
	else if(!strcmp(path, "/dev")){
		strcpy(devfs_p.p, path);
		//devfs_p.fs = fs[1];
		fs[1].path = devfs_p;
	}
	else if(!strcmp(path, "/")){
		strcpy(kvfs_p.p, path);
		//kvfs_p.fs = fs[2];
		fs[2].path = kvfs_p;
		
	}
	else{
		printf("wrong when mount %s!!!!\n", path);
		return -1;
	}
	return 0;
}
int unmount(const char *path)
{
	//TODO!!!!!!!???????
	if(!strcmp(path, "/proc")){
		//procfs_p.fs.path = NULL;
	}
	else if(!strcmp(path, "/dev")){
		//devfs_p.fs.path = NULL;
	}
	else if(!strcmp(path, "/")){
		//kvfs_p.fs.path = NULL;
	}
	else{
		printf("error when unmount %s\n", path);
	}
	return 0;
}
  /*====================================================================*/
 /*==============================file ops==============================*/
/*====================================================================*/
fileops_t procfile_op;
fileops_t devfile_op;
fileops_t kvfile_op;
int file_open(inode_t *inode, file_t *file, int flags)
{
	int current_fd = -1;
	switch(flags){
		case O_RDONLY:
			if(!inode->if_exist){
				printf("cannot open the file which is not existing while reading!\n");
				return -1;
			}
			else if(!inode->if_read){
				printf("open O_RDONLY can not read in file_open:%s\n", inode->name);
				printf("open mode error: have no permission to read %s\n", inode->name);
				return -1;
			}
			for(int i = 3; i<fd_cnt; i++){
				if(fd[i] == 0){
					fd[i] = 1;
					current_fd = i;
					break;
				}
			}
			if(current_fd == -1){
				printf("open fd error: there isn't enough fd left in read!\n");
				return -1;
			}
			file->fd = current_fd;
			strcpy(file->name, inode->name);
			strcpy(file->content, inode->content);
			file->f_inode = *inode;
			file->offset = 0;	
			file->if_read = 1;
			file->if_write = 0;									
			file_table[current_fd] = *file;
			break;
		case O_WRONLY:
			if(!inode->if_exist){
				printf("cannot open the file which is not existing while writing!\n");
				return -1;
			}
			else if(!inode->if_write){
				printf("open mode error: have no permission to write%s\n", inode->name);
				return -1;
			}
			for(int i = 3; i<fd_cnt; i++){
				if(fd[i] == 0){
					fd[i] = 1;
					current_fd = i;
					break;
				}
			}
			if(current_fd == -1){
				printf("open fd error: there isn't enough fd left in write!\n");
				return -1;
			}
			file->fd = current_fd;
			strcpy(file->name, inode->name);
			strcpy(file->content, inode->content);
			file->f_inode = *inode;
			file->offset = 0;	
			file->if_read = 0;
			file->if_write = 1;									
			file_table[current_fd] = *file;
			break;
		case O_RDWR:
			if(!inode->if_exist){			
				printf("cannot open the file which is not existing while writing and reading!\n");
				return -1;
			}
			else if(!inode->if_write || !inode->if_read){
				printf("open mode error: have no permission to write or read %s\n", inode->name);
				return -1;
			}
			for(int i = 3; i<fd_cnt; i++){
				if(fd[i] == 0){
					fd[i] = 1;
					current_fd = i;
					break;
				}
			}
			if(current_fd == -1){
				printf("open fd error: there isn't enough fd left in read&write!\n");
				return -1;
			}
			//printf("O_WRDR:kvfs_p->fs->inode[0]:%s if_read:%d if_write:%d\n", kvfs_p->fs->inode[0]->name,kvfs_p->fs->inode[0]->if_read, kvfs_p->fs->inode[0]->if_write);
			file->fd = current_fd;
			strcpy(file->name, inode->name);
			strcpy(file->content, inode->content);

			file->f_inode = *inode;
			file->offset = 0;	
			file->if_read = 1;
			file->if_write = 1;									
			file_table[current_fd] = *file;	
			break;	
		case O_CREATE:
			if(inode->if_exist){
				printf("this file %s has already existed!", inode->name);
				return -1;
			}
			for(int i = 3; i<fd_cnt; i++){
				if(fd[i] == 0){
					fd[i] = 1;
					current_fd = i;
					break;
				}
			}	
			if(current_fd == -1){
				printf("open fd error: there isn't enough fd left in create!\n");
				return -1;
			}
			inode->if_exist = 1; inode->if_read = 1; inode->if_write = 0;
			inode->thread_cnt++;
			inode->size = 0; //inode->content = "";
			strcpy(inode->content, "");
			
			file->fd = current_fd;
			strcpy(file->name, inode->name);
			strcpy(file->content, inode->content);
			file->f_inode = *inode;
			file->offset = 0;	
			file->if_read = 1;
			file->if_write = 0;									
			file_table[current_fd] = *file;
			break;									
		case O_CREATE|O_WRONLY:
			if(inode->if_exist){
				printf("this file %s has already existed!", inode->name);
				return -1;
			}
			for(int i = 3; i<fd_cnt; i++){
				if(fd[i] == 0){
					fd[i] = 1;
					current_fd = i;
					break;
				}
			}	
			if(current_fd == -1){
				printf("open fd error: there isn't enough fd left in create!\n");
				return -1;
			}
			inode->if_exist = 1; inode->if_read = 0; inode->if_write = 1;
			inode->size = 0; //inode->content = "";
			strcpy(inode->content, "");
			
			file->fd = current_fd;
			strcpy(file->name, inode->name);
			strcpy(file->content, inode->content);
			file->f_inode = *inode;
			file->offset = 0;	
			file->if_read = 0;
			file->if_write = 1;									
			file_table[current_fd] = *file;
			break;				
		case O_CREATE|O_RDWR:
			if(inode->if_exist){
				printf("this file %s has already existed!", inode->name);
				return -1;
			}
			for(int i = 3; i<fd_cnt; i++){
				if(fd[i] == 0){
					fd[i] = 1;
					current_fd = i;
					break;
				}
			}	
			if(current_fd == -1){
				printf("open fd error: there isn't enough fd left in create!\n");
				return -1;
			}
			inode->if_exist = 1; inode->if_read = 1; inode->if_write = 1;
			inode->size = 0; //inode->content[0] = '\0';
			strcpy(inode->content, "");
			printf("file_open:inode->name:%s\n", inode->name);			
			
			file->fd = current_fd;
			strcpy(file->name, inode->name);
			strcpy(file->content, inode->content);
			file->f_inode = *inode;
			file->offset = 0;	
			file->if_read = 1;
			file->if_write = 1;									
			file_table[current_fd] = *file;
			//printf("file_open:current_fd:%d file->offset:%d",current_fd, file->offset);
			break;				
	}
	return file->fd;
}
ssize_t kvproc_file_read(inode_t *inode, file_t *file, char *buf, size_t size)
{
	//printf("in file_read:name:%s size:%d inode_size:%d offset:%d\n", file->name, size, inode->size,file->offset);
	if(!inode->if_read){
		printf("read permission error: cannot read %s\n", file->name);
		return -1;
	}
	if(size > file->f_inode.size - file->offset){
		size = inode->size - file->offset;
	}
	strncpy(buf, file->content+file->offset, size);
	file->offset += size;
	file_table[file->fd] = *file;
	return size;
}
ssize_t dev_file_read(inode_t *inode, file_t *file, char*buf, size_t size)
{
	if(!inode->if_read){
		printf("read permission error: cannot read %s\n", file->name);
		return -1;
	}
	if(size > file->f_inode.size - file->offset){
		size = inode->size - file->offset;
	}	
	if(!strcmp(inode->name+strlen(devfs_p.p), "/zero")){
		strcpy(buf, "");
	}
	else if(!strcmp(inode->name+strlen(devfs_p.p), "/null")){
		strcpy(buf, "");
	}
	else if(!strcmp(inode->name+strlen(devfs_p.p), "/random")){	
		int num = rand() % 8192;
		strncpy(buf, itoa(num), 4);
	}
	else{
		strncpy(buf, file->content+file->offset, size);
	}
	file->offset += size;
	file_table[file->fd] = *file;
	return size;
}
ssize_t kvproc_file_write(inode_t *inode, file_t *file, const char *buf, size_t size)
{
	printf("in file_write:name:%s buf:%s size:%d\n", inode->name, buf, size);
	if(!inode->if_write){
		printf("write permission error: cannot write %s\n", file->name);
		return -1;
	}
	//printf("file_write:before renew size:%d\n", size);
	if((file->offset + size) >= file_content_maxn){
		size = file_content_maxn - file->offset;
	}
	strncpy(inode->content + file->offset, buf, size);
	strcpy(file->content, inode->content);	//先拷贝到inode再到文件
	inode->size = file->offset + size;
	file->offset += size;
	//printf("file_write:size:%d\n", size);
	file_table[file->fd] = *file;
	return size;
}
ssize_t dev_file_write(inode_t *inode, file_t *file, const char *buf, size_t size)
{
	if(!inode->if_write){
		printf("write permmison error: cannot write %s\n", file->name);
		return -1;
	}
	if(!strcmp(inode->name+strlen(devfs_p.p), "/zero")
	|| !strcmp(inode->name+strlen(devfs_p.p), "/null")
	|| !strcmp(inode->name+strlen(devfs_p.p), "/random")){
		return size;	//这几个文件写了也没用
	}
	else if(file->offset + size >= file_content_maxn){
		size = file_content_maxn - file->offset;
	}
	strncpy(inode->content + file->offset, buf, size);
	strcpy(file->content, inode->content);	//先拷贝到inode再到文件	
	inode->size = file->offset + size;
	file->offset += size;
	file_table[file->fd] = *file;
	return size;
}
off_t file_lseek(inode_t *inode, file_t *file, off_t offset, int whence)
{
	switch(whence){
		case SEEK_SET:
			if(offset >= file_content_maxn){
				printf("cannot set offset larger than file_content_maxn in SEEK_SET %s\n", file->name);
				return -1;
			}
			file->offset = offset;
			break;
		case SEEK_CUR:
			if((file->offset + offset) >= file_content_maxn){
				printf("cannot set offset larger than file_content_maxn in SEEK_CUR %s\n", file->name);
				return -1;
			}
			file->offset += offset;
			break;
		case SEEK_END:
			if((inode->size + offset) >= file_content_maxn){
				printf("cannot set offset larger than file_content_maxn in SEEK_END %s\n", file->name);
				return -1;
			}
			file->offset = inode->size + offset;
			break;
	}
	file_table[file->fd] = *file;
	return file->offset;
}
int file_close(inode_t *inode, file_t *file)
{
	int current_fd = file->fd;
	printf("in file_close:fd:%d\n", current_fd);
	fd[current_fd] = 0;
	file_table[current_fd].if_write = 0; file_table[current_fd].if_read = 0;
	file_table[current_fd].fd = -1;
	return 0;	//不知道什么时候会是-1
}
/*void fileop_init()
{
	procfile_op.open = &file_open;
	procfile_op.read = &kvproc_file_read;
	procfile_op.write = &kvproc_file_write;
	procfile_op.lseek = &file_lseek;
	procfile_op.close = &file_close;
	
	devfile_op.open = &file_open;
	devfile_op.read = &dev_file_read;
	devfile_op.write = &dev_file_write;
	devfile_op.lseek = &file_lseek;
	devfile_op.close = &file_close;
	
	kvfile_op.open = &file_open;
	kvfile_op.read = &kvproc_file_read;
	kvfile_op.write = &kvproc_file_write;
	kvfile_op.lseek = &file_lseek;
	kvfile_op.close = &file_close;	
	
	return;		
}*/
void vfs_init()
{
	
	fd[0] = 1; fd[1] = 1; fd[2] = 1;
	for(int i = 3; i<fd_cnt; i++){
		fd[i] = 0;
	}
	for(int i = 0; i<file_cnt; i++){
		file_table[i].if_write = 0; file_table[i].if_read = 0;
		file_table[i].fd = -1;
	}
	/*========random seed init========*/
	seed = 40;
	srand(seed);
	/*================================*/
	inode_num_proc = 0;
	inode_num_dev = 0;
	inode_num_kv = 0;
	//fsop_init();
	//fileop_init();
	create_procfs(); create_devfs(); create_kvfs();

	mount("/proc");
	mount("/dev");
	mount("/");
	kmt->spin_init(&vfs_lk, "vfs_lk");
	return;
}
  /*====================================================================*/
 /*==============================sys_call==============================*/
/*====================================================================*/
int access(const char *path, int mode)
{
	int ret = 0;
	kmt->spin_lock(&vfs_lk);

	/*=========================lock=========================*/
	int temp = -1; int fs_index = -1;//所在虚拟文件系统的索引号
	if(!strncmp(path, procfs_p.p, strlen(procfs_p.p))){
		fs_index = 0;
		temp = lookup(fs[0], path, mode);	//不知道是不是mode
	}
	else if(!strncmp(path, devfs_p.p, strlen(devfs_p.p))){
		fs_index = 1;
		temp = lookup(fs[1], path, mode);
	}
	else if(!strncmp(path, kvfs_p.p, strlen(kvfs_p.p))){
		fs_index = 2;
		temp = lookup(fs[2], path, mode);
	}
	switch(mode){
		case F_OK:
			if(temp < 0){
				printf("the file has not been created!!\n");
				ret = -1;
			}
			else{
				printf("access:temp name:%s\n", fs[fs_index].inode[temp].name);
			}
			break;
		case X_OK:
		case X_OK|W_OK:
		case X_OK|R_OK:
			printf("remain to be done to support executable file\n");
			break;
		case W_OK:
			if(temp < 0){
				printf("the file has not been created!!\n");
				ret = -1;
			}
			else if(!fs[fs_index].inode[temp].if_write){
				printf("have no permission to write when check in access %s\n", path);
				ret = -1;
			}
			break;
		case R_OK:
			if(temp < 0){
				printf("the file has not been created!!\n");
				ret = -1;
			}		
			else if(!fs[fs_index].inode[temp].if_read){
				printf("have no permission to read when check in access %s\n", path);
				ret = -1;
			}
			break;
		case W_OK|R_OK:
			if(temp < 0){
				printf("the file has not been created!!\n");
				ret = -1;
			}		
			else if(!fs[fs_index].inode[temp].if_read || !fs[fs_index].inode[temp].if_write){
				printf("have no permission to read or write when check in access %s\n", path);
				ret = -1;
			}
			break;
	}
	/*=========================unlock=========================*/
	kmt->spin_unlock(&vfs_lk);
	return ret;
}

int open(const char *path, int flags)
{
	kmt->spin_lock(&vfs_lk);
	/*=========================lock=========================*/
	int temp_fd = -1;
	int node_index = -1;
	file_t FILE; FILE.if_read = 0; FILE.if_write = 0; FILE.fd = -1; 
	FILE.if_read = 0; FILE.if_write = 0;
	if(!strncmp(path, procfs_p.p, strlen(procfs_p.p))){
		fs_index = 0;
		node_index = lookup(fs[0], path, flags);	//不知道是不是flag
		//FILE.ops = procfile_op;
		if(node_index < 0){
			if(inode_num_proc == inode_cnt){
				printf("the file is not exisiting while open and there is no inode to allocate!\n");
				return -1;
			}
			fs[0].inode[inode_num_proc].if_exist = 0; 
			fs[0].inode[inode_num_proc].if_read = 0;
			fs[0].inode[inode_num_proc].if_write = 0;
			fs[0].inode[inode_num_proc].thread_cnt = 0;
			strcpy(fs[0].inode[inode_num_proc].name, path);
			node_index = inode_num_proc;
			inode_num_proc++;
		}
		else{
	/*=========================unlock=========================*/
			kmt->spin_unlock(&vfs_lk);	
			while(fs[0].inode[node_index].thread_cnt > 0);
			kmt->spin_lock(&vfs_lk);
	/*=========================lock=========================*/						
		}
		temp_fd = file_open(&fs[0].inode[node_index], &FILE, flags);	
		
	}
	else if(!strncmp(path, devfs_p.p, strlen(devfs_p.p))){
		fs_index = 1;
		FILE.ops = devfile_op;
		node_index = lookup(fs[1], path, flags);		
		if(node_index < 0){
			if(inode_num_dev == inode_cnt){
				printf("the file is not exisiting while open and there is no inode to allocate!\n");
				return -1;
			}
			fs[1].inode[inode_num_dev].if_exist = 0; 
			fs[1].inode[inode_num_dev].if_read = 0;
			fs[1].inode[inode_num_dev].if_write = 0;
			fs[1].inode[inode_num_dev].thread_cnt = 0;
			strcpy(fs[1].inode[inode_num_dev].name, path);
			node_index = inode_num_dev;
			inode_num_dev++;
		}
		else{
	/*=========================unlock=========================*/
			kmt->spin_unlock(&vfs_lk);	
			while(fs[1].inode[node_index].thread_cnt > 0);
			kmt->spin_lock(&vfs_lk);
	/*=========================lock=========================*/					
		}
		temp_fd =file_open(&fs[1].inode[node_index], &FILE, flags);			
	}
	else if(!strncmp(path, kvfs_p.p, strlen(kvfs_p.p))){	
		fs_index = 2;
		FILE.ops = devfile_op;
		node_index = lookup(fs[2], path, flags);
		if(node.if_exist == 0){
			if(inode_num_kv == inode_cnt){
				printf("the file is not exisiting while open and there is no inode to allocate!\n");
				return -1;
			}
			fs[2].inode[inode_num_kv].if_exist = 0; 
			fs[2].inode[inode_num_kv].if_read = 0;
			fs[2].inode[inode_num_kv].if_write = 0;
			fs[2].inode[inode_num_kv].thread_cnt = 0;
			strcpy(fs[2].inode[inode_num_kv].name, path);
			node_index = inode_num_kv;
			inode_num_kv++;
		}	
		else{
	/*=========================unlock=========================*/
			kmt->spin_unlock(&vfs_lk);
			while(fs[2].inode[node_index].thread_cnt > 0);
			kmt->spin_lock(&vfs_lk);
	/*=========================lock=========================*/						
		}
		temp_fd =file_open(&fs[2].inode[node_index], &FILE, flags);						
	}
	printf("kuaidianxiehaoba!!\n");
	/*=========================unlock=========================*/
	kmt->spin_unlock(&vfs_lk);	
	return temp_fd;
}
ssize_t read(int fd, void *buf, size_t nbyte)
{
	kmt->spin_lock(&vfs_lk);
	/*=========================lock=========================*/
	if(fd < 0){
		printf("invalid fd:%d in read\n", fd);
		return -1;
	}
	int node_index; ssize_t size = -1;
	file_t FILE = file_table[fd];	//还未实现描述符为0、1、2的操作
	char *path = FILE.name;
	if(!strncmp(path, procfs_p.p, strlen(procfs_p.p))){
		node_index = lookup(fs[0], path, 0);
		if(node_index < 0){
			printf("invalid read for a non-exiting inode!\n");
			return -1;
		}			
		size = kvproc_file_read(&fs[0].inode[node_index], &FILE, buf, nbyte);
	}
	else if(!strncmp(path, devfs_p.p, strlen(devfs_p.p))){
		node_index = lookup(fs[1], path, 0);
		if(node_index < 0){
			printf("invalid read for a non-exiting inode!\n");
			return -1;
		}	
		size = dev_file_read(&fs[1].inode[node_index], &FILE, buf, nbyte);
	}
	else if(!strncmp(path, kvfs_p.p, strlen(kvfs_p.p))){
		node_index = lookup(fs[2], path, 0);
		if(node_index < 0){
			printf("invalid read for a non-exiting inode!\n");
			return -1;
		}			
		size = kvproc_file_read(&fs[2].inode[node_index], &FILE, buf, nbyte);
	}
	/*=========================unlock=========================*/
	kmt->spin_unlock(&vfs_lk);	
	return size;
}
ssize_t write(int fd, void *buf, size_t nbyte)
{
	kmt->spin_lock(&vfs_lk);
	/*=========================lock=========================*/
	
	if(fd < 0){
		printf("invalid fd:%d in read\n", fd);
		return -1;
	}
	int node_index; ssize_t size = -1;
	file_t FILE = file_table[fd];	//还未实现描述符为0、1、2的操作
	char *path = FILE.name;
	if(!strncmp(path, procfs_p.p, strlen(procfs_p.p))){
		node_index = lookup(fs[0], path, 0);
		if(node_index < 0){
			printf("invalid read for a non-exiting inode!\n");
			return -1;
		}			
		size = kvproc_file_write(&fs[0].inode[node_index], &FILE, buf, nbyte);
	}
	else if(!strncmp(path, devfs_p.p, strlen(devfs_p.p))){
		node_index = lookup(fs[1], path, 0);
		if(node_index < 0){
			printf("invalid read for a non-exiting inode!\n");
			return -1;
		}	
		size = dev_file_write(&fs[1].inode[node_index], &FILE, buf, nbyte);
	}
	else if(!strncmp(path, kvfs_p.p, strlen(kvfs_p.p))){
		node_index = lookup(fs[2], path, 0);
		if(node_index < 0){
			printf("invalid read for a non-exiting inode!\n");
			return -1;
		}			
		size = kvproc_file_write(&fs[2].inode[node_index], &FILE, buf, nbyte);
	}
	/*=========================unlock=========================*/
	kmt->spin_unlock(&vfs_lk);		
	return size;
}
off_t lseek(int fd, off_t offset, int whence)
{
	kmt->spin_lock(&vfs_lk);
	/*=========================lock=========================*/
	if(fd < 0){
		printf("invalid fd:%d in read\n", fd);
		return -1;
	}
	int node_index; off_t temp_offset = -1;
	file_t FILE = file_table[fd];	//还未实现描述符为0、1、2的操作
	char *path = FILE.name;
	if(!strncmp(path, procfs_p.p, strlen(procfs_p.p))){
		node_index = lookup(fs[0], path, 0);
		if(node_index < 0){
			printf("invalid read for a non-exiting inode!\n");
			return -1;
		}
		temp_offset = file_lseek(&fs[0].inode[node_index], &FILE, offset, whence);
	}
	else if(!strncmp(path, devfs_p.p, strlen(devfs_p.p))){
		node_index = lookup(fs[1], path, 0);
		if(node_index < 0){
			printf("invalid read for a non-exiting inode!\n");
			return -1;
		}
		temp_offset = file_lseek(&fs[1].inode[node_index], &FILE, offset, whence);
	}
	else if(!strncmp(path, kvfs_p.p, strlen(kvfs_p.p))){
		node_index = lookup(fs[2], path, 0);
		if(node_index < 0){
			printf("invalid read for a non-exiting inode!\n");
			return -1;
		}
		temp_offset = file_lseek(&fs[2].inode[node_index], &FILE, offset, whence);
	}
	/*=========================unlock=========================*/
	kmt->spin_unlock(&vfs_lk);		
	return temp_offset;
}
int close(int fd)
{
	kmt->spin_lock(&vfs_lk);
	/*=========================lock=========================*/
	if(fd < 0){
		printf("invalid fd:%d in read\n", fd);
		return -1;
	}
	int node_index = -1;
	file_t FILE = file_table[fd]; int ret = -1;
	char *path = FILE.name;
	if(!strncmp(path, procfs_p.p, strlen(procfs_p.p))){
		node_index = lookup(fs[0], path, 0);
		if(node_index < 0){
			printf("invalid close for a non-exiting inode!\n");
			return -1;
		}
		ret = file_close(&fs[0].inode[node_index], &FILE);
		fs[0].inode[node_index].thread_cnt--;
	}
	else if(!strncmp(path, devfs_p.p, strlen(devfs_p.p))){
		node_index = lookup(fs[1], path, 0);
		if(node_index < 0){
			printf("invalid close for a non-exiting inode!\n");
			return -1;
		}
		ret = file_close(&fs[1].inode[node_index], &FILE);
		fs[1].inode[node_index].thread_cnt--;
	}
	else if(!strncmp(path, kvfs_p.p, strlen(kvfs_p.p))){
		node_index = lookup(fs[2], path, 0);
		if(node_index < 0){
			printf("invalid close for a non-exiting inode!\n");
			return -1;
		}
		ret = file_close(&fs[1].inode[node_index], &FILE);
		fs[1].inode[node_index].thread_cnt--;
	}
	/*=========================unlock=========================*/
	kmt->spin_unlock(&vfs_lk);		
	return ret;
}
