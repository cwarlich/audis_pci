#include <assert.h>
#include <error.h>
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
#include "fpga1.h"

uint32_t read_reg32(uint8_t *regs, unsigned int offset) {
	return *(uint32_t *)(regs + offset);
}
void write_reg32(uint8_t *regs, unsigned int offset, uint32_t val) {
	*(uint32_t *)(regs + offset) = val;
}

uint8_t *regs;
int tmr_cnt = 0;
#define MY_SIGVAL_CODE 0xbeeb
//#define SIGINFO

void handler(union sigval val) {
	// Check value
	if (val.sival_int == MY_SIGVAL_CODE) {
		/* Trigger an interrupt at some point in the future, e.g.  in 1000ms. */
		write_reg32(regs, FPGA1_REGS_TIMER7_CMP, read_reg32(regs, FPGA1_REGS_TIMER7) + 50000000);
		// Code valid, increase counter
		tmr_cnt++;
	}
	printf("got event %x, count = %d\n", val.sival_int, tmr_cnt);
}

int main(int argc, char *argv[]) {
#ifdef SIGINFO
	siginfo_t info;
	sigset_t set;
#else
	pthread_attr_t th_attr;
	struct sched_param th_param;
#endif
	struct sigevent event;
	int fd, i;

	fd = open("/dev/fpga1", O_RDWR);
	assert(fd >= 0);
	regs = mmap(NULL, FPGA1_REGISTERS_SIZE, PROT_READ | PROT_WRITE,
		    MAP_SHARED, fd, FPGA1_OFFSET_REGISTERS);
	assert(regs != MAP_FAILED);

	// Make current thread a realtime thread
	pthread_setschedprio(pthread_self(), 80);

	// Fill in sigevent structure
	memset(&event, 0, sizeof(event));
	event.sigev_value.sival_int = MY_SIGVAL_CODE;
#ifdef SIGINFO
	// This fills in the remaining members
	assert(sigevent_set_notification(&event, 0, SIGRT0, pthread_self()) == 0);
#else
	// This fills in the remaining members
	event.sigev_notify = SIGEV_THREAD;
	event.sigev_notify_function = handler;
	event.sigev_notify_attributes = &th_attr;

	// Fill in pthread attributes for triggered thread to be a realtime thread
	pthread_attr_init(&th_attr);
	pthread_attr_setinheritsched(&th_attr, PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setschedpolicy(&th_attr, SCHED_FIFO);
	th_param.sched_priority = 80;
	pthread_attr_setschedparam(&th_attr, &th_param);
	pthread_attr_setname(&th_attr, "handler");
#endif

	assert(event_create(fd, &event, FPGA1_INT0_T7_INT_RISING) == 0);
	/* Trigger an interrupt at some point in the future, e.g.  in 1000ms. */
	write_reg32(regs, FPGA1_REGS_TIMER7_CMP, read_reg32(regs, FPGA1_REGS_TIMER7) + 50000000);

#ifdef SIGINFO
	sigemptyset(&set);
	sigaddset(&set, SIGRT0);
	for(i = 0; i < 10; i++) {
		sigwaitinfo(&set, &info);
		handler(info.si_value);
	}
#endif

	for(i = 0; i < 10; i++) {
		printf("main thread is waiting\n");
		sleep(1);
	}

	/* Event teardown */
	if(event_delete(fd, FPGA1_INT0_T7_INT_RISING) != 0) {
		error(-1, errno, "\n");
	}
        assert(close(fd) == 0);
	return 0;
}
