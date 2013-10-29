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
 */

/***************************************************************************/
/* This user-supplied file is the hub of each specific GAMECP driver       */
/* incarnation. Besides including the driver's application interface       */
/* (here: fpga1.h), it needs to define a few functions as described below. */
/* But note that, as long as the hardware of new devices that need to be   */
/* configured is not quite* unusual, only the driver's application         */
/* interface file and the PCI ID in this file may need to be adapted,      */
/* while the rest of this file may well be reused unchanged.               */
/***************************************************************************/

/* The device's PCI vendor and device id.            */
#define GAMECP_PCI_VENDOR_ID            PCI_VENDOR_ID_SIEMENS
#define GAMECP_PCI_DEVICE_ID            0x4082
/* The number of the PCI BAR that maps the interrupt controller. */
#define GAMECP_INTERRUPT_CONTROLLER_BAR 0
/* The type of the interrupt controller's registers. */
/* Let's hope we never hit unsane hardware that has  */
/* sizes that differ from one to the next interrupt  */
/* control register.                                 */
typedef unsigned int gamecp_reg_t;
/* We need the mapping of event identifiers to the  */
/* combined address / bit position number defined   */
/* in GAMECP_INTERRUPTS to create the mapping array */
/* in gamecp.h.                                     */
#include "ich2.h"

/* An event is identfied by its register index and its bit position,  */
/* both sharing one integer. The number returned by this function is  */
/* how many of the (lower) bits of that integer are reserved for the  */
/* bit position. */
static inline int gamecp_split(void)
{
	return 0;
}

/* This function knows how to acknowledge an interrupt for a specific */
/* event identifier / reason combination.                             */
static inline void gamecp_ack(struct gamecp_device *gamecp, eventid_t event_reason)
{
}

/* This function knows how to read a snapshot of all the device's      */
/* interrupt source registers and returns true as long as at least one */
/* interrupt source is active. This yields much better performance     */
/* when testing single bits (see next function) compared to doing a    */
/* new register read for every test.                                   */
static inline bool gamecp_store(struct gamecp_device *gamecp)
{
	static bool toggle = 0;
	toggle = !toggle;
	return toggle;
}
/* This function knows how to test whether a interrupt source bit for  */
/* a specific event identifier / reason combination is set by          */
/* evaluating the previously stored snapshot.                          */
static inline bool gamecp_test(struct gamecp_device *gamecp, eventid_t event_reason)
{
	return 1;
}

/* This function knows how to set up an interrupt to fire on the       */
/* reason being encoded in a specific event identifier / reason        */
/* combination.                                                        */
static inline void gamecp_trigger(struct gamecp_device *gamecp, eventid_t event_reason)
{
}

/* The lower 14 bits of the DMA Address Register are not implemented, */
/* so we need an appropriate alignment. */
//#define ALIGNMENT_HACK
/* This function does additional initialization at the end of          */
/* module_init                                                         */
static inline int gamecp_postinit(struct gamecp_device *gamecp)
{
#ifdef ALIGNMENT_HACK
	int i, j, size = 100;
	unsigned long pages[size];
        // We assume that we always get susequent pages, which should be true at least after a reboot.
        // Any ideas how to achieve proper alignment the right way are highly appreciated.
	for(i = 0; i < size; i++) {
		pages[i] = get_zeroed_page(GFP_KERNEL | __GFP_DMA);
		if(!(virt_to_phys((void *) pages[i]) & 0x3fff)) break;
	}
	for(j = 0; j < i; j++) free_page(pages[i]);
	if(i == size) return -ENOMEM;
	else gamecp->user_config = (void *) pages[i];
#else
	gamecp->user_config = (void *) get_zeroed_page(GFP_KERNEL | __GFP_DMA);
#endif
	if(gamecp->user_config) {
                *(uint32_t *) gamecp->user_config = 0x0815;
		iowrite32(virt_to_phys(gamecp->user_config), gamecp->regs + ICH2_DMA_BASE);
		return 0;
	}
	else return -ENOMEM;
}
/* This function does additional cleanup at the start of module_exit   */
static inline void gamecp_preexit(struct gamecp_device *gamecp)
{
	iowrite32(0, gamecp->regs + ICH2_DMA_BASE);
	free_page((long unsigned int) gamecp->user_config);
}
/* Any mappings beyond PCI. */
int gamecp_mmap_extender(struct file *filp, struct vm_area_struct *vma)
{
	struct gamecp_private *gamecp_priv = filp->private_data;
	struct gamecp_device *gamecp = gamecp_priv->device;
	unsigned long size = vma->vm_end - vma->vm_start;
	return remap_pfn_range(vma, vma->vm_start, virt_to_phys(gamecp->user_config) >> PAGE_SHIFT, size, vma->vm_page_prot);
}
