/*
 * CPU555 FPGA1 driver
 *
 * Copyright (c) Siemens AG, 2012, 2013
 *
 * Authors:
 *     Jan Kiszka <jan.kiszka@siemens.com>
 *     Christof Warlich <christof.warlich@siemens.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301, USA.
 *
 * As a special exception to the GNU General Public license, Siemens
 * allows you to use this header file in unmodified form to produce
 * application programs executing in user-space which use this driver by
 * normal system calls. The resulting executable will not be covered by the
 * GNU General Public License merely as a result of this header file use.
 * Instead, this header file use will be considered normal use of this driver
 * and not a "derived work" in the sense of the GNU General Public License.
 *
 * This exception does not apply when the application code is built as a
 * static or dynamically loadable portion of the Linux kernel nor does the
 * exception override other reasons justifying application of the GNU General
 * Public License.
 *
 * This exception applies only to the code released by Siemens as part of this
 * CPU555 FGPA1 driver and bearing this exception notice. If you copy code
 * from other sources into a copy of this driver, the exception does not apply
 * to the code that you add in this way.
 */

#ifndef __FGPA1_H
#define __FGPA1_H
/**************************************************************************/
/*  This is just the name of the device in the /dev directory.            */
/**************************************************************************/
#define GAMECP_NAME                     fpga1
/**************************************************************************/
/* The list below defines the events or clocks that may be registered     */
/* by the application. The macro hackery being used here is necessary     */
/* because that list is beeing seen from different perspectives from      */
/* both the user space application and the driver:                        */
/* 1) From the user space application's point of view, we just need       */
/*    a list of unique identifiers for each event or clock being          */
/*    offered by the device. These identifiers may then be passed to      */
/*    either event_create() or register_clock(). The perfect C            */
/*    structure for such a list is a simple enum.                         */
/* 2) From the driver's viewpoint, we need to set up or delete the        */
/*    requested event or clock by using the identifier being passed by    */
/*    the application. The driver needs to check whether the identifier   */
/*    is valid, and if so, it needs to set up the appropriate register    */
/*    bits and call the related realtime functions to set up or release   */
/*    the related event or clock. A device-specific driver could          */
/*    simply clutter its code with switch() case: statements to map       */
/*    the identifiers to the appropriate actions wherever required,       */
/*    thus hardcoding the available events or clocks into the driver's    */
/*    code. But to be generic, the driver needs an array that maps the    */
/*    identifiers that it gets from the application to the actions that   */
/*    it needs to perform.                                                */
/* The bottom line from this lengthy consideration is that we need to     */
/* generate both an enum _and_ an array from the same set of data. The    */
/* macro hackery allows exactly this, generating the enum                 */
/* through GAMECP_MAKE_EVENT(GAMECP_INTERRUPTS) and generating            */
/* the array through GAMECP_MAKE_VALUE(GAMECP_INTERRUPTS).                */
/* If you really want to understand how the related macros work, you may  */
/* want to have a look at the minimal but complete example in gamecp.h    */
/* (being surrounded by #if 0 ... #endif).                                */
/*                                                                        */
/* But how does the mapping of the event or clock identifier to the       */
/* action that is to be perform actually work? In its essence, an event   */
/* or clock must be mapped to a specific interrupt, i.e. to something     */
/* that lives in a certain bit position at certain register addresses.    */
/* Both of these two entities are defined in the second row of below's    */
/* table: The lower part of the numbers being found there are the bit     */
/* positions of the related interrupts, while the upper part serves as    */
/* an index that allows to calculate the related register addresses. The  */
/* exact split is defined by the value returned by the gamecp_split()     */
/* function in fpga1.c, see there for a more detailed description.        */
/**************************************************************************/
#define GAMECP_INTERRUPTS(x)\
    /* Bits for INT0. */\
    x(FPGA1_INT0_T7_INT,               0x01e)\
    x(FPGA1_INT0_IRQ7I_N,              0x01d)\
    x(FPGA1_INT0_IRQ6I_N,              0x01c)\
    x(FPGA1_INT0_IRQ4I_N,              0x01b)\
    x(FPGA1_INT0_IRQ3I_N,              0x01a)\
    x(FPGA1_INT0_IRQ1I_N,              0x019)\
    x(FPGA1_INT0_SSU_DCU_HW_NP_1,      0x018)\
    x(FPGA1_INT0_RTC_INT,              0x017)\
    x(FPGA1_INT0_IRQ2I_N,              0x016)\
    x(FPGA1_INT0_CP50M1_IN3_N,         0x015)\
    x(FPGA1_INT0_CP50M1_IN2_N,         0x014)\
    x(FPGA1_INT0_CP50M1_IN1_N,         0x013)\
    x(FPGA1_INT0_TIMER5_IRQ,           0x012)\
    x(FPGA1_INT0_TIMER4_IRQ,           0x011)\
    x(FPGA1_INT0_TIMER0_IRQ,           0x010)\
    x(FPGA1_INT0_T0_IN,                0x00d)\
    x(FPGA1_INT0_L4_IN,                0x00c)\
    x(FPGA1_INT0_L3_IN,                0x00b)\
    x(FPGA1_INT0_L2_IN,                0x00a)\
    x(FPGA1_INT0_L1_IN,                0x009)\
    x(FPGA1_INT0_L0_IN,                0x008)\
    x(FPGA1_INT0_PNIO_RES,             0x004)\
    x(FPGA1_INT0_PNIO_RT,              0x003)\
    x(FPGA1_INT0_PNIO_IRT,             0x002)\
    x(FPGA1_INT0_PNIO_SND_OUT,         0x001)\
    x(FPGA1_INT0_VME_PNIO_SND_OUT,     0x000)\
    /* Bits for INT1. */\
    x(FPGA1_INT1_PCIX2_INTA_N,         0x40b)\
    x(FPGA1_INT1_PCIX2_INTB_N,         0x40a)\
    x(FPGA1_INT1_PCIX2_INTC_N,         0x409)\
    x(FPGA1_INT1_PCIX2_INTD_N,         0x408)\
    x(FPGA1_INT1_PCIX1_INTA_N,         0x407)\
    x(FPGA1_INT1_PCIX1_INTB_N,         0x406)\
    x(FPGA1_INT1_PCIX1_INTC_N,         0x405)\
    x(FPGA1_INT1_PCIX1_INTD_N,         0x404)\
    x(FPGA1_INT1_FPGA_RTC_INT,         0x403)\
    x(FPGA1_INT1_SSU_DCU_HW_NP_2,      0x402)\
    x(FPGA1_INT1_PCI_MB3_N,            0x401)\
    x(FPGA1_INT1_PCI_MB2_N,            0x400)\
    /* Bits for INT3. */\
    x(FPGA1_INT3_IRQ5I_N,              0x805)\
    x(FPGA1_INT3_ACFAILI_N,            0x804)\
    x(FPGA1_INT3_BCLRI_N,              0x803)\
    x(FPGA1_INT3_SSU_DCU_HW_HP_2,      0x802)\
    x(FPGA1_INT3_PCI_MB1_N,            0x801)\
    x(FPGA1_INT3_PCI_MB0_N,            0x800)\
    /* Bits for INT4 */\
    x(FPGA1_INT4_SSU_DCU_HW_HP_1,      0xC03)\
    x(FPGA1_INT4_VME_OWN_T0_N,         0xC02)\
    x(FPGA1_INT4_VME_ABR_T0_N,         0xC01)\
    x(FPGA1_INT4_T0_WATCHDOG,          0xC00)
/**************************************************************************/
/* We still havn't covered the complete story w.r.t. event or clock       */
/* identifiers yet: Until now, the application can request a specific     */
/* interrupt to serve as either an event or clock source, but there is    */
/* no way to tell when the event or clock should be triggered, e.g. on a  */
/* rising or falling edge or as long as a certain level is applied. The   */
/* macro below allows just that: For every event or clock identifier      */
/* from the list above, it adds a trigger reason as defined below. Thus,  */
/* the elements of the enum that is finally generated by                  */
/* GAMECP_MAKE_EVENT(GAMECP_INTERRUPTS) are the concatenation of the      */
/* event or clock identifiers from the list above and the reason names    */
/* behind the ## from the list below.                                     */
/* Note that reasons may be added or removed according to the interrupt   */
/* controller's capabilities, e.g. name##_HIGH and name##LOW for level    */
/* triggered interrupts.                                                  */
/**************************************************************************/
#define GAMECP_EVENT_ITEM(name, value)\
	name##_RISING, name##_FALLING, name##_BOTH, name##_NONE,

/**************************************************************************/
/* The following definitions are just convinience macros for the          */
/* application. Hence, they do not affect the driver's operation          */
/* and are not used by the driver in any way.                             */
/* Thus one may add, remove or change these items as desired.             */
/**************************************************************************/
/* Offsets to the device's BARs suitable for mmap(). */
/* The GAMECP_BAR() macro is provided for this purpose, */
/* easily allowing to calculate the right offset that */
/* is to be passed to mmap() to map a specific BAR. */
#define FPGA1_OFFSET_BUFFERED_SRAM      GAMECP_BAR(0)
#define FPGA1_OFFSET_SOC1_RAM           GAMECP_BAR(2)
#define FPGA1_OFFSET_INTERNAL_SRAM      GAMECP_BAR(3)
#define FPGA1_OFFSET_REGISTERS          GAMECP_BAR(4)
/* Sizes of each of the BARs address space. */
#define FPGA1_1KB                       1024
#define FPGA1_1MB                       (FPGA1_1KB * 1024)
#define FPGA1_BUFFERED_SRAM_SIZE        ( 1 * FPGA1_1MB)
#define FPGA1_SOC1_RAM_SIZE             (16 * FPGA1_1MB)
#define FPGA1_INTERNAL_SRAM_SIZE        (64 * FPGA1_1KB)
#define FPGA1_REGISTERS_SIZE            (32 * FPGA1_1KB)
/* The device's register layout. */
#define FPGA1_REGS_TIMER1               0x1010  /* SubSecond  */
#define FPGA1_REGS_TIMER2               0x1000  /* T0 base cycle */
#define FPGA1_REGS_TIMER3               0x1004  /* T0 delay */
#define FPGA1_REGS_TIMER4               0x1008  /* C1 Alarm  */
#define FPGA1_REGS_TIMER5               0x100C  /* C2 Alarm  */
#define FPGA1_REGS_TIMER6               0x1014  /* T0 latency (load) */
#define FPGA1_REGS_TIMER7               0x1018  /* CPU Timer */
#define FPGA1_REGS_TIMER7_CMP           0x101C
#define FPGA1_REGS_BGR_CODE             0x2000  /* board id code, read only */
                                                /* 0x73 since version 0x223 */
#define FPGA1_REGS_FPGA_VERS            0x2004  /* fpga1 version, read only */
#define FPGA1_REGS_LED_MATRIX           0x3000
#define FPGA1_REGS_STATUS               0x4004

#define FPGA1_REGS_CONTROL0             0x4000
#define FPGA1_REGS_CONTROL0_SET         0x4080
#define FPGA1_REGS_CONTROL0_RESET       0x40C0

#define FPGA1_REGS_CONTROL1             0x4010
#define FPGA1_REGS_CONTROL1_SET         0x4090
#define FPGA1_REGS_CONTROL1_RESET       0x40D0

#define FPGA1_REGS_TIMER_CTR            0x400C
#define FPGA1_REGS_TIMER_CTR_SET        0x408C
#define FPGA1_REGS_TIMER_CTR_RESET      0x40CC

#define FPGA1_REGS_SOFT_INT_T0          0x4018
/**************************************************************************/
/* We need to include a small part of the generic driver's core header    */
/* file, contributing a few generic defines and finally expanding the     */
/* GAMECP_INTERRUPTS() event table into an enum.                          */
/**************************************************************************/
#include "gamecp.h"
#endif /* ! __FPGA1_H */
