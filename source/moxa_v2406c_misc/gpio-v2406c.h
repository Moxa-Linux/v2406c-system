#ifndef __GPIO_V2406C_H__
#define __GPIO_V2406C_H__

#define moxainb inb
#define moxaoutb outb

/* Because V-2406C BIOS has configured the superio, 
 * we don't need to configure it again. Turn the LED off.
 */
#define moxa_platform_led_init() {		\
	if ( ACTIVE_LOW )			\
		moxaoutb(0xFF, LED_PORT);	\
	else					\
		moxaoutb(0, LED_PORT);		\
}

#define superio_config(x)

#endif // ifndef __GPIO_V2406C_H__
