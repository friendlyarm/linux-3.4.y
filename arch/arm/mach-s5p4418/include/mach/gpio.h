/*
 */
#include "s5p4418_irq.h"

#define GPIO_NUM_PER_BANK	32
#define ARCH_NR_GPIOS 		(GPIO_NUM_PER_BANK * 6)	/* For GPIO A, B, C, D, E, ALVIE */

extern const unsigned char gpio_alt_no[][GPIO_NUM_PER_BANK];
#define GET_GPIO_ALTFUNC(io,idx)    (gpio_alt_no[io][idx])

#include <asm-generic/gpio.h>

