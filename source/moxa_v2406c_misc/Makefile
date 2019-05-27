TARGET	:= misc-moxa-v2406c
KVER	:= $(shell uname -r)
KDIR	:= /lib/modules/$(KVER)/build
PWD	:= $(shell pwd)

obj-m += misc-moxa-v2406c.o

all: modules

modules:
	@echo "Making modules $(TARGET).ko ..."
	$(MAKE) -C $(KDIR) M=$(PWD) modules

install: all
	mkdir -v -p "$(DESTDIR)/lib/modules/$(KVER)/kernel/drivers/gpio"
	install $(TARGET).ko $(DESTDIR)/lib/modules/$(KVER)/kernel/drivers/gpio/

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

