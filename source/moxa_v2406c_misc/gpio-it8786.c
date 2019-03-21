/*
 *  GPIO interface for Moxa Computer - IT8786 Super I/O chip
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License 2 as published
 *  by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/io.h>
#include <linux/errno.h>
#include <linux/ioport.h>
#include <linux/gpio.h>

#define GPIO_NAME		"moxa-it8786-gpio"
#define SIO_CHIP_ID		0x8786
#define CHIP_ID_HIGH_BYTE	0x20
#define CHIP_ID_LOW_BYTE	0x21

#define GPIO_BA_HIGH_BYTE	0x62
#define GPIO_BA_LOW_BYTE	0x63
#define GPIO_IOSIZE		8

#define GPIO_GROUP_1		0
#define GPIO_GROUP_2		1
#define GPIO_GROUP_3		2
#define GPIO_GROUP_4		3
#define GPIO_GROUP_5		4
#define GPIO_GROUP_6		5
#define GPIO_GROUP_7		6
#define GPIO_GROUP_8		7
#define GPIO_BIT_0		(1<<0)
#define GPIO_BIT_1		(1<<1)
#define GPIO_BIT_2		(1<<2)
#define GPIO_BIT_3		(1<<3)
#define GPIO_BIT_4		(1<<4)
#define GPIO_BIT_5		(1<<5)
#define GPIO_BIT_6		(1<<6)
#define GPIO_BIT_7		(1<<7)

#define RS232_MODE		0
#define RS485_2WIRE_MODE	1
#define RS422_MODE		2
#define RS485_4WIRE_MODE	3

/*
 * Customization for V-2406C
 */

/* UART interface */
static u8 uartif_pin_def[] = {
	GPIO_GROUP_1, GPIO_BIT_4,	/* UART1 RS232 */
	GPIO_GROUP_1, GPIO_BIT_1,	/* UART1 RS485 2 Wire */
	GPIO_GROUP_1, GPIO_BIT_2,	/* UART1 RS422/RS485 4 Wire */
	0xFF, 0xFF,
	GPIO_GROUP_3, GPIO_BIT_0,	/* UART2 RS232 */
	GPIO_GROUP_1, GPIO_BIT_5,	/* UART2 RS485 2 Wire */
	GPIO_GROUP_1, GPIO_BIT_6,	/* UART2 RS422/RS485 4 Wire */
	0xFF, 0xFF,
	GPIO_GROUP_3, GPIO_BIT_6,	/* UART3 RS232 */
	GPIO_GROUP_3, GPIO_BIT_2,	/* UART3 RS485 2 Wire */
	GPIO_GROUP_3, GPIO_BIT_3,	/* UART3 RS422/RS485 4 Wire */
	0xFF, 0xFF,
	GPIO_GROUP_5, GPIO_BIT_0,	/* UART4 RS232 */
	GPIO_GROUP_3, GPIO_BIT_7,	/* UART4 RS485 2 Wire */
	GPIO_GROUP_4, GPIO_BIT_7,	/* UART4 RS422/RS485 4 Wire */
	0xFF, 0xFF,
};

/* DI pin */
static u8 di_pin_def[] = {
	GPIO_GROUP_7, GPIO_BIT_0,	/* di1 */
	GPIO_GROUP_7, GPIO_BIT_1,	/* di2 */
	GPIO_GROUP_7, GPIO_BIT_2,	/* di3 */
	GPIO_GROUP_7, GPIO_BIT_3,	/* di4 */
};

/* DO pin */
static u8 do_pin_def[] = {
	GPIO_GROUP_7, GPIO_BIT_4,	/* do1 */
	GPIO_GROUP_7, GPIO_BIT_5,	/* do2 */
	GPIO_GROUP_7, GPIO_BIT_6,	/* do3 */
	GPIO_GROUP_7, GPIO_BIT_7,	/* do4 */
};

/* miniPCIe power */
static u8 pciepwr_pin_def[] = {
	GPIO_GROUP_8, GPIO_BIT_1,	/* pciepwr_1 */
	GPIO_GROUP_8, GPIO_BIT_3,	/* pciepwr_2 */
};

/* sim card select */
static u8 sim_sel_pin_def[] = {
	GPIO_GROUP_8, GPIO_BIT_0,	/* sim_sel_1 */
	GPIO_GROUP_8, GPIO_BIT_2,	/* sim_sel_2 */
};

/* DIP switch select */
static u8 dip_sel_pin_def[] = {
	GPIO_GROUP_2, GPIO_BIT_7,	/* dip_sel_1 */
	GPIO_GROUP_6, GPIO_BIT_7,	/* dip_sel_2 */
};

static u8 ports[1] = { 0x2e };
static u8 port;
 
static DEFINE_SPINLOCK(sio_lock);
static u16 gpio_ba;

static u8 read_reg(u8 addr, u8 port)
{
	outb(addr, port);
	return inb(port + 1);
}

static void write_reg(u8 data, u8 addr, u8 port)
{
	outb(addr, port);
	outb(data, port + 1);
}

static int enter_conf_mode(u8 port)
{
	/*
	 * Try to reserve REG and REG + 1 for exclusive access.
	 */
	if (!request_muxed_region(port, 2, GPIO_NAME))
		return -EBUSY;

	outb(0x87, port);
	outb(0x01, port);
	outb(0x55, port);
	outb((port == 0x2e) ? 0x55 : 0xaa, port);

	return 0;
}

static void exit_conf_mode(u8 port)
{
	outb(0x2, port);
	outb(0x2, port + 1);
	release_region(port, 2);
}

static void enter_gpio_mode(u8 port)
{
	write_reg(0x7, 0x7, port);
}

static void enter_uart_mode(u8 index, u8 port)
{
	u8 ldn = 0;
	if (index == 0) {
		ldn = 0x1;
	}
	else if (index == 1) {
		ldn = 0x2;
	}
	else if (index == 2) {
		ldn = 0x8;
	}
	else if (index == 3) {
		ldn = 0x9;
	}
	else if (index == 4) {
		ldn = 0xb;
	}
	else if (index == 5) {
		ldn = 0xc;
	}
	else {
		return ;
	}

	write_reg(ldn, 0x7, port);
}

void write_gpio(u8 *pindef, unsigned num, int val)
{
	u16 reg;
	u8 bit;
	u8 curr_vals;

	reg = gpio_ba + pindef[num*2];
	bit = pindef[num*2+1];

	spin_lock(&sio_lock);
	curr_vals = inb(reg);
	if (val)
		outb(curr_vals | bit, reg);
	else
		outb(curr_vals & ~bit, reg);
	spin_unlock(&sio_lock);
}

int read_gpio(u8 *pindef, unsigned num)
{
	u16 reg;
	u8 bit;

	reg = gpio_ba + pindef[num*2];
	bit = pindef[num*2+1];

	return !!(inb(reg) & bit);
}

int uartif_set(unsigned num, int val)
{
	u16 reg;
	u8 bit;
	u8 curr_vals;
	int i;

	if (num >= ((sizeof(uartif_pin_def)/2)/4)) {
		return -EINVAL;
	}

	if (val >= 4 || val < 0) {
		return -EINVAL;
	}

	/* Set output mode */
	for (i = 0; i < 4; i++) {
		reg = gpio_ba + uartif_pin_def[(num*4+i)*2];
		bit = uartif_pin_def[(num*4+i)*2+1];
		if (reg==0xff && bit==0xff) {
			continue;
		}

		spin_lock(&sio_lock);
		curr_vals = inb(reg);
		if (i==val)
			outb(curr_vals | bit, reg);
		else
			outb(curr_vals & ~bit, reg);
		spin_unlock(&sio_lock);
	}

	/* Switch on/off RS485DCE */
	spin_lock(&sio_lock);
	if (enter_conf_mode(port)) {
		spin_unlock(&sio_lock);
		return -EINVAL;
	}

	enter_uart_mode(num, port);
	curr_vals = read_reg(0xf0, port);
	if ( val == RS485_2WIRE_MODE) {
		curr_vals |= 0x80;
	} else {
		curr_vals &= ~0x80;
	}
	write_reg(curr_vals, 0xf0, port);
	exit_conf_mode(port);
	spin_unlock(&sio_lock);

	return 0;
}

int uartif_get(unsigned num, int *val)
{
	u16 reg;
	u8 bit;
	int i;

	if (num >= ((sizeof(uartif_pin_def)/2)/4)) {
		return -EINVAL;
	}

	/* Get output mode */
	for (i = 0; i < 4; i++) {
		reg = gpio_ba + uartif_pin_def[(num*4+i)*2];
		bit = uartif_pin_def[(num*4+i)*2+1];
		if ( reg==0xff && bit==0xff ) {
			continue;
		}

		if (read_gpio(uartif_pin_def, num*4+i)) {
			*val = i;
			return 0;
		}
	}

	return -EINVAL;
}

int di_get(unsigned num, int *val)
{
	if (num >= (sizeof(di_pin_def)/2)) {
		return -EINVAL;
	}
	*val = read_gpio(di_pin_def, num);
	return 0;
}

int do_set(unsigned num, int val)
{
	if (num >= (sizeof(do_pin_def)/2)) {
		return -EINVAL;
	}
	write_gpio(do_pin_def, num, val);
	return 0;
}

int do_get(unsigned num, int *val)
{
	if (num >= (sizeof(do_pin_def)/2)) {
		return -EINVAL;
	}
	*val = read_gpio(do_pin_def, num);
	return 0;
}

int pciepwr_set(unsigned num, int val)
{
	if (num >= (sizeof(pciepwr_pin_def)/2)) {
		return -EINVAL;
	}

	write_gpio(pciepwr_pin_def, num, val);
	return 0;
}

int pciepwr_get(unsigned num, int *val)
{
	if (num >= (sizeof(pciepwr_pin_def)/2)) {
		return -EINVAL;
	}

	*val = read_gpio(pciepwr_pin_def, num);
	return 0;
}

int sim_sel_set(unsigned num, int val)
{
	if (num >= (sizeof(sim_sel_pin_def)/2)) {
		return -EINVAL;
	}

	write_gpio(sim_sel_pin_def, num, val);
	return 0;
}

int sim_sel_get(unsigned num, int *val)
{
	if (num >= (sizeof(sim_sel_pin_def)/2)) {
		return -EINVAL;
	}

	*val = read_gpio(sim_sel_pin_def, num);
	return 0;
}
/*
int dip_sel_set(unsigned num, int val)
{
	return 0;
}
*/
int dip_sel_get(unsigned num, int *val)
{
	if (num >= (sizeof(dip_sel_pin_def)/2)) {
		return -EINVAL;
	}

	*val = read_gpio(dip_sel_pin_def, num);
	return 0;
}

int gpio_init(void)
{
	int err;
	int i, id;

	/* chip and port detection */
	for (i = 0; i < ARRAY_SIZE(ports); i++) {
		spin_lock(&sio_lock);
		err = enter_conf_mode(ports[i]);
		if (err) {
			spin_unlock(&sio_lock);
			return err;
		}

		id = (read_reg(CHIP_ID_HIGH_BYTE, ports[i]) << 8) +
			read_reg(CHIP_ID_LOW_BYTE, ports[i]);
		exit_conf_mode(ports[i]);
		spin_unlock(&sio_lock);

		if (id == SIO_CHIP_ID) {
			port = ports[i];
			break;
		}
	}

	if (!port)
		return -ENODEV;

	/* fetch GPIO base address */
	spin_lock(&sio_lock);
	err = enter_conf_mode(port);
	if (err) {
		spin_unlock(&sio_lock);
		return err;
	}

	enter_gpio_mode(port);
	gpio_ba = (read_reg(GPIO_BA_HIGH_BYTE, port) << 8) +
			read_reg(GPIO_BA_LOW_BYTE, port);
	exit_conf_mode(port);
	spin_unlock(&sio_lock);

	if (!request_region(gpio_ba, GPIO_IOSIZE, GPIO_NAME)) {
		printk("request_region address 0x%x failed!\n", gpio_ba);
		gpio_ba = 0;
		return -EBUSY;
	}

	return 0;
}

void gpio_exit(void)
{
	if (gpio_ba) {
		release_region(gpio_ba, GPIO_IOSIZE);
		gpio_ba = 0;
	}
}
