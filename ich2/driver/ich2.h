/*
 * CPU555 ICH2 driver
 *
 * Copyright (c) Siemens AG, 2012, 2013
 *
 * Authors:
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

#ifndef __ICH2_H
#define __ICH2_H
/**************************************************************************/
/*  This is just the name of the device in the /dev directory.            */
/**************************************************************************/
#define GAMECP_NAME                     ich2
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
#define GAMECP_INTERRUPTS(x) x(ICH2_TDM0,               0x0)
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
#define GAMECP_EVENT_ITEM(name, value) name##_ENABLE, name##_DISABLE,

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
#define ICH2_OFFSET_REGISTERS           GAMECP_BAR(0)
#define ICH2_OFFSET_DMA                 GAMECP_BAR(6)
/* Sizes of each of the BARs address space. */
#define ICH2_1KB                        1024
#define ICH2_1MB                        (ICH2_1KB * 1024)
#define ICH2_REGISTER_SIZE              (8 * ICH2_1KB)
#define ICH2_SIZE_DMA                   (4 * ICH2_1KB)
/* The device's register layout. */
#define ICH2_TEST_BASE                  0x08
#define ICH2_TDM_BASE                   0x0c
#define ICH2_IRQ_BASE                   0x10
#define ICH2_DMA_BASE                   0x14
/**************************************************************************/
/* We need to include a small part of the generic driver's core header    */
/* file, contributing a few generic defines and finally expanding the     */
/* GAMECP_INTERRUPTS() event table into an enum.                          */
/**************************************************************************/
#include "gamecp.h"
#endif /* ! __ICH2_H */
