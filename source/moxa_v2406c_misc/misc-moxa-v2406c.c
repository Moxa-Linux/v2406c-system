/*
 * This driver is for Moxa embedded computer programmble led, relay,
 * miniPCIe power, sim slot select, and DIP switch detect driver.
 * It based on IT8786 GPIO hardware.
 *
 * History:
 * Date		Aurhor		Comment
 * 2018/10/25	Elvis Yao	First create for V-2406C.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/miscdevice.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <asm/io.h>

#define SUPPORT_GPIOSYSFS

#include "gpio-it8786.c"
#include "dio_main.h"

#define DI_NUM			6
#define DI_PATTERN		"111111"
#define DI_ON			1
#define DI_OFF			0

#define DO_NUM			2
#define DO_PATTERN		"11"
#define DO_ON			1
#define DO_OFF			0

#define SIF_NUM			4
#define SIF_PATTERN		"12"

#define PCIEPWR_NUM		2
#define PCIEPWR_PATTERN		"11"
#define PCIEPWR_ON		1
#define PCIEPWR_OFF		0

#define SIM_NUM			2
#define SIM_PATTERN		"11"
#define	SIM_A			1
#define	SIM_B			0

#define DIP_NUM			2
#define DIP_PATTERN		"11"
#define	DIP_1			1
#define	DIP_2			0

/* mknod /dev/uart c 10 105 for this module */
#define MOXA_SIF_MINOR		105
#define MOXA_DI_MINOR		(MOXA_SIF_MINOR+1)
#define MOXA_DO_MINOR		(MOXA_DI_MINOR+1)
#define MOXA_SIM_MINOR		(MOXA_DO_MINOR+1)
#define MOXA_PCIEPWR_MINOR	(MOXA_SIM_MINOR+1)
#define MOXA_DIP_MINOR		(MOXA_PCIEPWR_MINOR+1)

#define SIF_NAME		"uart"
#define DI_NAME			"di"
#define DO_NAME			"do"
#define SIM_NAME		"sim_sel"
#define PCIEPWR_NAME		"pciepwr"
#define DIP_NAME		"dip"

/* Ioctl number */
#define MOXA			0x400
#define MOXA_SET_OP_MODE	(MOXA + 66)
#define MOXA_GET_OP_MODE	(MOXA + 67)

/* Debug message */
#ifdef DEBUG
#define dbg_printf(x...)	printk(x)
#else
#define dbg_printf(x...)
#endif

/*
 * module: DI section
 */
static int di_open(struct inode *inode, struct file *file)
{
	return 0;
}

/* Write function
 * Note: use echo 111111 > /dev/di
 * The order is [di0, di1, di2, di3, di4, di5\n]
 */
ssize_t di_write(struct file *filp, const char __user *buf, size_t count,
	loff_t *pos)
{
	printk("<1> %s[%d]\n", __FUNCTION__, __LINE__);
	return 0;
}

static int di_release(struct inode *inode, struct file *file)
{
	return 0;
}

ssize_t di_read(struct file *filp, char __user *buf, size_t count,
	loff_t *pos)
{
	int i;
	int ret;
	char stack_buf[DI_NUM];

	for (i = 0; (i < sizeof(stack_buf)) && (i < count); i++) {
		if ( !di_get(i, &ret) ) {
			if (ret) {
				stack_buf[i] = '0' + DI_ON;
			} else {
				stack_buf[i] = '0' + DI_OFF;
			}
		} else {
			return -EINVAL;
		}
	}

	ret = copy_to_user((void*)buf, (void*)stack_buf, i);
	if(ret < 0) {
		return -ENOMEM;
	}

	return i;
}

long di_ioctl(struct file *filp, unsigned int cmd, unsigned long arg) {
	struct dio_set_struct   set;
	int i = 0;
	int di_state = 0;

	/* check data */
	if (copy_from_user(&set, (struct dio_set_struct *)arg,
		sizeof(struct dio_set_struct)) == sizeof(struct dio_set_struct))
		return -EFAULT;

	switch (cmd) {
		case IOCTL_GET_DIN :
			dp("get din io_number:%x\n",set.io_number);

			if(set.io_number == ALL_PORT) {
				set.mode_data = 0;
				for( i=0; i< DI_NUM; i++) {
					di_get(set.io_number, &di_state);
					set.mode_data |= (di_state<<i);
				}
				dp(KERN_DEBUG "all port: %x", set.mode_data);
			} else { 
				if(set.io_number < 0 || set.io_number >= DI_NUM)
					return -EINVAL;

				if(di_get(set.io_number, &set.mode_data) < 0)
					printk(KERN_ALERT "di_get(%d, %d) fail\n",
						set.io_number, set.mode_data);
			}

			if(copy_to_user((struct dio_set_struct *)arg, &set,
			sizeof(struct dio_set_struct)) == sizeof(struct dio_set_struct))
				return -EFAULT;

			dp("mode_data: %x\n", (unsigned int)set.mode_data);

			break;

		default :
			printk(KERN_DEBUG "ioctl:error\n");
			return -EINVAL;
	}

	return 0; /* if switch ok */
}

/* define which file operations are supported */
struct file_operations di_fops = {
	.owner		= THIS_MODULE,
	.read		= di_read,
	.open		= di_open,
	.write		= di_write,
	.unlocked_ioctl	= di_ioctl,
	.release	= di_release,
};

/* register as misc driver */
static struct miscdevice di_miscdev = {
	.minor = MOXA_DI_MINOR,
	.name = DI_NAME,
	.fops = &di_fops,
};

/*
 * module: DO section
 */
static int do_open(struct inode *inode, struct file *file)
{
	return 0;
}

static int do_release(struct inode *inode, struct file *file)
{
	return 0;
}

/* Write function
 * Note: use echo 11 > /dev/do
 * The order is [do0, do1\n]
 */
ssize_t do_write(struct file *filp, const char __user *buf, size_t count,
	loff_t *pos)
{
	int i;
	char stack_buf[DO_NUM];

	/* check input value */
	if (count != (sizeof(stack_buf)+1)) {
		printk(KERN_ERR "Moxa do error! paramter should be %lu \
digits, like \"%s\" \n", sizeof(stack_buf), DO_PATTERN);
		return -EINVAL;
	}

	if (copy_from_user(stack_buf, buf, count-1)) {
		return 0;
	}

	for (i = 0; i < sizeof(stack_buf); i++) {
		if (stack_buf[i] == '1') {
			do_set(i, DO_ON);
		} else if (stack_buf[i] == '0') {
			do_set(i, DO_OFF);
		} else {
			printk("do: error, you input is %s", stack_buf);
			break;
		}
	}

	return count;
}

ssize_t do_read(struct file *filp, char __user *buf, size_t count,
	loff_t *pos)
{
	int i;
	int ret;
	char stack_buf[DO_NUM];

	for (i = 0; (i < sizeof(stack_buf)) && (i < count); i++) {
		if ( !do_get(i, &ret) ) {
			if ( ret ) {
				stack_buf[i] = '0' + DO_ON;
			} else {
				stack_buf[i] = '0' + DO_OFF;
			}
		} else {
			return -EINVAL;
		}
	}

	ret = copy_to_user((void*)buf, (void*)stack_buf, sizeof(stack_buf));
	if(ret < 0) {
		printk( KERN_ERR "Moxa do error! paramter should be %lu \
digits, like \"%s\" \n", sizeof(stack_buf), DO_PATTERN);
		return -ENOMEM;
	}

	return sizeof(stack_buf);
}

long do_ioctl(struct file *filp, unsigned int cmd, unsigned long arg) {
	struct dio_set_struct   set;
	int i = 0;
	int do_state = 0;

	/* check data */
	if(copy_from_user(&set, (struct dio_set_struct *)arg, sizeof(struct dio_set_struct)) == sizeof(struct dio_set_struct))
		return -EFAULT;

	switch (cmd) {
		case IOCTL_SET_DOUT :
			dp("set dio:%x\n",set.io_number);
			if (set.io_number < 0 || set.io_number >= DO_NUM)
				return -EINVAL;

			if (set.mode_data != DIO_HIGH && set.mode_data != DIO_LOW)
				return -EINVAL;

			do_set(set.io_number, set.mode_data);
			break;
		case IOCTL_GET_DOUT :
			dp("get do io_number:%x\n",set.io_number);

			if (set.io_number == ALL_PORT) {
				set.mode_data = 0;
				for( i=0; i< DO_NUM; i++) {
					do_get(set.io_number, &do_state);
					set.mode_data |= (do_state<<i);
				}
				dp(KERN_DEBUG "all port: %x", set.mode_data);
			} else { 
				if(set.io_number < 0 || set.io_number >= DO_NUM)
					return -EINVAL;

				if(do_get(set.io_number, &set.mode_data) < 0)
					printk(KERN_ALERT "di_get(%d, %d) fail\n", set.io_number, set.mode_data);
			}

			if(copy_to_user((struct dio_set_struct *)arg, &set, sizeof(struct dio_set_struct)) == sizeof(struct dio_set_struct))
				return -EFAULT;

			dp("mode_data: %x\n", (unsigned int)set.mode_data);
			break;
		default :
			printk(KERN_DEBUG "ioctl:error\n");
			return -EINVAL;
	}

	return 0; /* if switch ok */
}

/* define which file operations are supported */
struct file_operations do_fops = {
	.owner		= THIS_MODULE,
	.write		= do_write,
	.read		= do_read,
	.unlocked_ioctl	= do_ioctl,
	.open		= do_open,
	.release	= do_release,
};

/* register as misc driver */
static struct miscdevice do_miscdev = {
	.minor = MOXA_DO_MINOR,
	.name = DO_NAME,
	.fops = &do_fops,
};

/*
 * module: Serial interface section
 */
static int sif_open(struct inode *inode, struct file *file)
{
	return 0;
}

static int sif_release(struct inode *inode, struct file *file)
{
	return 0;
}

/* Write function
 * Note: use echo 1111 > /dev/uart
 * The order is [uart1, uart2, uart3, uart4\n]
 */
ssize_t sif_write(struct file *filp, const char __user *buf, size_t count,
	loff_t *pos)
{
	int i;
	char stack_buf[SIF_NUM];

	/* check input value */
	if(count != (sizeof(stack_buf)+1)) {
		printk( KERN_ERR "Moxa uart error! paramter should be %lu digits\
, like \"%s\" \n", sizeof(stack_buf)/sizeof(char), SIF_PATTERN);
		return -EINVAL;
	}

	if(copy_from_user(stack_buf, buf, count-1)) {
		return 0;
	}

	for (i = 0; i < sizeof(stack_buf); i++) {
		if (stack_buf[i] >= '0' && stack_buf[i] <= '3') {
			if(0 != uartif_set( i, stack_buf[i]-'0')) {
				printk("uart: the mode is not supported, \
the input is %s", stack_buf);
				break;
			}
		} else {
			printk("uart: error, the input is %s", stack_buf);
			break;
		}
	}

	return count;
}

ssize_t sif_read(struct file *filp, char __user *buf, size_t count,
	loff_t *pos)
{
	int i;
	int ret;
	char stack_buf[SIF_NUM];

	for (i = 0; (i < sizeof(stack_buf)) && (i < count); i++) {
		if (!uartif_get(i, &ret)) {
			stack_buf[i] = '0' + ret;
			dbg_printf("%s", stackbuf[i] );
		} else {
			return -EINVAL;
		}
	}

	ret = copy_to_user((void*)buf, (void*)stack_buf, sizeof(stack_buf));
	if(ret < 0) {
		printk(KERN_ERR "Moxa uart error! paramter should be %lu digits\
, like \"%s\" \n", sizeof(stack_buf), SIF_PATTERN);
		return -ENOMEM;
	}

	return sizeof(stack_buf);
}

/* define which file operations are supported */
struct file_operations sif_fops = {
	.owner		= THIS_MODULE,
	.write		= sif_write,
	.read		= sif_read,
	.open		= sif_open,
	.release	= sif_release,
};

/* register as misc driver */
static struct miscdevice sif_miscdev = {
	.minor = MOXA_SIF_MINOR,
	.name = SIF_NAME,
	.fops = &sif_fops,
};

/*
 * module: PCIe power section
 */
static int pciepwr_open(struct inode *inode, struct file *file)
{
	return 0;
}

static int pciepwr_release(struct inode *inode, struct file *file)
{
	return 0;
}

/* Write function
 * Note: use echo 11 > /dev/pciepwr
 * The order is [/dev/pciepwr1, /dev/pciepwr2]
 */
ssize_t pciepwr_write(struct file *filp, const char __user *buf, size_t count,
	loff_t *pos)
{
	int i;
	char stack_buf[PCIEPWR_NUM];

	/* check input value */
	if(count != (sizeof(stack_buf)+1)) {
		printk( KERN_ERR "Moxa miniPCIe power control error! paramter should be %lu \
digits, like \"%s\" \n", sizeof(stack_buf), PCIEPWR_PATTERN);
		return -EINVAL;
	}

	if(copy_from_user(stack_buf, buf, count-1)) {
		return 0;
	}

	for (i = 0; i < sizeof(stack_buf); i++) {
		if (stack_buf[i] == '1') {
			do_set(i, PCIEPWR_ON);
		} else if (stack_buf[i] == '0') {
			do_set(i, PCIEPWR_OFF);
		} else {
			printk("Moxa miniPCIe power: error, you input is %s", stack_buf);
			break;
		}
	}

	return count;
}

ssize_t pciepwr_read(struct file *filp, char __user *buf, size_t count,
	loff_t *pos)
{
	int i;
	int ret;
	char stack_buf[PCIEPWR_NUM];

	for (i = 0; (i < sizeof(stack_buf)) && (i < count); i++) {
		if (!do_get(i, &ret)) {
			if (ret) {
				stack_buf[i] = '0' + PCIEPWR_ON;
			} else {
				stack_buf[i] = '0' + PCIEPWR_OFF;
			}
		} else {
			return -EINVAL;
		}
	}

	ret = copy_to_user((void*)buf, (void*)stack_buf, sizeof(stack_buf));
	if(ret < 0) {
		printk(KERN_ERR "Moxa miniPCIe power control error! paramter should be %lu \
digits, like \"%s\" \n", sizeof(stack_buf), PCIEPWR_PATTERN);
		return -ENOMEM;
	}

	return sizeof(stack_buf);
}

/* define which file operations are supported */
struct file_operations pciepwr_fops = {
	.owner		= THIS_MODULE,
	.write		= pciepwr_write,
	.read		= pciepwr_read,
	.open		= pciepwr_open,
	.release	= pciepwr_release,
};

/* register as misc driver */
static struct miscdevice pciepwr_miscdev = {
	.minor = MOXA_PCIEPWR_MINOR,
	.name = PCIEPWR_NAME,
	.fops = &pciepwr_fops,
};


/*
 * module: sim slot select section
 */
static int sim_sel_open(struct inode *inode, struct file *file)
{
	return 0;
}

static int sim_sel_release(struct inode *inode, struct file *file)
{
	return 0;
}

/* Write function
 * Note: use echo 11 > /dev/sim_sel
 * The order is [/dev/sim_sel_1, /dev/sim_sel_2]
 */
ssize_t sim_sel_write(struct file *filp, const char __user *buf, size_t count,
	loff_t *pos)
{
	int i;
	char stack_buf[PCIEPWR_NUM];

	/* check input value */
	if(count != (sizeof(stack_buf)+1)) {
		printk( KERN_ERR "Moxa sim card select error! paramter should be %lu \
digits, like \"%s\" \n", sizeof(stack_buf), SIM_PATTERN);
		return -EINVAL;
	}

	if(copy_from_user(stack_buf, buf, count-1)) {
		return 0;
	}

	for (i = 0; i < sizeof(stack_buf); i++) {
		if (stack_buf[i] == '1') {
			do_set(i, SIM_A);
		} else if (stack_buf[i] == '0') {
			do_set(i, SIM_B);
		} else {
			printk("sim card select: error, you input is %s", stack_buf);
			break;
		}
	}

	return count;
}

ssize_t sim_sel_read(struct file *filp, char __user *buf, size_t count,
	loff_t *pos)
{
	int i;
	int ret;
	char stack_buf[PCIEPWR_NUM];

	for (i = 0; (i < sizeof(stack_buf)) && (i < count); i++) {
		if (!do_get(i, &ret)) {
			if (ret) {
				stack_buf[i] = '0' + SIM_A;
			} else {
				stack_buf[i] = '0' + SIM_B;
			}
		} else {
			return -EINVAL;
		}
	}

	ret = copy_to_user((void*)buf, (void*)stack_buf, sizeof(stack_buf));
	if(ret < 0) {
		printk(KERN_ERR "Moxa sim card select error! paramter should be %lu \
digits, like \"%s\" \n", sizeof(stack_buf), SIM_PATTERN);
		return -ENOMEM;
	}

	return sizeof(stack_buf);
}

/* define which file operations are supported */
struct file_operations sim_sel_fops = {
	.owner		= THIS_MODULE,
	.write		= sim_sel_write,
	.read		= sim_sel_read,
	.open		= sim_sel_open,
	.release	= sim_sel_release,
};

/* register as misc driver */
static struct miscdevice sim_sel_miscdev = {
	.minor = MOXA_SIM_MINOR,
	.name = SIM_NAME,
	.fops = &sim_sel_fops,
};

#ifdef SUPPORT_GPIOSYSFS
/*
 * Support sysfs
 */

/*
 * module: DO chip-section
 */
static int moxa_gpio_do_get(struct gpio_chip *gc, unsigned gpio_num)
{
	int val;

	if (0==do_get(gpio_num, &val)) {
		if (DO_ON==val)
			return 1;
	}
	return 0;
}

static void moxa_gpio_do_set(struct gpio_chip *gc, unsigned gpio_num, int val)
{
	if ( val )
		val = DO_ON;
	else
		val = DO_OFF;
	do_set(gpio_num, val);
}

const char *gpio_do_names[] = {
	"do0",
	"do1",
};

static struct gpio_chip moxa_gpio_do_chip = {
	.label		= "moxa-gpio-do",
	.owner		= THIS_MODULE,
	.get		= moxa_gpio_do_get,
	.set		= moxa_gpio_do_set,
	.base		= -1,
	.ngpio		= sizeof(do_pin_def)/2,
	.names		= gpio_do_names
};

/*
 * module: DI chip-section
 */
static int moxa_gpio_di_get(struct gpio_chip *gc, unsigned gpio_num)
{
	int val;

	if (0==di_get(gpio_num, &val)) {
		if (DI_ON==val)
			return 1;
	}
	return 0;
}

static void moxa_gpio_di_set(struct gpio_chip *gc, unsigned gpio_num, int val)
{
}

const char *gpio_di_names[] = {
	"di0",
	"di1",
	"di2",
	"di3",
	"di4",
	"di5",
};

static struct gpio_chip moxa_gpio_di_chip = {
	.label		= "moxa-gpio-di",
	.owner		= THIS_MODULE,
	.get		= moxa_gpio_di_get,
	.set		= moxa_gpio_di_set,
	.base		= -1,
	.ngpio		= sizeof(di_pin_def)/2,
	.names		= gpio_di_names
};

/*
 * module: Serial interface chip-section
 */
static int moxa_gpio_uartif_get(struct gpio_chip *gc, unsigned gpio_num)
{
	int val;

	if (0==uartif_get(gpio_num/3, &val)) {
		if (val==(gpio_num%3)) {
			return 1;
		}
	}

	return 0;
}

static void moxa_gpio_uartif_set(struct gpio_chip *gc, unsigned gpio_num, int val)
{
	if (val) {
		uartif_set(gpio_num/3, gpio_num%3);
	}
}

const char *gpio_uartif_names[] = {
	"uart1_232",
	"uart1_485",
	"uart1_422",
	"uart2_232",
	"uart2_485",
	"uart2_422",
	"uart3_232",
	"uart3_485",
	"uart3_422",
	"uart4_232",
	"uart4_485",
	"uart4_422",
};

static struct gpio_chip moxa_gpio_uartif_chip = {
	.label		= "moxa-gpio-uartif",
	.owner		= THIS_MODULE,
	.get		= moxa_gpio_uartif_get,
	.set		= moxa_gpio_uartif_set,
	.base		= -1,
	.ngpio		= sizeof(gpio_uartif_names)/sizeof(char*),
	.names		= gpio_uartif_names
};

/*
 * module: miniPCIe power chip-section
 */
static int moxa_gpio_pciepwr_get(struct gpio_chip *gc, unsigned gpio_num)
{
	int val;

	if (0==pciepwr_get(gpio_num, &val)) {
		if (PCIEPWR_ON==val)
			return 1;
	}
	return 0;
}

static void moxa_gpio_pciepwr_set(struct gpio_chip *gc, unsigned gpio_num, int val)
{
	if ( val )
		val = PCIEPWR_ON;
	else
		val = PCIEPWR_OFF;
	pciepwr_set(gpio_num, val);
}

const char *gpio_pciepwr_names[] = {
	"pciepwr_1",
	"pciepwr_2",
};

static struct gpio_chip moxa_gpio_pciepwr_chip = {
	.label		= "moxa-gpio-pciepwr",
	.owner		= THIS_MODULE,
	.get		= moxa_gpio_pciepwr_get,
	.set		= moxa_gpio_pciepwr_set,
	.base		= -1,
	.ngpio		= sizeof(pciepwr_pin_def)/2,
	.names		= gpio_pciepwr_names
};

/*
 * module: sim card select chip-section
 */
static int moxa_gpio_sim_sel_get(struct gpio_chip *gc, unsigned gpio_num)
{
	int val;
	if (0==sim_sel_get(gpio_num, &val)) {
		if (SIM_A==val)
			return 1;
	}
	return 0;
}

static void moxa_gpio_sim_sel_set(struct gpio_chip *gc, unsigned gpio_num, int val)
{
	if(val)
		val = SIM_A;
	else
		val = SIM_B;
	sim_sel_set(gpio_num, val);
}

const char *gpio_sim_sel_names[] = {
	"sim_sel_1",
	"sim_sel_2",
};

static struct gpio_chip moxa_gpio_sim_sel_chip = {
	.label		= "moxa-gpio-sim-sel",
	.owner		= THIS_MODULE,
	.get		= moxa_gpio_sim_sel_get,
	.set		= moxa_gpio_sim_sel_set,
	.base		= -1,
	.ngpio		= sizeof(sim_sel_pin_def)/2,
	.names		= gpio_sim_sel_names
};

/*
 * module: DIP switch select chip-section
 */
static int moxa_gpio_dip_sel_get(struct gpio_chip *gc, unsigned gpio_num)
{
	int val;
	if (0==dip_sel_get(gpio_num, &val)) {
		if (DIP_1==val)
			return 1;
	}
	return 0;
}

static void moxa_gpio_dip_sel_set(struct gpio_chip *gc, unsigned gpio_num, int val)
{
	/* do nothing */
	return ;
}

const char *gpio_dip_sel_names[] = {
	"dip_sel_1",
	"dip_sel_2",
};

static struct gpio_chip moxa_gpio_dip_sel_chip = {
	.label		= "moxa-gpio-dip-sel",
	.owner		= THIS_MODULE,
	.get		= moxa_gpio_dip_sel_get,
	.set		= moxa_gpio_dip_sel_set,
	.base		= -1,
	.ngpio		= sizeof(dip_sel_pin_def)/2,
	.names		= gpio_dip_sel_names
};

#endif /* SUPPORT_GPIOSYSFS */


/* initialize module (and interrupt) */
static int __init moxa_misc_init_module (void)
{
	int retval=0;

	printk("initializing MOXA misc. device module\n");

	// register misc driver
	retval = misc_register(&sim_sel_miscdev);
	if(retval != 0) {
		printk("Moxa sim card select driver: Register misc fail !\n");
		goto moxa_misc_init_module_err1;
	}

	retval = misc_register(&pciepwr_miscdev);
	if(retval != 0) {
		printk("Moxa miniPCIe driver: Register misc fail !\n");
		goto moxa_misc_init_module_err2;
	}

	retval = misc_register(&do_miscdev);
	if(retval != 0) {
		printk("Moxa DO driver: Register misc fail !\n");
		goto moxa_misc_init_module_err3;
	}

	retval = misc_register(&di_miscdev);
	if(retval != 0) {
		printk("Moxa DI driver: Register misc fail !\n");
		goto moxa_misc_init_module_err4;
	}

	retval = misc_register(&sif_miscdev);
	if(retval != 0) {
		printk("Moxa uart interface driver: Register misc fail !\n");
		goto moxa_misc_init_module_err5;
	}

	retval = gpio_init();
	if(retval != 0) {
		printk("Moxa GPIO init driver: gpio_init() fail !\n");
		goto moxa_misc_init_module_err6;
	}

#ifdef SUPPORT_GPIOSYSFS
	retval = gpiochip_add(&moxa_gpio_uartif_chip);
	if(retval < 0) {
		printk("Moxa uart interface driver: gpiochip_add(&moxa_gpio_uartif_chip) fail !\n");
		goto moxa_misc_init_module_err7;
	}

	retval = gpiochip_add(&moxa_gpio_do_chip);
	if(retval < 0) {
		printk("Moxa DO driver: gpiochip_add(&moxa_gpio_do_chip) fail !\n");
		goto moxa_misc_init_module_err8;
	}

	retval = gpiochip_add(&moxa_gpio_di_chip);
	if(retval < 0) {
		printk("Moxa DI driver: gpiochip_add(&moxa_gpio_di_chip) fail !\n");
		goto moxa_misc_init_module_err9;
	}

	retval = gpiochip_add(&moxa_gpio_pciepwr_chip);
	if(retval < 0) {
		printk("Moxa miniPCIe power control driver: gpiochip_add(&moxa_gpio_pciepwr_chip) fail !\n");
		goto moxa_misc_init_module_err10;
	}

	retval = gpiochip_add(&moxa_gpio_sim_sel_chip);
	if(retval < 0) {
		printk("Moxa sim card select control driver: gpiochip_add(&moxa_gpio_sim_sel_chip) fail !\n");
		goto moxa_misc_init_module_err11;
	}

	retval = gpiochip_add(&moxa_gpio_dip_sel_chip);
	if(retval < 0) {
		printk("Moxa DIP switch select control driver: gpiochip_add(&moxa_gpio_dip_sel_chip) fail !\n");
		goto moxa_misc_init_module_err12;
	}

#endif /* SUPPORT_GPIOSYSFS */

	return 0;
#ifdef CONFIG_GPIO_SYSFS
moxa_misc_init_module_err12:
	gpiochip_remove(&moxa_gpio_sim_sel_chip);
moxa_misc_init_module_err11:
	gpiochip_remove(&moxa_gpio_pciepwr_chip);
moxa_misc_init_module_err10:
	gpiochip_remove(&moxa_gpio_di_chip);
moxa_misc_init_module_err9:
	gpiochip_remove(&moxa_gpio_do_chip);
moxa_misc_init_module_err8:
	gpiochip_remove(&moxa_gpio_uartif_chip);
#endif
moxa_misc_init_module_err7:
	gpio_exit();
moxa_misc_init_module_err6:
	misc_deregister(&sif_miscdev);
moxa_misc_init_module_err5:
	misc_deregister(&di_miscdev);
moxa_misc_init_module_err4:
	misc_deregister(&do_miscdev);
moxa_misc_init_module_err3:
	misc_deregister(&pciepwr_miscdev);
moxa_misc_init_module_err2:
	misc_deregister(&sim_sel_miscdev);
moxa_misc_init_module_err1:
	return retval;
}

/* close and cleanup module */
static void __exit moxa_misc_cleanup_module (void)
{
	printk("cleaning up module\n");
	misc_deregister(&sif_miscdev);
	misc_deregister(&di_miscdev);
	misc_deregister(&do_miscdev);
	misc_deregister(&pciepwr_miscdev);
	misc_deregister(&sim_sel_miscdev);
#ifdef SUPPORT_GPIOSYSFS
	gpiochip_remove(&moxa_gpio_uartif_chip);
	gpiochip_remove(&moxa_gpio_di_chip);
	gpiochip_remove(&moxa_gpio_do_chip);
	gpiochip_remove(&moxa_gpio_pciepwr_chip);
	gpiochip_remove(&moxa_gpio_sim_sel_chip);
	gpiochip_remove(&moxa_gpio_dip_sel_chip);
#endif /* SUPPORT_GPIOSYSFS */
	gpio_exit();
}

module_init(moxa_misc_init_module);
module_exit(moxa_misc_cleanup_module);
MODULE_AUTHOR("ElvsiCW.Yao@moxa.com");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("MOXA misc. device driver");
