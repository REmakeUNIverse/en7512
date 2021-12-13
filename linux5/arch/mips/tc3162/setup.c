#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/delay.h>

// There is no header in linux5.
// #include <asm/mips-boards/prom.h>
#include <asm/reboot.h>
#include <asm/time.h>
#include <asm/tc3162/tc3162.h>
#include <asm/tc3162/TCIfSetQuery_os.h>

static void tc3162_machine_restart(char *command);
static void tc3162_machine_halt(void);
static void tc3162_machine_power_off(void);

void (*back_to_prom)(void) = (void (*)(void))0xbfc00000;

extern void timerSet(uint32 timer_no, uint32 timerTime, uint32 enable, uint32 mode, uint32 halt);
extern void timer_WatchDogConfigure(uint8 tick_enable, uint8 watchdog_enable);

static void hw_reset(void)
{
#if defined(CONFIG_MIPS_TC3262) && defined(TCSUPPORT_POWERSAVE_ENABLE)
	if(isRT63365){
		VPint(CR_AHB_CLK) |= 0x57e1;//restore ahb clk to default value
	}

#endif
	/* stop each module dma task */
	VPint(CR_INTC_IMR) = 0x0;
	VPint(CR_TIMER_CTL) = 0x0;

	/* stop mac dma */
#ifndef CONFIG_MIPS_TC3262
	VPint(CR_MAC_MACCR) = 0;
#endif

	if (isRT63165)
		goto watchdog_reset;

	/* stop atm sar dma */
	TSARM_GFR &= ~((1 << 1) | (1 << 0));

	/* reset USB */
	/* reset USB DMA */
	VPint(CR_USB_SYS_CTRL_REG) |= (1 << 31);
	/* reset USB SIE */
	VPint(CR_USB_DEV_CTRL_REG) |= (1 << 30);
	mdelay(5);

	/* restore USB SIE */
	VPint(CR_USB_DEV_CTRL_REG) &= ~(1 << 30);
	mdelay(5);
	VPint(CR_USB_SYS_CTRL_REG) &= ~(1 << 31);

#ifdef CONFIG_MIPS_TC3162U
	/*stop pcie*/
	VPint(CR_AHB_PCIC) &= 0x9fffffff;
	/*reset usb 2.0 device*/
	/*stop interrupt*/
	VPint(CR_USB20_INTR_ENABLE_REG) = 0x0;
	/*do usb reset*/
	VPint(CR_USB20_SYS_CTRL_REG) |= (1 << 31);
	mdelay(1);
	VPint(CR_USB20_SYS_CTRL_REG) &= ~(1 << 31);
	/*sw disconnect*/
	VPint(CR_USB20_DEV_CTRL_REG) |= (1 << 31);
#endif

watchdog_reset:
	/* watchdog reset */
//#ifdef CONFIG_MIPS_TC3262
#if defined(TCSUPPORT_WLAN_MT7592_PCIE) && defined(TCSUPPORT_CPU_MT7520)
	printk("0xbfb00834=0xeff88ce0 \n");
	VPint(CR_INTC_IMR) = 0x0;
	VPint(CR_TIMER_CTL) = 0x0;
#endif
	timerSet(5, 10 * TIMERTICKS_10MS, ENABLE, TIMER_TOGGLEMODE, TIMER_HALTDISABLE);
	timer_WatchDogConfigure(ENABLE, ENABLE);
#if defined(TCSUPPORT_WLAN_MT7592_PCIE) && defined(TCSUPPORT_CPU_MT7520)
	VPint(0xbfb00834) = 0xeff88ce0;
#endif
#if defined(TCSUPPORT_CPU_MT7505) || defined(TCSUPPORT_CPU_MT7510)
	VPint(CR_DRAMC_CONF) &= ~(0x1<<2);
#endif

	while (1);
//#endif
}

static void tc3162_machine_restart(char *command)
{
	printk("Machine restart ... \n");
	hw_reset();
	back_to_prom();
}

static void tc3162_machine_halt(void)
{
	printk("Machine halted ... \n");
	hw_reset();
	while (1);
}

static void tc3162_machine_power_off(void)
{
	printk("Machine poweroff ... \n");
	hw_reset();
	while (1);
}

static int panic_event(struct notifier_block *this, unsigned long event,
		       void *ptr)
{
	tc3162_machine_restart(NULL);
	return NOTIFY_DONE;
}

static struct notifier_block panic_block = {
	.notifier_call = panic_event,
};

void __init plat_mem_setup(void)
{
	_machine_restart = tc3162_machine_restart;
	_machine_halt = tc3162_machine_halt;
	pm_power_off = tc3162_machine_power_off;
	atomic_notifier_chain_register(&panic_notifier_list, &panic_block);
}
