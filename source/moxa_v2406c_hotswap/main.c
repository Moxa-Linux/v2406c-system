/*
 * This is Moxa hotswap driver
 * Date		Author		Comment
 * 03-01-2011	Wade Liang	Creat it
 * 10-03-2013	Jared Wu	Porting to V2616A
 * 12-11-2018	Elvis Yao	Porting to V2406C
 */
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/miscdevice.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/types.h>	/*define u8 ...*/
#include <linux/pci.h>
#include <linux/workqueue.h>
#include <linux/timer.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/spinlock.h>
#include "mxhtsp_ioctl.h"
#include "moxa_hotswap.h" 
#include "moxa_superio.h" 

MODULE_AUTHOR("ElvisCW.Yao@moxa.com");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("MOXA V2406C hotswap driver");

static unsigned long paddr; /* physical memory address */
static void *vaddr; /* virtual memory address */
static struct timer_list timer;
static disk_info d1, d2;

static int hotswap_led_control(int led_num, int on) {
	int led_bit;
	unsigned char pled_status;

	if (((led_num != 1) && (led_num !=2)) || ((on != 1) && (on != 0)))
		return -1;

	if (led_num == 1) {
		led_bit = BIT_DISK1_LED;
	} else {
		led_bit = BIT_DISK2_LED;
	}
	if (on == 1) {  /* change led status to 1, because high activate */
		pled_status = moxainb(LED_BTN_ADDR);
		pled_status &= ~led_bit;
		moxaoutb(pled_status, LED_BTN_ADDR);
	} else if (on == 0) {
		pled_status = moxainb(LED_BTN_ADDR);
		pled_status |= led_bit;
		moxaoutb(pled_status, LED_BTN_ADDR);
	}

	return 0;
}

static int hotswap_open(struct inode *inode, struct file *file) 
{
	p("\n");
	return 0;
}

static int hotswap_release(struct inode *inode, struct file *file) 
{
	p("\n");
	return 0;
}

static long hotswap_ioctl(struct file *file, unsigned int cmd, unsigned long arg) 
{
	void __user *addr = (void __user *)arg;
	int err = 0;
	mxhtsp_param p;
	if (_IOC_TYPE(cmd) != IOCTL_MAGIC)
		return -ENOTTY;
	if (_IOC_NR(cmd) > IOCTL_MAXNR)
		return -ENOTTY;

	if (_IOC_DIR(cmd) & _IOC_READ) {
		err = !access_ok(VERIFY_WRITE, (void __user*)arg, _IOC_SIZE(cmd));
	} else if (_IOC_DIR(cmd) & _IOC_WRITE) {
		err = !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
	}
	if (err)
	        return -EFAULT;

	switch (cmd) {
		case IOCTL_GET_DISK_STATUS:
			if (copy_from_user(&p, addr, sizeof(mxhtsp_param)))
				goto err;
			if (p.disk_num == 1) 
				p.val = d1.busy;
			else if (p.disk_num == 2) 
				p.val = d2.busy;
			else 
				return -EINVAL;
			if (copy_to_user(addr, &p, sizeof(mxhtsp_param)))  
				goto err;
			break;

		case IOCTL_SET_DISK_LED : 
			if (copy_from_user(&p, addr, sizeof(mxhtsp_param))) 
				goto err;
			return hotswap_led_control(p.disk_num, p.val);
			break;

		case IOCTL_GET_DISK_BTN :
			if (copy_from_user(&p, addr, sizeof(mxhtsp_param))) 
				goto err;
			if (p.disk_num == 1) {
				p.val = moxainb(LED_BTN_ADDR);
				/* if BIT_DISK1_BTN is 0 (button pressed), set p.val to 1 to indicated the button is pressed */
				p.val = ( (~p.val) & BIT_DISK1_BTN) ? 1 : 0 ;
			}
			else if (p.disk_num == 2) {
				p.val = moxainb(LED_BTN_ADDR);
				p.val = ( (~p.val) & BIT_DISK2_BTN) ? 1 : 0 ;
			}
			else 
				return -EINVAL;
			if (copy_to_user(addr, &p, sizeof(mxhtsp_param))) 
				goto err;
			break;
		
		case IOCTL_CHECK_DISK_PLUGGED :
			if (copy_from_user(&p, addr, sizeof(mxhtsp_param))) {
				goto err;
			}
			if (p.disk_num == 1) {
				p.val = (ioread32(vaddr + SATA_P1SSTS) == 0x4) ? 0: 1;/* 0x4 for un-plug */
				if (p.val == 1) {
					p.val = (ioread32(vaddr + SATA_P1SSTS) == 0x00) ? 0: 1;/* TODO: BIOS disable hotswap */
				}

			}
			else if (p.disk_num == 2) {
				p.val = (ioread32(vaddr + SATA_P2SSTS) == 0x4) ? 0: 1;/* 0x4 for un-plug */
				if (p.val == 1) {
                                        p.val = (ioread32(vaddr + SATA_P2SSTS) == 0x00) ? 0: 1;/* TODO: BIOS disable hotswap */
                                }
			}
			else 
				return -EINVAL;
			if (copy_to_user(addr, &p, sizeof(mxhtsp_param))) 
				goto err;
			break;

		default :
			mprintk("ioctl error\n");
			return -EINVAL;
	}
	return 0; 

err:
	mprintk("copy data error\n");
	return -1;
}

ssize_t hotswap_write(struct file *file, const char __user *buf, size_t count,
		loff_t *pos)
{
	char param[4];
	int led_num;
	int on;

	if (count != 4) 
		return -1;

	if(copy_from_user(param, buf, count)) 
		return -1;

	p("count=%d param[3]=%x\n", count, param[3]);
	param[3] = '\0'; /* change \n to \0 */
	sscanf(param, "%d %d", &led_num, &on);

	if (hotswap_led_control(led_num, on)) 
		return -1;

	return count;
}

static struct file_operations hotswap_fops = {
	.owner	=	THIS_MODULE,
	.unlocked_ioctl  = hotswap_ioctl,
	.open	=	hotswap_open,
	.write	=	hotswap_write,
	.release=	hotswap_release,
};

static struct miscdevice hotswap_dev = {
	.minor = MOXA_HOTSWAP_MINOR,
	.name = DEVICE_NAME,
	.fops = &hotswap_fops,
};


static void hotswap_check_disk(unsigned long arg) {
	unsigned long now;

	now = ioread32(vaddr+SATA_P1SSTS);

	if (now != d1.status) {
		d1.status = now;
		d1.busy = 1;
		d1.idle_cnt = 0;
	} else if (d1.idle_cnt++ > 10) {
		d1.busy = 0;
		d1.idle_cnt = 0;
	}

	now = ioread32(vaddr+SATA_P2SSTS);

	if (now != d2.status) {
		d2.status = now;
		d2.busy = 1;
		d2.idle_cnt = 0;
	} else if (d2.idle_cnt++ > 10) {
		d2.busy = 0;
		d2.idle_cnt = 0;
	}

	mod_timer(&timer, jiffies+100*HZ/1000);

 	p("disk 0x128=%x, 0x1A8=%x, 0x228=%x, 0x2A8=%x\n",
		ioread32(vaddr+0x128), ioread32(vaddr+0x1A8), ioread32(vaddr+0x228),ioread32(vaddr+0x2A8));
}

static int __init hotswap_init_module (void) 
{
	struct pci_dev *dev;

	printk("Initializing MOXA hotswap module\n");

	/* get base address for ACHI controller */
	dev = pci_get_device(PCI_VENDOR_ID_INTEL, MY_DEVICE_ID1, NULL);
	if (!dev) {
		mprintk("can't find pci_device  \n");
		return -1;
	}
	paddr = pci_resource_start(dev, 5);
	p("paddr=%x\n", paddr);

#ifdef DEBUG
	unsigned long flag = pci_resource_flags(dev, 5);
	if (flag & IORESOURCE_IO) 
		printk("ioresource io\n");
	else if (flag & IORESOURCE_MEM) 
		printk("ioresource mem\n");
#endif
	/* register in /proc/iomem, 1024 can count in the same file 
	if (!request_mem_region(paddr, 1024, "hotswap")) {
		mprintk("can't reguest mem region");
	}
	*/
	vaddr = ioremap(paddr, 1024);

	if (misc_register(&hotswap_dev)) {
		mprintk("register misc fail !\n");
		return -1;
	}

	/* initial */
	d1.status = ioread32(vaddr + SATA_P1SSTS);
	d1.idle_cnt = 0;
	d2.status = ioread32(vaddr + SATA_P2SSTS);
	d2.idle_cnt = 0;
	hotswap_led_control(1, 0);
	hotswap_led_control(2, 0);

	init_timer(&timer);
	timer.data = 0;
	timer.function = hotswap_check_disk; 
	timer.expires = jiffies + 100*HZ/1000;
	add_timer(&timer);

	return 0;
}

/*
 * close and cleanup module
 */
static void __exit hotswap_cleanup_module (void) 
{
	p("\n");
	misc_deregister(&hotswap_dev);
	iounmap(vaddr);
	//release_mem_region(paddr, 1024); 
	del_timer(&timer);
}

module_init(hotswap_init_module);
module_exit(hotswap_cleanup_module);
