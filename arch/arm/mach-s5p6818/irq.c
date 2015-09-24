/*
 * (C) Copyright 2009
 * jung hyun kim, Nexell Co, <jhkim@nexell.co.kr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/export.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/system.h>
#include <asm/hardware/gic.h>
#include <asm/mach/irq.h>
#include <mach/platform.h>
#include <mach/gpio.h>

/*
#define pr_debug	printk
*/

#define	INTC_BASE		(void __iomem *)IO_ADDRESS(PHY_BASEADDR_INTC)
#define	GIC_PHY_OFFSET	(0)

//----------------------------------------------------------------------------
static void __init __gic_init(void __iomem *dist_base, void __iomem *cpu_base);
static void __init gpio_init(void __iomem *base, unsigned int irq_start,
							u32 irq_sources, u32 resume_sources);
static void __init alive_init(void __iomem *base, unsigned int irq_start,
							u32 irq_sources, u32 resume_sources);

/*----------------------------------------------------------------------------
 *  cpu irq handler
 */
#define GIC_DIST_BASE		(void __iomem *)(INTC_BASE + 0x00001000)		// 0xC0009000
#define GIC_CPUI_BASE		(void __iomem *)(INTC_BASE + 0x00002000)		// 0xC000a000

#define GPIO_INT_BASE		(void __iomem *)IO_ADDRESS(PHY_BASEADDR_GPIOA)
#define GPIO_BASE_OFFSET	(0x1000)
#define GPIO_INT_MASK		(0xFFFFFFFF)

#define ALIVE_INT_BASE		(void __iomem *)IO_ADDRESS(PHY_BASEADDR_CLKPWR_MODULE + 0x800)
#define ALIVE_INT_MASK		(0x000000FF)

/*
 *  cpu irq handler
 */
void __init nxp_cpu_irq_init(void)
{
	pr_debug("%s:%d\n", __func__, __LINE__);

	__gic_init(GIC_DIST_BASE, (void __iomem *)GIC_CPUI_BASE);
	gpio_init(GPIO_INT_BASE , IRQ_GPIO_START, GPIO_INT_MASK, 0);	/* 64 ~ 223 (A,B,C,D,E) */
	alive_init(ALIVE_INT_BASE, IRQ_ALIVE_START, ALIVE_INT_MASK, 0); /* 224 ~ 231 */

#ifdef CONFIG_FIQ
	init_FIQ();
#endif

	/* wake up source from idle */
	irq_set_irq_wake(IRQ_PHY_CLKPWR_ALIVEIRQ + GIC_PHY_OFFSET, 1);
#if PM_RTC_WAKE
	irq_set_irq_wake(IRQ_PHY_CLKPWR_RTCIRQ + GIC_PHY_OFFSET, 1);
#endif
}

static void __init __gic_init(void __iomem *dist_base, void __iomem *cpu_base)
{
	int irq = IRQ_GIC_PPI_VIC;

	printk(KERN_INFO "GIC   @%p: start %3d (gic %d)\n",
		dist_base, IRQ_GIC_START, (irq-IRQ_GIC_START));

	gic_init(0, IRQ_GIC_PPI_START, dist_base, cpu_base);
}

/*----------------------------------------------------------------------------
 *  ALIVE irq chain handler
 *  start  -> request_irq -> alive irq_unmask
 *  do IRQ -> alive handler -> alive irq_mask -> alive irq_ack -> driver handler -> alive irq_unmask ->
 *  end    -> disable
 ----------------------------------------------------------------------------*/
#define	ALIVE_MOD_REST		(0x04)	// detect mode reset
#define	ALIVE_MOD_SET		(0x08)	// detect mode
#define	ALIVE_MOD_READ		(0x0C)	// detect mode read

#define	ALIVE_DET_RESET		(0x4C)
#define	ALIVE_DET_SET		(0x50)
#define	ALIVE_DET_READ		(0x54)

#define	ALIVE_INT_RESET		(0x58)	// interrupt reset 	: disable
#define	ALIVE_INT_SET		(0x5C)	// interrupt set	: enable
#define	ALIVE_INT_SET_READ	(0x60)	// interrupt set read
#define	ALIVE_INT_STATUS	(0x64)	// interrupt detect pending and clear

#define	ALIVE_OUT_RESET		(0x74)
#define	ALIVE_OUT_SET		(0x78)
#define	ALIVE_OUT_READ		(0x7C)

static void alive_ack_irq(struct irq_data *d)
{
	void __iomem *base = irq_data_get_irq_chip_data(d);
	int bit = (d->irq - IRQ_ALIVE_START) & 0x1F;
	pr_debug("%s: alive irq = %d, io = %d\n", __func__, d->irq, bit);

	/* alive ack : irq pend clear */
	writel((1<<bit), base + ALIVE_INT_STATUS);
	readl(base + ALIVE_INT_STATUS);
}

static void alive_mask_irq(struct irq_data *d)
{
	void __iomem *base = irq_data_get_irq_chip_data(d);
	int bit = (d->irq - IRQ_ALIVE_START) & 0x1F;
	pr_debug("%s: alive irq = %d, io = %d\n", __func__, d->irq, bit);

	/* alive mask : irq reset (disable) */
	writel((1<<bit), base + ALIVE_INT_RESET);
}

static void alive_unmask_irq(struct irq_data *d)
{
	void __iomem *base = irq_data_get_irq_chip_data(d);
	int bit = (d->irq - IRQ_ALIVE_START) & 0x1F;
	pr_debug("%s: alive irq = %d, io = %d\n", __func__, d->irq, bit);

	/* alive unmask : irq set (enable) */
	writel((1<<bit), base + ALIVE_INT_SET);
	readl(base + ALIVE_INT_SET_READ);
}

static int alive_set_type_irq(struct irq_data *d, unsigned int type)
{
	void __iomem *base = irq_data_get_irq_chip_data(d);
	u32 reg = 0;
	int bit = (d->irq - IRQ_ALIVE_START) & 0x1F;
	int offs = 0, i = 0;
	NX_ALIVE_DETECTMODE mode = 0;

	pr_debug("%s: alive irq = %d, io = %d, type=0x%x\n",
		__func__, d->irq, bit, type);

	switch (type) {
	case IRQ_TYPE_NONE:	printk(KERN_WARNING "%s: No edge setting!\n", __func__);
		break;
	case IRQ_TYPE_EDGE_FALLING:	mode = NX_ALIVE_DETECTMODE_SYNC_FALLINGEDGE; break;
	case IRQ_TYPE_EDGE_RISING:	mode = NX_ALIVE_DETECTMODE_SYNC_RISINGEDGE;	break;
	case IRQ_TYPE_EDGE_BOTH:	mode = NX_ALIVE_DETECTMODE_SYNC_FALLINGEDGE; break;	/* and Rising Edge */
	case IRQ_TYPE_LEVEL_LOW:	mode = NX_ALIVE_DETECTMODE_ASYNC_LOWLEVEL; break;
	case IRQ_TYPE_LEVEL_HIGH:	mode = NX_ALIVE_DETECTMODE_ASYNC_HIGHLEVEL; break;
	default:
		printk(KERN_ERR "%s: No such irq type %d", __func__, type);
		return -1;
	}

	for ( ; 6 > i; i++, offs += 0x0C) {
		reg = (i == mode ? ALIVE_MOD_SET : ALIVE_MOD_REST);
		writel(1<<bit, (base + reg  + offs));	/* set o reset mode */
	}

	/*
	 * set risingedge mode for both edge
	 * 0x2C : Risingedge
	 */
	if (IRQ_TYPE_EDGE_BOTH == type)
		writel(1<<bit, (base + 0x2C));

	writel(1<<bit, base + ALIVE_DET_SET);
	writel(1<<bit, base + ALIVE_INT_SET);
	writel(1<<bit, base + ALIVE_OUT_RESET);

	return 0;
}

static int alive_set_wake(struct irq_data *d, unsigned int on)
{
#if (0)
	void __iomem *base = irq_data_get_irq_chip_data(d);
	int bit = (d->irq - IRQ_ALIVE_START) & 0x1F;

	pr_info("%s: alive irq = %d, io = %d wake %s\n",
		__func__, d->irq, bit, on?"on":"off");
#endif
	return 0;
}

static void alive_irq_enable(struct irq_data *d)
{
	void __iomem *base = irq_data_get_irq_chip_data(d);
	int bit = (d->irq - IRQ_ALIVE_START) & 0x1F;
	pr_debug("%s: alive irq = %d, io = %d\n", __func__, d->irq, bit);

	/* alive unmask : irq set (enable) */
	writel((1<<bit), base + ALIVE_INT_SET);
	readl(base + ALIVE_INT_SET_READ);
}

static void alive_irq_disable(struct irq_data *d)
{
	void __iomem *base = irq_data_get_irq_chip_data(d);
	int bit = (d->irq - IRQ_ALIVE_START) & 0x1F;
	pr_debug("%s: alive irq = %d, io = %d\n", __func__, d->irq, bit);

	/* alive mask : irq reset (disable) */
	writel((1<<bit), base + ALIVE_INT_RESET);
}

static struct irq_chip alive_chip = {
	.name			= "ALIVE",
	.irq_ack		= alive_ack_irq,
	.irq_mask		= alive_mask_irq,
	.irq_unmask		= alive_unmask_irq,
	.irq_set_type	= alive_set_type_irq,
	.irq_set_wake	= alive_set_wake,
	.irq_enable		= alive_irq_enable,
	.irq_disable	= alive_irq_disable,
};

static void alive_handler(unsigned int irq, struct irq_desc *desc)
{
	void __iomem *base = irq_desc_get_handler_data(desc);
	u32 stat, mask;
	int phy, bit;

	mask = readl(base + ALIVE_INT_SET_READ);
	stat = readl(base + ALIVE_INT_STATUS) & mask;
	bit  = ffs(stat) - 1;
	phy  = irq;

	pr_debug("%s: alive irq=%d [io=%d], stat=0x%02x, mask=0x%02x\n",
		__func__, phy, bit, stat, mask);

	if (-1 == bit) {
		printk(KERN_ERR "Unknown alive irq=%d, stat=0x%08x, mask=0x%02x\r\n",
			phy, stat, mask);
		writel(-1, (base + ALIVE_INT_STATUS));	/* clear alive status all */
		writel_relaxed(phy, GIC_CPUI_BASE + GIC_CPU_EOI);
		return;
	}

	/* alive descriptor */
	irq  = IRQ_ALIVE_START + bit;
	desc = irq_desc + irq;

	if (desc && desc->action) {
		desc->action->flags |= IRQF_DISABLED;	/* disable irq reentrant */
		generic_handle_irq_desc(irq, desc);
	} else {
		printk(KERN_ERR "Error, not registered alive interrupt=%d (%d.%d), disable !!!\n",
			irq, phy, bit);
		writel(readl(base + ALIVE_INT_SET) & ~(1<<bit), base + ALIVE_INT_SET);		/* alive mask : irq disable */
		writel(readl(base + ALIVE_INT_STATUS) | (1<<bit), base + ALIVE_INT_STATUS);	/* alive ack  : irq pend clear */
		readl(base + ALIVE_INT_STATUS);	/* Guarantee */
	}

	writel_relaxed(phy, GIC_CPUI_BASE + GIC_CPU_EOI);
	return;
}

static void __init alive_init(void __iomem *base, unsigned int irq_start,
		     u32 irq_sources, u32 resume_sources)
{
	int irq_alive = IRQ_PHY_CLKPWR_ALIVEIRQ + GIC_PHY_OFFSET;
	int num = IRQ_ALIVE_END - IRQ_ALIVE_START;
	int i = 0;

	printk(KERN_INFO "ALIVE @%p: start %3d, mask 0x%08x (alive %d, num %d)\n",
		base, irq_start, irq_sources, irq_alive, num);

	/* set alive irq handler */
	for (i = 0; num > i; i++) {
		if (irq_sources & (1 << i)) {
			int irq = irq_start + i;
			irq_set_chip_data(irq, base);
			irq_set_chip_and_handler(irq, &alive_chip, handle_level_irq);
			set_irq_flags(irq, IRQF_VALID | IRQF_PROBE);
		}
	}

	/* register alive irq handler data */
	irq_set_handler_data(irq_alive, base);

	/*
	 * call alive_mask_irq
	 * chip and chip data is registerd at gic_init
	 */
	irq_set_chained_handler(irq_alive, alive_handler);
}

/*----------------------------------------------------------------------------
 *  GPIO irq chain handler
 *  start  -> request_irq -> gpio irq_unmask
 *  do IRQ -> gpio handler -> gpio irq_mask -> gpio irq_ack -> driver handler -> gpio irq_unmask ->
 *  end    -> disable
 ----------------------------------------------------------------------------*/
static const char *io_name[] = { "GPIOA", "GPIOB", "GPIOC", "GPIOD", "GPIOE", };

#define	PIO_IRQ_BASE	IRQ_PHY_GPIOA
#define	VIO_IRQ_BASE	IRQ_GPIO_START
#define	VIO_NAME(i)		(io_name[(i-VIO_IRQ_BASE)/32])
#define	PIO_NAME(i)		(io_name[(i-PIO_IRQ_BASE)])

#define	GPIO_OUT_ENB		0x04
#define	GPIO_INT_MODE0		0x08	// 0x08,0x0C
#define	GPIO_INT_MODE1		0x28
#define	GPIO_INT_ENB		0x10
#define	GPIO_INT_STATUS		0x14
#define	GPIO_ALT_MODE		0x20	// 0x20,0x24
#define	GPIO_INT_DET		0x3C

static void gpio_ack_irq(struct irq_data *d)
{
	void __iomem *base = irq_data_get_irq_chip_data(d);
	int bit = (d->irq - IRQ_GPIO_START) & 0x1F;
	pr_debug("%s: gpio irq = %d, %s.%d\n", __func__, d->irq, VIO_NAME(d->irq), bit);

	/* gpio ack : irq pend clear */
	writel((1<<bit), base + GPIO_INT_STATUS);
	readl(base + GPIO_INT_STATUS);
}

static void gpio_mask_irq(struct irq_data *d)
{
	void __iomem *base = irq_data_get_irq_chip_data(d);
	int bit = (d->irq - IRQ_GPIO_START) & 0x1F;
	pr_debug("%s: gpio irq = %d, %s.%d\n", __func__, d->irq, VIO_NAME(d->irq), bit);

	/* gpio mask : irq disable */
	writel(readl(base + GPIO_INT_ENB) & ~(1<<bit), base + GPIO_INT_ENB);
	writel(readl(base + GPIO_INT_DET) & ~(1<<bit), base + GPIO_INT_DET);
}

static void gpio_unmask_irq(struct irq_data *d)
{
	void __iomem *base = irq_data_get_irq_chip_data(d);
	int bit = (d->irq - IRQ_GPIO_START) & 0x1F;
	pr_debug("%s: gpio irq = %d, %s.%d\n", __func__, d->irq, VIO_NAME(d->irq), bit);

	/* gpio unmask : irq enable */
	writel(readl(base + GPIO_INT_ENB) | (1<<bit), base + GPIO_INT_ENB);
	writel(readl(base + GPIO_INT_DET) | (1<<bit), base + GPIO_INT_DET);
	readl(base + GPIO_INT_ENB);
}

static int gpio_set_type_irq(struct irq_data *d, unsigned int type)
{
	void __iomem *base = irq_data_get_irq_chip_data(d);
	int bit = (d->irq - IRQ_GPIO_START) & 0x1F;
	unsigned int reg, val, alt;
	NX_GPIO_INTMODE mode = 0;
	pr_debug("%s: gpio irq = %d, %s.%d, type=0x%x\n",
		__func__, d->irq, VIO_NAME(d->irq), bit, type);

	switch (type) {
	case IRQ_TYPE_NONE:	printk(KERN_WARNING "%s: No edge setting!\n", __func__);
		break;
	case IRQ_TYPE_EDGE_RISING:	mode = NX_GPIO_INTMODE_RISINGEDGE;	break;
	case IRQ_TYPE_EDGE_FALLING:	mode = NX_GPIO_INTMODE_FALLINGEDGE;	break;
	case IRQ_TYPE_EDGE_BOTH:	mode = NX_GPIO_INTMODE_BOTHEDGE;	break;
	case IRQ_TYPE_LEVEL_LOW:	mode = NX_GPIO_INTMODE_LOWLEVEL;	break;
	case IRQ_TYPE_LEVEL_HIGH:	mode = NX_GPIO_INTMODE_HIGHLEVEL;	break;
	default:
		printk(KERN_ERR "%s: No such irq type %d", __func__, type);
		return -1;
	}

	/* gpio out : output disable */
	writel(readl(base + GPIO_OUT_ENB) & ~(1<<bit), base + GPIO_OUT_ENB);

	/* gpio mode : interrupt mode */
	reg  = (unsigned int)(base + GPIO_INT_MODE0 + (bit/16) * 4);
	val  = readl(reg) & ~(3<<((bit&0xf) * 2));
	val |= (mode&0x3) << ((bit&0xf) * 2);
	pr_debug("reg=0x%08x, val=0x%08x\n", reg, val);

	writel(val, reg);

	reg  = (unsigned int)(base + GPIO_INT_MODE1);
	val  = readl(reg) & ~(1<<bit);
	val |= ((mode>>2) & 0x1) << bit;
	pr_debug("reg=0x%08x, val=0x%08x\n", reg, val);

	writel(val, reg);

	/* gpio alt : gpio mode for irq */
	reg  = (unsigned int)(base + GPIO_ALT_MODE + (bit/16) * 4);
	val  = readl(reg) & ~(3<<((bit&0xf) * 2));
	alt  = gpio_alt_no[(d->irq-VIO_IRQ_BASE)/32][bit];
	val |= alt << ((bit&0xf) * 2);
	pr_debug("reg=0x%08x, val=0x%08x\n", reg, val);
	writel(val, reg);

	return 0;
}

static int gpio_set_wake(struct irq_data *d, unsigned int on)
{
#if (0)
	void __iomem *base = irq_data_get_irq_chip_data(d);
	int bit = (d->irq - IRQ_GPIO_START) & 0x1F;
	pr_debug("%s: gpio irq = %d, %s.%d wake %s\n",
		__func__, d->irq, VIO_NAME(d->irq), bit, on?"on":"off");
#endif
	return 0;
}

static void gpio_irq_enable(struct irq_data *d)
{
	void __iomem *base = irq_data_get_irq_chip_data(d);
	int bit = (d->irq - IRQ_GPIO_START) & 0x1F;
	pr_debug("%s: gpio irq = %d, %s.%d\n", __func__, d->irq, VIO_NAME(d->irq), bit);

	/* gpio unmask : irq enable */
	writel(readl(base + GPIO_INT_ENB) | (1<<bit), base + GPIO_INT_ENB);
	writel(readl(base + GPIO_INT_DET) | (1<<bit), base + GPIO_INT_DET);
}

static void gpio_irq_disable(struct irq_data *d)
{
	void __iomem *base = irq_data_get_irq_chip_data(d);
	int bit = (d->irq - IRQ_GPIO_START) & 0x1F;
	pr_debug("%s: gpio irq = %d, %s.%d\n", __func__, d->irq, VIO_NAME(d->irq), bit);

	/* gpio mask : irq disable */
	writel(readl(base + GPIO_INT_ENB) & ~(1<<bit), base + GPIO_INT_ENB);
	writel(readl(base + GPIO_INT_DET) & ~(1<<bit), base + GPIO_INT_DET);
}

static struct irq_chip gpio_chip = {
	.name			= "GPIO",
	.irq_ack		= gpio_ack_irq,
	.irq_mask		= gpio_mask_irq,
	.irq_unmask		= gpio_unmask_irq,
	.irq_set_type	= gpio_set_type_irq,
	.irq_set_wake	= gpio_set_wake,
	.irq_enable		= gpio_irq_enable,
	.irq_disable	= gpio_irq_disable,
};

static void gpio_handler(unsigned int irq, struct irq_desc *desc)
{
	void __iomem *base = irq_desc_get_handler_data(desc);
	u32 stat, mask;
	int phy, bit;

	mask = readl(base + GPIO_INT_ENB);
	stat = readl(base + GPIO_INT_STATUS) & mask;
	bit  = ffs(stat) - 1;
	phy  = irq;

	pr_debug("%s: gpio irq=%d [%s.%d], stat=0x%08x, mask=0x%08x\n",
		__func__, phy, PIO_NAME(phy), bit, stat, mask);

	if (-1 == bit) {
		printk(KERN_ERR "Unknown gpio phy irq=%d, status=0x%08x, mask=0x%08x\r\n",
			phy, stat, mask);
		writel(-1, (base + GPIO_INT_STATUS));	/* clear gpio status all */
		writel_relaxed(phy, GIC_CPUI_BASE + GIC_CPU_EOI);
		return;
	}

	/* gpio descriptor */
	irq  = (VIO_IRQ_BASE + bit + (32 * (phy - PIO_IRQ_BASE)));	// virtual irq
	desc = irq_desc + irq;

	if (desc && desc->action) {
		/* disable irq reentrant */
		desc->action->flags |= IRQF_DISABLED;
		generic_handle_irq_desc(irq, desc);
	} else {
		printk(KERN_ERR "Error, not registered gpio interrupt=%d (%s.%d), disable !!!\n",
			irq, PIO_NAME(phy), bit);
		writel(readl(base + GPIO_INT_ENB) & ~(1<<bit), base + GPIO_INT_ENB);		/* gpio mask : irq disable */
		writel(readl(base + GPIO_INT_STATUS) | (1<<bit), base + GPIO_INT_STATUS);	/* gpio ack  : irq pend clear */
		readl(base + GPIO_INT_STATUS);	/* Guarantee */
	}

	writel_relaxed(phy, GIC_CPUI_BASE + GIC_CPU_EOI);
	return;
}

static void __init gpio_init(void __iomem *base, unsigned int irq_start,
		     u32 irq_sources, u32 resume_sources)
{
	int irq_gpio = IRQ_PHY_GPIOA + GIC_PHY_OFFSET;
	int num = 5;  /* A,B,C,D,E */
	int ios = 32; /* GPIO 32 */
	int n = 0,i = 0;

	/* set gpio irq handler */
	for (n = 0; num > n; n++) {
		printk(KERN_INFO "GPIO  @%p: start %3d, mask 0x%08x (gpio %d)\n",
			base, irq_start, irq_sources, irq_gpio);

		for (i = 0; ios > i; i++) {
			if (irq_sources & (1 << i)) {
				int irq = irq_start + i;
				irq_set_chip_data(irq, base);
				irq_set_chip_and_handler(irq, &gpio_chip, handle_level_irq);
				set_irq_flags(irq, IRQF_VALID | IRQF_PROBE);
			}
		}
		/* init gpio irq register  */
		writel(0xFFFFFFFF, base + GPIO_INT_STATUS);
		writel(0x0, base + GPIO_INT_ENB);
		writel(0x0, base + GPIO_INT_DET);

		/* register gpio irq handler data */
		irq_set_handler_data(irq_gpio, base);

		/*
	 	 * call gpio_mask_irq
	 	 * chip and chip data is registerd at gic_init
	 	 */
		irq_set_chained_handler(irq_gpio, gpio_handler);

		/* next */
		irq_gpio++;
		irq_start += ios;
		base += GPIO_BASE_OFFSET;
	}
}

