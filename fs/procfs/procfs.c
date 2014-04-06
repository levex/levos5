#include <hal.h>
#include <vfs.h>
#include <procfs.h>
#include <device.h>
#include <mm.h>
#include <string.h>

#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wpointer-sign"

static int procfs_mounted = 0;

struct procfile {
	char *name;
	struct file_operations *fops;
};

struct procfile *file_list = 0;
static int files = 0;

uint32_t procfs_writefile(struct file *fl, uint8_t *buf, uint32_t len, struct device *dev);
uint8_t procfs_findinode(char *fn, uint32_t *out, struct device *dev);
struct dirent *procfs_readdir(struct file *f, struct device *dev);
uint8_t procfs_isdir(struct file *f, struct device *dev);
uint32_t procfs_read(struct file *f, uint8_t *buf, uint32_t len, struct device *dev);

struct device procfs_dev = {
	.valid = 1,
	.id = 0x1337,
	.flags = DEVICE_FLAG_BLOCK | DEVICE_FLAG_NOREAD | DEVICE_FLAG_NOWRITE,
	.name = "procdev",
	
	.read = 0,
	.write = 0,
};

uint8_t *dummy_buffer = 0;
uint32_t dummy_len = 0;

uint8_t procfs_stat(char *file, struct stat *st, struct device *dev)
{
	dev = dev;
	if(!strcmp(file, "/devconf")) {
		st->st_size = dummy_len;
		return 0;
	}
	return 1;
}

struct filesystem procfs_default = {
	.name = "procfs",
	.probe = procfs_probe,
	.read = procfs_read,
	.readfull = 0,//uint8_t (*read)(char *, char *, struct device *, void *);
	.read_dir = procfs_readdir, //uint8_t (*read_dir)(char *, char *, struct device *, void *);
	.findinode = procfs_findinode,
	.isdir = procfs_isdir,
	.stat = procfs_stat,
	.touch = 0,//uint8_t (*touch)(char *fn, struct device *, void *);
	.writefile = procfs_writefile,//uint8_t (*writefile)(char *fn, char *buf, uint32_t len, struct device *, void *);
	.exist = 0,//uint8_t (*exist)(char *filename, struct device *, void *);
	.mount = procfs_mount,//uint8_t (*mount)(struct device *, void *);
};

struct dirent *procfs_readdir(struct file *f, struct device *dev)
{
	if(!f || !f->isdir) return 0;
	dev = dev;
	//printk("procfs_readdir! inode: %d dpos:%d\n", f->inode, f->dpos);
	switch(f->inode)
	{
		case 99:
			switch(f->dpos) {
				case 0: {
					struct dirent *de = malloc(sizeof(struct dirent));
					de->d_ino = 99;
					memcpy(de->d_name, ".", 2);
					f->dpos ++;
					return de;
				}
				case 1: {
					struct dirent *de = malloc(sizeof(struct dirent));
					de->d_ino = 99;
					memcpy(de->d_name, "..", 3);
					f->dpos ++;
					return de;
				}
				default:
					if (f->dpos >= files)
						return 0;
					struct procfile *pf = &file_list[f->dpos - 2];
					struct dirent *de = malloc(sizeof(*de));
					memcpy(de->d_name, pf->name, strlen(pf->name) + 1);
					de->d_ino = 101 + f->dpos;
					return de;
			}
	}
	return 0;
}

int procfs_create_file(char *path, struct file_operations *fops)
{
	struct procfile *file = malloc(sizeof(*file));
	file->fops = fops;
	file->name = path;
	files ++;
	file_list = realloc(file_list, (files) * sizeof(*file));
	file_list[files] = *file;
	return 0;
}

uint8_t procfs_isdir(struct file *f, struct device *dev)
{
	dev = dev;
	return (f->inode == 99);
}

uint8_t procfs_findinode(char *fn, uint32_t *out, struct device *dev)
{
	dev = dev;
	if(strcmp(fn, "/devconf") == 0) {
		*out = 100;
		return 0;
	}
		
	if(strcmp(fn, "/") == 0) {
		*out = 99;
		return 0;
	}
	*out = 0;
	return 1;
}

uint32_t procfs_writefile(struct file *fl, uint8_t *buf, uint32_t len, struct device *dev)
{
	dev = dev;
	switch(fl->inode)
	{
		case 100:
			if(len > dummy_len) {
				dummy_buffer = realloc(dummy_buffer, len);
				dummy_len = len;
			}
			memcpy(dummy_buffer, buf, len);
			return len;
		default:
			if(fl->inode < 103)
				return 0;
			struct procfile *pf = &file_list[fl->inode - 103];
			return pf->fops->write(fl, buf, len);
	}
	return 0;
}

uint32_t procfs_read(struct file *f, uint8_t *buf, uint32_t len, struct device *dev)
{
	dev = dev;
	switch(f->inode)
	{
		case 100:
			if(f->lpos >= dummy_len)
				return 0;
			if(dummy_len > f->lpos + len) {
				memcpy(buf, dummy_buffer + f->lpos, len);
				f->lpos += dummy_len;
				return len;
			} else {
				memcpy(buf, dummy_buffer + f->lpos, dummy_len - f->lpos);
				f->lpos = dummy_len;
				return dummy_len - f->lpos;
			}
			break;
		default:
			if(f->inode < 103)
				return 0;
			struct procfile *pf = &file_list[f->inode - 103];
			return pf->fops->read(f, buf, len);
	}
	return 0;
}

int procfs_probe(struct device *dev)
{
	if (!dev || !dev->valid || dev->id != 0x1337)
		return 1;

	if (procfs_mounted)
		return 1;
		
	/* allocate space for a new filesystem */
	struct filesystem *fs = malloc(sizeof(struct filesystem));
	/* copy the default to it */
	memcpy((uint8_t *)fs, (uint8_t *)&procfs_default, sizeof(struct filesystem));	
	/* set the device's filesystem to it */
	dev->fs = fs;
	
	printk("Probing for procfs\n");
	
	dummy_buffer = malloc(1024);
	dummy_len = 1024;
	
	/* probe successful */
	return 0;
}

uint8_t devconf_write(struct file *f, uint8_t *buf, uint32_t len)
{
	if(len > dummy_len) {
		dummy_buffer = realloc(dummy_buffer, len);
		dummy_len = len;
	}
	memcpy(dummy_buffer, buf, len);
	return len;

}

uint8_t devconf_read(struct file *f, uint8_t *buf, uint32_t len)
{
	if(f->lpos >= dummy_len)
		return 0;
	if(dummy_len > f->lpos + len) {
		memcpy(buf, dummy_buffer + f->lpos, len);
		f->lpos += dummy_len;
		return len;
	} else {
		memcpy(buf, dummy_buffer + f->lpos, dummy_len - f->lpos);
		f->lpos = dummy_len;
		return dummy_len - f->lpos;
	}

}

int procfs_mount(struct device *dev)
{
	if (!dev)
		return 1;
		
	procfs_mounted = 1;
	printk("Mount procfs successful!\n");	
	
	file_list = malloc(1 * sizeof(struct procfile));
	struct file_operations *_fops = malloc(sizeof(*_fops));
	_fops->write = devconf_write;
	_fops->read = devconf_read;
	procfs_create_file("/test", _fops);
	return 0;
}
