#include <drivers/block.h>
#include <drivers/block/floppy.h>
#include <drivers/block/ide.h>
#include <drivers/fs/fat.h>

void block_init(void)
{
	floppy_init();
	ide_init();
	fs_fat_init();
}
