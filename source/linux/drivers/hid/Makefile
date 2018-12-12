TARGET	:= hid-cp2112
KVER	:= $(shell uname -r)
KDIR	:= /lib/modules/$(KVER)/build
PWD	:= $(shell pwd)

obj-m += hid-cp2112.o

all: modules

modules:
	@echo "Making modules $(TARGET).ko ..."
	$(MAKE) -C $(KDIR) M=$(PWD) modules

install: all
	mkdir -v -p "$(DESTDIR)/lib/modules/$(KVER)/kernel/drivers/hid"
	install $(TARGET).ko $(DESTDIR)/lib/modules/$(KVER)/kernel/drivers/hid

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

