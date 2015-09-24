#ifndef __NXP_DFS_BCLK_H__
#define __NXP_DFS_BCLK_H__

enum {
    BCLK_USER_SD = 0,
    BCLK_USER_USB_DEVICE,
    BCLK_USER_USB_HOST,
    BCLK_USER_VIP,
    BCLK_USER_DMA,
    BCLK_USER_OGL,
    BCLK_USER_MPEG
};

/* Events for notifier */
#define BCLK_CHANGED        0x0001

typedef int(*dfs_bclk_func)(uint32_t, uint32_t, uint32_t, uint32_t);

/**
 * export funcs
 */
int bclk_get(uint32_t user);
int bclk_put(uint32_t user);
int register_dfs_bclk_func(dfs_bclk_func);
void bclk_dfs_register_notify(struct notifier_block *nb);
void bclk_dfs_unregister_notify(struct notifier_block *nb);

#endif
