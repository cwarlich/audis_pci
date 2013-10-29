.PHONY: all
all:
	make -C fpga1
	make -C ich2
clean:
	make -C fpga1 clean
	make -C ich2 clean
