obj-m := ich2.o
.PHONY: modules
EXTRA_CFLAGS := -g -O0 -I$(src)/../..
modules: ; $(MAKE) -C $(KERNEL) CROSS_COMPILE=$(CROSS_COMPILE) SUBDIRS=`pwd` $@
clean:
	rm -f *.o ich2.mod.c ich2.ko modules.order Module.symvers .ich2*.cmd 
	rm -rf .tmp_versions
