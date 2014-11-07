
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

static struct regulator_consumer_supply toucan_sgtl5000_consumer_vdda = {
	.supply = "VDDA",
	.dev_name = "0-000a", /* Modified load time */
};

/* ------------------------------------------------------------------------ */

static struct regulator_consumer_supply toucan_sgtl5000_consumer_vddio = {
	.supply = "VDDIO",
	.dev_name = "0-000a", /* Modified load time */
};

/* ------------------------------------------------------------------------ */

static struct regulator_init_data toucan_sgtl5000_vdda_reg_initdata = {
	.num_consumer_supplies = 1,
	.consumer_supplies = &toucan_sgtl5000_consumer_vdda,
};

/* ------------------------------------------------------------------------ */

static struct regulator_init_data toucan_sgtl5000_vddio_reg_initdata = {
	.num_consumer_supplies = 1,
	.consumer_supplies = &toucan_sgtl5000_consumer_vddio,
};

/* ------------------------------------------------------------------------ */

static struct fixed_voltage_config toucan_sgtl5000_vdda_reg_config = {
	.supply_name		= "VDDA",
	.microvolts		= 2500000,
	.gpio			= -1,
	.init_data		= &toucan_sgtl5000_vdda_reg_initdata,
};

/* ------------------------------------------------------------------------ */

static struct fixed_voltage_config toucan_sgtl5000_vddio_reg_config = {
	.supply_name		= "VDDIO",
	.microvolts		= 3300000,
	.gpio			= -1,
	.init_data		= &toucan_sgtl5000_vddio_reg_initdata,
};

/* ------------------------------------------------------------------------ */

static struct platform_device toucan_sgtl5000_vdda_reg_devices = {
	.name	= "reg-fixed-voltage",
	.id	= 0,
	.dev	= {
		.platform_data = &toucan_sgtl5000_vdda_reg_config,
	},
};

/* ------------------------------------------------------------------------ */

static struct platform_device toucan_sgtl5000_vddio_reg_devices = {
	.name	= "reg-fixed-voltage",
	.id	= 1,
	.dev	= {
		.platform_data = &toucan_sgtl5000_vddio_reg_config,
	},
};

/* ------------------------------------------------------------------------ */

static struct platform_device toucan_audio_device = {
	.name = "imx-sgtl5000",
};

/* ------------------------------------------------------------------------ */

static const struct i2c_board_info toucan_sgtl5000_i2c_data __initdata = {
	I2C_BOARD_INFO("sgtl5000", 0x0a)
};

/* ------------------------------------------------------------------------ */

static char toucan_sgtl5000_dev_name[8] = "0-000a";

static __init int toucan_init_sgtl5000(void) {
	int ret = -ENODEV, i2c_bus = 1;

	if (edm_audio_data[0].enabled && edm_i2c[i2c_bus] >= 0) {
		edm_audio_data[0].bus_type = EDM_BUS_I2C;
		edm_audio_data[0].bus_number = edm_i2c[i2c_bus];
		edm_audio_data[0].bus_address = 0x000a;

		toucan_sgtl5000_dev_name[0] = '0' + edm_i2c[i2c_bus];
		toucan_sgtl5000_consumer_vdda.dev_name = toucan_sgtl5000_dev_name;
		toucan_sgtl5000_consumer_vddio.dev_name = toucan_sgtl5000_dev_name;
        
		toucan_audio_device.dev.platform_data = edm_audio_data[0].platform_data;
		platform_device_register(&toucan_audio_device);
        
		i2c_register_board_info(edm_i2c[i2c_bus], &toucan_sgtl5000_i2c_data, 1);
		platform_device_register(&toucan_sgtl5000_vdda_reg_devices);
		platform_device_register(&toucan_sgtl5000_vddio_reg_devices);
	}
	return 0;
}


/****************************************************************************
 *                                                                          
 * ADS7846 Touchscreen
 *                                                                          
 ****************************************************************************/

#include <linux/spi/ads7846.h>
#include <linux/spi/spi.h>

static int toucan_tsc2046_gpio_index = 8;

int toucan_get_tsc2046_pendown_state(void) {
	return !gpio_get_value(edm_external_gpio[toucan_tsc2046_gpio_index]);
}

static struct ads7846_platform_data toucan_tsc2046_config = {
	.x_max              	= 0x0fff,
	.y_max                  = 0x0fff,
	.pressure_max           = 1024,
	.get_pendown_state      = toucan_get_tsc2046_pendown_state,
	.keep_vref_on           = 1,
	.wakeup			= true,
	.model                  = 7846,

	.x_plate_ohms  = 90,
	.y_plate_ohms  = 90,
	.debounce_max  = 70,
	.debounce_tol  = 3,
	.debounce_rep  = 2,
	.settle_delay_usecs = 150
};

static struct spi_board_info toucan_tsc2046_spi_data  = {
	.modalias		= "ads7846",
	.bus_num		= 0,
	.chip_select		= 0,
	.max_speed_hz		= 1500000,
	.irq			= -EINVAL, /* Set programmatically */
	.platform_data		= &toucan_tsc2046_config,
};

static struct spi_board_info toucan_spidev_data  = {
	.modalias		= "spidev",
	.bus_num		= 0,
	.chip_select		= 0,
	.max_speed_hz		= 300000,
};

/* ------------------------------------------------------------------------ */

void __init toucan_init_ts(void) {
	gpio_request(edm_external_gpio[toucan_tsc2046_gpio_index], "tsc2046 irq");
	toucan_tsc2046_spi_data.irq = gpio_to_irq(edm_external_gpio[toucan_tsc2046_gpio_index]);
	spi_register_board_info(&toucan_tsc2046_spi_data, 1);
}


/****************************************************************************
 *                                                                          
 * DS1337 RTC
 *                                                                          
 ****************************************************************************/

static const struct i2c_board_info toucan_ds1337_binfo = {
	I2C_BOARD_INFO("ds1337", 0x68),
};

void __init toucan_init_rtc() {
	i2c_register_board_info(edm_i2c[2], &toucan_ds1337_binfo, 1);
}


/****************************************************************************
 *                                                                          
 * main-function for fairyboard
 *                                                                          
 ****************************************************************************/

static __init int toucan_init(void) {
	toucan_init_sgtl5000();
	toucan_init_ts();
	toucan_init_rtc();
        
	return 0;
}
arch_initcall_sync(toucan_init);

static __exit void toucan_exit(void) {
	/* Actually, this cannot be unloaded. Or loaded as a module..? */
} 
module_exit(toucan_exit);

MODULE_DESCRIPTION("Toucan HMI");
MODULE_AUTHOR("Tapani Utriainen <tapani@technexion.com>");
MODULE_LICENSE("GPL");

