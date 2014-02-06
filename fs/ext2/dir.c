#include <ext2.h>
#include <device.h>
#include <mm.h>
#include <string.h>
#include <vfs.h>

#pragma GCC diagnostic ignored "-Wsign-compare"

/**
 * ext2_read_directory - read a directory
 * 
 * This functions returns the inode of file @filename from directory
 * @dir on device @dev.
 * 
 * If @filename is zero, then output the @dir's stuff.
 */
uint32_t ext2_read_directory(char *filename, struct ext2_dir *dir, struct device *dev)
{
	struct ext2_inode *inode = malloc(sizeof(struct ext2_inode));
	while(dir->inode != 0) {
		char *name = malloc(dir->namelength + 1);
		memcpy((uint8_t *)name, (uint8_t *) &dir->reserved+1, dir->namelength);
		name[dir->namelength] = 0;
		if(filename && strcmp(filename, name) == 0)
		{
			/* If we are looking for a file, we had found it */
			ext2_read_inode(inode, dir->inode, dev);
			free(name);
			free(inode);
			return dir->inode;
		}
		if(!filename && (uint32_t)filename != 1) {
			printk("%s\n", name);
		}
		dir = (struct ext2_dir *)((uint32_t)dir + dir->size);
		free(name);
	}
	free(inode);
	return 0;
}
/**
 * ext2_read_root_directory - read the root directory
 * 
 * This functions returns the inode of file @filename from directory
 * / on device @dev.
 * 
 * If @filename is zero, then output the @dir's stuff.
 */
uint8_t ext2_read_root_directory(char *filename, struct device *dev)
{
	struct ext2_priv_data *priv = EXT2_PRIV(dev);
	/* The root directory is always inode#2, so find BG and read the block. */
	struct ext2_inode *inode = malloc(sizeof(struct ext2_inode));
	uint8_t *root_buf = malloc(priv->blocksize);
	ext2_read_inode(inode, 2, dev);
	if((inode->type & 0xF000) != INODE_TYPE_DIRECTORY)
	{
		printk("FATAL: Root directory is not a directory!\n");
		goto r0;
	}
	/* We have found the directory!
	 * Now, load the starting block
	 */
	for(int i = 0;i < 12; i++)
	{
		uint32_t b = inode->dbp[i];
		if(b == 0) break;
		ext2_read_block(root_buf, b, dev);
		/* Now loop through the entries of the directory */
		if(ext2_read_directory(filename, (struct ext2_dir *)root_buf, dev)) {
			goto r1;
		}
	}
	if(filename && (uint32_t)filename != 1) goto r0;

r1:
	free(root_buf);
	free(inode);
	return 1;
r0:
	free(root_buf);
	free(inode);
	return 0;
}
/**
 * ext2_list_directory - List a directory
 * 
 * list the directory @dd into buffer @buffer.
 */
struct dirent *ext2_list_directory(struct file *f, struct device *dev)
{
	return 0;
	/*char *dir = dd;
	struct ext2_inode *inode = ext2_find_file_inode(dir, (struct ext2_inode *)buffer, dev);
	if(!inode) return 0;
	uint8_t *root_buf = malloc(EXT2_PRIV(dev)->blocksize);
	for(int i = 0;i < 12; i++)
	{
		uint32_t b = inode->dbp[i];
		if(!b) break;
		ext2_read_block(root_buf, b, dev);
		ext2_read_directory(0, (struct ext2_dir *)root_buf, dev);
	}
	free(root_buf);
	return 1;*/
}

