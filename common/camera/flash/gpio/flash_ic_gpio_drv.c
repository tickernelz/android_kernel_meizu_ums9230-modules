/*
 * Copyright (C) 2021-2022 UNISOC Communications Inc.
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

#include <linux/delay.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/platform_device.h>

#include "sprd_cam.h"
#include "flash_drv.h"

#define FLASH_DRIVER_NAME "gpio-flash"
#define FLASH_GPIO_MAX 2

enum {
	GPIO_FLASH_TORCH_EN,
	GPIO_FLASH_EN,
};

struct flash_driver_data {
	int gpio_tab[FLASH_GPIO_MAX];
	u32 torch_led_index;
};

static const char *const flash_gpio_names[FLASH_GPIO_MAX] = {
	"flash-torch-en-gpios-2",
	"flash-en-gpios-2",
};

static int sprd_flash_gpio_set(struct flash_driver_data *drv_data, int gpio,
			       int value)
{
	if (!drv_data)
		return -EFAULT;

	if (!gpio_is_valid(drv_data->gpio_tab[gpio]))
		return 0;

	return gpio_direction_output(drv_data->gpio_tab[gpio], value);
}

static void sprd_flash_gpio_pwm(struct flash_driver_data *drv_data, int level)
{
	printk("qzhu %s %d \n", __func__, level);

	if (gpio_is_valid(drv_data->gpio_tab[GPIO_FLASH_TORCH_EN])) {
		gpio_direction_output(drv_data->gpio_tab[GPIO_FLASH_TORCH_EN],
				      SPRD_FLASH_ON);
		udelay(5);
	}
}

static int sprd_flash_gpio_open_torch(void *drvd, uint8_t idx)
{
	printk("qzhu %s\n", __func__);

	return sprd_flash_gpio_set(drvd, GPIO_FLASH_TORCH_EN, SPRD_FLASH_ON);
}

static int sprd_flash_gpio_close_torch(void *drvd, uint8_t idx)
{
	printk("qzhu %s\n", __func__);

	return sprd_flash_gpio_set(drvd, GPIO_FLASH_TORCH_EN, SPRD_FLASH_OFF);
}

static int sprd_flash_gpio_open_preflash(void *drvd, uint8_t idx)
{
	printk("qzhu %s\n", __func__);

	return sprd_flash_gpio_set(drvd, GPIO_FLASH_TORCH_EN, SPRD_FLASH_ON);
}

static int sprd_flash_gpio_close_preflash(void *drvd, uint8_t idx)
{
	printk("qzhu %s\n", __func__);

	return sprd_flash_gpio_set(drvd, GPIO_FLASH_TORCH_EN, SPRD_FLASH_OFF);
}

static int sprd_flash_gpio_open_highlight(void *drvd, uint8_t idx)
{
	struct flash_driver_data *drv_data = drvd;

	printk("qzhu %s\n", __func__);

	if (!drv_data)
		return 0;

	gpio_direction_output(drv_data->gpio_tab[GPIO_FLASH_EN], SPRD_FLASH_ON);
	sprd_flash_gpio_pwm(drv_data, 0);

	return 0;
}

static int sprd_flash_gpio_close_highlight(void *drvd, uint8_t idx)
{
	struct flash_driver_data *drv_data = drvd;
	int ret;

	printk("qzhu %s\n", __func__);

	ret = sprd_flash_gpio_set(drv_data, GPIO_FLASH_EN, SPRD_FLASH_OFF);
	if (ret)
		return ret;

	udelay(600);

	return sprd_flash_gpio_set(drv_data, GPIO_FLASH_TORCH_EN, SPRD_FLASH_OFF);
}

static int sprd_flash_gpio_cfg_value_torch(void *drvd, uint8_t idx,
					   struct sprd_flash_element *element)
{
	return 0;
}

static int sprd_flash_gpio_cfg_value_preflash(void *drvd, uint8_t idx,
					      struct sprd_flash_element *element)
{
	return 0;
}

static int sprd_flash_gpio_cfg_value_highlight(void *drvd, uint8_t idx,
					       struct sprd_flash_element *element)
{
	return 0;
}

static const struct sprd_flash_driver_ops flash_gpio_ops = {
	.open_torch = sprd_flash_gpio_open_torch,
	.close_torch = sprd_flash_gpio_close_torch,
	.open_preflash = sprd_flash_gpio_open_preflash,
	.close_preflash = sprd_flash_gpio_close_preflash,
	.open_highlight = sprd_flash_gpio_open_highlight,
	.close_highlight = sprd_flash_gpio_close_highlight,
	.cfg_value_torch = sprd_flash_gpio_cfg_value_torch,
	.cfg_value_preflash = sprd_flash_gpio_cfg_value_preflash,
	.cfg_value_highlight = sprd_flash_gpio_cfg_value_highlight,
};

static int sprd_flash_gpio_init(void *drvd)
{
	int ret;

	if (!drvd)
		return -EFAULT;

	ret = sprd_flash_register(&flash_gpio_ops, drvd, SPRD_FLASH_REAR);
	printk("qzhu %s\n", __func__);

	return ret;
}

static const struct of_device_id gpio_flash_of_match[] = {
	{ .compatible = "sprd,flash-gpio", },
	{},
};

static int sprd_flash_gpio_probe(struct platform_device *pdev)
{
	struct flash_driver_data *drv_data;
	int gpio[FLASH_GPIO_MAX];
	int ret;
	int i;

	if (IS_ERR_OR_NULL(pdev))
		return -EINVAL;

	if (!pdev->dev.of_node) {
		pr_err("no device node %s", __func__);
		return -ENODEV;
	}

	pr_info("tjd flash-gpio probe\n");

	drv_data = devm_kzalloc(&pdev->dev, sizeof(*drv_data), GFP_KERNEL);
	if (!drv_data)
		return -ENOMEM;

	platform_set_drvdata(pdev, drv_data);

	for (i = 0; i < FLASH_GPIO_MAX; i++) {
		gpio[i] = of_get_named_gpio(pdev->dev.of_node,
					    flash_gpio_names[i], 0);
		if (!gpio_is_valid(gpio[i]))
			continue;

		ret = devm_gpio_request(&pdev->dev, gpio[i],
					flash_gpio_names[i]);
		if (ret) {
			pr_err("flash gpio err\n");
			return ret;
		}

		ret = gpio_direction_output(gpio[i], SPRD_FLASH_OFF);
		if (ret) {
			pr_err("flash gpio output err\n");
			return ret;
		}
	}

	drv_data->gpio_tab[GPIO_FLASH_TORCH_EN] = gpio[GPIO_FLASH_TORCH_EN];
	drv_data->gpio_tab[GPIO_FLASH_EN] = gpio[GPIO_FLASH_EN];

	return sprd_flash_gpio_init(drv_data);
}

static int sprd_flash_gpio_remove(struct platform_device *pdev)
{
	return 0;
}

static struct platform_driver sprd_flash_gpio_drvier = {
	.probe = sprd_flash_gpio_probe,
	.remove = sprd_flash_gpio_remove,
	.driver = {
		.name = FLASH_DRIVER_NAME,
		.of_match_table = of_match_ptr(gpio_flash_of_match),
	},
};

module_platform_driver(sprd_flash_gpio_drvier);
MODULE_DESCRIPTION("Revo GPIO Flash Driver");
MODULE_LICENSE("GPL");
