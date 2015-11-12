#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/err.h>
#include <linux/smp.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/kfifo.h>
#include <linux/kallsyms.h>

#include <mach/platform.h>
#include <mach/pm.h>

#ifndef CONFIG_ARCH_S5P6818_REV

#define	IPI_USE_QUEUE
//#define	IPI_DEBUG_COUNTER

#define	R_WFE() 	IO_ADDRESS(0xC00111BC)
#define	R_GIC() 	IO_ADDRESS(0xC0011078)

#define	CPU_RAISE(core)	{ __raw_writel(core, SCR_SMP_WAKE_CPU_ID); }
#define	GIC_RAISE(bits)	{ dsb(); __raw_writel(__raw_readl(R_GIC()) & ~(bits<<8), R_GIC()); }
#define	GIC_CLEAR(bits)	{ __raw_writel(__raw_readl(R_GIC()) | (bits<<8), R_GIC());  dsb(); }
#define	GIC_STATUS()	((__raw_readl(R_GIC()) >> 8) & 0xFE)

#ifdef IPI_USE_QUEUE
struct kqueue {
	int qno;
	int in;
	int out;
	int size;
	int debug;
	unsigned int *data;
};

static struct kqueue ipi_queue[NR_CPUS];
static unsigned int ipi_buffer[NR_CPUS][32];

#define	_inc_queue_pos(p, s)	{ p++; if (p > (s-1)) p = 0; }
#define	_dec_queue_pos(p, s)	{ p--; if (p < 0) p = (s-1); }

static inline void kqueue_init(int qno, struct kqueue *queue, unsigned int *data, unsigned int size)
{
	queue->qno = qno;
	queue->in = 0;
	queue->out = 0;
	queue->size = size;
	queue->data = data;
}

static inline int kqueue_push(struct kqueue *queue, int in, bool overwrite)
{
	int ret = 1;
	_inc_queue_pos(queue->in, queue->size);
	if (queue->in == queue->out) {
		pr_debug("[full qno.%d in:%d, out:%d, overwrite]\n",
			queue->qno, queue->in, queue->out);
		_dec_queue_pos(queue->in, queue->size);
		ret = 0;
		if (false == overwrite)
			return ret;
	}
	queue->data[queue->in] = (unsigned long)in;
	return ret;
}

static inline int kqueue_pop(struct kqueue *queue, int *out)
{
	if (queue->out == queue->in)
		return 0;

	_inc_queue_pos(queue->out, queue->size);
	*out = queue->data[queue->out];
	return 1;
}

static inline int kqueue_is_empty(struct kqueue *queue)
{
	if (queue->in == queue->out)
		return 1;
	return 0;
}
#else	/* IPI_USE_QUEUE */
struct ipi_monitor {
	unsigned long ipi;
};

static struct ipi_monitor _smp_ipi_monitor[NR_CPUS];
struct ipi_monitor *smp_ipi_monitor = _smp_ipi_monitor;
#endif	/* !IPI_USE_QUEUE */

#if (1)
static DEFINE_RAW_SPINLOCK(raise_lock);

/* no wait with wfe */
static inline void __local_spin_lock(arch_spinlock_t *lock)
{
    unsigned long tmp;

    __asm__ __volatile__(
	"1: ldrex   %0, [%1]\n"
	"   teq %0, #0\n"
	"   strexeq %0, %2, [%1]\n"
	"   teqeq   %0, #0\n"
	"   bne 1b"
    	: "=&r" (tmp)
    	: "r" (&lock->lock), "r" (1)
    	: "cc");

   dmb();
}

/* not send sev */
static inline void __local_spin_unlock(arch_spinlock_t *lock)
{
	dmb();

    __asm__ __volatile__(
	"   str %1, [%0]\n"
    	:
    	: "r" (&lock->lock), "r" (0)
    	: "cc");

    /*
     * must for idle
     */
    dsb();
}

#define __spin_raise_lock()		__local_spin_lock(&raise_lock.raw_lock)
#define __spin_raise_unlock()	__local_spin_unlock(&raise_lock.raw_lock)
#else
#define __spin_raise_lock()
#define __spin_raise_unlock()
#endif

/*
 * For debug
 */
#ifdef IPI_DEBUG_COUNTER
unsigned long smp_irq_counter[NR_CPUS] = { 0, };
unsigned long smp_ipi_counter[NR_CPUS] = { 0, };
unsigned long smp_skp_counter[NR_CPUS] = { 0, };
#define	inc_irq_counter(c)		do { smp_irq_counter[c]++; } while (0)
#define	inc_ipi_counter(c)		do { smp_ipi_counter[c]++; } while (0)
#else
#define	inc_irq_counter(c)
#define	inc_ipi_counter(c)
#endif

/*
 * core IPI apis
 */
static inline int is_raise_ipi(int cpu)
{
	return (GIC_STATUS() & (1<<cpu)) ? 0 : 1;
}

int core_IPI_rclear(int cpu)
{
	int ipi = 0;

	__spin_raise_lock();

#ifdef IPI_USE_QUEUE
	kqueue_pop(&ipi_queue[cpu], &ipi);
	inc_irq_counter(cpu);
	GIC_CLEAR(1<<cpu);

	/* next ipi */
	if (!kqueue_is_empty(&ipi_queue[cpu]))
		GIC_RAISE(1<<cpu);
#else
	ipi = smp_ipi_monitor[cpu].ipi;
	smp_ipi_monitor[cpu].ipi = 0;
	inc_irq_counter(cpu);

	GIC_CLEAR(1<<cpu);
#endif

	__spin_raise_unlock();

	return ipi;
}

static inline int core_IPI_stat(int cpu)
{
	int ipi = 0;
#ifndef IPI_USE_QUEUE
	ipi = smp_ipi_monitor[cpu].ipi;
#endif
	return (ipi & 0xFE);
}

void core_IPI_raise(long map, int ipi)
{
	int cpu;

	/* ignore core 0 */
	map &= 0xFE;

	/* set mask to rasie second cores */
	if (1 == ipi) {
		cpu = ffs(map)-1;
		CPU_RAISE(cpu);
		GIC_RAISE(1<<cpu);
		GIC_CLEAR(1<<cpu);
		return;
	}

	__spin_raise_lock();

#ifdef IPI_USE_QUEUE
	for (cpu = 1; NR_CPUS > cpu; cpu++) {
		if (!(map & 1<<cpu))
			continue;
		kqueue_push(&ipi_queue[cpu], ipi, true);
		inc_ipi_counter(cpu);
	}
#else
	for (cpu = 1; NR_CPUS > cpu; cpu++) {
		if (!(map & 1<<cpu))
			continue;
		smp_ipi_monitor[cpu].ipi = ipi;
		inc_ipi_counter(cpu);
	}
#endif

	GIC_RAISE(map);

	__spin_raise_unlock();
}

#ifdef IPI_USE_QUEUE
static __init int core_IPI_init(void)
{
	int i = 0;
	for (; NR_CPUS > i; i++)
		kqueue_init(i, &ipi_queue[i], ipi_buffer[i], ARRAY_SIZE(ipi_buffer[0]));
	return 0;
}
early_initcall(core_IPI_init);
#endif
#endif	/* CONFIG_ARCH_S5P6818_REV */
