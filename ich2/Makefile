.PHONY: all
all:
	make -C driver KERNEL=$(KERNEL) CROSS_COMPILE=$(CROSS_COMPILE)
	make -C tests CROSS_COMPILE=$(CROSS_COMPILE)
clean:
	make -C driver clean
	make -C tests clean
