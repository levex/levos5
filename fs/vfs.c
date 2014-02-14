/** @author Levente Kurusa <levex@linux.com> **/
#include <stdint.h>
#include <vfs.h>
#include <mm.h>
#include <string.h>
#include <procfs.h>
#include <dummyfs.h>
#include <ext2.h>

#pragma GCC diagnostic ignored "-Wpointer-sign"

#define MAX_MOUNTS 16


uint8_t __init_vfs = 0;

mount_info_t **mount_points = 0;
int last_mount_id = 0;

struct device *check_mount(char *loc)
{
	for(int i = 0; i < last_mount_id; i++)
	{
		if(strcmp(loc, mount_points[i]->loc) == 0)
		{
			return mount_points[i]->dev;
		}
	}
	return 0;
}

int list_mount()
{
	for(int i = 0;i < MAX_MOUNTS;i ++)
	{
		if(!mount_points[i])break;
		printk("%s on %s type: %s\n", mount_points[i]->dev->name,
		 mount_points[i]->loc, mount_points[i]->dev->fs->name);
	}
	return 1;
}

int device_try_to_mount(struct device *dev, char *loc)
{
	if(!dev) return 1;
	if(check_mount(loc)) return 1;
	if(procfs_probe(dev) == 0)
	{
		if(procfs_mount(dev) == 0)
		{
			mount_info_t *m = (mount_info_t *)malloc(sizeof(mount_info_t));
			m->loc = loc;
			m->dev = dev;
			last_mount_id++;
			mount_points[last_mount_id - 1] = m;
			return 0;
		}
		return 1;
	}
	if(ext2_probe(dev) == 0)
	{
		if(ext2_mount(dev) == 0)
		{
			mount_info_t *m = (mount_info_t *)malloc(sizeof(mount_info_t));
			m->loc = loc;
			m->dev = dev;
			last_mount_id++;
			mount_points[last_mount_id - 1] = m;
			return 0;
		}
		return 1;
	}
	if(dummyfs_probe(dev) == 0)
	{
		if(dummyfs_mount(dev) == 0)
		{
			mount_info_t *m = (mount_info_t *)malloc(sizeof(mount_info_t));
			m->loc = loc;
			m->dev = dev;
			last_mount_id++;
			mount_points[last_mount_id - 1] = m;
			return 0;
		}
		return 1;
	}
	return 1;
}

inline uint8_t __find_mount(char *filename, int *adjust)
{
	 char *orig = (char *)malloc(strlen(filename) + 1);
	 memset(orig, 0, strlen(filename) + 1);
	 memcpy(orig, filename, strlen(filename) + 1);
	 if(orig[strlen(orig)] == '/') str_backspace(orig, '/');
	 while(1)
	 {
		//printk("check one\n");
	 	for(int i = 0;i<MAX_MOUNTS; i++)
	 	{
	 		if(!mount_points[i]) break;
	 		//printk("%s with %s\n", mount_points[i]->loc, orig);
	 		if(strcmp(mount_points[i]->loc, orig) == 0)
	 		{
	 			/* Adjust the orig to make it relative to fs/dev */
	 			*adjust = (strlen(orig) - 1);
	 			//printk("[%s] -> [%s]\n", filename, filename + *adjust);
	 			free(orig);
				return i;
	 		}
	 	}
	 	if(strcmp(orig, "/") == 0)
			break;
	 	str_backspace(orig, '/');
	 	//printk("backspaced one\n");
	 }
	 return 0;
}

uint8_t vfs_isdir(struct file *fl)
{
	int i = fl->mountid;
	return mount_points[i]->dev->fs->isdir(fl, mount_points[i]->dev);
}

struct file *vfs_open(char *filename)
{
	struct file *fl = malloc(sizeof(struct file));
	fl->fullpath = filename;
	int adjust = 0;
	int i = __find_mount(filename, &adjust);
	filename += adjust;
	fl->respath = filename;
	fl->mountid = i;
	int rc = mount_points[i]->dev->fs->findinode(fl->respath, &fl->inode,
			mount_points[i]->dev);
	if(rc)
		goto err;
		
	if(vfs_isdir(fl)) {
		fl->isdir = 1;
		fl->dpos = 0;
	}
	return fl;
err:
	free(fl);
	return 0;
}

uint32_t vfs_write(struct file *fl, uint8_t *buf, uint32_t nbytes)
{
	int i = fl->mountid;
	return mount_points[i]->dev->fs->writefile(fl, buf, nbytes,
			mount_points[i]->dev);
}

uint8_t vfs_stat(struct file *fl, struct stat *st)
{
	if(!fl) return 0;
	/*int adjust = 0;*/
	int i = /*__find_mount(filename, &adjust);*/ fl->mountid;
	/*filename += adjust;*/
	//printk("stating loc\n");
	int rc = mount_points[i]->dev->fs->stat(/*filename*/fl->respath, st,
			mount_points[i]->dev);
	return rc;
}

uint32_t vfs_read(struct file *fl, uint8_t *buf, uint32_t len)
{
	if(!fl) return 0;
	
	return mount_points[fl->mountid]->dev->fs->read(fl, buf, len,
			mount_points[fl->mountid]->dev);
}

uint8_t vfs_read_full(struct file *fl, char *buffer)
{
	/* Correct algorithm to resolve mounts:
	 * In a loop remove until '/' and then look for match
	 * if no match, continue until last '/' and then we know
	 * it is on the root_device
	 */
	 if(!fl) return 0;
	 int i = fl->mountid;
	 int rc = mount_points[i]->dev->fs->readfull(fl->respath, buffer,
				mount_points[i]->dev);
	 return rc;
}

struct dirent *vfs_readdir(struct file *dirp)
{
	int i = dirp->mountid;

	return mount_points[i]->dev->fs->read_dir(dirp, 
		mount_points[i]->dev);
}

uint32_t vfs_ls(char *dir, char* buffer)
{
	/* Algorithm:
	 * For each mount, backspace one, compare with 'dir'
	 * if yes, print out its dir name!
	 */
	buffer = buffer;
	char *orig = (char *)malloc(strlen(dir) + 1);
	memset(orig, 0, strlen(dir) + 1);
	memcpy(orig, dir, strlen(dir) + 1);
	while(1)
	{
		for(int i = 0; i < MAX_MOUNTS; i++)
		{
			if(!mount_points[i]) break;
			/* Backspace one, check if it equals dir, if so print DIR name */
			/* If the mount's location equals the backspaced location...*/
			if(strcmp(mount_points[i]->loc, orig) == 0)
			{
				/* Then adjust and send. */
				mount_points[i]->dev->fs->read_dir((struct file *)(dir + strlen(mount_points[i]->loc) - 1),
					/*buffer, */mount_points[i]->dev);
				/* Now, we have found who hosts this directory, look
				 * for those that are mounted to this directory's host.
				 */
				for(int k = 0; k < MAX_MOUNTS; k++)
				{
					if(!mount_points[k]) break;
					char *mount = (char *)malloc(strlen(mount_points[k]->loc) + 1);
					memcpy(mount, mount_points[k]->loc, strlen(mount_points[k]->loc) + 1);
					str_backspace(mount, '/');
					if(strcmp(mount, dir) == 0)
					{
						char *p = mount_points[k]->loc + strlen(dir);
						if(strlen(p) == 0 || strlen(p) == 1) continue;
						printk("%s\n", p);
					}
					free(mount);
				}
				break;
			}
		}
		if(strcmp(orig, "/") == 0) break;
		str_backspace(orig, '/');
	}
	free(orig);
	return 1;
}

uint8_t vfs_exist_in_dir(char *wd, char *fn)
{
	char *filename = (char *)malloc(strlen(wd) + 2 + strlen(fn));
	memset(filename, 0, strlen(wd) + 2 + strlen(fn));
	memcpy(filename, wd, strlen(wd));
	memcpy(filename+strlen(wd), fn, strlen(fn));
	memset(filename+strlen(wd)+strlen(fn) + 1, '\0', 1);
	/* Algorithm:
	 * For each mount, check if it is mounted to wd
	 * If it is, return 1
	 */
	 /* @TODO: fix */

	if(filename[strlen(filename)] != '/') 
	{
		uint32_t index = strlen(filename);
		filename[index] = '/';
		filename[index+1] = 0;
	}
	int rc = 0;
	char *o = (char *)malloc(strlen(filename) + 2);
	memset(o, 0, strlen(filename) + 2);
	memcpy(o, filename, strlen(filename) + 1);
	/*if(o[strlen(o)] != '/') 
	{
		uint32_t index = strlen(o);
		o[index] = '/';
		o[index+1] = 0;
	}*/

	while(1)
	{
		for(int i = 0;i < MAX_MOUNTS; i++)
		{
			if(!mount_points[i]) break;
			//printk("Checking %s with %s\n", o, mount_points[i]->loc);
			if(strcmp(o, mount_points[i]->loc) == 0)
			{
				//printk("filename:%s\n", filename);
				//printk("strlen: %d str:%s\n",strlen(mount_points[i]->loc), mount_points[i]->loc );
				filename += strlen(mount_points[i]->loc) - 1;
				/*printk("Passing: %s fn:%s, wd:%s to %s (%d)\n", filename, fn, wd,
					mount_points[i]->dev->name, mount_points[i]->dev->unique_id);*/
				rc = mount_points[i]->dev->fs->exist(filename,
					mount_points[i]->dev);
				free(o);
				free(filename);
				return rc;
			}
		}
		if(strcmp(o, "/") == 0)
			break;
		str_backspace(o, '/');
	}
	free(o);
	free(filename);
	return rc;
}

int vfs_init()
{
	mount_points = (mount_info_t **)malloc(sizeof(uint32_t) * MAX_MOUNTS);
	__init_vfs = 1;
	return 0;
}
