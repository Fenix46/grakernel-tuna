/*
 * /drivers/mfd/twl6030-poweroff.c
 *
 * Power off device
 *
 * Copyright (C) 2011 Texas Instruments Corporation
 *
 * Written by Rajeev Kulkarni <rajeevk@ti.com>
 *
 * This file is subject to the terms and conditions of the GNU General
 * Public License. See the file "COPYING" in the main directory of this
 * archive for more details.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <linux/module.h>
#include <linux/pm.h>
#include <linux/i2c/twl.h>
#include <linux/reboot.h>
#include <asm/system_misc.h>

#define CONTROLLER_STAT1 0x03

#define VBUS_DET    (1<<2)

#define APP_DEVOFF	(1<<0)
#define CON_DEVOFF	(1<<1)
#define MOD_DEVOFF	(1<<2)

#define LED_PWM_CTRL1 0x14
#define LED_PWM_CTRL2 0x15

#define LED_PWM_ON 0x1

static void twl6030_enable_power_led(void)
{
	u8 oldval = 0;

	/* set LED intensity to full */
	twl_i2c_write_u8(TWL6030_MODULE_CHARGER, 0xff, LED_PWM_CTRL1);

	/* enable LED independently from charger state */
	twl_i2c_read_u8(TWL6030_MODULE_CHARGER, &oldval, LED_PWM_CTRL2);
	oldval = (oldval & 0xfc) | LED_PWM_ON;
	twl_i2c_write_u8(TWL6030_MODULE_CHARGER, oldval, LED_PWM_CTRL2);
}

void twl6030_poweroff(void)
{
	u8 val = 0;
	int err = 0;
	u8 stat1;

	/* if VBUS is present resort to warm reset, reboot reason should already be "off" */
	twl_i2c_read_u8(TWL6030_MODULE_CHARGER, &stat1, CONTROLLER_STAT1);

	if (stat1 & VBUS_DET) {
		arm_pm_restart(0, 0);
	} else {
		err = twl_i2c_read_u8(TWL_MODULE_PM_MASTER, &val,
				TWL6030_PHOENIX_DEV_ON);
		if (err) {
			pr_warning("I2C error %d reading PHOENIX_DEV_ON\n", err);
			return;
		}

		val |= APP_DEVOFF | CON_DEVOFF | MOD_DEVOFF;

		err = twl_i2c_write_u8(TWL_MODULE_PM_MASTER, val,
				TWL6030_PHOENIX_DEV_ON);

		if (err) {
			pr_warning("I2C error %d writing PHOENIX_DEV_ON\n", err);
			return;
		}
	}
}

static int __init twl6030_poweroff_init(void)
{
	pm_power_off = twl6030_poweroff;
	pm_power_off_prepare = twl6030_enable_power_led;

	return 0;
}

static void __exit twl6030_poweroff_exit(void)
{
	pm_power_off = NULL;
	pm_power_off_prepare = NULL;
}

module_init(twl6030_poweroff_init);
module_exit(twl6030_poweroff_exit);

MODULE_DESCRIPTION("TLW6030 device power off");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Rajeev Kulkarni");
