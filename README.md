EN7512 SoC.

Tp-Link Archer VR300 source code, EN7512, can be downloaded from: https://www.tp-link.com/it/support/gpl-code/

I could not find source code of the Linux ethernet driver in the sources, only binary object, eth.ko. But there is source code of the bootloader ethernet driver: mtk/bootrom/bootram/net/7512\_eth.c. Newer linux support some of the Mediatek SoCs. Looking at mtk\_probe code:  https://github.com/torvalds/linux/blob/master/drivers/net/ethernet/mediatek/mtk_eth_soc.c#L3069
```
if (MTK_HAS_CAPS(eth->soc->caps, MTK_QDMA)) {
```

From bootloader driver, EN7512 has QDMA capability, so may be the SoC will work with newer linux driver. Update: from drivers/net/ethernet/mediatek/Kconfig, in linux4, the driver depends on ARCH_MEDIATEK, which is ARM. In linux5, dependency: `depends on ARCH_MEDIATEK || SOC_MT7621 || SOC_MT7620`

Did not compare fully, but MT7620 (ralink_mt7620.pdf) ethernet MAC registers and values seem to be equal to values from bootloader 7512 ethernet driver (bootloader-en7512-net-src directory).

`diff` - file in the repository, will print diff of plain linux-2.6 against Tp-Link linux, ignoring some of the files. It will also ignore USB drivers and ralink, since ralink is already in newer linux.

* mips.files - list of changed and new files in arch/mips (`diff -q` output)
* mips.diff - does not include new files (`diff -u`)
* mips.diff-full - includes new files.

mips.diff is 2.4k lines. I did try to apply mips.diff-full to linux-3.9 and there was not so many rejects, see: linux-3.9-apply.rej. Same with 4.9.99: linux-4.9.99-apply.rej. 4.9 does not have SMTC file, https://www.linux-mips.org/wiki/34K

`0diffs` - patches for VR300 linux.

`TCSUPPORT_XPON_HAL_API_EXT` could be ignored for the moment.

The goal is to make and apply minimal mips/arch patch for linux5, see if router boots and what works.

My router, Innbox80, PCB photos and some of the serial output: https://saturn.ffzg.hr/rot13/index.cgi?action=display_html;page_name=innbox_v80
The router allows tftp and ssh access. The `myrouter` directory contains some files I downloaded from /proc/. I've also made a copy of /dev/mtdblock*, see tftp.txt. The version of linux is 3.1, while VR300 version is 2.6.

mediatek-linux-sdk-release-notes.pdf - how to upload using tftp.

Some SoC specs (see EcoNet): https://wikidevi.wi-cat.ru/MediaTek

Linux-mediatek mailing list: http://lists.infradead.org/pipermail/linux-mediatek/
