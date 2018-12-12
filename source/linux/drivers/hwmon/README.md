## V2406C hwmon driver source
### Get driver source
`KERNEL_VERSION`: `4.9.88`
```
git clone git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git
cd linux
git checkout v4.9.88
```

### hwmon driver path
```
drivers/hwmon/ltc4151.c
drivers/hwmon/ina2xx.c
```

### Build modules
```
apt update
apt install linux-headers-4.9.0-6-amd64
cd v2406c-system/source/linux/drivers/hid
make
```
