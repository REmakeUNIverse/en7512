#include <linux/init.h>
#include <linux/types.h>
#include <linux/root_dev.h>
#include <linux/kernel.h>
#include <linux/mtd/map.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/vmalloc.h>
#include <asm/io.h>
#include <asm/tc3162/tc3162.h>
// #include <flash/flash_global_def.h>

#define WINDOW_ADDR 0x1fc00000
#define WINDOW_SIZE 0x400000
#define BUSWIDTH 	2

static struct mtd_info *tc3162_mtd_info;

static struct map_info tc3162_map = {
       .name = "tc3162",
       .size = WINDOW_SIZE,
       .bankwidth = BUSWIDTH,
       .phys = WINDOW_ADDR,
};

static struct mtd_partition tc3162_parts[] = {
	{
		name:	"bootloader",
		size:	0x40000,
		offset: 0,
	},
	{
		name:	"romfile",
		size:	0x40000,
		offset: MTDPART_OFS_APPEND
	},
	{
		name:	"kernel",
		size:	2 * 1024 * 1024,
		offset: MTDPART_OFS_APPEND
	},
	{
		name:	"rootfs",
		size:	30 * 1024 * 1024,
		offset: MTDPART_OFS_APPEND
	}
};

static int tc3162_parts_size = sizeof(tc3162_parts) / sizeof(tc3162_parts[0]);

int tc3162_map_init(void) {
#ifdef TCSUPPORT_ADDR_MAPPING
	/* add address mapping on 7510. Pork */
	if (isMT751020 || isMT7505 || isEN751221) {
		uint32 tmpVal;
		tmpVal = regRead32(0xbfb00038);
		tmpVal &= 0xffe0e0e0;
		tmpVal |= 0x80070f00;
		regWrite32(0xbfb00038, tmpVal);
		// VPint(0xbfb00038) |= 0x80070F00;
		printk("tc3162: flash device 0x%08x at 0x%08x\n", 0x1000000, 0x1c000000);
		tc3162_map.virt = ioremap(0x1c000000, 0x1000000);
		tc3162_map.phys = 0x1c000000;
		tc3162_map.size = 0x1000000;
		ioremap(WINDOW_ADDR, WINDOW_SIZE);
	}
	/* add 8M 16M flash support. shnwind */
	else if (isTC3162U || isTC3182 || isRT65168 || isRT63165 || isRT63365 || isRT63260) {
#else
	if (isTC3162U || isTC3182 || isRT65168 || isRT63165 || isRT63365 || isRT63260 || isMT751020 || isMT7505 || isEN751221) {
#endif //TCSUPPORT_ADDR_MAPPING
		// header = (unsigned int *)0xb0020000;
		/* Enable addr bigger than 4M support. */
		VPint(0xbfb00038) |= 0x80000000;
		printk("tc3162: flash device 0x%08x at 0x%08x\n", 0x1000000, 0x10000000);
		tc3162_map.virt = ioremap(0x10000000, 0x1000000);
		tc3162_map.phys = 0x10000000;
		tc3162_map.size = 0x1000000;
		ioremap(WINDOW_ADDR, WINDOW_SIZE);
	} else {
		// header = (unsigned int *)0xbfc20000;
		printk("tc3162: flash device 0x%08x at 0x%08x\n", WINDOW_SIZE, WINDOW_ADDR);
		tc3162_map.virt = ioremap(WINDOW_ADDR, WINDOW_SIZE);
	}
	if (!tc3162_map.virt) {
   		printk("tc3162: ioremap failed.\n");
		return -EIO;
	}

	simple_map_init(&tc3162_map);

	return 0;
}

static int tc3162_mtd_info_init(void) {
	if (IS_NANDFLASH) {
		tc3162_mtd_info = do_map_probe("nandflash_probe", &tc3162_map);
	} else if (IS_SPIFLASH) {
		printk("tc3162: SPIFLASH driver is not supported at the momemnt. There is a driver in TP-link VR300 sources, but it is not upgraded.\n")
		// tc3162_mtd_info = do_map_probe("spiflash_probe", &tc3162_map);
	} else {
		tc3162_mtd_info = do_map_probe("cfi_probe", &tc3162_map);
	}

	if (!tc3162_mtd_info) {
		iounmap(tc3162_map.virt);
		return -ENXIO;
	}

  	tc3162_mtd_info->owner = THIS_MODULE;

	return 0;
}

static void tc3162_put_rootfs(void) {
	struct mtd_info *mtd = get_mtd_device_nm("rootfs");
	/* From init/do_mounts.c, it looks like ROOT_DEV will be
	   mounted as root fs if CONFIG_BLOCK is defined. */
	ROOT_DEV = MKDEV(MTD_BLOCK_MAJOR, mtd->index);
	put_mtd_device(mtd);
}

static int __init tc3162_mtd_init(void)
{
	int ret = 0;

	if (ret = tc3162_map_init()) {
		printk("tc3162_map_init() fail\n");
		return ret;
	}
	if (ret = tc3162_mtd_info_init()) {
		printk("tc3162_mtd_info_init() fail\n");
		return ret;
	}
	add_mtd_partitions(tc3162_mtd_info, tc3162_parts, tc3162_parts_size);
	tc3162_put_rootfs();

	return 0;
}

static void __exit tc3162_mtd_cleanup(void)
{
	if (tc3162_mtd_info) {
		del_mtd_partitions(tc3162_mtd_info);
		map_destroy(tc3162_mtd_info);
	}

   	if (tc3162_map.virt) {
   		iounmap(tc3162_map.virt);
		tc3162_map.virt = 0;
	}
}

module_init(tc3162_mtd_init);
module_exit(tc3162_mtd_cleanup);

