#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/string.h>

static char bl_name[8];
static bool aw99703_probe_flag = false;
static bool ktz3136_probe_flag = false;
extern struct i2c_driver aw99703_i2c_driver;
extern struct i2c_driver ktz3136_driver;
extern struct i2c_driver bl_none_driver;
extern int aw99703_set_brightness(int brt_val);
extern void ktz3136_brightness(int brightness);

static int get_bl_type_bootargs(char *current_mode, char *boot_param)
{
	struct device_node *np;
	const char *cmd_line;
	char *s = NULL;

	int ret = 0;
	np = of_find_node_by_path("/chosen");

	if (!np) {
		pr_err("Can't get the /chosen\n");
		return -EIO;
	}

	ret = of_property_read_string(np, "bootargs", &cmd_line);
	if (ret < 0) {
		pr_err("Can't get the bootargs\n");
		return ret;
	}

	s = strstr(cmd_line, boot_param);

	if(s){
		s += (strlen(boot_param) + 1);
		while(*s != ' ')
			*current_mode++ = *s++;
		*current_mode = '\0';
	}
	return 0;
}

int i2c_backlight_set_brightness(int value)
{
	if (aw99703_probe_flag)
		aw99703_set_brightness(value);
	else if (ktz3136_probe_flag)
		ktz3136_brightness(value);
	else
		pr_err("Unknown bl type name:%s,value:%d\n", bl_name, value);
	return 0;
}
EXPORT_SYMBOL(i2c_backlight_set_brightness);

static int __init i2c_backlight_init(void)
{
	int err = 0;

	err = get_bl_type_bootargs(bl_name,"bl_type");
	if (!err) {
		if (strcmp(bl_name, "aw99703") == 0) {
			err = i2c_add_driver(&aw99703_i2c_driver);
			if (err)
				printk(KERN_WARNING "aw99703 driver failed (errno = %d)\n",
					   err);
			else
				aw99703_probe_flag = true;
		} else if (strcmp(bl_name, "ktz3136") == 0) {
			err = i2c_add_driver(&ktz3136_driver);
			if (err)
				printk(KERN_WARNING "ktz3136 driver failed (errno = %d)\n",
					   err);
			else
				ktz3136_probe_flag = true;
		} else {
			err = i2c_add_driver(&bl_none_driver);
			if (err)
				printk(KERN_WARNING "bl_none driver failed (errno = %d)\n",
					   err);
		}
	} else {
			pr_err("get  bl type fail: %s\n", bl_name);
			return -EINVAL;
		}
	return err;
}

static void __exit i2c_backlight_exit(void)
{
    if (aw99703_probe_flag) {
        i2c_del_driver(&aw99703_i2c_driver);
    } else if (ktz3136_probe_flag) {
        i2c_del_driver(&ktz3136_driver);
    } else
		printk(KERN_INFO "i2c backlight driver exit\n");
}

module_init(i2c_backlight_init);
module_exit(i2c_backlight_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ontim team");
MODULE_DESCRIPTION("Dynamic loading of i2c drivers based on command line input");