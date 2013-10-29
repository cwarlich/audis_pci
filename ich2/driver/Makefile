KBUILD = /root/views/dcuos7/derived/tdc/kernelObjects
obj-m := ich2.o
.PHONY: modules
XXX:=$(CURDIR)
EXTRA_CFLAGS := -g -O0 -I$(src)/../..
#modules: ; $(MAKE) -C $(KBUILD) SUBDIRS=`pwd` V=1 $@
modules: ; $(MAKE) -C $(KBUILD) SUBDIRS=`pwd` $@
clean:
	rm -f *.o ich2.mod.c ich2.ko modules.order Module.symvers .ich2*.cmd 
	rm -rf .tmp_versions
