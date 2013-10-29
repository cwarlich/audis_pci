tests = ich2-dma ich2-mmap

CFLAGS = -g -Wall -I../driver -I ../.. -D_AUD_SOURCE
LDFLAGS = -specs=specs-audrt-prio -laudis -lrt -lpthread -Xlinker -dynamic-linker -Xlinker /audislib/ld-linux.so.2
CC := /root/views/dcuos7/derived/tdc/tools/bin/i686-v42-gcc

all: $(tests)

$(foreach i, $(tests), $(eval $i: $i.o) $(eval $i.o: $i.c ../driver/ich2.h ../../gamecp.h Makefile))

clean:
	rm -rf $(tests) $(addsuffix .o,$(tests))
