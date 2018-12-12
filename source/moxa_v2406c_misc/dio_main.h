#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/miscdevice.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <asm/uaccess.h>
#include <asm/io.h>
//#include "moxa_superio.h"

/* mknod /dev/dio c 10 104 for this module */
#define MOXA_DIO_MINOR 104
#define NAME "dio"

/*
 * DIO file operaiton function call
 */

#define DIO_INPUT               1
#define DIO_OUTPUT              0
#define DIO_HIGH                1
#define DIO_LOW                 0
#define IOCTL_DIO_GET_MODE      1
#define IOCTL_DIO_SET_MODE      2
#define IOCTL_DIO_GET_DATA      3
#define IOCTL_DIO_SET_DATA      4
#define IOCTL_SET_DOUT          15
#define IOCTL_GET_DOUT          16
#define IOCTL_GET_DIN           17
#define ALL_PORT -1

/*
 * dp - debug print
 */
#ifdef DEBUG 
#define dp(fmt, args...) printk("moxa-device-dio, %s: "fmt, __FUNCTION__,##args)
#else
#define dp(fmt, args...) 
#endif

struct dio_set_struct {
	int     io_number;
	int     mode_data;// 1 for input, 0 for output, 1 for high, 0 for low
 };

/* data ----------------------------------------------------------------*/
typedef enum emethod {gpio, superio} gpio_method;
typedef struct entry {
    char *name;
    u8 do_state; /* save dout states */
    u32 addr; /* address of dio status */
    int di_num; 
    int do_num; 
    gpio_method method; /* superio/ gpio */
    int log_dev; /* use for super io */
} dio_dev;

/* global shared function */ 
extern dio_dev* moxa_platform_dio_init(void);

typedef u8 (*INB_CALLBACK)(dio_dev* mdev, struct dio_set_struct* set);
typedef void (*OUTB_CALLBACK)(dio_dev* mdev, struct dio_set_struct* set);
typedef void (*GET_DOUT_CALLBACK)(dio_dev* mdev, struct dio_set_struct* set);
int register_dio_inb( INB_CALLBACK inb_fun );
int register_dio_outb( OUTB_CALLBACK outb_fun );
int register_get_dout( GET_DOUT_CALLBACK get_outb_fun);
