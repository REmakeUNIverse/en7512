`NET_DSA_MT7530` is ethernet switch driver.

Device Tree files, for example: https://git.openwrt.org/?p=openwrt/openwrt.git;a=blob;f=target/linux/ramips/dts/mt7621.dtsi;hb=HEAD

https://elixir.bootlin.com/linux/latest/source/Documentation/devicetree/bindings/net/dsa/mt7530.txt

`NET_VENDOR_MEDIATEK` - mainline ethernet driver.

At the moment, 
```
mt7530 mdio-bus:1f: MT7530 adapts as multi-chip module
mt7530 mdio-bus:1f: Couldn't get our reset line
mtk_soc_eth bfb50000.ethernet: generated random MAC address aa:21:2c:03:f4:c9
mtk_soc_eth bfb50000.ethernet eth0: mediatek frame engine at 0xbfb50000, irq 22
...
mt7530 mdio-bus:1f: MT7530 adapts as multi-chip module
mt7530 mdio-bus:1f: Couldn't get our reset line
libphy: dsa slave smi: probed
mt7530 mdio-bus:1f: configuring for fixed/rgmii link mode
mt7530 mdio-bus:1f: Link is Up - 1Gbps/Full - flow control off
...
$ ifconfig eth0 up
... seconds later..
mtk_soc_eth bfb50000.ethernet eth0: Link is Up - 1Gbps/Full - flow control off
IPv6: ADDRCONF(NETDEV_CHANGE): eth0: link becomes ready
# ifconfig eth0 uprandom: crng init done
------------[ cut here ]------------
WARNING: CPU: 0 PID: 0 at net/sched/sch_generic.c:442 dev_watchdog+0x2d0/0x2d8
NETDEV WATCHDOG: eth0 (mtk_soc_eth): transmit queue 0 timed out
CPU: 0 PID: 0 Comm: swapper/0 Not tainted 5.9.9 #151
Stack : 
...
mtk_soc_eth bfb50000.ethernet eth0: transmit timed out
mtk_soc_eth bfb50000.ethernet eth0: Link is Down
```

No ping reply before or after the message.

/proc/interrupts - all eth (MAC_INT) interrupts count is 0.

clk is not used with mt7621, required_clk bitmap is 0 and clk\_prpare(NULL), clk\_set\_rate(NULL) return 0. 

Some registers match, the mt7530 registers and PDMA registers. QDMA registers have different values. See eth.diff. I've took the register values from bootloaders eth driver.

See eth.diff, what registers, etc, I've changed.

The eth driver will call `mt7621_gmac0_rgmii_adjust` to set `ETHSYS_CLKCFG0` register, but if I try to read from the register, I get DEADBEEF. I can't find any hints on this address in the bootloader source code. I tried 0x82c, as defined in tx3162.h, but this too, returns DEADBEEF. Same with `ETHSYS_SYSCFG0`.

