`NET_DSA_MT7530` - ethernet switch driver.

`NET_VENDOR_MEDIATEK` - mainline ethernet driver that is using QDMA.

### eth driver

The QDMA looks similar, MASK and STATUS registers, GLB CFG, ring DMA pointer.. I could not make the Mediatek driver work. So I've tried using the same structures as Mediatek SDK bootloader ethernet driver. eth.c in this directory. eth.c can be copied over `mtk_eth_soc.c`, also apply eth.diff.

The eth.c driver does not work. It can receive, at least I see that the rx ring buffer entry is marked as done and packet len is set. It will mark tx ring buffer entries as done when sending packets, but I'm monitoring ethernet interface with (iptraf) and nothing shows up.

I do not load Mediatec mt7530 driver (it can be loaded). I assume the switch is already initialized by the bootloader. The bootloader, lib/commands.c `do_jump` will read bfb40004 register and if it is zero, it will not reset the switch. I can confirm that at least GLB CFG, MASK and some other registers have different values when the register is set to 0.

```
# tftp...
memwl bfb40004 0
jump 80020000
```

#### Modes

The driver can operate in two modes. 

1. `GLB_CFG_IRQ_EN BIT(19)` in `GLB_CFG` is set and `TX_INT` in INT_MASK is unset. In this case Irq queue is used and this queue should be cleared once in a while or `IRQ_FULL` is raised.  
2. `GLB_CFG_IRQ_EN` is not set, `TX_INT` set. In this case `TX_INT` is raised when packet is sent. But it only marks 7 or so ring buffer entries as done and then stops raising TX interrupts and marking entries as done. `INT_MASK_NO_TX0_CPU_DSCP` is set all the time. I could not find how to reset this state or what have to be done.

To set DMA packet addr 
```
phys_addr = dma_map_single(eth->dev, skb->data, skb_headlen(skb), DMA_TO_DEVICE);
p->pkt_addr = phys_addr;
```

Do I have to tell the kernel how to initialize DMA, which address ranges to use? 

In the bootloader, they allocate some space at particular address for all skb's,

#### Etc..

I've tried removing MTK_QDMA form the capabilities, thought may be PDMA will work.

The eth driver will call `mt7621_gmac0_rgmii_adjust` to set `ETHSYS_CLKCFG0` register, but if I try to read from the register, I get DEADBEEF. I tried 0x82c, as defined in tx3162.h, but this too returns DEADBEEF. Same with `ETHSYS_SYSCFG0`.

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
