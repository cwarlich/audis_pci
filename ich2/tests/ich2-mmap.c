#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "ich2.h"
int main(int argc, char *argv[]) {
	uint8_t *regs;
	int fd;
	fd = open("/dev/ich2", O_RDWR);
	assert(fd >= 0);
	regs = mmap(NULL, ICH2_REGISTER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, ICH2_OFFSET_REGISTERS);
	assert(regs != MAP_FAILED);
        while(1) {
            *(int *) (regs + ICH2_TEST_BASE) = 0xf;
            printf("%x\n", *(int *) (regs + ICH2_TEST_BASE));
            sleep(1);
            *(int *) (regs + ICH2_TEST_BASE) = 0;
            sleep(1);
        }
	close(fd);
	return 0;
}
