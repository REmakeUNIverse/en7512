EN7512 SoC.

Tp-Link Archer VR300 (EN7512) source code: https://www.tp-link.com/it/support/gpl-code/

An attempt at upgrading this code, the part of it required for EN7512, to linux5.

* mips.files - diff of arch/mips against plain linux-2.6.38, list of changed and new files (`diff -qr` output)
* mips.diff - does not include new files (`diff -ur`)
* mips.diff-full - includes new files
* mips.notes - notes, why certain files or changes were not included.
* mips5.diff - linux5 arch/mips diff.

To apply the patch to linux5 and view rejects:
```
cd linux5
mkdir tmp
patch -p1 -o tmp/mips.apply < ../mips.diff-full
less tmp/mips.apply.rej
```

Upstream linux ethernet driver: drivers/net/ethernet/mediatek/mtk\_eth\_soc.c
Depends on `ARCH_MEDIATEK || SOC_MT7621 || SOC_MT7620`.

SOC\_MT7621, SOC\_MT7620 is little endian. VR300 is big endian, although, from Kconfig, VR300 platform allows both.

I did not find datasheet for EN7512.

MT7620 (ralink_mt7620.pdf) ethernet MAC registers and values look similar (did not compare all) to values from bootloader EN7512 ethernet driver (bootloader-en7512-net-src directory).

WiFi in my router: https://www.mediatek.com/products/broadbandWifi/mt7603e
- from myrouter/lsmod.txt, mt7603eap module. Upstream linux driver: drivers/net/wireless/mediatek/mt76/mt7603/Kconfig

My router - Innbox80, PCB photos and some of the serial output: https://saturn.ffzg.hr/rot13/index.cgi?action=display_html;page_name=innbox_v80
tftp and ssh access can be enabled from Administrator user. The `myrouter` directory contains some files from /proc/. I've also made a copy of /dev/mtdblock*, see tftp.txt. The version of linux is 3.1, while VR300 version is 2.6.

mediatek-linux-sdk-release-notes.pdf - firmware uploading.

Some SoC specs (see EcoNet): https://wikidevi.wi-cat.ru/MediaTek

TC3162u: https://web.archive.org/web/20140319175335/http://www.mediatek.com/en/products/connectivity/xdsl/adsl-wifi/tc3162u/
