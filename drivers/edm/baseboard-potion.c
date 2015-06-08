
#include <asm/mach/arch.h>

#include <linux/clk.h>
#include <linux/edm.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/regulator/fixed.h>
#include <linux/regulator/machine.h>

#include <mach/common.h>
#include <mach/devices-common.h>
#include <mach/gpio.h>
#include <mach/iomux-mx6dl.h>
#include <mach/iomux-v3.h>
#include <mach/mx6.h>

/****************************************************************************
 *                                                                          
 * SGTL5000 Audio Codec
 *                                                                          
 ****************************************************************************/

static struct regulator_consumer_supply potion_sgtl5000_consumer_vdda = {
	.supply = "VDDA",
	.dev_name = "0-000a", /* Modified load time */
};

/* ------------------------------------------------------------------------ */

static struct regulator_consumer_supply potion_sgtl5000_consumer_vddio = {
	.supply = "VDDIO",
	.dev_name = "0-000a", /* Modified load time */
};

/* ------------------------------------------------------------------------ */

static struct regulator_init_data potion_sgtl5000_vdda_reg_initdata = {
	.num_consumer_supplies = 1,
	.consumer_supplies = &potion_sgtl5000_consumer_vdda,
};

/* ------------------------------------------------------------------------ */

static struct regulator_init_data potion_sgtl5000_vddio_reg_initdata = {
	.num_consumer_supplies = 1,
	.consumer_supplies = &potion_sgtl5000_consumer_vddio,
};

/* ------------------------------------------------------------------------ */

static struct fixed_voltage_config potion_sgtl5000_vdda_reg_config = {
	.supply_name		= "VDDA",
	.microvolts		= 2500000,
	.gpio			= -1,
	.init_data		= &potion_sgtl5000_vdda_reg_initdata,
};

/* ------------------------------------------------------------------------ */

static struct fixed_voltage_config potion_sgtl5000_vddio_reg_config = {
	.supply_name		= "VDDIO",
	.microvolts		= 3300000,
	.gpio			= -1,
	.init_data		= &potion_sgtl5000_vddio_reg_initdata,
};

/* ------------------------------------------------------------------------ */

static struct platform_device potion_sgtl5000_vdda_reg_devices = {
	.name	= "reg-fixed-voltage",
	.id	= 0,
	.dev	= {
		.platform_data = &potion_sgtl5000_vdda_reg_config,
	},
};

/* ------------------------------------------------------------------------ */

static struct platform_device potion_sgtl5000_vddio_reg_devices = {
	.name	= "reg-fixed-voltage",
	.id	= 1,
	.dev	= {
		.platform_data = &potion_sgtl5000_vddio_reg_config,
	},
};

/* ------------------------------------------------------------------------ */

static struct platform_device potion_audio_device = {
	.name = "imx-sgtl5000",
};

/* ------------------------------------------------------------------------ */

static const struct i2c_board_info potion_sgtl5000_i2c_data __initdata = {
	I2C_BOARD_INFO("sgtl5000", 0x0a)
};

/* ------------------------------------------------------------------------ */

static char potion_sgtl5000_dev_name[8] = "0-000a";

static __init int potion_init_sgtl5000(void) {
	int ret = -ENODEV, i2c_bus = 1;

	if (edm_audio_data[0].enabled && edm_i2c[i2c_bus] >= 0) {
		edm_audio_data[0].bus_type = EDM_BUS_I2C;
		edm_audio_data[0].bus_number = edm_i2c[i2c_bus];
		edm_audio_data[0].bus_address = 0x000a;

		potion_sgtl5000_dev_name[0] = '0' + edm_i2c[i2c_bus];
		potion_sgtl5000_consumer_vdda.dev_name = potion_sgtl5000_dev_name;
		potion_sgtl5000_consumer_vddio.dev_name = potion_sgtl5000_dev_name;
        
		potion_audio_device.dev.platform_data = edm_audio_data[0].platform_data;
		platform_device_register(&potion_audio_device);
        
		i2c_register_board_info(edm_i2c[i2c_bus], &potion_sgtl5000_i2c_data, 1);
		platform_device_register(&potion_sgtl5000_vdda_reg_devices);
		platform_device_register(&potion_sgtl5000_vddio_reg_devices);
                ret = 0;
	}
	return ret;
}


/****************************************************************************
 *                                                                          
 * FXAS21002 Gyrocompass
 *                                                                          
 ****************************************************************************/

static struct i2c_board_info potion_fxas21002_binfo = {
	I2C_BOARD_INFO("fxas2100x", 0x21),
        .irq = -EINVAL,
};

void __init potion_init_gyro(void) {
        potion_fxas21002_binfo.irq = gpio_to_irq(edm_external_gpio[7]);
	i2c_register_board_info(edm_i2c[2], &potion_fxas21002_binfo, 1);
}


/****************************************************************************
 *                                                                          
 * DS1337 RTC
 *                                                                          
 ****************************************************************************/

static const struct i2c_board_info potion_ds1337_binfo = {
	I2C_BOARD_INFO("ds1337", 0x68),
};

void __init potion_init_rtc() {
	i2c_register_board_info(edm_i2c[2], &potion_ds1337_binfo, 1);
}


/****************************************************************************
 *                                                                          
 * main-function for potionboard
 *                                                                          
 ****************************************************************************/

static __init int potion_init(void) {
	potion_init_sgtl5000();
	potion_init_gyro();
	potion_init_rtc();
        
	return 0;
}
arch_initcall_sync(potion_init);

static __exit void potion_exit(void) {
	/* Actually, this cannot be unloaded. Or loaded as a module..? */
} 
module_exit(potion_exit);

MODULE_DESCRIPTION("Potionboard");
MODULE_AUTHOR("Tapani Utriainen <tapani@technexion.com>");
MODULE_LICENSE("GPL");

