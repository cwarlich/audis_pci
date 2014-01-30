.PHONY: all
all:
	make -C fpga1 KERNEL=$(KERNEL) CROSS_COMPILE=$(CROSS_COMPILE)
	make -C ich2 KERNEL=$(KERNEL) CROSS_COMPILE=$(CROSS_COMPILE)
clean:
	make -C fpga1 clean
	make -C ich2 clean
