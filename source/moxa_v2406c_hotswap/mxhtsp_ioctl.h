/*
 * IOCTL
 */

typedef struct _mxhtsp_param {
	int disk_num;
	int val;
} mxhtsp_param;

#define IOCTL_MAXNR			4
#define IOCTL_MAGIC			'k'
#define IOCTL_GET_DISK_STATUS		_IOR(IOCTL_MAGIC,  1, mxhtsp_param)
#define IOCTL_SET_DISK_LED		_IOWR(IOCTL_MAGIC,  2, mxhtsp_param)
#define IOCTL_GET_DISK_BTN		_IOR(IOCTL_MAGIC, 3, mxhtsp_param)
#define IOCTL_CHECK_DISK_PLUGGED	_IOWR(IOCTL_MAGIC, 4, mxhtsp_param)
