/*
 * CPU555 ICH2 driver test application
 * Synchronous clock test
 *
 * Copyright (C) Siemens AG, 2012, 2013
 * All Rights Reserved
 *
 * Authors:
 *     Christof Warlich <christof.warlich@siemens.com>
 */

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <rt/rtime.h>
#include "ich2.h"

int main(int argc, char *argv[])
{
	struct sigevent irq_event;
	struct timespec to;
	siginfo_t info;
	sigset_t set;
	int err, fd, i;
	uint8_t *regs, *mem;

	fd = open("/dev/ich2", O_RDWR);
	assert(fd >= 0);

	mem = mmap(NULL, ICH2_SIZE_DMA, PROT_READ | PROT_WRITE, MAP_SHARED, fd, ICH2_OFFSET_DMA);
	assert(mem != MAP_FAILED);
        // Verication that we see the data being written by the driver during module loading:
	printf("before: pmem=%p, mem=%x\n", mem, *(uint32_t *) mem);
        // Writing something else.
        *(uint32_t *) mem = 0x4711;
        // Verification that the write from user space succeded.
	printf("after: pmem=%p, mem=%x\n", mem, *(uint32_t *) mem);

	regs = mmap(NULL, ICH2_REGISTER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, ICH2_OFFSET_REGISTERS);
	assert(regs != MAP_FAILED);
	pthread_setschedprio(pthread_self(), 80);

	/* Event 1 setup. */
	memset(&irq_event, 0, sizeof(irq_event));
	err = sigevent_set_notification(&irq_event, 0, SIGRT0, pthread_self());
	irq_event.sigev_value.sival_ptr = gamecp_name(ICH2_TDM0_ENABLE);
	assert(err == 0);
	err = event_create(fd, &irq_event, ICH2_TDM0_ENABLE);
	assert(err == 0);
	*(uint32_t *) (regs + ICH2_IRQ_BASE) = 0x000c0010;
	*(uint32_t *) (regs + ICH2_TDM_BASE) = 0x17701906;
	*(uint32_t *) (regs + ICH2_TDM_BASE) = 0x17701907;

	sigemptyset(&set);
	sigaddset(&set, SIGRT0);
	to.tv_sec = 1;
	to.tv_nsec = 0;
	for(i = 0; i < 5; i++) {
		err = sigtimedwait(&set, &info, &to);
		if (err < 0 && errno == EAGAIN) {
			printf("Timeout!\n");
			break;
		}
		assert(info.si_signo == SIGRT0);
		printf("Got signal %d from event %s.\n", info.si_signo, (char *) info.si_ptr);
	}

	*(uint32_t *) (regs + ICH2_IRQ_BASE) = 0;
	*(uint32_t *) (regs + ICH2_TDM_BASE) = 0;
	/* Event 1 teardown */
	err = event_delete(fd, ICH2_TDM0_ENABLE);
	assert(err == 0);
	// Obviously, the ICH2 FPGA already works on the memory page being passed to it.
	printf("at last: pmem=%p, mem=%x\n", mem, *(uint32_t *) mem);
	return 0;
}
