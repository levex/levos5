/** @author Levente Kurusa <levex@linux.com> **/
#include <display.h>
#include <ext2.h>
#include <vfs.h>
#include <device.h>
#include <mm.h>

uint8_t ext2_isdir(struct file *f, struct device *dev)
{
	f = f;
	dev = dev;
	/* @stub */
	return 0;
}

uint32_t ext2_writefile(struct file *fl, uint8_t *buf, uint32_t len, struct device *dev)
{
	/* @stub */
	fl = fl;
	buf = buf;
	len = len;
	dev = dev;
	return 0;
}

int ext2_probe(struct device *dev)
{
	/* Read in supposed superblock location and check sig */
	if(!dev->read)
	{
		printk("no read on device %s\n", dev->name);
		return 1;
	}
	uint8_t *buf = (uint8_t *)malloc(1024);
	dev->read(dev, buf, 2, 2);
	struct ext2_superblock *sb = (struct ext2_superblock *) buf;
	if(sb->ext2_sig != EXT2_SIGNATURE)
	{
		printk("device %s failed sig test\n", dev->name);
		return 1;
	}
	struct filesystem *fs = malloc(sizeof(struct filesystem));
	struct ext2_priv_data *priv = malloc(sizeof(struct ext2_priv_data));
	memcpy((uint8_t *)&priv->sb, (uint8_t *)sb, sizeof(struct ext2_superblock));
	/* Calculate volume length */
	uint32_t blocksize = 1024 << sb->blocksize_hint;
	priv->blocksize = blocksize;
	priv->inodes_per_block = blocksize / sizeof(struct ext2_inode);
	priv->sectors_per_block = blocksize / 512;
	printk("ext2: size of volume: %d bytes\n", blocksize*(sb->blocks));
	/* Calculate the number of block groups */
	uint32_t number_of_bgs0 = sb->blocks / sb->blocks_in_blockgroup;
	if(!number_of_bgs0) number_of_bgs0 = 1;
	priv->number_of_bgs = number_of_bgs0;
	/* Now, we have the size of a block,
	 * calculate the location of the Block Group Descriptor
	 * The BGDT is located directly after the SB, so obtain the
	 * block of the SB first. This is located in the SB.
	 */
	uint32_t block_bgdt = sb->superblock_id + (sizeof(struct ext2_superblock) / blocksize);
	priv->first_bgd = block_bgdt;
	fs->name = "EXT2";
	fs->probe = ext2_probe;
	fs->mount = ext2_mount;
	fs->readfull = ext2_read_file_full;
	fs->read = ext2_read_file;
	fs->exist = ext2_exist;
	fs->writefile = ext2_writefile;
	fs->stat = ext2_stat;
	fs->isdir = ext2_isdir;
	fs->findinode = ext2_findinode;
	fs->read_dir = ext2_list_directory;
	fs->priv_data = (uint8_t *)priv;
	dev->fs = fs;
	printk("ext2 probe successful!\n");
	return 0;
}

int ext2_mount(struct device *dev)
{
	printk("Mounting ext2 on device %s\n", dev->name);
	/*ext2_priv_data *priv = privd;
	if(ext2_read_root_directory((char *)1, dev, priv))
		return 1;*/
	return 0;
}
