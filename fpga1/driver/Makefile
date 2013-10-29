KBUILD = /root/views/dcuos7/derived/tdc/kernelObjects
obj-m := fpga1.o
.PHONY: modules
XXX:=$(CURDIR)
EXTRA_CFLAGS := -g -O0 -I$(src)/../..
#modules: ; $(MAKE) -C $(KBUILD) SUBDIRS=`pwd` V=1 $@
modules: ; $(MAKE) -C $(KBUILD) SUBDIRS=`pwd` $@
clean:
	rm -f *.o fpga1.mod.c fpga1.ko modules.order Module.symvers .fpga1*.cmd 
	rm -rf .tmp_versions
