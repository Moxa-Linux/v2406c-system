#define MOXA_HOTSWAP_MINOR 115
#define DEVICE_NAME "hotswap"
#define mprintk(fmt, args...) printk(KERN_ERR "moxa_hotswap:"fmt,  ##args) 

/* 
 * Supersit bits 
 */
#define	LED_BTN_ADDR	0x307
#define	BIT_DISK1_LED	(1<<4)
#define	BIT_DISK1_BTN	(1<<5)
#define	BIT_DISK2_LED	(1<<6)
#define	BIT_DISK2_BTN	(1<<7)

/*
 * Hardware specific part
 */
#define	MY_DEVICE_ID1	0x9d03	/* SATA controller: Intel Corporation Sunrise Point-LP SATA Controller */

/* Ref Intel 100 Series Chipset Family Platform Controller Hub */
#define SATA_P1SSTS	0x1A8	/* Port 1 Serial ATA Status */
#define SATA_P2SSTS	0x228	/* Port 2 Serial ATA Status */

#define moxainb inb_p
#define moxaoutb outb_p

typedef struct _disk_info {
	unsigned int status;
	int busy;
	int idle_cnt;
} disk_info;

/*
 * Debug
 */
#ifdef DEBUG 
#define p(fmt, args...) printk("%s: "fmt, __FUNCTION__, ##args) // use ## to remove commaa, for not args condition
#define pp(fmt, args...)  /* pp: not print debug message */
#else
#define p(fmt, args...) 
#define pp(fmt, args...)
#endif
