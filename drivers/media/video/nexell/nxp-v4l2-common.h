#ifndef _NXP_V4L2_COMMON_H
#define _NXP_V4L2_COMMON_H

#include <linux/atomic.h>
#include <linux/irqreturn.h>

/**
 * structures
 */
struct nxp_v4l2_irq_entry {
    struct list_head entry;
    u32 irqs; /* bit orring */
    void *priv;
    irqreturn_t (*handler)(void *);
};

/**
 * common util functions for nxp v4l2
 */

/* for atomic operations */
#define NXP_ATOMIC_SET(V, I) atomic_set(V, I)
#define NXP_ATOMIC_SET_MASK(MASK, PTR)  \
    do { \
        unsigned long oldval = atomic_read(PTR); \
        unsigned long newval = oldval | MASK; \
        atomic_cmpxchg(PTR, oldval, newval); \
    } while (0)
#define NXP_ATOMIC_CLEAR_MASK(MASK, PTR) \
        atomic_clear_mask(MASK, (unsigned long *)&((PTR)->counter))
#define NXP_ATOMIC_READ(PTR)    atomic_read(PTR)
#define NXP_ATOMIC_INC(PTR)     atomic_inc(PTR)
#define NXP_ATOMIC_DEC(PTR)     atomic_dec(PTR)

/**
 * util functions
 */
struct i2c_client;
struct nxp_v4l2_i2c_board_info;

struct i2c_client *
nxp_v4l2_get_i2c_client(struct nxp_v4l2_i2c_board_info *);

#endif
