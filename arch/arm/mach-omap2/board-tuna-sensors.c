/* Sensor support for Samsung Tuna Board.
 *
 * Copyright (C) 2011 Google, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/mpu.h>

#include "mux.h"
#include "board-tuna.h"

#define GPIO_GYRO_INT		45
#define GPIO_ACC_INT		122
#define GPIO_MAG_INT		176

static s8 orientation_back_right_90[] = {
	 0, -1,  0,
	-1,  0,  0,
	 0,  0, -1,
};

static s8 orientation_back_left_90[] = {
	 0,  1,  0,
	 1,  0,  0,
	 0,  0, -1,
};

static s8 orientation_back_180[] = {
	 1,  0,  0,
	 0, -1,  0,
	 0,  0, -1,
};

static void rotcpy(s8 dst[3 * 3], const s8 src[3 * 3])
{
	memcpy(dst, src, 3 * 3);
}

static struct mpu_platform_data mpu_data = {
	.int_config  = 0x10,
	.orientation = {  1,  0,  0,
			  0,  1,  0,
			  0,  0,  1 },
	/* accel */
	.accel = {
		.adapt_num   = 4,
		.bus         = EXT_SLAVE_BUS_SECONDARY,
		.address     = 0x18,
		.orientation = {  0,  1,  0,
				  1,  0,  0,
				  0,  0, -1 },
	},
	/* compass */
	.compass = {
		.adapt_num   = 4,
		.bus         = EXT_SLAVE_BUS_PRIMARY,
		.address     = 0x2E,
		.orientation = {  1,  0,  0,
				  0,  1,  0,
				  0,  0,  1 },
	},
};

static struct i2c_board_info __initdata tuna_sensors_i2c4_boardinfo[] = {
	[0] = {
		I2C_BOARD_INFO("mpu3050", 0x68),
		.platform_data = &mpu_data,
	},
	[1] = {
		I2C_BOARD_INFO("bma250", 0x18),
		.platform_data = &mpu_data.accel,
	},
	[2] = {
		I2C_BOARD_INFO("yas530", 0x2e),
		.platform_data = &mpu_data.compass,
	},
};


void __init omap4_tuna_sensors_init(void)
{
	omap_mux_init_gpio(GPIO_GYRO_INT, OMAP_PIN_INPUT);
	omap_mux_init_gpio(GPIO_ACC_INT, OMAP_PIN_INPUT);
	omap_mux_init_gpio(GPIO_MAG_INT, OMAP_PIN_INPUT);

	gpio_request(GPIO_GYRO_INT, "GYRO_INT");
	gpio_direction_input(GPIO_GYRO_INT);
	gpio_request(GPIO_ACC_INT, "ACC_INT");
	gpio_direction_input(GPIO_ACC_INT);
	gpio_request(GPIO_MAG_INT, "MAG_INT");
	gpio_direction_input(GPIO_MAG_INT);

	tuna_sensors_i2c4_boardinfo[0].irq = gpio_to_irq(GPIO_GYRO_INT);
	tuna_sensors_i2c4_boardinfo[1].irq = gpio_to_irq(GPIO_ACC_INT);
	tuna_sensors_i2c4_boardinfo[2].irq = gpio_to_irq(GPIO_MAG_INT);

	if (omap4_tuna_get_type() == TUNA_TYPE_MAGURO &&
	    omap4_tuna_get_revision() >= 2) {
		rotcpy(mpu_data.orientation, orientation_back_right_90);
		rotcpy(mpu_data.accel.orientation, orientation_back_180);
	}
	if (omap4_tuna_get_type() == TUNA_TYPE_TORO &&
	    omap4_tuna_get_revision() >= 1) {
		rotcpy(mpu_data.orientation, orientation_back_left_90);
		rotcpy(mpu_data.accel.orientation, orientation_back_180);
		rotcpy(mpu_data.compass.orientation, orientation_back_left_90);
	}

	i2c_register_board_info(4, tuna_sensors_i2c4_boardinfo,
				ARRAY_SIZE(tuna_sensors_i2c4_boardinfo));
}
