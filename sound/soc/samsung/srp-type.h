/* linux/sound/soc/samsung/srp-type.h
 *
 * Copyright (c) 2010-2011 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * EXYNOS - SRP Type definitions
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef __SND_SOC_SAMSUNG_SRP_TYPE_H
#define __SND_SOC_SAMSUNG_SRP_TYPE_H

#ifdef CONFIG_SND_SAMSUNG_ALP
bool srp_enabled_status(void) {return 1;}
extern unsigned int srp_get_idma_addr(void);
#else
bool srp_enabled_status(void) {return 0;}
unsigned int srp_get_idma_addr(void) {return 0;}
#endif

#endif /* __SND_SOC_SAMSUNG_SRP_TYPE_H */
