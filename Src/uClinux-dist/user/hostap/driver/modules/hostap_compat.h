#ifndef HOSTAP_COMPAT_H
#define HOSTAP_COMPAT_H

#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,5,47))
#define NEW_MODULE_CODE
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,4,0))
/* 2.2 compatibility */

#include <linux/skbuff.h>
#include <linux/kcomp.h>
#include <asm/hardirq.h>

#ifndef spin_lock_bh
#define spin_lock_bh(lock) spin_lock_irq(lock)
#define spin_unlock_bh(lock) spin_unlock_irq(lock)
#endif
#ifndef __constant_cpu_to_le16
#define __constant_cpu_to_le16 __cpu_to_le16
#endif

#define PRISM2_NETDEV_EXTRA IFNAMSIZ
#define prism2_set_dev_name(dev, pos) (dev)->name = (char *) (pos)

#define HOSTAP_QUEUE struct tq_struct

/* tq_scheduler was removed in 2.4.0-test12 */
#define PRISM2_SCHEDULE_TASK(q) \
MOD_INC_USE_COUNT; \
queue_task((q), &tq_scheduler);

#define PRISM2_FLUSH_SCHEDULED_TASKS() do { schedule(); schedule(); } while (0)

static inline void HOSTAP_QUEUE_INIT(struct tq_struct *tq,
				     void (*routine)(void *), void *data)
{
	tq->next = NULL;
	tq->sync = 0;
	tq->routine = routine;
	tq->data = data;
}

#define HOSTAP_TASKLET struct tq_struct

#define HOSTAP_TASKLET_SCHEDULE(q) \
do { queue_task((q), &tq_immediate); mark_bh(IMMEDIATE_BH); } while (0)

typedef void (*tasklet_func)(void *);

#define HOSTAP_TASKLET_INIT(q, f, d) \
do { memset((q), 0, sizeof(*(q))); \
(q)->routine = (tasklet_func) (f); \
(q)->data = (void *) (d); } \
while (0)


static inline void dev_kfree_skb_any(struct sk_buff *skb)
{
	if (in_interrupt())
		dev_kfree_skb_irq(skb);
	else
		dev_kfree_skb(skb);
}

static __inline__ void list_del_init(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
	INIT_LIST_HEAD(entry);
}

/* end 2.2 compatibility */

#else /* kernel < 2.4.0 */

/* no extra space needed for 2.4.x net_device */
#define PRISM2_NETDEV_EXTRA 0
#define prism2_set_dev_name(dev, pos) do { } while (0)

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,5,44))

#define HOSTAP_QUEUE struct tq_struct

#define PRISM2_SCHEDULE_TASK(q) \
MOD_INC_USE_COUNT; \
if (schedule_task((q)) == 0) \
	MOD_DEC_USE_COUNT;

#define PRISM2_FLUSH_SCHEDULED_TASKS() flush_scheduled_tasks()

static inline void HOSTAP_QUEUE_INIT(struct tq_struct *tq,
				     void (*routine)(void *), void *data)
{
	INIT_LIST_HEAD(&tq->list);
	tq->sync = 0;
	tq->routine = routine;
	tq->data = data;
}

#else /* kernel < 2.5.44 */

#define HOSTAP_QUEUE struct work_struct

#ifdef NEW_MODULE_CODE
#define PRISM2_SCHEDULE_TASK(q) schedule_work(q);
#else /* NEW_MODULE_CODE */
#define PRISM2_SCHEDULE_TASK(q) \
MOD_INC_USE_COUNT; \
if (schedule_work((q)) == 0) \
	MOD_DEC_USE_COUNT;
#endif /* NEW_MODULE_CODE */

#define PRISM2_FLUSH_SCHEDULED_TASKS() flush_scheduled_work()

static inline void HOSTAP_QUEUE_INIT(struct work_struct *wq,
				     void (*routine)(void *), void *data)
{
	INIT_WORK(wq, routine, data);
}

#endif /* kernel < 2.5.44 */

#define HOSTAP_TASKLET struct tasklet_struct

#define HOSTAP_TASKLET_SCHEDULE(q) tasklet_schedule((q))

#define HOSTAP_TASKLET_INIT(q, f, d) \
do { memset((q), 0, sizeof(*(q))); (q)->func = (f); (q)->data = (d); } \
while (0)

#endif /* kernel < 2.4.0 */


/* Interrupt handler backwards compatibility for Linux < 2.5.69 */
#ifndef IRQ_NONE
#define IRQ_NONE
#define IRQ_HANDLED
#define IRQ_RETVAL(x)
typedef void irqreturn_t;
#endif


#if WIRELESS_EXT > 12
#if IW_HANDLER_VERSION < 3
extern void wireless_send_event(struct net_device *dev,
                                unsigned int cmd,
                                union iwreq_data *wrqu,
                                char *extra);
#endif /* IW_HANDLER_VERSION < 3 */
#endif /* WIRELESS_EXT > 12 */


#endif /* HOSTAP_COMPAT_H */
