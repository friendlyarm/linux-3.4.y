#include <linux/dma-mapping.h>
#include <linux/platform_device.h>
#include <linux/ion.h>
#include <linux/nxp_ion.h>
#include <linux/slab.h>
#include <mach/ion.h>

/* platform device is defined in mach/device.c */
extern struct platform_device nxp_device_ion;

void __init nxp_ion_set_platdata(void)
{
    struct ion_platform_data *pdata;

    pdata = kzalloc(sizeof(struct ion_platform_data), GFP_KERNEL);
    pdata->heaps = kzalloc(5 * sizeof(struct ion_platform_heap), GFP_KERNEL);

    if (pdata) {
        pdata->nr = 3;
        pdata->heaps[0].type = ION_HEAP_TYPE_SYSTEM;
        pdata->heaps[0].name = "ion_noncontig_heap";
        pdata->heaps[0].id   = ION_HEAP_TYPE_SYSTEM;
        pdata->heaps[1].type = ION_HEAP_TYPE_SYSTEM_CONTIG;
        pdata->heaps[1].name = "ion_contig_heap";
        pdata->heaps[1].id   = ION_HEAP_TYPE_SYSTEM_CONTIG;
        pdata->heaps[2].type = ION_HEAP_TYPE_NXP_CONTIG;
        pdata->heaps[2].name = "nxp_contig_heap";
        pdata->heaps[2].id   = ION_HEAP_TYPE_NXP_CONTIG;
        nxp_device_ion.dev.platform_data = pdata;
    }
}
