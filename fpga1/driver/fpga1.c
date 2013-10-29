/*
 * CPU555 FPGA1 driver
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
#define GAMECP_PCI_DEVICE_ID            0x407b
/* The number of the PCI BAR that maps the interrupt controller. */
#define GAMECP_INTERRUPT_CONTROLLER_BAR 4
/* The type of the interrupt controller's registers. */
/* Let's hope we never hit unsane hardware that has  */
/* sizes that differ from one to the next interrupt  */
/* control register.                                 */
typedef unsigned int gamecp_reg_t;
/* We need the mapping of event identifiers to the  */
/* combined address / bit position number defined   */
/* in GAMECP_INTERRUPTS to create the mapping array */
/* in gamecp.h.                                     */
#include "fpga1.h"

/* An event is identfied by its register index and its bit position,  */
/* both sharing one integer. The number returned by this function is  */
/* how many of the (lower) bits of that integer are reserved for the  */
/* bit position. */
static inline int gamecp_split(void) {return 8;}

/* This function knows how to acknowledge an interrupt for a specific */
/* event identifier / reason combination.                             */
#define FPGA1_REGS_INT0_SRC             0x0000  /* INT_SOURCE1 */
static inline void gamecp_ack(struct gamecp_device *gamecp, eventid_t event_reason)
{
	/* Calculate the value being mapped to the event identifier.   */
	int indexAndBit = GAMECP_INTERRUPTS[event_reason / gamecp->reason_num];
	/* Get the register index part of that value.                  */
	/* Note that we shamelessly defined the register index in the  */
	/* event identifier table so that we find the interrupt source */
	/* register offsets by a simple addition of the register index */
	/* to the first interrupt source register.                     */
	int offset = gamecp_registerid(indexAndBit) + FPGA1_REGS_INT0_SRC;
	/* Check if the event is within the allowed range.             */
	BUG_ON(event_reason >= ARRAY_NUMBER(GAMECP_INTERRUPTS) * gamecp->reason_num);
	/* For our interrupt controller, the interrupt is acknowledged */
	/* by writing a 1 to its source register's bit position.       */
	iowrite32(gamecp_bitposition(indexAndBit), gamecp->regs + offset);
}

/* This function knows how to read a snapshot of all the device's      */
/* interrupt source registers and returns true as long as at least one */
/* interrupt source is active. This yields much better performance     */
/* when testing single bits (see next function) compared to doing a    */
/* new register read for every test.                                   */
static inline bool gamecp_store(struct gamecp_device *gamecp)
{
	/* The generic part of the driver magically took care to       */
	/* allocate sufficient space for all interrupt source          */
	/* registers ...                                               */
	gamecp_reg_t *regs = (gamecp_reg_t *) gamecp->src_regs;
	int i;
	bool ret = false;
	/* ... as it knows how many source registers are there.        */
        /* So we iterate over all these registers ...                  */
	for(i = 0; i < gamecp->reg_num; i++) {
		/* ... reading one after the other ...                 */
		regs[i] = ioread32(gamecp->regs + i * sizeof(gamecp_reg_t) + FPGA1_REGS_INT0_SRC);
		/* ... and cumulating their values in the function's   */
		/* return value.                                       */
		ret |= regs[i];
	}
	return ret;
}
/* This function knows how to test whether a interrupt source bit for  */
/* a specific event identifier / reason combination is set by          */
/* evaluating the previously stored snapshot.                          */
static inline bool gamecp_test(struct gamecp_device *gamecp, eventid_t event_reason)
{
	/* Assuming a snapshot of the interrupt source registers has   */
	/* already been stored.                                        */
	gamecp_reg_t *regs = (gamecp_reg_t *) gamecp->src_regs;
	/* Calculate the value being mapped to the event identifier.   */
	int indexAndBit = GAMECP_INTERRUPTS[event_reason / gamecp->reason_num];
	/* Check if the event is within the allowed range.             */
	BUG_ON(event_reason >= ARRAY_NUMBER(GAMECP_INTERRUPTS) * gamecp->reason_num);
	/* A 1 in the related address / bit position indicates an      */
	/* an active interrupt request.                                */ 
	return regs[gamecp_registerid(indexAndBit) / sizeof(gamecp_reg_t)] & gamecp_bitposition(indexAndBit);
}

#define SET |
#define RESET & ~
#define SET_BIT(operation, type) {\
	int offset = gamecp_registerid(indexAndBit) + type;\
	gamecp_reg_t tmp = ioread32(gamecp->regs + offset) operation gamecp_bitposition(indexAndBit);\
	iowrite32(tmp, gamecp->regs + offset);\
}
#define FPGA1_REGS_INT0_MASK            0x0020  /* INT_MASK1 */
#define FPGA1_REGS_INT0_TRIGGER_01      0x0030  /* INT_TRIGGER_MODE11 (rise) */
#define FPGA1_REGS_INT0_TRIGGER_10      0x0040  /* INT_TRIGGER_MODE12 (fall) */
#define SET_BITS(trigger01, trigger10, mask)\
	SET_BIT(trigger01, FPGA1_REGS_INT0_TRIGGER_01);\
	SET_BIT(trigger10, FPGA1_REGS_INT0_TRIGGER_10);\
	SET_BIT(mask, FPGA1_REGS_INT0_MASK);\
	break
/* This function knows how to set up an interrupt to fire on the       */
/* reason being encoded in a specific event identifier / reason        */
/* combination.                                                        */
static inline void gamecp_trigger(struct gamecp_device *gamecp, eventid_t event_reason)
{
	/* Calculate the value being mapped to the event identifier.   */
	int indexAndBit = GAMECP_INTERRUPTS[event_reason / gamecp->reason_num];
	/* Calculate the reason. */
	int reason = event_reason % gamecp->reason_num;
	/* Check if the event is within the allowed range.             */
	BUG_ON(event_reason >= ARRAY_NUMBER(GAMECP_INTERRUPTS) * gamecp->reason_num);
	switch(reason) {
		case GAMECP_INTERRUPTS_RISING: SET_BITS(SET, RESET, SET);
		case GAMECP_INTERRUPTS_FALLING: SET_BITS(SET, RESET, SET);
		case GAMECP_INTERRUPTS_BOTH: SET_BITS(RESET, SET, SET);
		case GAMECP_INTERRUPTS_NONE: SET_BITS(RESET, RESET, RESET);
		default: BUG_ON(true);
	}
	/* Cleaning up related pending interrupts to start in a clean  */
	/* state. */
	gamecp_ack(gamecp, event_reason);
}

/* This function does additional initialization at the end of          */
/* module_init                                                         */
static inline int gamecp_postinit(struct gamecp_device *gamecp)
{
	/* Set the LED matrix display to show the character *.         */
	iowrite8('+', gamecp->regs + FPGA1_REGS_LED_MATRIX);
	return 0;
}
/* This function does additional cleanup at the start of module_exit   */
static inline void gamecp_preexit(struct gamecp_device *gamecp)
{
	/* Switch the LED matrix display off.                          */
	iowrite8(0, gamecp->regs + FPGA1_REGS_LED_MATRIX);
}
