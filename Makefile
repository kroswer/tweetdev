
obj-m += tweetdev.o 

KDIR  = /usr/src/linux-headers-$(shell uname -r)

default:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

clean:
	rm -rf *.o *.ko *.mod