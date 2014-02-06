#include <ext2.h>
#include <device.h>
#include <mm.h>
#include <vfs.h>

#pragma GCC diagnostic ignored "-Wsign-compare"

/**
 * ext2_read_block - Read a block
 * 
 * This function reads block @block from device @dev to
 * buffer @buf.
 */
void ext2_read_block(uint8_t *buf, uint32_t block, struct device *dev)
{
	uint32_t sectors_per_block = EXT2_PRIV(dev)->sectors_per_block;
	if(!sectors_per_block) sectors_per_block = 1;
	dev->read(dev, buf, block*sectors_per_block, sectors_per_block);
}

/**
 * ext2_write_block - Write a block
 * 
 * This function writes block @block to device @dev from
 * buffer @buf.
 */
void ext2_write_block(uint8_t *buf, uint32_t block, struct device *dev)
{
	uint32_t sectors_per_block = EXT2_PRIV(dev)->sectors_per_block;
	if(!sectors_per_block) sectors_per_block = 1;
	dev->write(dev, buf, block*sectors_per_block, sectors_per_block);
}

/**
 * ext2_alloc_block - Allocate a new block id
 * 
 * This functions allocates a new block from device @dev, and applies
 * the allocation to the superblock. Outputs to @out
 */
void ext2_alloc_block(uint32_t *out, struct device *dev)
{
	/* Algorithm: Loop through block group descriptors,
	 * find which bg has a free block
	 * and set that.
	 */
	 struct ext2_priv_data *priv = EXT2_PRIV(dev);
	 uint8_t *buffer = malloc(priv->blocksize);
	 ext2_read_block(buffer, priv->first_bgd, dev);
	 struct ext2_block_group_desc *bg = (struct ext2_block_group_desc *)buffer;
	 for(int i = 0; i < priv->number_of_bgs; i++)
	 {
	 	if(bg->num_of_unalloc_block)
	 	{
	 		*out = priv->sb.blocks - bg->num_of_unalloc_block + 1;
	 		bg->num_of_unalloc_block --;
	 		ext2_write_block(buffer, priv->first_bgd + i, dev);

	 		ext2_read_block(buffer, priv->sb.superblock_id, dev);
			struct ext2_superblock *sb = (struct ext2_superblock *)buffer;
			sb->unallocatedblocks --;
			ext2_write_block(buffer, priv->sb.superblock_id, dev);
			goto out;
	 	}
	 	bg++;
	 }
out: free(buffer);
}
