#ifndef __ASM_MACH_CLKDEV_H
#define __ASM_MACH_CLKDEV_H

struct clk {
	unsigned int rate;
};


#define __clk_get(clk) ({ 1; })
#define __clk_put(clk) do { } while (0)

/* clk id */
#define CORECLK_NAME_PLL0 		"pll0"	/* cpu clock */
#define CORECLK_NAME_PLL1 		"pll1"
#define CORECLK_NAME_PLL2 		"pll2"
#define CORECLK_NAME_PLL3 		"pll3"
#define CORECLK_NAME_FCLK 		"fclk"
#define CORECLK_NAME_MCLK 		"mclk"
#define CORECLK_NAME_BCLK 		"bclk"
#define CORECLK_NAME_PCLK 		"pclk"
#define CORECLK_NAME_HCLK 		"hclk"

#define CORECLK_ID_PLL0 		0
#define CORECLK_ID_PLL1 		1
#define CORECLK_ID_PLL2 		2
#define CORECLK_ID_PLL3 		3
#define CORECLK_ID_FCLK 		4
#define CORECLK_ID_MCLK 		5
#define CORECLK_ID_BCLK 		6
#define CORECLK_ID_PCLK 		7
#define CORECLK_ID_HCLK 		8

#endif
