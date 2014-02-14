#include <ext2.h>
#include <device.h>
#include <mm.h>
#include <vfs.h>
#include <string.h>

#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wpointer-sign"

/**
 * ext2_find_file_inode - Put the inode to @inode for file @ff
 */
uint32_t ext2_find_file_inode(char *ff, struct ext2_inode *inode, struct device *dev)
{
	char *filename = malloc(strlen(ff) + 1);
	memcpy(filename, ff, strlen(ff) +1);
	size_t n = strsplit(filename, '/');
	filename ++; // skip the first crap
	uint32_t retnode = 0;
	uint8_t *root_buf = malloc(EXT2_PRIV(dev)->blocksize);
	if(n > 1)
	{
		/* Read inode#2 (Root dir) into inode */
		ext2_read_inode(inode, 2, dev);
		/* Now, loop through the DPB's and see if it contains this filename */
		while(--n)
		{
			for(int i = 0; i < 12; i++)
			{
				uint32_t b = inode->dbp[i];
				if(!b) break;
				ext2_read_block(root_buf, b, dev);
				uint32_t rc = ext2_read_directory(filename, (struct ext2_dir *)root_buf, dev);
				if(!rc)
				{
					if(strcmp(filename, "") == 0)
					{
						free(filename);
						return strcmp(ff, "/")?retnode:2;
					}
					free(filename);
					return 0;
				} else {
					/* inode now contains that inode
					* get out of the for loop and continue traversing
					*/
					retnode = rc;
					goto fix;
				}
			}
			fix:;
			uint32_t s = strlen(filename);
			filename += s + 1;
			ext2_read_inode(inode, retnode, dev);
		}
	} else {
		/* This means the file is in the root directory */
		retnode = ext2_read_root_directory(filename, dev);
		ext2_read_inode(inode, retnode, dev);
	}
	free(filename);
	return retnode;
}

uint8_t ext2_findinode(char *fn, uint32_t *out, struct device *dev)
{
	struct ext2_inode *in = malloc(sizeof(struct ext2_inode));
	int node = ext2_find_file_inode(fn, in, dev);
	free(in);
	*out = node;
	return node ? 0 : 1;
}

#define SIZE_OF_SINGLY (priv->blocksize * priv->blocksize / 4)
/**
 * ext2_read_singly_linked - Read a singly linked list
 * 
 * This function reads the singly linked list of @blockid into @buf.
 */
uint8_t ext2_read_singly_linked(uint32_t blockid, uint8_t *buf, struct device *dev)
{
	struct ext2_priv_data *priv = EXT2_PRIV(dev);
	uint32_t maxblocks = ((priv->blocksize) / (sizeof(uint32_t)));
	uint8_t *root_buf = malloc(priv->blocksize);
	/* A singly linked block is essentially an array of
	 * uint32_t's storing the block's id which points to data
	 */
	 /* Read the block into root_buf */
	 ext2_read_block(root_buf, blockid, dev);
	 /* Loop through the block id's reading them into the appropriate buffer */
	 uint32_t *block = (uint32_t *)root_buf;
	 for(int i =0;i < maxblocks; i++)
	 {
	 	/* If it is zero, we have finished loading. */
	 	if(block[i] == 0) break;
	 	/* Else, read the block into the buffer */
	 	ext2_read_block(buf + i * priv->blocksize, block[i], dev);
	 }
	 free(root_buf);
	 return 1;
}
/**
 * ext2_read_doubly_linked - Read a doubly linked list
 * 
 * This function reads the doubly linked list of @blockid into @buf.
 */
uint8_t ext2_read_doubly_linked(uint32_t blockid, uint8_t *buf, struct device *dev)
{
	struct ext2_priv_data *priv = EXT2_PRIV(dev);
	uint32_t maxblocks = ((priv->blocksize) / (sizeof(uint32_t)));
	uint8_t *block_buf = malloc(priv->blocksize);
	/* A singly linked block is essentially an array of
	 * uint32_t's storing the block's id which points to data
	 */
	 /* Read the block into root_buf */
	 ext2_read_block(block_buf, blockid, dev);
	 /* Loop through the block id's reading them into the appropriate buffer */
	 uint32_t *block = (uint32_t *)block_buf;
	 uint32_t s = SIZE_OF_SINGLY;
	 for(int i =0;i < maxblocks; i++)
	 {
	 	/* If it is zero, we have finished loading. */
	 	if(block[i] == 0) break;
	 	/* Else, read the block into the buffer */
	 	ext2_read_singly_linked(block[i], buf + i * s , dev);
	 }
	 return 1;
}

/**
 * ext2_read_file - Read a file
 * 
 * This functions reads the file @fn into buffer @buffer.
 */
uint8_t ext2_read_file_full(char *fn, char *buffer, struct device *dev)
{
	struct ext2_priv_data *priv = EXT2_PRIV(dev);
	/* Put the file's inode to the buffer */
	struct ext2_inode *minode = malloc(sizeof(struct ext2_inode));
	uint8_t *root_buf = malloc(priv->blocksize);
	char *filename = fn;
	uint32_t inodeid = ext2_find_file_inode(filename, minode, dev);
	if(!inodeid)
	{
		printk("File inode not found.\n");
		return 0;
	}
	ext2_read_inode(minode, inodeid, dev);
	for(int i = 0; i < 12; i++)
	{
		uint32_t b = minode->dbp[i];
		if(b == 0) { return 1; }
		if(b > priv->sb.blocks) {
			printk("%s: block %d outside range (max: %d)!\n", __func__,
				b, priv->sb.blocks);
			for(;;);
		}
		ext2_read_block(root_buf, b, dev);
		memcpy((uint8_t *)(buffer + i*(priv->blocksize)), (uint8_t *)root_buf, priv->blocksize);
	}
	if(minode->singly_block) {
		//kprintf("Block of singly: %d\n", minode->singly_block);
		ext2_read_singly_linked(minode->singly_block, buffer + 12*(priv->blocksize), dev);
	}
	if(minode->doubly_block) {
		uint32_t s = SIZE_OF_SINGLY + 12* (priv->blocksize);
		ext2_read_doubly_linked(minode->doubly_block, buffer + s, dev);
	}
	return 1;
}

uint32_t ext2_read_file(struct file *fl, uint8_t *buf, uint32_t len, struct device *dev)
{
	if(!fl || !buf || !dev || !len) return 0;
	struct ext2_inode *inode = malloc(sizeof(struct ext2_inode));
	ext2_read_inode(inode, fl->inode, dev);
	struct ext2_priv_data *priv = EXT2_PRIV(dev);
	uint32_t finalpos = fl->lpos + len;
	/* get size */
	struct stat st;
	vfs_stat(fl, &st);
	/* if zero, empty file! */
	if(!st.st_size) {
		if(fl->lpos) {
			printk("WARN: Inconsistency detected between lposition and file size!\n");
		}
		return 0;
	}
	if(fl->lpos > st.st_size)
		panic("File position overloaded.\n");
	if(finalpos > st.st_size)
		finalpos = st.st_size;
	/* oh finally we are cool */
	uint32_t bpos = finalpos / priv->blocksize;
	uint8_t *block_buf = malloc(priv->blocksize);
	if(bpos < 12) {
		/* direct block pointer */
		uint32_t fpos = bpos % 12;
		ext2_read_block(block_buf, inode->dbp[fpos], dev);
		memcpy(buf, block_buf + fl->lpos, len);
		fl->lpos += len;
		return len;
	} else if(bpos < SIZE_OF_SINGLY) {
		/* in the singly linked list */
	}
	return 0;
}

uint8_t ext2_stat(char *file, struct stat *st, struct device *dev)
{
	//printk("%s: file=%s\n", __func__, file);
	struct ext2_inode *inode = malloc(sizeof(struct ext2_inode));
	uint32_t in = ext2_find_file_inode(file, inode, dev);
	if (!in) goto out;
	/* first check the type */
	if(inode->type & 0xC000) st->st_mode |= S_IFSOCK;
	if(inode->type & 0xA000) st->st_mode |= S_IFLNK;
	if(inode->type & 0x8000) st->st_mode |= S_IFREG;
	if(inode->type & 0x6000) st->st_mode |= S_IFBLK;
	if(inode->type & 0x4000) st->st_mode |= S_IFDIR;
	if(inode->type & 0x2000) st->st_mode |= S_IFCHR;
	if(inode->type & 0x1000) st->st_mode |= S_IFIFO;
	
	st->st_dev = dev->id;
	st->st_ino = in;
	st->st_nlink = 1;
	st->st_uid = 1000;
	st->st_gid = 1000;
	st->st_rdev = 0;
	st->st_size = inode->size;
	/*st->st_blksize = priv->blocksize;
	st->st_blocks = (inode->size / priv->blocksize) + 1;*/
out:
	//printk("returning from %s\n", __func__);
	free(inode);
	return 0;
}

uint8_t ext2_exist(char *file, struct device *dev)
{
	return ext2_read_file_full(file, 0, dev);
}
