/** @author Levente Kurusa <levex@linux.com> **/
#ifndef __VFS_H_
#define __VFS_H_

#include <device.h>
#include <time.h>
#include <stdint.h>

#define		S_IFDIR		0040000	/* directory */
#define		S_IFCHR		0020000	/* character special */
#define		S_IFBLK		0060000	/* block special */
#define		S_IFREG		0100000	/* regular */
#define		S_IFLNK		0120000	/* symbolic link */
#define		S_IFSOCK	0140000	/* socket */
#define		S_IFIFO		0010000	/* fifo */

struct stat {
	uint16_t st_dev;
	uint16_t st_ino;
	uint32_t st_mode;
	uint16_t st_nlink;
	uint16_t st_uid;
	uint16_t st_gid;
	uint16_t st_rdev;
	uint32_t st_size;
	/*32*/

	/*struct tm st_atime;
	struct tm st_mtime;
	struct tm st_ctime;

	uint32_t st_blksize;
	uint32_t st_blocks;*/
};

struct DIR {
};

struct dirent {
	uint16_t d_ino;
	char d_name[124];
};

struct file {
	char *fullpath; /* full path from root */
	char *respath; /* path inside the mount */
	char *name; /* filename */
	uint32_t inode; /* inode number */
	uint32_t mountid; /* which mount am I on? */
	uint32_t lpos; /* position when not using readfull */
	
	uint8_t isdir; /* is a directory? */
	uint32_t dpos; /* if directory, the position of readdir */
};

struct filesystem {
	char *name;
	int (*probe)(struct device *);
	uint8_t (*readfull)(char *, char *, struct device *);
	uint32_t (*read)(struct file *, uint8_t *buf, uint32_t len, struct device *);
	struct dirent * (*read_dir)(struct file *f, struct device *dev);
	uint8_t (*isdir)(struct file *f, struct device *dev);
	uint8_t (*touch)(char *fn, struct device *);
	uint32_t (*writefile)(struct file *f, uint8_t *buf, uint32_t len, struct device *);
	uint8_t (*stat)(char *fn, struct stat *st, struct device *dev);
	uint8_t (*exist)(char *filename, struct device *);
	uint8_t (*findinode)(char *fn, uint32_t *out, struct device *dev);
	int (*mount)(struct device *);
	void *priv_data;
};

typedef struct __mount_info_t {
	char *loc;
	struct device *dev;
} mount_info_t;

extern struct dirent *vfs_readdir(struct file *dirp);
extern struct file *vfs_open(char *filename);
extern uint8_t vfs_isdir(struct file *fl);
extern uint8_t vfs_stat(struct file *fl, struct stat *st);
extern uint32_t vfs_read(struct file *fl, uint8_t *buf, uint32_t len);
extern uint32_t vfs_write(struct file *fl, uint8_t *buf, uint32_t nbytes);
extern uint8_t vfs_read_full(struct file *fl, char *buffer);

extern uint8_t vfs_readfull(struct file *f, char* buf);
extern uint32_t vfs_ls(char *d, char *buf);
extern uint8_t vfs_stat(struct file *f, struct stat *st);
extern uint8_t vfs_exist_in_dir(char *wd, char* fn);

extern int vfs_init();

extern int list_mount();

extern int device_try_to_mount(struct device *dev, char *location);


struct initrd_priv {
	uint32_t start;
	uint32_t end;
};
extern int initrd_create(uint32_t *s, uint32_t *e);

#endif
