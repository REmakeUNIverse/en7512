`NET_DSA_MT7530` - ethernet switch driver.

`NET_VENDOR_MEDIATEK` - mainline ethernet driver that is using QDMA.

### eth driver

The QDMA looks similar, MASK and STATUS registers, GLB CFG, ring DMA pointer.. I could not make the Mediatek driver work. So I've tried using the same structures as Mediatek SDK bootloader ethernet driver. eth.c in this directory. eth.c can be copied over `mtk_eth_soc.c`, also apply eth.diff.

Sending/receiving packets works, but sending broadcast packets does not. 
So 
1. ping the router from client, some first packets will be lost. This will add client to the ARP table
2. Or on the router, add ARP entry manually: `arp -s 192.168.1.2 your-ether-addr`

Problem 2: I do not know which switch port packet came from. The packet from the blue, WAN, port looks the same as from any other port.

The ring object's fport field is always 1, it is set to `GDM_P_GDMA1`. A packet came from blue port or LAN port, it is 1. resv3/pmap does not show any relation with the switch port.

I do not load Mediatec mt7530 switch driver. If desired, the bootlaoder switch config can be used, but this is not necessary for the driver to work. The bootloader, lib/commands.c `do_jump` will read bfb40004 register and if it is zero, the bootloader will not reset the switch. I can confirm that at least GLB CFG, MASK and some other registers have different values.

```
# tftp...
memwl bfb40004 0
jump 80020000
```

#### Modes

The driver can operate in two modes. 

1. `GLB_CFG_IRQ_EN BIT(19)` in `GLB_CFG` is set and `TX0_DONE` in INT_MASK is unset. In this case irq queue is used and this queue should be cleared once in a while or `IRQ_FULL` is raised. 
2. `GLB_CFG_IRQ_EN` is not set, `TX0_DONE` is enabled. In this case `TX_INT` is raised when packet is sent. This only mark 6 DSCP ring objects as done and the stops. 

#### Clk and SYSCFG registers

Reading from CLKCFG0 and SYSCFG0 registers returns DEADBEEF. I tried 0x82c, as defined in tx3162.h, same thing.

### mt7530

```
mt7530_pad_clk_setup is similar to the bootloader driver initialization. 
ncpo1 = 0x1000;
ssc_delta = 0x57;
```

```
mt7530_cpu_port_enable:
mt7530_write(priv, MT7530_MFC, (0xff << 24) | (0xff << 16) | (1 << 8) | (1 << 7) | (6 << 4));
```

### Sources

Bootloader driver. `mtk/linux-2.6.36/*.i` files.


Device Tree file, for example: https://git.openwrt.org/?p=openwrt/openwrt.git;a=blob;f=target/linux/ramips/dts/mt7621.dtsi;hb=HEAD

See innboxe80_pw2.dtsi in this dir.

https://elixir.bootlin.com/linux/latest/source/Documentation/devicetree/bindings/net/dsa/mt7530.txt
