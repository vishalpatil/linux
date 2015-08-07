
#include <osl.h>

#ifdef CUSTOMER_HW

#ifdef CONFIG_MACH_ODROID_4210
#include <mach/gpio.h>
#include <mach/regs-gpio.h>
#include <plat/gpio-cfg.h>

#include <plat/sdhci.h>
#include <plat/devs.h>	// modifed plat-samsung/dev-hsmmcX.c EXPORT_SYMBOL(s3c_device_hsmmcx) added

#define	sdmmc_channel	s3c_device_hsmmc0
#endif

#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/of_device.h>

struct wifi_platform_data {
	int (*set_power)(bool val);
	int (*set_carddetect)(bool val);
	void *(*mem_prealloc)(int section, unsigned long size);
	int (*get_mac_addr)(unsigned char *buf);
	void *(*get_country_code)(char *ccode);
};

struct resource dhd_wlan_resources = {0};
struct wifi_platform_data dhd_wlan_control = {0};

#ifdef CUSTOMER_OOB
uint bcm_wlan_get_oob_irq(void)
{
	uint host_oob_irq;
	int wl_host_wake;
	struct device_node *wifi_np;

	wifi_np = of_find_compatible_node(NULL, NULL, "bcmdhd,wifi-gpio");
	wl_host_wake = of_get_named_gpio(wifi_np, "wl-host-wake", 0);

	if (gpio_is_valid(wl_host_wake)) {
		gpio_request(wl_host_wake, "wl-host-wake");
		gpio_direction_input(wl_host_wake);
		host_oob_irq = gpio_to_irq(wl_host_wake);
	} else {
		printk("incorrect wl_host_wake gpios (%d)\n", wl_host_wake);
	}

#ifdef CONFIG_MACH_ODROID_4210
	printk("GPIO(WL_HOST_WAKE) = EXYNOS4_GPX0(7) = %d\n", EXYNOS4_GPX0(7));
	host_oob_irq = gpio_to_irq(EXYNOS4_GPX0(7));
	gpio_direction_input(EXYNOS4_GPX0(7));
#endif
	printk("host_oob_irq: %d \r\n", host_oob_irq);

	return host_oob_irq;
}

uint bcm_wlan_get_oob_irq_flags(void)
{
	uint host_oob_irq_flags = 0;

	host_oob_irq_flags = (IORESOURCE_IRQ | IORESOURCE_IRQ_HIGHLEVEL | IORESOURCE_IRQ_SHAREABLE) & IRQF_TRIGGER_MASK;

#ifdef CONFIG_MACH_ODROID_4210
	host_oob_irq_flags = (IORESOURCE_IRQ | IORESOURCE_IRQ_HIGHLEVEL | IORESOURCE_IRQ_SHAREABLE) & IRQF_TRIGGER_MASK;
#endif
	printk("host_oob_irq_flags=%d\n", host_oob_irq_flags);

	return host_oob_irq_flags;
}
#endif

int bcm_wlan_set_power(bool on)
{
	int err = 0;
	struct device_node *wifi_np;
	int wl_reg_on;

	wifi_np = of_find_compatible_node(NULL, NULL, "bcmdhd,wifi-gpio");
	wl_reg_on = of_get_named_gpio(wifi_np, "wl-reg-on", 0);

	if (gpio_is_valid(wl_reg_on)) {
		gpio_request(wl_reg_on, "wl-reg-on");
	} else {
		printk("incorrect wl_reg_on gpios (%d)\n", wl_reg_on);
	}

	if (on) {
		printk("======== PULL WL_REG_ON HIGH! ========\n");
		gpio_direction_output(wl_reg_on, 0);
		mdelay(10);
		gpio_set_value(wl_reg_on, 1);
#ifdef CONFIG_MACH_ODROID_4210
		err = gpio_set_value(EXYNOS4_GPK1(0), 1);
#endif
		/* Lets customer power to get stable */
		mdelay(100);
	} else {
		printk("======== PULL WL_REG_ON LOW! ========\n");
		gpio_direction_output(wl_reg_on, 0);
#ifdef CONFIG_MACH_ODROID_4210
		err = gpio_set_value(EXYNOS4_GPK1(0), 0);
#endif
	}

	return err;
}

#include <linux/mmc/host.h>
int bcm_wlan_set_carddetect(bool present)
{
	int err = 0;

	if (present) {
		printk("======== Card detection to detect SDIO card! ========\n");
		mmc_host_rescan(NULL, 1, 1);//NULL => SDIO host
#ifdef CONFIG_MACH_ODROID_4210
		err = sdhci_s3c_force_presence_change(&sdmmc_channel, 1);
#endif
	} else {
		printk("======== Card detection to remove SDIO card! ========\n");
		mmc_host_rescan(NULL, 0, 1);//NULL => SDIO host
#ifdef CONFIG_MACH_ODROID_4210
		err = sdhci_s3c_force_presence_change(&sdmmc_channel, 0);
#endif
	}

	return err;
}

#ifdef CONFIG_DHD_USE_STATIC_BUF
extern void *bcmdhd_mem_prealloc(int section, unsigned long size);
void* bcm_wlan_prealloc(int section, unsigned long size)
{
	void *alloc_ptr = NULL;
	alloc_ptr = bcmdhd_mem_prealloc(section, size);
	if (alloc_ptr) {
		printk("success alloc section %d, size %ld\n", section, size);
		if (size != 0L)
			bzero(alloc_ptr, size);
		return alloc_ptr;
	}
	printk("can't alloc section %d\n", section);
	return NULL;
}
#endif

int bcm_wlan_set_plat_data(void) {
	printk("======== %s ========\n", __FUNCTION__);
	dhd_wlan_control.set_power = bcm_wlan_set_power;
	dhd_wlan_control.set_carddetect = bcm_wlan_set_carddetect;
#ifdef CONFIG_DHD_USE_STATIC_BUF
	dhd_wlan_control.mem_prealloc = bcm_wlan_prealloc;
#endif
	return 0;
}

#endif /* CUSTOMER_HW */
