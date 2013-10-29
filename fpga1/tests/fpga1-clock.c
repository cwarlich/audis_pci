/*
 * CPU555 FPGA1 driver test application
 * Synchronous clock test
 *
 * Copyright (C) Siemens AG, 2012, 2013
 * All Rights Reserved
 *
 * Authors:
 *     Jan Kiszka <jan.kiszka@siemens.com>
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
#include "fpga1.h"

uint32_t read_reg32(uint8_t *regs, unsigned int offset)
{
	return *(uint32_t *)(regs + offset);
}
void write_reg32(uint8_t *regs, unsigned int offset, uint32_t val)
{
	*(uint32_t *)(regs + offset) = val;
}

/* Converts nanoseconds to timer 0 or timer 5 period. */
#define NSEC_TO_T0(x) (x / 1000 - 1)
/* Converts nanoseconds to timer 7 oneshot delay. */
#define NSEC_TO_T7(x) (x / 20)

int main(int argc, char *argv[])
{
	struct timespec clock_period1 = {0, 100000000},
	                clock_period2 = {1, 0}, /* Cannot be changed. */
	                clock_period3 = {0, 200000000};
	struct timespec timer_period1 = {0, 800000000},
	                timer_period2 = {0, 500000000},
	                timer_period3 = {0, 200000000},
	                timer_period4 = {0, 100000000};
	struct sigevent timer_event1, timer_event2, timer_event3, timer_event4,
	                irq_event1, irq_event2;
	clockid_t clockid1, clockid2, clockid3;
	timer_t tid1, tid2, tid3, tid4;
	struct itimerspec itimer;
	struct timespec to;
	siginfo_t info;
	uint8_t *regs;
	sigset_t set;
	int err, fd, i;

	fd = open("/dev/fpga1", O_RDWR);
	assert(fd >= 0);
	regs = mmap(NULL, FPGA1_REGISTERS_SIZE, PROT_READ | PROT_WRITE,
		    MAP_SHARED, fd, FPGA1_OFFSET_REGISTERS);
	assert(regs != MAP_FAILED);
	pthread_setschedprio(pthread_self(), 80);

	/* Clock 1 setup for a rising edge as sync. */
	clockid1 = register_clock(fd, FPGA1_INT0_TIMER0_IRQ_RISING, &clock_period1);
	assert(clockid1 >= 0);
	memset(&timer_event1, 0, sizeof(timer_event1));
	err = sigevent_set_notification(&timer_event1, 0, SIGRT0, pthread_self());
	timer_event1.sigev_value.sival_ptr = gamecp_name(FPGA1_INT0_TIMER0_IRQ_RISING);
	assert(err == 0);
	err = timer_create(clockid1, &timer_event1, &tid1);
	assert(err == 0);
	itimer.it_interval = timer_period1;
	itimer.it_value = clock_period1;
	err = timer_settime(tid1, 0, &itimer, NULL);
	assert(err == 0);
	err = clock_settime(clockid1, &clock_period1);
	assert(err == 0);
	write_reg32(regs, FPGA1_REGS_TIMER2, NSEC_TO_T0(clock_period1.tv_nsec));
	write_reg32(regs, FPGA1_REGS_TIMER_CTR, 0x1);

	/* Clock 2 setup for a falling edge as sync. */
	clockid2 = register_clock(fd, FPGA1_INT0_RTC_INT_FALLING, &clock_period2);
	assert(clockid2 >= 0);
	memset(&timer_event2, 0, sizeof(timer_event2));
	err = sigevent_set_notification(&timer_event2, 0, SIGRT1, pthread_self());
	timer_event2.sigev_value.sival_ptr = gamecp_name(FPGA1_INT0_RTC_INT_FALLING);
	assert(err == 0);
	err = timer_create(clockid2, &timer_event2, &tid2);
	assert(err == 0);
	itimer.it_interval = timer_period2;
	itimer.it_value = clock_period1;
	err = timer_settime(tid2, 0, &itimer, NULL);
	assert(err == 0);
	err = clock_settime(clockid2, &clock_period2);
	assert(err == 0);

	/* Clock 3 setup for a rising edge as sync. */
	/* The periodic sync interupt is triggered by SW from event below, */
	/* being kicked in the for loop. */
	clockid3 = register_clock(fd, FPGA1_INT0_T7_INT_RISING, &clock_period3);
	assert(clockid3 >= 0);
	memset(&timer_event3, 0, sizeof(timer_event3));
	/* We share SIGRT0. */
	err = sigevent_set_notification(&timer_event3, 0, SIGRT2, pthread_self());
	timer_event3.sigev_value.sival_ptr = gamecp_name(FPGA1_INT0_T7_INT_RISING);
	assert(err == 0);
	err = timer_create(clockid3, &timer_event3, &tid3);
	assert(err == 0);
	itimer.it_interval = timer_period3;
	itimer.it_value = clock_period1;
	err = timer_settime(tid3, 0, &itimer, NULL);
	printf("%d, %d\n", errno, err);
	assert(err == 0);

	/* Event 1 setup: We can even set up an event for an */
	/* interrupt that belongs to an existing clock. */
	/* Note however that the event reason for a shared */
	/* event and clock should match, otherwise the last one wins. */
	memset(&irq_event1, 0, sizeof(irq_event1));
	err = sigevent_set_notification(&irq_event1, 0, SIGRT0, pthread_self());
	irq_event1.sigev_value.sival_ptr = gamecp_name(FPGA1_INT0_T7_INT_RISING);
	assert(err == 0);
	err = event_create(fd, &irq_event1, FPGA1_INT0_T7_INT_RISING);
	assert(err == 0);
	/* Trigger an interrupt at some point in the future, e.g.  in 100ms. */
	write_reg32(regs, FPGA1_REGS_TIMER7_CMP, read_reg32(regs, FPGA1_REGS_TIMER7) + 5000000UL);

	/* Event 2 setup: We cannot set up neither events nor clocks */
	/* that only differs by its reason (here: RISING instead of */
        /* FALLING. Good. */
	memset(&irq_event2, 0, sizeof(irq_event2));
	err = sigevent_set_notification(&irq_event2, 0, SIGRT1, pthread_self());
	irq_event2.sigev_value.sival_ptr = " irq from event 2";
	assert(err == 0);
	err = event_create(fd, &irq_event2, FPGA1_INT0_T7_INT_FALLING);
	assert(err != 0);

	/* We can however set up additional timers for the same clock. */
	err = sigevent_set_notification(&timer_event4, 0, SIGRT2, pthread_self());
	timer_event4.sigev_value.sival_ptr = gamecp_name(FPGA1_INT0_T7_INT_RISING);
	assert(err == 0);
	err = timer_create(clockid3, &timer_event4, &tid4);
	assert(err == 0);
	itimer.it_interval = timer_period4;
	itimer.it_value = clock_period1;
	err = timer_settime(tid4, 0, &itimer, NULL);
	assert(err == 0);
	err = clock_settime(clockid3, &clock_period3);
	assert(err == 0);

	/* Write a A(ctive) to LED matrix display. */
	*(regs + FPGA1_REGS_LED_MATRIX) = 'A';

	sigemptyset(&set);
	sigaddset(&set, SIGRT0);
	sigaddset(&set, SIGRT1);
	sigaddset(&set, SIGRT2);
	to.tv_sec = 10;
	to.tv_nsec = 0;
	for(i = 0; i < 500; i++) {
		char *code;
		char timerid[10];
		err = sigtimedwait(&set, &info, &to);
		if(info.si_ptr == irq_event1.sigev_value.sival_ptr) {
			/* Trigger an interrupt with the clock's period. */
			write_reg32(regs, FPGA1_REGS_TIMER7_CMP, 
			            read_reg32(regs, FPGA1_REGS_TIMER7) + NSEC_TO_T7(clock_period3.tv_nsec));
		}
		if (err < 0 && errno == EAGAIN) {
			printf("Timeout!\n");
			break;
		}
		assert(info.si_signo == SIGRT0 || info.si_signo == SIGRT1 || info.si_signo == SIGRT2);
		if(info.si_code == SI_TIMER) {
			code = "timer";
			sprintf(timerid, " %d", info.si_timerid);
		}
		else {
			code = "interrupt";
			timerid[0] = 0;
		}
		printf("Got signal %d from event %s triggered by %s%s, %d overruns\n", info.si_signo, (char *) info.si_ptr, code, timerid, info.si_overrun);
	}

	/* Write a D(one) to LED matrix display. */
	*(regs + FPGA1_REGS_LED_MATRIX) = 'D';

	/*************************************************/
	/* Note that proper teardown is optionally here: */
	/* The driver takes care of proper cleanup for   */
	/* both clocks and events when the application   */
	/* terminates, or more precise, when the last    */
	/* file descriptor for that device has been      */
	/* closed.                                       */
	/*************************************************/
	/* Clock 3 teardown */
	clock_period3.tv_sec = 0;
	clock_period3.tv_nsec = 0;
	clock_settime(clockid3, &clock_period3);
	err = timer_delete(tid4);
	err = timer_delete(tid3);
	assert(err == 0);
	err = unregister_clock(fd, clockid3);
	assert(err == 0);

	/* Event 1 teardown */
	err = event_delete(fd, FPGA1_INT0_T7_INT_RISING);
	assert(err == 0);

	/* Clock 2 teardown */
	clock_period2.tv_sec = 0;
	clock_period2.tv_nsec = 0;
	clock_settime(clockid2, &clock_period2);
	err = timer_delete(tid2);
	assert(err == 0);
	err = unregister_clock(fd, clockid2);
	assert(err == 0);

	/* Clock 1 teardown */
	clock_period1.tv_sec = 0;
	clock_period1.tv_nsec = 0;
	clock_settime(clockid1, &clock_period1);
	err = timer_delete(tid1);
	assert(err == 0);
	err = unregister_clock(fd, clockid1);
	assert(err == 0);

	return 0;
}
