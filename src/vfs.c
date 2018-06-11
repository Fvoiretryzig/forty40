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
void procfs_init(filesystem_t *fs, inode_t *dev)
{
	if(inode_num_proc == inode_cnt){
		printf("the inode is full in procfs\n");
		return;
	}
	inode_t *cpuinfo = (inode_t *)pmm->alloc(sizeof(inode_t));
	cpuinfo->inode_no = inode_num_proc; 
	cpuinfo->if_exist = 1; cpuinfo->if_write = 0; cpuinfo->if_read = 1;
	strcpy(cpuinfo->name, fs->mount_path);
	strcat(cpuinfo->name, "/cpuinfo");
	fs->inode[inode_num_proc++] = cpuinfo;

	if(inode_num_proc == inode_cnt){
		printf("the inode is full in procfs\n");
		return;
	}		
	inode_t *meminfo = (inode_t *)pmm->alloc(sizeof(inode)t));
	meminfo->inode_no = inode_num_proc; 
	meminfo->if_exist = 1; meminfo->if_write = 0; meminfo->if_read = 1;
	strcpy(meminfo->name, fs->mount_path);
	strcat(meminfo->name, "/meminfo");
	fs->inode[inode_num_proc++] = meminfo;	
	
	//线程？？
	return;
}
void devfs_init(filesystem_t *fs, inode_t *dev)
{
	if(inode_num_dev == inode_cnt){
		printf("the inode is full in procfs\n");
		return;
	}
	inode_t *null = (inode_t *)pmm->alloc(sizeof(inode_t));
	null->inode_no = inode_num_dev; 
	null->if_exist = 1; null->if_write = 1; null->if_read = 1;
	strcpy(null->name, fs->mount_path);
	strcat(null->name, "/null");
	fs->inode[inode_num_dev++] = null;
	
	if(inode_num_dev == inode_cnt){
		printf("the inode is full in procfs\n");
		return;
	}	
	inode_t *zero = (inode_t *)pmm->alloc(sizeof(inode)t));
	zero->inode_no = inode_num_dev; 
	zero->if_exist = 1; zero->if_write = 1; zero->if_read = 1;
	strcpy(zero->name, fs->mount_path);
	strcat(zero->name, "/zero");
	fs->inode[inode_num_dev++] = zero;
		
	if(inode_num_dev == inode_cnt){
		printf("the inode is full in procfs\n");
		return;
	}	
	inode_t *random = (inode_t *)pmm->alloc(sizeof(inode)t));
	random->inode_no = inode_num_dev; 
	random->if_exist = 1; random->if_write = 1; random->if_read = 1;
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
		fs->file[i] = NULL;
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
void op_init()
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
	strcpy(fs->name, "procfs"); strcpy(fs->mount_path, "/proc");
	if (!fs) panic("procfs allocation failed");
	fs->ops = &procfs_ops; // 你为procfs定义的fsops_t，包含函数的实现
	fs->ops->init(fs, "procfs", NULL);
	return fs;
}
filesystem_t *create_devfs() 
{
	filesystem_t *fs = (filesystem_t *)pmm->alloc(sizeof(filesystem_t));
	strcpy(fs->name, "devfs"); strcpy(fs->mount_path, "/dev");
	if (!fs) panic("devfs allocation failed");
	fs->ops = &devfs_op; // 你为procfs定义的fsops_t，包含函数的实现
	fs->ops->init(fs, "devfs", NULL);
	return fs;
}
filesystem_t *create_kvfs() 
{
	filesystem_t *fs = (filesystem_t *)pmm->alloc(sizeof(filesystem_t));
	strcpy(fs->name, "kvfs"); strcpy(fs->mount_path, "/");
	if (!fs) panic("fs allocation failed");
	fs->ops = &kvfs_op; // 你为procfs定义的fsops_t，包含函数的实现
	fs->ops->init(fs, "kvfs", NULL);
	return fs;
}
