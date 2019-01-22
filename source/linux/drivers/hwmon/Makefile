TARGET	:= ina2xx.ko ltc4151.ko
KVER	:= $(shell uname -r)
KDIR	:= /lib/modules/$(KVER)/build
PWD	:= $(shell pwd)

obj-m += ina2xx.o ltc4151.o

all: modules

modules:
	@echo "Making modules $(TARGET) ..."
	$(MAKE) -C $(KDIR) M=$(PWD) modules

install: all
	mkdir -v -p "$(DESTDIR)/lib/modules/$(KVER)/kernel/drivers/hwmon"
	install $(TARGET) $(DESTDIR)/lib/modules/$(KVER)/kernel/drivers/hwmon/

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

