/*
* bl_none_bl.c   bl_none_bl backlight module
*
* Copyright (c) 2021 hisense Technology
*
* Author: XueHua.li <lixuehua@hisense.com>
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of the GNU General  Public  License as published by the
* Free Software Foundation;  either version 2 of the License, or (at your
* option) any later version.
*/

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/jiffies.h>
#include <linux/i2c.h>
#include <linux/fs.h>
#include <linux/workqueue.h>
#include <asm/uaccess.h>
#include <linux/delay.h>
#include <linux/types.h>
#include <linux/of_gpio.h>
#include <linux/backlight.h>
#include <linux/pwm.h>
#include <linux/leds.h>
#include <linux/debugfs.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/bitops.h>

#define BL_NONE_NAME    "bl_none"

static int bl_none_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	pr_info("bl_none probe ok\n");
	return 0;
}

static int bl_none_remove(struct i2c_client *client)
{
	pr_info("bl_none remove ok\n");
	return 0;
}

static const struct i2c_device_id bl_none_id[] = {
	{BL_NONE_NAME, 0},
	{ }
};
MODULE_DEVICE_TABLE(i2c, bl_none_id);

static struct of_device_id bl_none_match_table[] = {
	{ .compatible = "ontim,i2c-bl",},
	{ },
};

struct i2c_driver bl_none_driver = {
	.driver = {
		.name	= BL_NONE_NAME,
		.owner = THIS_MODULE,
		.of_match_table = bl_none_match_table,
	},
	.probe = bl_none_probe,
	.remove = bl_none_remove,
	.id_table = bl_none_id,
};
EXPORT_SYMBOL(bl_none_driver);

MODULE_AUTHOR("ontim");
MODULE_LICENSE("GPL");

