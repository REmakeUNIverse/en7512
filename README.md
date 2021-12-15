EN7512 SoC.

Tp-Link Archer VR300 (EN7512) source code: https://www.tp-link.com/it/support/gpl-code/

An attempt at upgrading this code, the part required for EN7512 SoC, to linux5. At the moment TC3262 kernel compiles, but not tested. I do not have USB <-> serial converter, if interested and you live in Moldova or close by, I will accept the converter as donation (I think it is 3.3 V).

Building: `make HOST_CFLAGS='-DTCSUPPORT_CPU_EN7512' linux-rebuild`

* full.diff - patch for Linux 5. Everything, but not later fixes. At the moment without USB/PCI, flash is not enabled and will need fixes, see mtd.txt and mips.notes, drivers.notes
* full-ct.diff - compile time fixes, etc fixes, apply after full.diff.

For analysis, diffs do not include later fixes, do not need to apply

* mips.files - diff of arch/mips against plain linux-2.6.38, list of changed and new files (`diff -qr` output)
* mips.diff - does not include new files (`diff -ur`), does not include rejects when applied to linux5 or not many
* mips5.diff - linux5 diff that only include files which were rejected, plus some trivial edits like deleting SMTC
* linux-5-new - directory with new file, trivial fixes.

mips.notes - notes, why certain files or changes were not included, etc.

Upstream linux ethernet driver: drivers/net/ethernet/mediatek/mtk\_eth\_soc.c
Depends on `ARCH_MEDIATEK || SOC_MT7621 || SOC_MT7620`.

SOC\_MT7621, SOC\_MT7620 is little endian. VR300 is big endian, although, from Kconfig, VR300 platform allows both.

I did not find datasheet for EN7512.

MT7620 (ralink_mt7620.pdf) ethernet MAC registers and values look similar (did not compare all) to values from bootloader EN7512 ethernet driver (bootloader-en7512-net-src directory).

My router - Innbox80, PCB photos and some of the serial output: https://saturn.ffzg.hr/rot13/index.cgi?action=display_html;page_name=innbox_v80
tftp and ssh access can be enabled from Administrator user. The `myrouter` directory contains some files from /proc/. I've also made a copy of /dev/mtdblock*, see tftp.txt. The version of linux is 3.1, while VR300 version is 2.6.

WiFi: https://www.mediatek.com/products/broadbandWifi/mt7603e
- from myrouter/lsmod.txt, mt7603eap module. Upstream linux driver: drivers/net/wireless/mediatek/mt76/mt7603/Kconfig

mediatek-linux-sdk-release-notes.pdf - firmware uploading.

Some SoC specs (see EcoNet): https://wikidevi.wi-cat.ru/MediaTek

TC3162u: https://web.archive.org/web/20140319175335/http://www.mediatek.com/en/products/connectivity/xdsl/adsl-wifi/tc3162u/

### Bootloader
Bootloader sources: mtk/bootrom.

Emacs modes for hiding {} blocks (hs-minor-mode), for hiding ifdefs (hide-ifdef-mode), will be helpfull. The sources include SPI and NAND code, 7512 ethernet code, even leds (light diods) IO configs.

Disclaimer: I did not test this.

Bootloader may support `jump addr` command. The command will call function (jump to) at addr. tftp will save file to `TFTP_BUF_BASE`: 0x80020000 addr. (net/tftpput.c).

`flash dst src len` command will call `flash_write(dst, len, &retlen, (const unsigned char *) (src))` src should be set to `TFT_BUF_BASE`. dst.. go figure.

On power-up, bootloader calls boot_kernel (init/main.c) which loads image, performs tests and jumps to loaded kernel memory address.
First, the `trx_header` (mtk/tools/trx/trx.h) struct len, magic, crc32 will be read from flash and compared. `tx_header` address: `flash_base + flash_tclinux_start`. The variables set in flash/flashhal.c, in `*_init` functions. 0x0 + 0x40000 * 2 if `IS_NANDFLASH and TCSUPPORT_BB_NAND`, flash command dst will be 0x80000.

```
$ head -9 myrouter/proc/mtd
dev:    size   erasesize  name
mtd0: 00040000 00020000 "bootloader"
mtd1: 00040000 00020000 "romfile"
mtd2: 003cadc3 00020000 "kernel"
mtd3: 010b0000 00020000 "rootfs"
mtd4: 02800000 00020000 "tclinux"
mtd5: 003cadc3 00020000 "kernel_slave"
mtd6: 010b0000 00020000 "rootfs_slave"
mtd7: 02800000 00020000 "tclinux_slave"
```

linux/drivers/mtd/maps/tc3162-flash.c adds mtd partitions and trx header is used to set kernel and rootfs offsets, see `tc3162_set_kernel_rootfs_part`.

If `CONFIG_DUAL_IMAGE = y` and `TCSUPPORT_NAND_BADBLOCK_CHECK` is not set:

If trx->decompAddr is zero, 0x80020000 will be used as `output` RAM address.

I think it is tp-link specific: google "en751221 the tag fsLen" shows only tp-link entries. But in this case, bootloader will ignore trx values and load `LINUX_FILE_TAG` structure defined in bootloader src include/asm/tc3162.h.

`set_lzma_addr(...`, `decompress_kernel(output, ...` and jump, supposedly, to `output`.

Trx header utils

* https://github.com/vasvir/tcrevenge
* mtk/tools/trx directory
* https://openwrt.org/docs/techref/brcm63xx.imagetag
