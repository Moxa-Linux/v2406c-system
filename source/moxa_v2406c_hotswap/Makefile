#	EXTRA_CFLAGS += -DDEBUG
#KDIR:=$(ROOTDIR)/$(LINUXDIR)
KDIR=/lib/modules/`uname -r`/build
KERNEL_VER=`cat $(ROOTDIR)/$(LINUXDIR)/include/config/kernel.release`

TARGET_MODULE=moxa_hotswap
obj-m += $(TARGET_MODULE).o

#Use multiple files
#WARNING:  TARGET_MODULE can't use the same name as TARGET_MODULE
$(TARGET_MODULE)-objs := main.o 

modules: $(TARGET_MODULE).ko 

#$(TARGET_MODULE).ko:  main.c $(TARGET_MODULE).h mxhtsp_ioctl.h Module.symvers-moxa_superio
$(TARGET_MODULE).ko:  main.c $(TARGET_MODULE).h mxhtsp_ioctl.h
	@echo "Making modules..."
	#read symbol from moxa_superio
	$(MAKE) -C $(KDIR) M=`pwd` modules

clean:
	$(MAKE) -C $(KDIR) M=`pwd` clean

romfs:
	mkdir -v -p "$(ROMFSDIR)/lib/modules/$(KERNEL_VER)/kernel/drivers/char"
	$(ROMFSINST) -v $(TARGET_MODULE).ko "/lib/modules/$(KERNEL_VER)/kernel/drivers/char"
