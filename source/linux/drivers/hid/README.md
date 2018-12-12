## V2406C hid driver source
### Build modules
```
apt update
apt install linux-headers-4.9.0-6-amd64
cd v2406c-system/source/linux/drivers/hid
make
```

### Get driver source
`KERNEL_VERSION`: `4.9.88`
```
git clone git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git
cd linux
git checkout v4.9.88
```

### HID driver path
```
drivers/hid/hid-cp2112.c
drivers/hid/hid-ids.h
```
