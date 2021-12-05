EN7512 SoC.

Tp-Link Archer VR300 source code, EN7512, can be downloaded from: https://www.tp-link.com/it/support/gpl-code/

I could not find Linux ethernet driver source in the sources, only binary object, eth.ko. But there is source code of the bootloader ethernet driver: mtk/bootrom/bootram/net/7512\_eth.c. This driver can't be compiled as a linux module. Newer linux support some of the Mediatek SoCs. Looking at mtk\_probe code:  https://github.com/torvalds/linux/blob/master/drivers/net/ethernet/mediatek/mtk_eth_soc.c#L3069
```
if (MTK_HAS_CAPS(eth->soc->caps, MTK_QDMA)) {
```

From bootloader driver, EN7512 has QDMA capability, so may be the SoC will work with newer linux driver.

`diff` - file in the repository, will make a diff of plain linux-2.6 and Tp-Link linux, but will ignore some of the files. It will also ignore USB drivers and ralink, since ralink is already in newer linux.

* mips.files - list of changed and new files in arch/mips (`diff -q` output)
* mips.diff - does not include new files (`diff -u`)
* mips.diff-full - includes new files.

mips.diff is 2.4k LOC. I did try to apply mips.diff-full to linux-3.9 and there was not so many rejects, see rejects file: linux-3.9-apply.rej. Same with 4.9.99: linux-4.9.99-apply.rej. 4.9 does not have SMTC file, https://www.linux-mips.org/wiki/34K

`TCSUPPORT_XPON_HAL_API_EXT` could be ignored for the moment.

The goal is to make and apply minimal mips/arch patch to 4.9, use to compile (buildroot) and see if router boots and what works. See mediatek-linux-sdk-release-notes.pdf on how to upload using tftp.


My router, Innbox80, PCB photos and some of the serial output: https://saturn.ffzg.hr/rot13/index.cgi?action=display_html;page_name=innbox_v80)
The router allows tftp and ssh access. The `myrouter` directory contains some files I downloaded from /proc/. I've also made a copy of /dev/mtdblock*, see tftp.txt. The version of linux is 3.1, while VR300 version is 2.6.

Some SoC specs (see EcoNet): https://wikidevi.wi-cat.ru/MediaTek

Linux-mediatek mailing list: http://lists.infradead.org/pipermail/linux-mediatek/
