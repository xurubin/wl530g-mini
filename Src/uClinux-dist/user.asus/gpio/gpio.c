/*
 * GPIO char driver
 *
 * Copyright 2004, ASUSTek COMPUTER INC.
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND ASUSTek GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id$
 */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>

#ifdef LIBERTAS
#include<gpio.h>
#define ARRAYSIZE(a)		(sizeof(a)/sizeof(a[0]))

#else
#include <typedefs.h>
#include <bcmutils.h>
#include <sbutils.h>
#include <bcmdevs.h>

static void *gpio_sbh;
#endif

static int gpio_major;
static devfs_handle_t gpio_dir;
static struct {
	char *name;
	devfs_handle_t handle;
} gpio_file[] = {
	{ "in", NULL },
	{ "out", NULL },
	{ "outen", NULL },
	{ "control", NULL }
};

#ifdef LIBERTAS
#define TRUE 1
#define FALSE 0
#define uint32 unsigned int

unsigned int gpio_inval=0;
unsigned int gpio_outval=0;
unsigned int gpio_outen=0;
unsigned int gpio_ctrl=0;

void gpioSelectSource(unsigned char source, unsigned char port)
{
    	unsigned long mask;
    	mask = 0x03 << (port%8)*2; 

    	if (port < 8) {
        		gpioSetRegWithMask(GPIO_REG_SELECT_0_7, (source<<(2*port)), mask);

    	}
    	else {
        		gpioSetRegWithMask(GPIO_REG_SELECT_8_15, (source<<(2*(port-8))), mask);
    	}
}

int GPIOGet(unsigned char port)
{
	//printk("LED GPIO : %d\n", LED_REG(GPIO_REG_INPUTS));
	if ((LED_REG(GPIO_REG_INPUTS)&(1<<port)))
	{
		return FALSE;
	}
	else return TRUE;	
}

void GPIOSet(unsigned char port, int val)
{
	if (val==1) gpioClrRegBits(GPIO_REG_OUTPUTS, (1<<port)); // GPIO on
	else gpioSetRegBits(GPIO_REG_OUTPUTS, (1<<port));                // LED off
    	gpioSelectSource(SELECT_GPIO, port); // select SELECT_GPIO as source
    	gpioClrRegBits(GPIO_REG_OUTPUT_ENABLE, (1<<port)); // output enable
}
void apctlLedBlinkStart(unsigned char port)
{
    	gpioSetRegBits(SW_LED_OUTPUT_ENABLE, (1<<port));            // LED start to blink
    	gpioSelectSource(SELECT_SW_LED, port);                      // select SELECT_SW_LED as source
    	gpioClrRegBits(GPIO_REG_OUTPUT_ENABLE, (1<<port));          // output enable
}

void apctlLedBlinkStop(unsigned char port)
{
    	gpioSelectSource(SELECT_GPIO, port);                        // select SELECT_GPIO as source
    	gpioClrRegBits(GPIO_REG_OUTPUT_ENABLE, (1<<port));          // output enable
}


void apctlLedSetRate(unsigned char port, unsigned long rate, unsigned long duty)
{
    	unsigned long mask;

    	rate = rate << (port%4)*4;
    	duty = duty << (port%4)*4;
    	mask = 0x0f << (port%4)*4; 

    	switch (port/4) {
    		case 0:
        			gpioSetRegWithMask(SW_LED_CYCLE_3_0, rate, mask);       // set blinking rate
        			gpioSetRegWithMask(SW_LED_DUTY_CYCLE_3_0, duty, mask);  // set blinking duty cycle
        			break;
    		case 1:
        			gpioSetRegWithMask(SW_LED_CYCLE_7_4, rate, mask);
        			gpioSetRegWithMask(SW_LED_DUTY_CYCLE_7_4, duty, mask);
        			break;
    		case 2:
        			gpioSetRegWithMask(SW_LED_CYCLE_11_8, rate, mask);
        			gpioSetRegWithMask(SW_LED_DUTY_CYCLE_11_8, duty, mask);
        			break;
    		case 3:
        			gpioSetRegWithMask(SW_LED_CYCLE_15_12, rate, mask);
        			gpioSetRegWithMask(SW_LED_DUTY_CYCLE_15_12, duty, mask);
        			break;
    	}
    	gpioClrRegBits(GPIO_REG_OUTPUT_ENABLE, (1<<port));          // output enable
}

void GPIOInit(void)
{
	// Everything set as GPIO input as default
	LED_REG(GPIO_REG_SELECT_0_7) = 0x5555;
	LED_REG(GPIO_REG_SELECT_8_15) = 0x5555;
	LED_REG(GPIO_REG_OUTPUT_ENABLE) = 0xffff;
	//LED_REG(GPIO_REG_OUTPUTS) = 0xffff;
}

unsigned int
GPIOGetIn(void)
{
	gpio_inval = LED_REG(GPIO_REG_INPUTS);
	return(gpio_inval);
}

void
GPIOSetOut(unsigned val)
{
	gpio_outval = val;
	LED_REG(GPIO_REG_OUTPUTS) = val;
}

unsigned int
GPIOGetOut(void)
{
	gpio_outval = LED_REG(GPIO_REG_OUTPUTS);
	return(gpio_outval);
}

void GPIOSetOuten(unsigned int val)
{
	gpio_outen = val;
	LED_REG(GPIO_REG_OUTPUT_ENABLE) = gpio_outen;
}

unsigned int
GPIOGetOuten(void)
{
	gpio_outen = LED_REG(GPIO_REG_OUTPUT_ENABLE);
	return(gpio_outen);
}

void GPIOSetCtrl(unsigned int val)
{
	unsigned int i, reg;

	gpio_ctrl = val;
	
	//LED_REG(GPIO_REG_SELECT_0_7) = (unsigned int)(gpio_ctrl&0x0000ffff);
	//LED_REG(GPIO_REG_SELECT_8_15) = (unsigned int)((gpio_ctrl&0xffff0000)>>16);
	reg=gpio_ctrl;

	for(i=0;i<16;i++)
	{
		if (i==0) reg = LED_REG(GPIO_REG_SELECT_0_7);
		if (i==8) reg = LED_REG(GPIO_REG_SELECT_8_15);

		if ((reg&0x03)==0x03)
		{
			printk("start blink");
			apctlLedSetRate(i, RATE_1192MS, DUTY_50_PERCENT);
    			gpioSetRegBits(SW_LED_OUTPUT_ENABLE, (1<<i));
    			gpioSelectSource(SELECT_SW_LED, i); 
    			gpioClrRegBits(GPIO_REG_OUTPUT_ENABLE, (1<<i));
		}
		else 
		{
    			gpioSelectSource(SELECT_GPIO, i);
    			gpioClrRegBits(GPIO_REG_OUTPUT_ENABLE, (1<<i));
		}

		reg=reg>>2;
	}
}

unsigned int
GPIOGetCtrl(void)
{
	gpio_ctrl = LED_REG(GPIO_REG_SELECT_0_7) | (LED_REG(GPIO_REG_SELECT_8_15)<<16);	
	return(gpio_ctrl);
}
#endif

static int
gpio_open(struct inode *inode, struct file * file)
{
	if (MINOR(inode->i_rdev) > ARRAYSIZE(gpio_file))
		return -ENODEV;

	MOD_INC_USE_COUNT;
	return 0;
}

static int
gpio_release(struct inode *inode, struct file * file)
{
	MOD_DEC_USE_COUNT;
	return 0;
}

static ssize_t
gpio_read(struct file *file, char *buf, size_t count, loff_t *ppos)
{
	u32 val;

	switch (MINOR(file->f_dentry->d_inode->i_rdev)) {
	case 0:
#ifdef LIBERTAS
		val = GPIOGetIn();
#else
		val = sb_gpioin(gpio_sbh);
#endif
		break;
	case 1:
#ifdef LIBERTAS
		val = GPIOGetOut();
#else
		val = sb_gpioout(gpio_sbh, 0, 0);
#endif
		break;
	case 2:
#ifdef LIBERTAS
		val = GPIOGetOuten();
#else
		val = sb_gpioouten(gpio_sbh, 0, 0);
#endif
		break;
	case 3:

#ifdef LIBERTAS
		val = GPIOGetCtrl();
#else
		val = sb_gpiocontrol(gpio_sbh, 0, 0);
#endif
		break;
	default:
		return -ENODEV;
	}

	if (put_user(val, (u32 *) buf))
		return -EFAULT;

	return sizeof(val);
}

static ssize_t
gpio_write(struct file *file, const char *buf, size_t count, loff_t *ppos)
{
	u32 val;

	if (get_user(val, (u32 *) buf))
		return -EFAULT;

	switch (MINOR(file->f_dentry->d_inode->i_rdev)) {
	case 0:
		return -EACCES;
	case 1:
#ifdef LIBERTAS
		GPIOSetOut(val);
#else
		sb_gpioout(gpio_sbh, ~0, val);
#endif
		break;
	case 2:
#ifdef LIBERTAS
		GPIOSetOuten(val);
#else
		sb_gpioouten(gpio_sbh, ~0, val);
#endif
		break;
	case 3:
#ifdef LIBERTAS
		GPIOSetCtrl(val);
#else
		sb_gpiocontrol(gpio_sbh, ~0, val);
#endif

		break;
	default:
		return -ENODEV;
	}

	return sizeof(val);
}

static struct file_operations gpio_fops = {
	owner:		THIS_MODULE,
	open:		gpio_open,
	release:	gpio_release,
	read:		gpio_read,
	write:		gpio_write,
};

static int __init
gpio_init(void)
{
	int i;

#ifndef LIBERTAS
	if (!(gpio_sbh = sb_kattach()))
		return -ENODEV;

	sb_gpiosetcore(gpio_sbh);
#endif

	if ((gpio_major = devfs_register_chrdev(0, "gpio", &gpio_fops)) < 0)
		return gpio_major;

	gpio_dir = devfs_mk_dir(NULL, "gpio", NULL);

	for (i = 0; i < ARRAYSIZE(gpio_file); i++) {
		gpio_file[i].handle = devfs_register(gpio_dir,
						     gpio_file[i].name,
						     DEVFS_FL_DEFAULT, gpio_major, i,
						     S_IFCHR | S_IRUGO | S_IWUGO,
						     &gpio_fops, NULL);
	}

	return 0;
}

static void __exit
gpio_exit(void)
{
	int i;

	for (i = 0; i < ARRAYSIZE(gpio_file); i++)
		devfs_unregister(gpio_file[i].handle);
	devfs_unregister(gpio_dir);
	devfs_unregister_chrdev(gpio_major, "gpio");
#ifndef LIBERTAS
	sb_detach(gpio_sbh);
#endif
}

module_init(gpio_init);
module_exit(gpio_exit);
