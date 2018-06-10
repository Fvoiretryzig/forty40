#include <os.h>
#inlucde <vfs.h>
#include<libc.h>


typedef struct filesystem filesystem_t;
typedef struct inode inode_t;
typedef struct file file_t;

static void vfs_init();
static int access(const char *path, int mode);
static int mount(const char *path, filesystem_t *fs);
static int unmount(const char *path);
static int open(const char *path, int flags);
static ssize_t read(int fd, void *buf, size_t nbyte);
static ssize_t write(int fd, void *buf, size_t nbyte);
static off_t lseek(int fd, off_t offset, int whence);
static int close(int fd);

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
