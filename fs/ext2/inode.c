#include <ext2.h>
#include <vfs.h>
#include <device.h>
#include <mm.h>

#pragma GCC diagnostic ignored "-Wsign-compare"

/**
 * ext2_find_new_inode_id - find and apply new inode id
 * 
 * This functions finds a new inode id from device @dev,
 * outputs to @id, and once found updates superblock as well.
 */
void ext2_find_new_inode_id(uint32_t *id, struct device *dev)
{
	/* Algorithm: Loop through the block group descriptors,
	 * and find the number of unalloc inodes
	 */

	/* Loop through the block groups */
	struct ext2_priv_data *priv = EXT2_PRIV(dev);
	uint8_t *buffer = malloc(priv->blocksize);
	if (!buffer)
		return;
	ext2_read_block(buffer, priv->first_bgd, dev);
	struct ext2_block_group_desc *bg = (struct ext2_block_group_desc *)buffer;
	for(int i = 0; i < priv->number_of_bgs; i++)
	{
		if(bg->num_of_unalloc_inode)
		{
			/* If the bg has some unallocated inodes,
			 * find which inode is unallocated
			 * This is easy:
			 * For each bg we have sb->inodes_in_blockgroup inodes,
			 * this one has num_of_unalloc_inode inodes unallocated,
			 * therefore the latest id is:
			 */
			 *id = ((i + 1) * priv->sb.inodes_in_blockgroup) - bg->num_of_unalloc_inode + 1;
			 bg->num_of_unalloc_inode --;
			 ext2_write_block(buffer, priv->first_bgd + i, dev);
			 /* Now, update the superblock as well */
			 ext2_read_block(buffer, priv->sb.superblock_id, dev);
			 struct ext2_superblock *sb = (struct ext2_superblock *)buffer;
			 sb->unallocatedinodes --;
			 ext2_write_block(buffer, priv->sb.superblock_id, dev);
			 goto out;
		}
		bg++;
	}
out: free(buffer);
}

/**
 * ext2_read_inode - Read an inode
 * 
 * This functions reads inode @inode from device @dev,
 * and copies the read data to @inode_buf
 */
void ext2_read_inode(struct ext2_inode *inode_buf, uint32_t inode, struct device *dev)
{
	struct ext2_priv_data *priv = EXT2_PRIV(dev);
	uint32_t bg = (inode - 1) / priv->sb.inodes_in_blockgroup;
	uint32_t i = 0;
	/* Now we have which BG the inode is in, load that desc */
	uint8_t *block_buf = malloc(priv->blocksize);
	ext2_read_block(block_buf, priv->first_bgd, dev);
	struct ext2_block_group_desc *bgd = (struct ext2_block_group_desc *)block_buf;
	/* Seek to the BG's desc */
	for(i = 0; i < bg; i++)
		bgd++;
	/* Find the index and seek to the inode */
	uint32_t index = (inode - 1) % priv->sb.inodes_in_blockgroup;
	uint32_t block = (index * sizeof(struct ext2_inode))/ priv->blocksize;
	ext2_read_block(block_buf, bgd->block_of_inode_table + block, dev);
	struct ext2_inode* _inode = (struct ext2_inode *)block_buf;
	index = index % priv->inodes_per_block;
	for(i = 0; i < index; i++)
		_inode++;
	/* We have found the inode! */
	memcpy((uint8_t *)inode_buf,(uint8_t *) _inode, sizeof(struct ext2_inode));

	free(block_buf);
}
/**
 * ext2_write_inode - Write an inode
 * 
 * This function writes inode @ii to device @dev from buffer
 * @inode_buf
 */
void ext2_write_inode(struct ext2_inode *inode_buf, uint32_t ii, struct device *dev)
{
	struct ext2_priv_data *priv = EXT2_PRIV(dev);
	uint32_t bg = (ii - 1) / priv->sb.inodes_in_blockgroup;
	uint32_t i = 0;
	/* Now we have which BG the inode is in, load that desc */
	uint8_t *block_buf = malloc(priv->blocksize);
	ext2_read_block(block_buf, priv->first_bgd, dev);
	struct ext2_block_group_desc *bgd = (struct ext2_block_group_desc *)block_buf;
	/* Seek to the BG's desc */
	for(i = 0; i < bg; i++)
		bgd++;
	/* Find the index and seek to the inode */
	uint32_t index = (ii - 1) % priv->sb.inodes_in_blockgroup;
	uint32_t block = (index * sizeof(struct ext2_inode))/ priv->blocksize;
	uint32_t final = bgd->block_of_inode_table + block;
	ext2_read_block(block_buf, final, dev);
	struct ext2_inode *_inode = (struct ext2_inode *)block_buf;
	index = index % priv->inodes_per_block;
	for(i = 0; i < index; i++)
		_inode++;
	/* We have found the inode! */
	memcpy((uint8_t *)_inode, (uint8_t *)inode_buf, sizeof(struct ext2_inode));
	ext2_write_block(block_buf, final, dev);
	
	free(block_buf);
}

/**
 * ext2_get_inode_block - get inode block information
 * 
 * This functions finds the block (to @b) and inodeoffset (to @ioff)
 * of inode @inode from device @dev
 */
uint32_t ext2_get_inode_block(uint32_t inode, uint32_t *b, uint32_t *ioff, struct device *dev)
{
	struct ext2_priv_data *priv = EXT2_PRIV(dev);
	uint32_t bg = (inode - 1) / priv->sb.inodes_in_blockgroup;
	uint32_t i = 0;
	/* Now we have which BG the inode is in, load that desc */
	uint8_t *block_buf = malloc(priv->blocksize);
	ext2_read_block(block_buf, priv->first_bgd, dev);
	struct ext2_block_group_desc *bgd = (struct ext2_block_group_desc *)block_buf;
	/* Seek to the BG's desc */
	for(i = 0; i < bg; i++)
		bgd++;
	/* Find the index and seek to the inode */
	uint32_t index = (inode - 1) % priv->sb.inodes_in_blockgroup;
	uint32_t block = (index * sizeof(struct ext2_inode))/ priv->blocksize;
	index = index % priv->inodes_per_block;
	*b = block + bgd->block_of_inode_table;
	*ioff = index;
	return 1;
}
