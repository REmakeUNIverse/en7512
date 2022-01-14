`NET_DSA_MT7530` - ethernet switch driver.

`NET_VENDOR_MEDIATEK` - mainline ethernet driver that is using QDMA.

### eth driver

The QDMA looks similar, MASK and STATUS registers, GLB CFG, ring DMA pointer.. I could not make the Mediatek driver work. So I've tried using the same structures as Mediatek SDK bootloader ethernet driver. eth.c in this directory. eth.c can be copied over `mtk_eth_soc.c`, also apply eth.diff.

Sending packets does not work. When sending, it will begin marking TX DSCPs as done and HWFWDs change, but this continue only up to 6 packet and DSCP marking stops, LMGR STATUS register begin to decrease. For the first 6 packets, nothing shows up in the network monitor, although the diode blinks.
When there is an incomming packet, RX DSCPS will be market as done and packet len will be set.

I do not load Mediatec mt7530 switch driver. I assume the switch is already initialized by the bootloader. The bootloader, lib/commands.c `do_jump` will read bfb40004 register and if it is zero, the bootloader will not reset the switch. I can confirm that at least GLB CFG, MASK and some other registers have different values.

```
# tftp...
memwl bfb40004 0
jump 80020000
```

#### Modes

The driver can operate in two modes. 

1. `GLB_CFG_IRQ_EN BIT(19)` in `GLB_CFG` is set and `TX_INT` in INT_MASK is unset. In this case Irq queue is used and this queue should be cleared once in a while or `IRQ_FULL` is raised. TX DSCPs marking does not stop, but LMGR status begin to decrease after 6'th packet and goes to 0.
2. `GLB_CFG_IRQ_EN` is not set, `TX_INT` set. In this case `TX_INT` is raised when packet is sent.

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


Device Tree file, for example: https://git.openwrt.org/?p=openwrt/openwrt.git;a=blob;f=target/linux/ramips/dts/mt7621.dtsi;hb=HEAD

See innboxe80_pw2.dtsi in this dir.

https://elixir.bootlin.com/linux/latest/source/Documentation/devicetree/bindings/net/dsa/mt7530.txt
