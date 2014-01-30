obj-m := fpga1.o
.PHONY: modules
EXTRA_CFLAGS := -g -O0 -I$(src)/../..
modules: ; $(MAKE) -C $(KERNEL) CROSS_COMPILE=$(CROSS_COMPILE) SUBDIRS=`pwd` $@
clean:
	rm -f *.o fpga1.mod.c fpga1.ko modules.order Module.symvers .fpga1*.cmd 
	rm -rf .tmp_versions
