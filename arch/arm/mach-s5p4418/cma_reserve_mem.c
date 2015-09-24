/*
 * CMA Region reserve helper funtion
 * from Samsung Exynos5 code
 */

#ifdef CONFIG_CMA_DEBUG
#define DEBUG
#endif

#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/memblock.h>
#include <linux/mm.h>
#include <linux/cma.h>

void __init nxp_cma_region_reserve(struct cma_region *regions, const char *map)
{
    struct cma_region *reg;
    phys_addr_t paddr_last = 0xFFFFFFFF;

    for (reg = regions; reg->size != 0; reg++) {
        phys_addr_t paddr;

        if (!IS_ALIGNED(reg->size, PAGE_SIZE)) {
            pr_debug("NXP/CMA: size of '%s' is NOT page-aligned\n", reg->name);
            reg->size = PAGE_ALIGN(reg->size);
        }

        if (reg->reserved) {
            pr_err("NXP/CMA: '%s' already reserved\n", reg->name);
            continue;
        }

        if (reg->alignment) {
            if ((reg->alignment & ~PAGE_MASK) ||
                (reg->alignment & ~reg->alignment)) {
                pr_err("NXP/CMA: failed to reserve '%s': "
                        "incorrect alignment 0x%08x.\n",
                        reg->name, reg->alignment);
                continue;
            }
        } else {
            reg->alignment = PAGE_SIZE;
        }

        if (reg->start) {
            if (!memblock_is_region_reserved(reg->start, reg->size)
                && (memblock_reserve(reg->start, reg->size) == 0)) {
                reg->reserved = 1;
            } else {
                pr_err("NXP/CMA: failed to reserve '%s'\n", reg->name);
            }

        } else {
            paddr = memblock_find_in_range(0, MEMBLOCK_ALLOC_ACCESSIBLE,
                    reg->size, reg->alignment);
            if (paddr) {
                if (memblock_reserve(paddr, reg->size)) {
                    pr_err("NXP/CMA: failed to reserve '%s': memblock_reserve() failed\n",
                            reg->name);
                    continue;
                }

                reg->start = paddr;
                reg->reserved = 1;
            } else {
                pr_err("NXP/CMA: No free space in memory for '%s': size(%d)\n",
                        reg->name, reg->size);
            }
        }

        if (reg->reserved) {
            pr_debug("NXP/CMA: "
                    "Reserved 0x%08x/0x%08x for '%s'\n",
                    reg->start, reg->size, reg->name);
            printk("NXP/CMA: "
                    "Reserved 0x%08x/0x%08x for '%s'\n",
                    reg->start, reg->size, reg->name);

            if (0 == cma_early_region_register(reg)) {
                paddr_last = min(paddr, paddr_last);
                pr_debug("NXP/CMA: success register cma region for '%s'\n",
                        reg->name);
                printk("NXP/CMA: success register cma region for '%s'\n",
                        reg->name);
            } else {
                pr_err("NXP/CMA: failed to cma_early_region_register for '%s'\n",
                        reg->name);
                memblock_free(reg->start, reg->size);
            }
        }
    }

    if (map) {
        cma_set_defaults(NULL, map);
    }
}
