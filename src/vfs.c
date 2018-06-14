#include <os.h>
#include <vfs.h>
#include<libc.h>


//typedef struct filesystem filesystem_t;
//typedef struct inode inode_t;
//typedef struct file file_t;
static void vfs_init();
static int access(const char *path, int mode);
static int mount(const char *path, filesystem_t *fs);
static int unmount(const char *path);
static int open(const char *path, int flags);
static ssize_t read(int fd, void *buf, size_t nbyte);
static ssize_t write(int fd, void *buf, size_t nbyte);
static off_t lseek(int fd, off_t offset, int whence);
static int close(int fd);

static int inode_num_proc;// = 0;
static int inode_num_dev;
static int inode_num_kv;
/*filesystem_t* procvfs;
filesystem_t* devfs;
filesystem_t* kvfs;*/

int fd[fd_cnt];
file_t* file_table[file_cnt];
 
//int fd_num;
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
}

fsops_t procfs_op;
fsops_t devfs_op;
fsops_t kvfs_op;
fileops_t file_op;
void procfs_init(filesystem_t *fs, inode_t *dev)
{
	/*================cpuinfo================*/
	if(inode_num_proc == inode_cnt){
		printf("the inode is full in procfs\n");
		return;
	}
	inode_t *cpuinfo = (inode_t *)pmm->alloc(sizeof(inode_t));
	cpuinfo->inode_no = inode_num_proc; 
	cpuinfo->if_write = 0; cpuinfo->if_read = 1;
	strcpy(cpuinfo->name, fs->mount_path);
	strcat(cpuinfo->name, "/cpuinfo");
	char* c_info = "My cpuinfo:remain to be done"
	strcpy(cpuinfo->content, c_info);
	cpuinfo->size = strlen(c_info);
	fs->inode[inode_num_proc++] = cpuinfo;
	/*================meminfo================*/
	if(inode_num_proc == inode_cnt){
		printf("the inode is full in procfs\n");
		return;
	}		
	inode_t *meminfo = (inode_t *)pmm->alloc(sizeof(inode)t));
	meminfo->inode_no = inode_num_proc; 
	meminfo->if_write = 0; meminfo->if_read = 1;
	strcpy(meminfo->name, fs->mount_path);
	strcat(meminfo->name, "/meminfo");
	char* m_info = "My meminfo:remain to be done"
	strcpy(meminfo->content, m_info);
	meminfo->size = strlen(m_info);
	fs->inode[inode_num_proc++] = meminfo;	
	
	//线程？？
	return;
}
void devfs_init(filesystem_t *fs, inode_t *dev)
{
	/*================null================*/	
	if(inode_num_dev == inode_cnt){
		printf("the inode is full in procfs\n");
		return;
	}
	inode_t *null = (inode_t *)pmm->alloc(sizeof(inode_t));
	null->inode_no = inode_num_dev; 
	null->if_write = 1; null->if_read = 1; null->size = 0;
	strcpy(null->name, fs->mount_path);
	strcat(null->name, "/null");
	fs->inode[inode_num_dev++] = null;
	/*================zero================*/	
	if(inode_num_dev == inode_cnt){
		printf("the inode is full in procfs\n");
		return;
	}	
	inode_t *zero = (inode_t *)pmm->alloc(sizeof(inode)t));
	zero->inode_no = inode_num_dev; 
	zero->if_write = 1; zero->if_read = 1; zero->size = 0;
	strcpy(zero->name, fs->mount_path);
	strcat(zero->name, "/zero");
	fs->inode[inode_num_dev++] = zero;
	/*================random================*/		
	if(inode_num_dev == inode_cnt){
		printf("the inode is full in procfs\n");
		return;
	}	
	inode_t *random = (inode_t *)pmm->alloc(sizeof(inode)t));
	random->inode_no = inode_num_dev; 
	random->if_write = 1; random->if_read = 1; random->size = 0;	//不知道要初始成多大
	strcpy(random->name, fs->mount_path);
	strcat(random->name, "/random");
	fs->inode[inode_num_dev++] = random;
	
	return;		
}
void kvfs_init(filesystem_t *fs, inode_t *dev)
{
	//TODO();目前不知道这里kvfs如何初始化
}
void fs_init(filesystem_t *fs, const char *name, inode_t *dev)	//dev的作用
{
	for(int i = 0; i<inode_cnt; i++){
		fs->inode[i] = NULL;
	}
	if(!strcmp(name, "procfs")){
		procfs_init(fs, dev);		
	}
	else if(!strcmp(name, "devfs")){
		devfs_init(fs, dev);
	}
	else if(!strcmp(name, "kvfs")){
		kvfs_init(fs, dev);
	}
}
inode_t *lookup(filesystem_t *fs, const char *path, int flag)
{
	inode_t *ans = NULL;	//????????????????
	int index = 0; int if_find = 0;
	while(fs->inode[index] && index < inode_cnt){
		if(!strcmp(path, fs->inode[index]->inode)){
			if_find = 1;
			break;
		}
		index++;
	}
	if(if_find && index < inode_cnt){
		ans = fs->inode[index];
	}
	else{
		printf("cannot find the matching inode!\n");
	}
	return ans;
}
int close(inode_t *inode)
{
	//TODO()目前不知道这里要干什么
	return 0;
}
void fsop_init()
{
	procfs_op.init = &fs_init;
	procfs_op.lookup = &lookup;
	procfs_op.close = &close;
	
	devfs_op.init = &fs_init;
	devfs_op.lookup = &lookup;
	devfs_op.close = &close;	
	
	kvfs_op.init = &fs_init;
	kvfs_op.lookup = &lookup;
	kvfs_op.close = &close;		
	
	return;		
}
filesystem_t *create_procfs() 
{
	filesystem_t *fs = (filesystem_t *)pmm->alloc(sizeof(filesystem_t));
	strcpy(fs->name, "procfs");
	if (!fs) panic("procfs allocation failed");
	fs->ops = &procfs_ops; // 你为procfs定义的fsops_t，包含函数的实现
	fs->ops->init(fs, "procfs", NULL);
	return fs;
}
filesystem_t *create_devfs() 
{
	//filesystem_t *fs = (filesystem_t *)pmm->alloc(sizeof(filesystem_t));
	strcpy(fs->name, "devfs"); 
	if (!fs) panic("devfs allocation failed");
	fs->ops = &devfs_op; // 你为procfs定义的fsops_t，包含函数的实现
	fs->ops->init(fs, "devfs", NULL);
	return fs;
}
filesystem_t *create_kvfs() 
{
	//filesystem_t *fs = (filesystem_t *)pmm->alloc(sizeof(filesystem_t));
	strcpy(fs->name, "kvfs"); 
	if (!fs) panic("fs allocation failed");
	fs->ops = &kvfs_op; // 你为procfs定义的fsops_t，包含函数的实现
	fs->ops->init(fs, "kvfs", NULL);
	return fs;
}
int mount(const char *path, filesystem_t *fs)
{
	//TODO!!!!!!!???????mount原理不明觉厉
	strcpy(fs->mount_path, path);
	return 0;
}
int unmount(const char *path)
{
	//TODO!!!!!!!???????
	fs->mount_path[0] = '\0';
	return 0;
}
void vfs_init()
{
	fd[0] = 1; fd[1] = 1; fd[2] = 1;
	for(int i = 3; i<fd_cnt; i++){
		fd[i] = 0;
	}
	for(int i = 0; i<file_cnt; i++){
		file_table[i] = NULL;		
	}
	fsop_init();
	mount("/proc", create_procfs());
	mount("/dev", create_devfs());
	mount("/", create_kvfs());
	return;
}
int file_open(inode_t *inode, file_t *file, int flags)
{
	if(flag == O_RDONLY && !inode->if_read){
		printf("open mode error: have no permission to read %s\n", inode->name);
		return -1;
	}
	else if(flag == O_WRONLY && !inode->if_write){
		printf("open mode error: have no permission to write %s\n", inode->name);
		return -1;
	}
	else if(flag == O_RDWR && (!indoe->if_write || !inode->if_read)){
		printf("open mode error: have no permission to read/write %s\n", inode->name);
		return -1;
	}
	int current_fd = -1;
	for(int i = 3; i<fd_cnt; i++){
		if(fd[i] == 0){
			current_fd = 1;
			break;
		}
	}
	if(current_fd == -1){
		printf("open fd error: there isn't enough fd left!\n");
		return -1;
	}
	strcpy(file->name, inode->name);
	strcpy(file->content, inode->content);
	file->f_inode = inode;
	file->offset = 0;
	if(flag == O_RDONLY){
		file->if_read = 1;
		file->if_write = 0;
	}
	else if(flag == O_WRONLY){
		file->if_read = 0;
		file->if_write = 1;
	}
	else if(flag == O_RDWR){
		file->if_read = 1;
		file->if_write = 1;
	}
	file->fd = current_fd;
	file_table[current_fd] = file;
	return file->fd;
}
ssize_t kvproc_file_read(inode_t *inode, file_t *file, char *buf, size_t size)
{
	if(!inode->if_read){
		printf("read permission error: cannot read %s\n", file->name);
		return -1;
	}
	if(size > file->f_inode->size - file->offset){
		size = inode->size - file->offset;
	}
	strncpy(buf, file->content+file->offset, size);
	return size;
}
ssize_t dev_file_read(inode_T *inode, file_t *file, char*buf, size_t size)
{
	if(!inode-if_read){
		printf("read permission error: cannot read %s\n", file->name);
		return -1;
	}
	if(!strcmp(inode->name+strlen()))
}
ssize_t kvproc_file_write(inode_t *inode, file_t *file, const char *buf, size_t size)
{
	if(!inode->if_write){
		printf("write permission error: cannot write %s\n", file->name);
		return -1;
	}
	if((file->f_inode->size + size) >= file_content_maxn){
		size = file_content_maxn - file->f_inode->size;
	}
	strncpy(inode->content + file->offset, buf, size);
	strcpy(file->content, inode->content);	//先拷贝到inode再到文件
	//strncpy(file->content + file->offset, buf, size);
	inode->size += size;
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
	return offset;
}
int file_close(inode_t *inode, file_t *file)
{
	int current_fd = file->fd;
	fd[current_fd] = 0;
	file[current_fd] = NULL;
	return 0;	//不知道什么时候会是-1
}


