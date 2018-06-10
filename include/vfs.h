#ifndef __VFS_H__
#define __VFS_H__

typedef struct inodeindex
{
	int inode_no;
	char name[64];
	int if_occupy;
} inodeindex_t;
typedef struct inode
{
	int i_mode;	//在这个inode的文件类型
	int size;	//文件大小
	int i_ctime; //inode的生成时间  
	int i_atime; //inode最近一次的存取时间
	int i_mtime; //inode最近一次修改时间 
	uint32_t block_addr; //所在块的地址
} inode_t;
typedef struct fsops 
{
	void (*init)(struct filesystem *fs, const char *name, inode_t *dev);
	inode_t *(*lookup)(struct filesystem *fs, const char *path, int flags);
	int (*close)(inode_t *inode);
} fsops_t;
struct filesystem
{
	char name[16];
	inodeindext_t i_inode[80];	
	inode_t inode[80];
	fsops_t ops;
};
typedef struct fileops 
{
	int (*open)(inode_t *inode, file_t *file, int flags);
	ssize_t (*read)(inode_t *inode, file_t *file, char *buf, size_t size);
	ssize_t (*write)(inode_t *inode, file_t *file, const char *buf, size_t size);
	off_t (*lseek)(inode_t *inode, file_t *file, off_t offset, int whence);
} fileops_t;
struct file
{
	int mode;
	int flag;
	int offset; //文件当前偏移量
	char name[64];
	fileops_t ops;
	
};

#endif
