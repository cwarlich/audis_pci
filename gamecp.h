/*
 * Generic Audis Memory Event Clock PCI driver core (GAMECP)
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
 * GAMECP driver and bearing this exception notice. If you copy code
 * from other sources into a copy of this driver, the exception does not apply
 * to the code that you add in this way.
 */

/***************************************************************************/
/* This file contains the generic functionality that is needed for an      */
/* AUDIS realtime driver offering the following functionality to user      */
/* space applications:                                                     */
/* - direct (i.e. memory mapped) access to the configured device's PCI     */
/*   memory through the mmap() system call                                 */
/* - forwarding of the configured device's interrupts to dedicated user    */
/*   space signals through the A&D API function event_create()             */
/* - instantiation of user-defined clocks, each being in sync with the     */
/*   related device's interrupt source through the A&D API function        */
/*   register_clock()                                                      */
/* The device specific configuration of the driver needs to be done in     */
/* two device specific files:                                              */
/* 1) A header file that defines the driver's interface to user space.     */
/*    It comprises the configuration for:                                  */
/*    - the name of the driver's device node (needed to open the device)   */
/*    - the clocks and events related to the device's interrupt sources    */
/*      (needed for event_create() and register_clock())                   */
/*    - the device's PCI BARs being mapped (needed for mmap())             */
/*    - the memory layout of these BARs (needed to read and write PCI      */
/*      memory)                                                            */
/*    Please refer to the example (fpga1.h) for further details.           */
/* 2) A C source file that serves as the frame to build the related module */
/*    and that must include the header file defining the user space        */
/*    interface.                                                           */
/*    Furthermore, that C source file needs to define a few items which    */
/*    define device specific things like the device's PCI id and the       */
/*    functionality of the device's interrupt controller.                  */
/*    Please refer to the example (fpga1.c) for further details.           */
/* I strongly suggest to continue reading the comments in the two example  */
/* files now and come back as desired.                                     */
/***************************************************************************/

#ifndef __GAMECP_H
#define __GAMECP_H
/********************************************************************/
/* These defines are needed by both the user space applications and */
/* the driver.                                                      */
/********************************************************************/
/* Proper stringification ... */
#define GAMECP_STRINGIFY_(x) #x
/* ... and concatenation ... */
#define GAMECP_CONCAT_(x, y) x ## y
/* need one indirection. */
#define GAMECP_STRINGIFY(x) GAMECP_STRINGIFY_(x)
#define GAMECP_CONCAT(x, y) GAMECP_CONCAT_(x, y)
/* The driver's device file. */
#define GAMECP_DEVICE "/dev/" GAMECP_STRINGIFY(GAMECP_NAME)
/* The offset into a BAR must not become bigger than that value. */
#define GAMECP_BAR_WINDOW_SIZE 0x20000000UL
/* May be used as the base for offsets being passed to mmap(). */
#define GAMECP_BAR(x) (x * GAMECP_BAR_WINDOW_SIZE)
/* Used to create enums. Look at the explanation above and */
/* gamecp.h for a nice usage example showing why this is useful. */
#define GAMECP_MAKE_EVENT(name) enum GAMECP_CONCAT(GAMECP_NAME,_events) {\
    GAMECP_LIST_GENERATOR(name, GAMECP_EVENT_ITEM)\
}
/* The indirection is needed for proper macro expansion. */
#define GAMECP_LIST_GENERATOR(name, x) name(x)
/* Actually generating the FPGA1_EVENTS enum with member names from the  */
/* lists above, i.e. extending the elements of the first list by */
/* _RISING, _FALLING _BOTH and _NONE. */
GAMECP_MAKE_EVENT(GAMECP_INTERRUPTS);
/********************************************************/
/* The array defined by GAMECP_MAKE_NAME() is just nice */
/* to have for user space applications.                 */
/********************************************************/
/*  To make an array of strings */
#define GAMECP_MAKE_NAME(name) static const char *GAMECP_CONCAT(GAMECP_NAME,_event_names)[] = {\
    GAMECP_LIST_GENERATOR(name, GAMECP_NAME_ITEM)\
}\
/*  To make an array item. */
#define GAMECP_NAME_ITEM(name, value) #name,
/* Calucate the size of an array. */
#define ARRAY_NUMBER(name) (sizeof(name) / sizeof(name[0]))
/* This function returns a suitable name for an event */
/* being passed. The event reason is ignored. */
static inline char *gamecp_name(int event) {
	GAMECP_MAKE_NAME(GAMECP_INTERRUPTS);
	enum GAMECP_INTERRUPT_REASONS {GAMECP_EVENT_ITEM(GAMECP_INTERRUPTS,)}; 
	const int size[] = {GAMECP_EVENT_ITEM(GAMECP_INTERRUPTS,)};
	return (char *) GAMECP_CONCAT(GAMECP_NAME,_event_names)[event / ARRAY_NUMBER(size)];
}
/* These generic macros now did their job and are now */
/* only needed by the driver code.  Thus, we undef them */
/* to minimize namespace pollution in the application. */
/* Note howerver that GAMECP_BAR_WINDOW_SIZE and GAMECP_BAR() */
/* remain defined as they are needed to map the BARs. */
/* But this is no problem as these two macros always remain */
/* unchanged for every driver incarnation. Furthermore, */
/* we don't undef ARRAY_NUMBER(), as it always remains */
/* unchanged and is useful to application code. */
#ifndef __KERNEL__
#undef GAMECP_NAME
#undef GAMECP_INTERRUPTS
#undef GAMECP_EVENT_ITEM
#undef GAMECP_STRINGIFY_
#undef GAMECP_CONCAT_
#undef GAMECP_STRINGIFY
#undef GAMECP_CONCAT
#undef GAMECP_DEVICE
#undef GAMECP_MAKE_EVENT
#undef GAMECP_LIST_GENERATOR
#undef GAMECP_MAKE_NAME
#undef GAMECP_NAME_ITEM
/***********************************************************/
/* The remaining part of the file is driver code that must */
/* (and will) not be included by user space applicatiions. */
/***********************************************************/
#else /* #ifdef __KERNEL__ */
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/version.h>
#include <linux/signal.h> /* due to missing include in rt_driver.h */
#include <linux/aud/rt_driver.h>

MODULE_AUTHOR("Christof Warlich");
MODULE_DESCRIPTION("GAMECP driver");
MODULE_LICENSE("GPL");

/***********************************************************/
/* Example of using the MAKE_ENUM and MAKE_ARRAY macros,   */
/* including the definition and use of a third macro       */
/* (MAKE_NAME), that strinifies the enum member names.     */
/* As can be seen, this technique not only allows to       */
/* iterate through the enumed data, but even allows a      */
/* certain degree of reflection by allowing to print the   */
/* enum names.                                             */
/***********************************************************/
#if 0
#include <stdio.h>
/***********************************************************/
/* This is to be considered generic library code to be put */
/* some global header file                                 */
/***********************************************************/
#define MAKE_ARRAY(name) static const int name[] = {LIST_GENERATOR(name, ARRAY_ITEM)}
/* Make an array item. */
#define ARRAY_ITEM(name, value) value,
/* Calucate the size of the array and thus, the enum */
#define ARRAY_NUMBER(name) (sizeof(name) / sizeof(name[0]))
/* The indirection is needed for proper macro expansion. */
#define LIST_GENERATOR(name, x) name(x)
/* Make an enum */
#define MAKE_ENUM(name) enum name##_PAIR {LIST_GENERATOR(name, ENUM_ITEM)}
/* Make an enum item */
#define ENUM_ITEM(name, value) name = value,
/* Make an array */
#define MAKE_NAME(name) static const char *name##_NAME[] = {\
    LIST_GENERATOR(name, NAME_ITEM)\
}
/* Make an array item. */
#define NAME_ITEM(name, value) #name,
/***********************************************************/
/* The rest should go into a .c file.                      */
/***********************************************************/
/* We define our "enum". */
#define SOME_IDENTIFIER(item)\
	item(SOME_ITEM, 4711)\
	item(ANOTHER_ITEM, 815)\
	item(YET_ANOTHER_ITEM, 7)
/* Then, we actually create it. */
MAKE_ENUM(SOME_IDENTIFIER);
/* We make it iteable ... */
MAKE_ARRAY(SOME_IDENTIFIER);
/* ... and reflexible. Note that our library */
/* code could alternatively have defined a */
/* macro MAKE_ITERABLE_REFLEXIBLE_ENUM(name) */
/* that does these three things in one step. */ 
MAKE_NAME(SOME_IDENTIFIER);
int main(void) {
	int i;
	/* Demonstration of symbolic access: */
	printf("The value of ANOTHER_ITEM is %d.\n", ANOTHER_ITEM);
	/* Demonstration of reflexibility and iteration over the enum: */
	printf("List of the items in the SOME_IDENTIFIER_ENUM enun:\n");
	for(i = 0; i < ARRAY_NUMBER(SOME_IDENTIFIER); i++)
		printf("%s = %d\n", SOME_IDENTIFIER_NAME[i], SOME_IDENTIFIER[i]);
	return 0;
}
#endif

/* To make an array from the values of a list of comma */
/* separated pairs. This is allows to iterate over */
/* the values of an enum generated from the same data. */
#define GAMECP_MAKE_ARRAY(name) static const int name[] = {GAMECP_LIST_GENERATOR(name, GAMECP_ARRAY_ITEM)}
/*  To make an array item. */
#define GAMECP_ARRAY_ITEM(name, value) value,
/* Actually generate the event value array. */
GAMECP_MAKE_ARRAY(GAMECP_INTERRUPTS);

/* These functions must be implemented to match the real hardware. */
struct gamecp_device;
static int gamecp_split(void);
static void gamecp_ack(struct gamecp_device *gamecp, eventid_t event);
static void gamecp_trigger(struct gamecp_device *gamecp, eventid_t event);
static bool gamecp_store(struct gamecp_device *gamecp);
static bool gamecp_test(struct gamecp_device *gamecp, eventid_t event);
static int gamecp_postinit(struct gamecp_device *gamecp);
static void gamecp_preexit(struct gamecp_device *gamecp);

/* Calculate bit position. */
static inline int gamecp_bitposition(int indexAndBit)
{
	return 1UL << ((indexAndBit) & ((1 << gamecp_split()) - 1));
}
/* Calculate address identifier. */
static inline int gamecp_registerid(int indexAndBit)
{
	return (indexAndBit) >> (sizeof(int) * 8 - gamecp_split());
}

/* We ensure that GAMECP_INTERRUPT_REASONS ... */
enum GAMECP_INTERRUPT_REASONS {GAMECP_EVENT_ITEM(GAMECP_INTERRUPTS,)}; 
/* ... and gamecp_numberOfReasons() are in sync with */
/* the definition of GAMECP_EVENT_ITEM. */
inline static size_t gamecp_numberOfReasons(void)
{
	const int size[] = {GAMECP_EVENT_ITEM(GAMECP_INTERRUPTS,)};
	return ARRAY_NUMBER(size);
}

static size_t gamecp_numberOfRegisters(void)
{
	size_t ret = 1;
	int store[ARRAY_NUMBER(GAMECP_INTERRUPTS)], i, j;
	store[0] = gamecp_registerid(GAMECP_INTERRUPTS[0]);
	for(i = 1; i < ARRAY_NUMBER(GAMECP_INTERRUPTS); i++) {
		bool found = false;
		int addressIdentifier = gamecp_registerid(GAMECP_INTERRUPTS[i]);
		for(j = 0; j < ret; j++) {
			if(store[j] == addressIdentifier) {
				found = true;
				break;
			}
		}
		if(!found) {
			store[ret] = addressIdentifier;
			ret++;
		}
	}
	return ret;
}

/*********************************************************/
/* The remaining part of the file just only contains the */
/* standard funcctions that every driver of this type    */
/* must implement.                                       */
/*********************************************************/
/* The central data structures of the driver. */
struct gamecp_device {
	struct pci_dev *pci_dev;
	void __iomem *regs;
	struct miscdevice miscdev;
	struct rt_event ev[ARRAY_NUMBER(GAMECP_INTERRUPTS)];
	bool nonrt_event[ARRAY_NUMBER(GAMECP_INTERRUPTS)];
	rtx_spinlock_t rt_dev_lock;
	int event_handle;
	void (*clock_callback[ARRAY_NUMBER(GAMECP_INTERRUPTS)])(void);
	int clock_id[ARRAY_NUMBER(GAMECP_INTERRUPTS)];
	size_t reason_num;
	size_t reg_num;
	void *src_regs;
	void *user_config;
};
struct gamecp_private {
	struct gamecp_device *device;
};
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,35)
static struct gamecp_device *gamecp_device;
#endif

/* Common interrupt handler for clocks and events. */
irqreturn_t gamecp_irq_handler(int irq, void *devid)
{
	struct gamecp_device *gamecp = devid;
	int i;
	bool nonrt = false;
	rtx_spin_lock(&gamecp->rt_dev_lock);
	/* We need to loop until no interrupts are pending so that a new edge may be generated */
        while(gamecp_store(gamecp)) {
		for(i = 0; i < ARRAY_NUMBER(GAMECP_INTERRUPTS); i++) {
			int indexAndBit = GAMECP_INTERRUPTS[i];
			if(gamecp_test(gamecp, i * gamecp->reason_num)) {
                                bool found = false;
                                /* Both clock ... */
				if(gamecp->clock_callback[i]) {
					gamecp->clock_callback[i]();
                                        found = true;
				}
				/* ... and event may be registered for the same bit, ... */
				if(gamecp->ev[i].ev_rt == EV_RT) {
					if(rt_send_event(&gamecp->ev[i]) == 0) found = true;
                                }
				else {
					gamecp->nonrt_event[i] = true;
					nonrt = true;
					found = true;
				}
                                /* ... but at least one must be there. */
                                if(!found) printk(KERN_WARNING "Neither clock nor RT- or NonRT-event for event %s, register %x, bit %x\n",
						  gamecp_name(i * gamecp_numberOfReasons()),
						  gamecp_registerid(indexAndBit),
						  gamecp_bitposition(indexAndBit));
				/* The interrupting bit must be acknowledged in any case. */
				gamecp_ack(gamecp, i * gamecp->reason_num);
			}
		}
        }
        if(nonrt) execute_nonrt_handler(0, irq);
	rtx_spin_unlock(&gamecp->rt_dev_lock);
	return IRQ_HANDLED;
}
irqreturn_t gamecp_irq_nonrt_handler(int irq, void *devid)
{
	struct gamecp_device *gamecp = devid;
	int i, ret;
	for(i = 0; i < ARRAY_NUMBER(GAMECP_INTERRUPTS); i++) {
		if(gamecp->nonrt_event[i]) {
			ret = rt_send_event(&gamecp->ev[i]);
			if(ret < 0) printk(KERN_WARNING "Failed to send NonRT-event %s, reason: %d\n", gamecp_name(i * gamecp_numberOfReasons()), ret);
			gamecp->nonrt_event[i] = false;
		}
	}
	return IRQ_HANDLED;
}

/* Event registration and deregistration. */
static int gamecp_event_disable(void *arg, struct rt_event *ev)
{
	struct gamecp_device *gamecp = arg;
	gamecp_trigger(gamecp, ev->ev_id * gamecp->reason_num + gamecp->reason_num - 1);
	return 0;
}
static int gamecp_bind_irq_event(struct gamecp_private *gamecp_priv, struct rt_ev_desc __user *user_ev_desc)
{
	struct rt_ev_desc ev_desc;
	struct gamecp_device *gamecp = gamecp_priv->device;
	unsigned long flags;
	int ret = 0;
	enum GAMECP_CONCAT(GAMECP_NAME,_events) ev_id;

	if (rt_copy_from_user(&ev_desc, user_ev_desc, sizeof(ev_desc))) return -EFAULT;

	/* We get the event id, i.e the event including its reason */
	/* (RISING, FALLING, BOTH, NONE) ... */
	ev_id = ev_desc.event;
	/* ... so we must devide by the number of reasons ... */
	ev_desc.event /= gamecp->reason_num;
	if (ev_desc.event >= ARRAY_NUMBER(GAMECP_INTERRUPTS)) return -EINVAL;
	rtx_spin_lock_irqsave(&gamecp->rt_dev_lock, flags);
	/* ... to register the event. */
	ret = rt_register_event(gamecp->event_handle, &ev_desc);
	if(ret) goto err_register_event;

	/* If the event is registered, set mask and trigger registers as encoded in event. */
	/* We cannot do this in an fpga_event_enable() function, because we don't have the */
	/* event reason at this point any more. Note however that if the event is deleted, */
	/* the event _must_ be masked in gamecp_event_disable(). It must be done there because the */
	/* the event deletion may be done by the kernel instead of a call to event_delete() */
	/* being triggered by the user. */
	if(ev_desc.sigevent.sigev_notify != SIGEV_NONE) gamecp_trigger(gamecp, ev_id);

err_register_event:
	rtx_spin_unlock_irqrestore(&gamecp->rt_dev_lock, flags);
	return ret;
}

/* Clock registration and deregistration. */
void clock_cleanup_callback(clocksrcid_t event, struct file *filp) {
	struct gamecp_private *gamecp_priv = filp->private_data;
	struct gamecp_device *gamecp = gamecp_priv->device;
        int r = gamecp->reason_num;
	/* mask corresponding bit */
	gamecp_trigger(gamecp, event / r * r + r - 1);
}
static int gamecp_register_clock(struct gamecp_private *gamecp_priv,
				struct file *filp,
				struct rt_clock_desc __user *user_clock_desc)
{
	struct gamecp_device *gamecp = gamecp_priv->device;
	void (*clock_callback)(void);
	struct rt_clock_desc clock_desc;
	unsigned long flags;
	int ret;
	eventid_t event;

	if (rt_copy_from_user(&clock_desc, user_clock_desc, sizeof(clock_desc))) return -EFAULT;
        event = clock_desc.clock_srcid;
	clock_desc.clock_cleanup_callback = clock_cleanup_callback;
	ret = rt_register_sync_clock(filp, &clock_desc, CLOCK_SYNC_HARD, &clock_callback);
	if (ret < 0) return ret;

	rtx_spin_lock_irqsave(&gamecp->rt_dev_lock, flags);
	gamecp->clock_callback[event / gamecp->reason_num] = clock_callback;
	gamecp->clock_id[event / gamecp->reason_num] = ret;
	gamecp_trigger(gamecp, event);
	rtx_spin_unlock_irqrestore(&gamecp->rt_dev_lock, flags);

	return ret;
}
static int gamecp_unregister_clock(struct gamecp_private *gamecp_priv, struct file *filp, clockid_t clockid)
{
	struct gamecp_device *gamecp = gamecp_priv->device;
	unsigned long flags;
	int ret, i;

	rtx_spin_lock_irqsave(&gamecp->rt_dev_lock, flags);
	for(i = 0; i < ARRAY_NUMBER(GAMECP_INTERRUPTS); i++) {
		if(gamecp->clock_id[i] == clockid) break;
	}
	if(i < ARRAY_NUMBER(GAMECP_INTERRUPTS)) {
		gamecp_trigger(gamecp, i * gamecp->reason_num + gamecp->reason_num - 1);
		gamecp->clock_callback[i] = NULL;
		gamecp->clock_id[i] = 0;
	}
	else printk(KERN_WARNING "%d is not a valid clock id\n", clockid);
	rtx_spin_unlock_irqrestore(&gamecp->rt_dev_lock, flags);

	ret = rt_unregister_sync_clock(filp, clockid);

	return ret;
}

/* Required by libauidis. */
static long gamecp_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct gamecp_private *gamecp_priv = filp->private_data;
	long ret;
	switch (cmd) {
	case AuD_EVENT_CREATE:
		ret = gamecp_bind_irq_event(gamecp_priv, (struct rt_ev_desc __user *)arg);
		break;
	case AuD_REGISTER_CLOCK:
		ret = gamecp_register_clock(gamecp_priv, filp, (struct rt_clock_desc __user *)arg);
		break;
	case AuD_UNREGISTER_CLOCK:
		ret = gamecp_unregister_clock(gamecp_priv, filp, (clockid_t)arg);
		break;
	default:
		ret = -ENOTTY;
		break;
	}
	return ret;
}

extern int gamecp_mmap_extender(struct file *filp, struct vm_area_struct *vma) __attribute__((weak));
/* PCI memory mapping. */
#define GAMECP_BAR_WINDOW_MASK    (GAMECP_BAR_WINDOW_SIZE - 1)
static int gamecp_mmap(struct file *filp, struct vm_area_struct *vma)
{
	struct gamecp_private *gamecp_priv = filp->private_data;
	unsigned long offset, size;
	unsigned int bar_number;
	phys_addr_t addr;

	offset = vma->vm_pgoff << PAGE_SHIFT;
	size = vma->vm_end - vma->vm_start;
	bar_number = offset / GAMECP_BAR_WINDOW_SIZE;

	if (bar_number > (PCI_BASE_ADDRESS_5 - PCI_BASE_ADDRESS_0) / sizeof(int32_t)) {
		if(gamecp_mmap_extender) return gamecp_mmap_extender(filp, vma);
		else return -EINVAL;
	}
	/* do not allow mapping beyond the end of the windows */
	if ((offset & GAMECP_BAR_WINDOW_MASK) + size >
	    pci_resource_len(gamecp_priv->device->pci_dev, bar_number))
		return -EINVAL;

	addr = pci_resource_start(gamecp_priv->device->pci_dev, bar_number);
	addr += offset & GAMECP_BAR_WINDOW_MASK;

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,7,0)
	vma->vm_flags |= VM_IO | VM_RESERVED;
#endif
	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

	return remap_pfn_range(vma, vma->vm_start, addr >> PAGE_SHIFT, size, vma->vm_page_prot);
}

/* To open() and close() the device. */
static int gamecp_open(struct inode *inode, struct file *filp)
{
	struct gamecp_private *gamecp_priv;
	struct gamecp_device *gamecp;
	int err;

	gamecp_priv = kmalloc(sizeof(struct gamecp_private), GFP_KERNEL);
	if (!gamecp_priv) return -ENOMEM;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,35)
	/* The miscdevice layer puts the registered miscdevice structure
	 * in filp->private_data. */
	gamecp_priv->device = container_of(filp->private_data, struct gamecp_device, miscdev);
#else
	gamecp_priv->device = gamecp_device;
#endif
	gamecp = gamecp_priv->device;

	filp->private_data = gamecp_priv;

	if (IS_REALTIME_PROCESS(current)) {
		err = rt_allow_access(filp, RT_IO_IOCTL);
		if (err) {
			kfree(gamecp_priv);
			return err;
		}
	}

	return 0;
}
static int gamecp_release(struct inode *inode, struct file *filp)
{
	struct gamecp_private *gamecp_priv = filp->private_data;

	if (IS_REALTIME_PROCESS(current)) rt_remove_access(filp);

	kfree(gamecp_priv);
	return 0;
}

/* Module load and unload. */
static const struct file_operations gamecp_fops = {
	.owner = THIS_MODULE,
	.open = gamecp_open,
	.release = gamecp_release,
	.unlocked_ioctl = gamecp_ioctl,
	.mmap = gamecp_mmap,
};
static int gamecp_pci_probe(struct pci_dev *dev, const struct pci_device_id *id)
{
	struct gamecp_device *gamecp;
	int i, err = -ENOMEM;

	gamecp = kzalloc(sizeof(*gamecp), GFP_KERNEL);
	if (!gamecp) return err;

	gamecp->reason_num = gamecp_numberOfReasons();
	gamecp->reg_num = gamecp_numberOfRegisters();
	gamecp->src_regs =  kzalloc(sizeof(gamecp_reg_t) * gamecp->reg_num, GFP_KERNEL);
	if (!gamecp->src_regs) goto err_kfree1;

	rtx_spin_lock_init(&gamecp->rt_dev_lock);

	err = pci_enable_device(dev);
	if (err) goto err_kfree2;

	pci_set_master(dev);

	err = pci_request_regions(dev, GAMECP_STRINGIFY(GAMECP_NAME));
	if (err) goto err_dev_disable;

	gamecp->regs = pci_ioremap_bar(dev, GAMECP_INTERRUPT_CONTROLLER_BAR);
	if (!gamecp->regs) goto err_release_regions;

	gamecp->pci_dev = dev;
	pci_set_drvdata(dev, gamecp);

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,35)
	BUG_ON(gamecp_device != NULL);
	gamecp_device = gamecp;
#endif

	gamecp->miscdev.minor = MISC_DYNAMIC_MINOR;
	gamecp->miscdev.name = GAMECP_STRINGIFY(GAMECP_NAME);
	gamecp->miscdev.fops = &gamecp_fops;

	err = misc_register(&gamecp->miscdev);
	if (err) goto err_iounmap;

	for(i = 0; i < ARRAY_NUMBER(GAMECP_INTERRUPTS); i++) {
		gamecp_trigger(gamecp, i * gamecp->reason_num + gamecp->reason_num - 1);
        	gamecp->clock_callback[i] = 0;
		gamecp->ev[i].ev_id = i;
		gamecp->ev[i].ev_disable = gamecp_event_disable;
		gamecp->ev[i].ev_enable = 0;
		gamecp->ev[i].endisable_par = gamecp;
	}
	gamecp->event_handle = rt_init_event_area(gamecp->ev, ARRAY_NUMBER(GAMECP_INTERRUPTS));
	if (gamecp->event_handle < 0) {
		err = gamecp->event_handle;
		goto err_miscunregister;
	}

	/*
	 * Note: MSI / edge-triggering are mandatory as we do not
	 * silence the IRQ sources in gamecp_irq_handler. But it is
	 * also faster.
	 */
	err = pci_enable_msi(gamecp->pci_dev);
	if (err) goto err_destroy_event;

	err = rt_request_irq(gamecp->pci_dev->irq, gamecp_irq_handler, 0, GAMECP_STRINGIFY(GAMECP_NAME), gamecp, gamecp_irq_nonrt_handler);
	if (err) goto err_disable_pci;

	err = gamecp_postinit(gamecp);
	if(err) goto err_free_irq;

	return 0;

err_free_irq:
	rt_free_irq(gamecp->pci_dev->irq, gamecp);
err_disable_pci:
	pci_disable_msi(gamecp->pci_dev);
err_destroy_event:
	rt_destroy_event_area(gamecp->event_handle);
err_miscunregister:
	misc_deregister(&gamecp->miscdev);
err_iounmap:
	pci_iounmap(dev, gamecp->regs);
err_release_regions:
	pci_release_regions(dev);
err_dev_disable:
	pci_clear_master(dev);
err_kfree2:
	kfree(gamecp->src_regs);
err_kfree1:
	kfree(gamecp);
	return err;
}
static void gamecp_pci_remove(struct pci_dev *dev)
{
	struct gamecp_device *gamecp = pci_get_drvdata(dev);
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,35)
	gamecp_device = NULL;
#endif
	gamecp_preexit(gamecp);
	rt_free_irq(gamecp->pci_dev->irq, gamecp);
	pci_disable_msi(gamecp->pci_dev);
	rt_destroy_event_area(gamecp->event_handle);
	misc_deregister(&gamecp->miscdev);
	pci_iounmap(dev, gamecp->regs);
	pci_release_regions(dev);
	pci_disable_device(dev);
	kfree(gamecp->src_regs);
	kfree(gamecp);
}
static DEFINE_PCI_DEVICE_TABLE(gamecp_pci_ids) = {
	{ GAMECP_PCI_VENDOR_ID, GAMECP_PCI_DEVICE_ID, GAMECP_PCI_VENDOR_ID, GAMECP_PCI_DEVICE_ID },
	{ 0 }
};
MODULE_DEVICE_TABLE(pci, gamecp_pci_ids);
static struct pci_driver gamecp_pci_driver = {
	.name = GAMECP_STRINGIFY(GAMECP_NAME),
	.id_table = gamecp_pci_ids,
	.probe = gamecp_pci_probe,
	.remove = gamecp_pci_remove,
};
static int __init gamecp_init(void)
{
	return pci_register_driver(&gamecp_pci_driver);
}
static void __exit gamecp_exit(void)
{
	pci_unregister_driver(&gamecp_pci_driver);
}
module_init(gamecp_init);
module_exit(gamecp_exit);
#endif
#endif
