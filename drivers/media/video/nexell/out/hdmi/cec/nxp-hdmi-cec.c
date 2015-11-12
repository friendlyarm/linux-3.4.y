#define DEBUG
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/poll.h>
#include <linux/miscdevice.h>
#include <linux/sched.h>
#include <linux/clk.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/module.h>

#include <mach/platform.h>
#include <mach/soc.h>
#include <mach/clkdev.h>
#include <mach/hdmi/nxp-hdmi-context.h>
#include <mach/nxp-hdmi-cec.h>

/**
 * constants
 */
#define CEC_RX_BUFF_SIZE        16
#define CEC_TX_BUFF_SIZE        16

#define SECS_TO_NS(sec)         (sec * (1000000000))
#define CLK_DIVISOR_BASE_NS     (50000)

#define CEC_MESSAGE_BROADCAST_MASK	0x0F
#define CEC_MESSAGE_BROADCAST		0x0F
#define CEC_FILTER_THRESHOLD		0x15

/**
 * state
 */
enum cec_state {
    STATE_RX,
    STATE_TX,
    STATE_DONE,
    STATE_ERROR
};

/**
 * struct
 */
struct nxp_hdmi_cec {
    /* general */
    int open_count;

    /* rx */
    spinlock_t lock_rx;
    wait_queue_head_t waitq_rx;
    atomic_t state_rx;
    u8 buffer_rx[CEC_RX_BUFF_SIZE];
    unsigned int size_rx;

    /* tx */
    wait_queue_head_t waitq_tx;
    atomic_t state_tx;

    /* irq callback */
    struct hdmi_irq_callback *cb_entry;
};

static struct nxp_hdmi_cec *__me = NULL;

/**
 * low level control
 */
static void _cec_set_clk(unsigned int input_clk)
{
    // calc divisor
    // CEC_DIVISOR x (clock cycle time in ns) = 0.05ms
    // CEC_DIVISOR = 0.05ms / (clock cycle time in ns)
    u32 input_clk_cycle_ns = SECS_TO_NS(1) / input_clk;
    u32 clk_divisor  = CLK_DIVISOR_BASE_NS / input_clk_cycle_ns;

    printk("%s: input_clk(%u), input_clk_cycle_ns(%u), clk_divisor(%u)\n",
            __func__, input_clk, input_clk_cycle_ns, clk_divisor);

    // set
    hdmi_writeb(HDMI_CEC_CLK_DIVISOR_3, clk_divisor >> 24);
    hdmi_writeb(HDMI_CEC_CLK_DIVISOR_2, (clk_divisor & 0x00ff0000) >> 16);
    hdmi_writeb(HDMI_CEC_CLK_DIVISOR_1, (clk_divisor & 0x0000ff00) >> 8);
    hdmi_writeb(HDMI_CEC_CLK_DIVISOR_0, (clk_divisor & 0x000000ff));
}

static inline void _cec_rx_reset(void)
{
    hdmi_writeb(HDMI_CEC_RX_CONTROL, HDMI_CEC_RX_CTRL_RESET);
}

static inline void _cec_tx_reset(void)
{
    hdmi_writeb(HDMI_CEC_TX_CONTROL, HDMI_CEC_TX_CTRL_RESET);
}

static inline void _cec_reset(void)
{
    _cec_rx_reset();
    _cec_tx_reset();
}

static inline void _cec_enable_irq(void)
{
    hdmi_write_mask(HDMI_INTC_CON_0, ~0, HDMI_INTC_EN_CEC);
}

static inline void _cec_disable_irq(void)
{
    hdmi_write_mask(HDMI_INTC_CON_0, 0, HDMI_INTC_EN_CEC);
}

static inline void _cec_enable_rx(void)
{
    u8 reg = hdmi_readb(HDMI_CEC_RX_CONTROL);
    reg |= HDMI_CEC_RX_CTRL_ENABLE;
    hdmi_writeb(HDMI_CEC_RX_CONTROL, reg);
}

static inline void _cec_enable_rx_intr(void)
{
    u8 reg = hdmi_readb(HDMI_CEC_IRQ_MASK);
    reg &= ~HDMI_CEC_IRQ_RX_DONE;
    reg &= ~HDMI_CEC_IRQ_RX_ERROR;
    hdmi_writeb(HDMI_CEC_IRQ_MASK, reg);
}

static inline void _cec_disable_rx_intr(void)
{
    u8 reg = hdmi_readb(HDMI_CEC_IRQ_MASK);
    reg |= HDMI_CEC_IRQ_RX_DONE | HDMI_CEC_IRQ_RX_ERROR;
    hdmi_writeb(HDMI_CEC_IRQ_MASK, reg);
}

static inline void _cec_enable_tx_intr(void)
{
    u8 reg = hdmi_readb(HDMI_CEC_IRQ_MASK);
    reg &= ~HDMI_CEC_IRQ_TX_DONE;
    reg &= ~HDMI_CEC_IRQ_TX_ERROR;
    hdmi_writeb(HDMI_CEC_IRQ_MASK, reg);
}

static inline void _cec_disable_tx_intr(void)
{
    u8 reg = hdmi_readb(HDMI_CEC_IRQ_MASK);
    reg |= HDMI_CEC_IRQ_TX_DONE | HDMI_CEC_IRQ_TX_ERROR;
    hdmi_writeb(HDMI_CEC_IRQ_MASK, reg);
}

static inline void _cec_set_threshold(u8 val)
{
    hdmi_writeb(HDMI_CEC_RX_FILTER_TH, val);
    hdmi_writeb(HDMI_CEC_RX_FILTER_CTRL, 0);
}

static void _cec_tx(char *data, size_t count)
{
    int i = 0;
    u8 reg;

    while (i < count) {
        hdmi_writeb(HDMI_CEC_TX_BUFF0 + (i * 4), data[i]);
        i++;
    }

    hdmi_writeb(HDMI_CEC_TX_BYTES, count);
    /*_cec_set_tx_state(STATE_TX);*/
    reg = hdmi_readb(HDMI_CEC_TX_CONTROL);
    reg |= HDMI_CEC_TX_CTRL_START;

    if ((data[0] & CEC_MESSAGE_BROADCAST_MASK) == CEC_MESSAGE_BROADCAST)
        reg |= HDMI_CEC_TX_CTRL_BCAST;
    else
        reg &= ~HDMI_CEC_TX_CTRL_BCAST;

    reg |= 0x50;
    hdmi_writeb(HDMI_CEC_TX_CONTROL, reg);
}

static inline void _cec_set_addr(u32 addr)
{
    hdmi_writeb(HDMI_CEC_LOGIC_ADDRESS, addr & 0x0F);
}

static inline u32 _cec_get_tx_status(void)
{
    u32 status = hdmi_readb(HDMI_CEC_TX_STAT_0);
    status |= hdmi_readb(HDMI_CEC_TX_STAT_1) << 8;
    status |= hdmi_readb(HDMI_CEC_TX_STAT_2) << 16;
    status |= hdmi_readb(HDMI_CEC_TX_STAT_3) << 24;
    return status;
}

static inline u32 _cec_get_rx_status(void)
{
    u32 status = hdmi_readb(HDMI_CEC_RX_STAT_0);
    status |= hdmi_readb(HDMI_CEC_RX_STAT_1) << 8;
    status |= hdmi_readb(HDMI_CEC_RX_STAT_2) << 16;
    status |= hdmi_readb(HDMI_CEC_RX_STAT_3) << 24;
    return status;
}

static inline void _cec_clr_pending_tx(void)
{
    hdmi_writeb(HDMI_CEC_IRQ_CLEAR, HDMI_CEC_IRQ_TX_DONE | HDMI_CEC_IRQ_TX_ERROR);
}

static inline void _cec_clr_pending_rx(void)
{
    hdmi_writeb(HDMI_CEC_IRQ_CLEAR, HDMI_CEC_IRQ_RX_DONE | HDMI_CEC_IRQ_RX_ERROR);
}

static inline void _cec_rx(u32 size, u8 *buffer)
{
    u32 i = 0;
    while (i < size) {
        buffer[i] = hdmi_readb(HDMI_CEC_RX_BUFF0 + (i * 4));
        i++;
    }
}

/**
 * state management
 */
static inline void _cec_set_tx_state(enum cec_state state)
{
    atomic_set(&__me->state_tx, state);
}

static inline void _cec_set_rx_state(enum cec_state state)
{
    atomic_set(&__me->state_rx, state);
}

/**
 * irq callback
 */
static int _irq_callback(void *data)
{
    struct nxp_hdmi_cec *me = (struct nxp_hdmi_cec *)data;

    u32 tx_status = _cec_get_tx_status();
    u32 rx_status = _cec_get_rx_status();

    pr_debug("%s: tx_status 0x%x, rx_status 0x%x\n", __func__, tx_status, rx_status);
    if (tx_status & HDMI_CEC_STATUS_TX_DONE) {
        if (tx_status & HDMI_CEC_STATUS_TX_ERROR) {
            pr_err("%s: tx error intr\n",  __func__);
            _cec_set_tx_state(STATE_ERROR);
        } else {
            _cec_set_tx_state(STATE_DONE);
        }

        _cec_clr_pending_tx();
        wake_up_interruptible(&me->waitq_tx);
    }

    if (rx_status & HDMI_CEC_STATUS_RX_DONE) {
        if (rx_status & HDMI_CEC_STATUS_RX_ERROR) {
            pr_err("%s: rx error intr\n",  __func__);
            _cec_rx_reset();
            _cec_set_rx_state(STATE_ERROR);
        } else {
            u32 size = rx_status >> 8;
            spin_lock(&me->lock_rx);
            _cec_rx(size, me->buffer_rx);
            me->size_rx = size;
            _cec_set_rx_state(STATE_DONE);
            spin_unlock(&me->lock_rx);
            pr_debug("%s: rx %d bytes\n", __func__, size);
            _cec_enable_rx();
        }

        _cec_clr_pending_rx();
        wake_up_interruptible(&me->waitq_rx);
    }

    return 0;
}

/**
 * externs
 */
extern struct hdmi_irq_callback *hdmi_register_irq_callback(int, int (*)(void *), void *);
extern void hdmi_unregister_irq_callback(struct hdmi_irq_callback *);

/**
 * file operations
 */
static int nxp_hdmi_cec_open(struct inode *inode, struct file *file)
{
    struct nxp_hdmi_cec *me = __me;
    unsigned int input_clk;

    if (me->open_count > 0) {
         pr_err("%s: restrict multiple open\n", __func__);
         return -EBUSY;
    }

    // reset
    _cec_reset();

    // clk
    input_clk = nxp_cpu_clock_hz(CORECLK_ID_PCLK);
    _cec_set_clk(input_clk);

    // threshold
    _cec_set_threshold(CEC_FILTER_THRESHOLD);

    // irq
    me->cb_entry = hdmi_register_irq_callback(HDMI_INTC_EN_CEC, _irq_callback, me);
    if (!me->cb_entry) {
         pr_err("%s: failed to register_hdmi_irq_callback()\n", __func__);
         return -ENODEV;
    }
    _cec_enable_irq();

    _cec_enable_tx_intr();
    _cec_enable_rx_intr();

    _cec_enable_rx();
    _cec_set_rx_state(STATE_RX);

    me->open_count++;

    file->private_data = me;

    printk("%s success\n", __func__);
    return 0;
}

static int nxp_hdmi_cec_release(struct inode *inode, struct file *file)
{
    struct nxp_hdmi_cec *me = file->private_data;
    _cec_disable_irq();
    hdmi_unregister_irq_callback(me->cb_entry);
    me->open_count--;
    return 0;
}

static ssize_t nxp_hdmi_cec_read(struct file *file, char __user *buffer, size_t count, loff_t *ppos)
{
    unsigned long flags;
    ssize_t ret = -1;
    struct nxp_hdmi_cec *me = file->private_data;

    pr_debug("%s entered\n", __func__);

    /*_cec_set_rx_state(STATE_RX);*/

    // wait until state_rx == STATE_DONE
    if (wait_event_interruptible(me->waitq_rx, atomic_read(&me->state_rx) == STATE_DONE)) {
        printk("%s: return -ERESTARTSYS\n", __func__);
        return -ERESTARTSYS;
    }
    pr_debug("rx state ==> 0x%x\n", atomic_read(&me->state_rx));

    spin_lock_irqsave(&me->lock_rx, flags);

    if (me->size_rx > count) {
        ret = -EINVAL;
        goto err_out;
    }

    if (copy_to_user(buffer, me->buffer_rx, me->size_rx)) {
        ret = -EFAULT;
        goto err_out;
    }

    ret = me->size_rx;
    _cec_set_rx_state(STATE_RX);

err_out:
    spin_unlock_irqrestore(&me->lock_rx, flags);
    return ret;
}

static ssize_t nxp_hdmi_cec_write(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
{
    char *data;
    struct nxp_hdmi_cec *me = file->private_data;

    printk("%s entered\n", __func__);

    if (count > CEC_TX_BUFF_SIZE || count == 0) {
        pr_err("%s: invalid tx count %d\n", __func__, count);
        return -1;
    }

    data = kmalloc(count, GFP_KERNEL);
    if (!data) {
        pr_err("%s: failed to kmalloc for tx buffer\n", __func__);
        return -1;
    }

    if (copy_from_user(data, buffer, count)) {
        pr_err("%s: failed to copy_from_user()\n", __func__);
        kfree(data);
        return -EFAULT;
    }

    _cec_set_tx_state(STATE_TX);
    _cec_tx(data, count);

    kfree(data);

    if (wait_event_interruptible(me->waitq_tx, atomic_read(&me->state_tx) != STATE_TX)) {
         return -ERESTARTSYS;
    }

    if (atomic_read(&me->state_tx) == STATE_ERROR)
        return -1;

    return count;
}

static long nxp_hdmi_cec_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    u32 laddr;
    /*struct nxp_hdmi_cec *me = file->private_data;*/

    printk("%s entered\n", __func__);

    switch (cmd) {
    case IOCTL_HDMI_CEC_SETLADDR:
        if (get_user(laddr, (u32 __user *)arg))
            return -EFAULT;
        _cec_set_addr(laddr);
        break;

    default:
        return -EINVAL;
    }

    return 0;
}

static u32 nxp_hdmi_cec_poll(struct file *file, poll_table *wait)
{
    struct nxp_hdmi_cec *me = file->private_data;

    printk("%s entered\n", __func__);

    poll_wait(file, &me->waitq_rx, wait);

    if (atomic_read(&me->state_rx) == STATE_DONE)
        return POLLIN | POLLRDNORM;

    return 0;
}

static const struct file_operations nxp_hdmi_cec_fops = {
    .owner          = THIS_MODULE,
    .open           = nxp_hdmi_cec_open,
    .release        = nxp_hdmi_cec_release,
    .read           = nxp_hdmi_cec_read,
    .write          = nxp_hdmi_cec_write,
    .unlocked_ioctl = nxp_hdmi_cec_ioctl,
    .poll           = nxp_hdmi_cec_poll,
};

static struct miscdevice nxp_hdmi_cec_misc_device = {
    .minor  = MISC_DYNAMIC_MINOR,
    .name   = "hdmi-cec",
    .fops   = &nxp_hdmi_cec_fops,
};

/**
 * platform driver interface
 */
static inline void _init_me(struct nxp_hdmi_cec *me)
{
    /* rx */
    init_waitqueue_head(&me->waitq_rx);
    spin_lock_init(&me->lock_rx);

    /* tx */
    init_waitqueue_head(&me->waitq_tx);
}

static int __devinit nxp_hdmi_cec_probe(struct platform_device *pdev)
{
    /* gpio setting */
    uint32_t io = PAD_GPIO_C + 3;
    nxp_soc_gpio_set_io_func(io, NX_GPIO_PADFUNC_2);
    nxp_soc_gpio_set_io_pull_sel(io, 1); // pull up select
    nxp_soc_gpio_set_io_pull_enb(io, 1); // enable

    __me = (struct nxp_hdmi_cec *)kzalloc(sizeof(struct nxp_hdmi_cec), GFP_KERNEL);
    if (!__me) {
        printk(KERN_ERR "failed to kzalloc for struct nxp_hdmi_cec\n");
        return -ENOMEM;
    }

    _init_me(__me);

    if (misc_register(&nxp_hdmi_cec_misc_device)) {
         printk(KERN_ERR "can't register nxp hdmi misc device\n");
         kfree(__me);
         __me = NULL;
         return -EBUSY;
    }

    printk("%s success\n", __func__);
    return 0;
}

static int __devexit nxp_hdmi_cec_remove(struct platform_device *pdev)
{
    if (__me) {
         kfree(__me);
         __me = NULL;
    }

    return 0;
}

#ifdef CONFIG_PM
static int nxp_hdmi_cec_suspend(struct platform_device *pdev, pm_message_t state)
{
    return 0;
}

static int nxp_hdmi_cec_resume(struct platform_device *pdev)
{
     return 0;
}
#else
#define nxp_hdmi_cec_suspend    NULL
#define nxp_hdmi_cec_resume     NULL
#endif

static struct platform_driver nxp_hdmi_cec_driver = {
    .probe      = nxp_hdmi_cec_probe,
    .remove     = __devexit_p(nxp_hdmi_cec_remove),
    .suspend    = nxp_hdmi_cec_suspend,
    .resume     = nxp_hdmi_cec_resume,
    .driver     = {
        .name   = NXP_HDMI_CEC_DRV_NAME,
        .owner  = THIS_MODULE,
    },
};

/**
 * module interface
 */
static int __init nxp_hdmi_cec_init(void)
{
    return platform_driver_register(&nxp_hdmi_cec_driver);
}

static void __exit nxp_hdmi_cec_exit(void)
{
    platform_driver_unregister(&nxp_hdmi_cec_driver);
}

MODULE_AUTHOR("swpark@nexell.co.kr");
MODULE_DESCRIPTION("SLsiAP HDMI CEC driver");
MODULE_LICENSE("GPL");

module_init(nxp_hdmi_cec_init);
module_exit(nxp_hdmi_cec_exit);
