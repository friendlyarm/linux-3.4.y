/*
 * (C) Copyright 2009
 * jung hyun kim, Nexell Co, <jhkim@nexell.co.kr>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#ifndef __DEVICES_H__
#define __DEVICES_H__

#include <linux/platform_device.h>
#include <linux/i2c-gpio.h>

#include "clkdev.h"
#include "display.h"
#include "dmadev.h"

/* cpu */
#define DEV_NAME_UART           "nxp-uart"        // pl0115 (amba-pl011.c)
#define DEV_NAME_FB             "nxp-fb"
#define DEV_NAME_DISP           "nxp-disp"
#define DEV_NAME_LCD            "nxp-lcd"
#define DEV_NAME_LVDS           "nxp-lvds"
#define DEV_NAME_HDMI           "nxp-hdmi"
#define DEV_NAME_RESCONV        "nxp-resconv"
#define DEV_NAME_MIPI           "nxp-mipi"
#define DEV_NAME_PCM            "nxp-pcm"
#define DEV_NAME_I2S            "nxp-i2s"
#define DEV_NAME_SPDIF_TX       "nxp-spdif-tx"
#define DEV_NAME_SPDIF_RX       "nxp-spdif-rx"
#define DEV_NAME_I2C            "nxp-i2c"
#define DEV_NAME_NAND           "nxp-nand"
#define DEV_NAME_KEYPAD         "nxp-keypad"
#define DEV_NAME_SDHC           "nxp-sdhc"
#define DEV_NAME_PWM            "nxp-pwm"
#define DEV_NAME_TIMER          "nxp-timer"
#define DEV_NAME_SOC_PWM        "nxp-soc-pwm"
#define DEV_NAME_GPIO           "nxp-gpio"
#define DEV_NAME_RTC            "nxp-rtc"
#define DEV_NAME_GMAC           "nxp-gmac"
#define DEV_NAME_MPEGTSI        "nxp-mpegtsi"
#define DEV_NAME_VR             "nxp-vr"
#define DEV_NAME_DIT            "nxp-deinterlace"
#define DEV_NAME_PPM            "nxp-ppm"
#define DEV_NAME_VIP            "nxp-vip"
#define DEV_NAME_CODA           "nxp-coda"
#define DEV_NAME_USB2HOST       "nxp-usb2h"
#define DEV_NAME_CRYPTO         "nxp-crypto"
#define DEV_NAME_SCALER         "nxp-scaler"
#define DEV_NAME_PDM            "nxp-pdm"
#define DEV_NAME_SPI            "nxp-spi"
#define DEV_NAME_ADC            "nxp-adc"
#define DEV_NAME_CPUFREQ        "nxp-cpufreq"
#define DEV_NAME_USBOTG         "nxp-otg"
#define DEV_NAME_RFKILL         "nxp-rfkill"
#define DEV_NAME_WDT    	"nxp-wdt"

/*
 *  Frame buffer platform data and display controller
 */
struct nxp_fb_plat_data {
    int             module;         /* 0: primary, 1: secondary */
    int             layer;          /* RGB 0, 1, 2 */
    unsigned int    format;         /* RGB format */
    unsigned int    bgcolor;
    int             bitperpixel;    /* bit per pixel */
    int             x_resol_max;    /* x resolution for change resolution */
    int             y_resol_max;    /* y resolution for change resolution  */
    int             x_resol;        /* x resolution */
    int             y_resol;        /* y resolution */
    int             buffers;        /* set 2 when support pan, default 1 */
    /* for direct fb region */
    unsigned int    fb_mem_base;
    unsigned int    fb_mem_end;
    /* for lcd dpi (default 0) */
    long            lcd_with_mm;    /* with (mm), default 0 */
    long            lcd_height_mm;  /* height (mm), default 0 */
    int             skip_pan_vsync;
};

struct nxp_lcd_plat_data {
    enum   disp_dev_type 	 display_in;						/* DISP_DEVICE_SYNCGEN0 / DISP_DEVICE_SYNCGEN1 */
    enum   disp_dev_type	 display_dev;					/* LCD, LVDS, MiPi, HDMI, Refer to display.h */
    struct disp_vsync_info 	*vsync;
    struct disp_syncgen_par *sync_gen;
    union  disp_dev_param 	*dev_param;		/* LCD, LVDS, MiPi, HDMI, Refer to display.h */
};

extern void nxp_platform_disp_device_data(enum disp_dev_type device,
				struct disp_vsync_info *vsync, void *dev_par, struct disp_syncgen_par *vsgen);

/*
 *    Sound platform data
 */
#include <sound/pcm.h>

/* I2S */
struct nxp_i2s_plat_data {
    int     master_mode;
    int     master_clock_in;
    int     trans_mode;                 /* 0:I2S, 1:Left 2:Right justified */
    int     sample_rate;
    int     sample_bit;                 /* support only 8, 16, 24 */
    int     frame_bit;                  /* support only 32, 48 */
    int     LR_pol_inv;
    int     pre_supply_mclk;            /* codec require mclk out, before codec initialize */
	bool	(*ext_is_en)(void);
	unsigned long (*set_ext_mclk)(unsigned long clk, int ch);
    bool    (*dma_filter)(struct dma_chan *chan, void *filter_param);
    const char *dma_play_ch;
    const char *dma_capt_ch;
};

/* SPDIF */
struct nxp_spdif_plat_data {
    int sample_rate;
    int hdmi_out;
    bool (*dma_filter)(struct dma_chan *chan, void *filter_param);
    const char *dma_ch;
};

/* PDM */
struct nxp_pdm_plat_data {
    int sample_rate;
    bool (*dma_filter)(struct dma_chan *chan, void *filter_param);
    const char *dma_ch;
};

/* sound DAI (I2S/SPDIF and codec interface) */
struct nxp_snd_jack_pin {
    int    support;
    int    detect_level;
    int detect_io;
    int debounce_time;
};

struct nxp_snd_dai_plat_data {
    int i2s_ch;
    unsigned int sample_rate;
    unsigned int pcm_format;        /* SNDRV_PCM_FMTBIT_S16_LE, SNDRV_PCM_FMTBIT_S24_LE, .. (include/sound/pcm.h) */
    struct nxp_snd_jack_pin hp_jack;
    struct nxp_snd_jack_pin mic_jack;
};

/*
 *  I2C platform data
 */
#include <cfg_type.h>

enum nxp_i2c_pin_descript {
    NXP_I2C0_MOD_SCL = PAD_GPIO_D + 2,
    NXP_I2C0_MOD_SDA = PAD_GPIO_D + 3,
    NXP_I2C1_MOD_SCL = PAD_GPIO_D + 4,
    NXP_I2C1_MOD_SDA = PAD_GPIO_D + 5,
    NXP_I2C2_MOD_SCL = PAD_GPIO_D + 6,
    NXP_I2C2_MOD_SDA = PAD_GPIO_D + 7
};

struct nxp_i2c_plat_data {
    int  port;
    int  irq;
	int retry_cnt;
	int retry_delay;
    long rate;
    unsigned int base_addr;
    struct i2c_gpio_platform_data *gpio;
	
};

/*
 *  Touch calibration platform data
 */
struct nxp_ts_cali_plat_data {
    int touch_points;    /* support touch points num when multi touch */
    int x_resol;
    int y_resol;
    int rotate;          /* 0, 90, 180, 270 */
    long pointercal[10];     /* calibration value (tslib) */
};

/*
 *    CPU Freq platform data
 */
struct nxp_cpufreq_plat_data {
    int pll_dev;                    /* core pll : 0, 1, 2, 3 */
    unsigned long (*freq_table)[2]; /* [freq KHz].[u volt] */
    int    table_size;
    long max_cpufreq;       /* unit Khz */
    long max_retention;     /* unit msec */
    long rest_cpufreq;      /* unit Khz */
    long rest_retention;    /* unit msec */
	char *supply_name;		/* voltage regulator name */
	long supply_delay_us;
};

/*
 *  SDHC platform data
 */
#include <linux/mmc/dw_mmc.h>
#include <linux/mmc/host.h>

extern void __init nxp_mmc_add_device(int ch, struct dw_mci_board *mci);

/*
 *  Keypad platform data
 */
struct nxp_key_plat_data {
    int  bt_count;
    unsigned int *bt_io;
    unsigned int *bt_code;
    unsigned int *bt_detect_high;
    unsigned int bt_repeat;       /* key repeat 1 = on , 0 = off */
    struct input_dev *bt_device;
    int	resume_delay_ms;
};

/*
 * NAND platform data
 */
struct nxp_nand_plat_data {
    struct mtd_partition * parts;
    int nr_parts;
    int chip_delay;
};

/*
 *  MPEGTSIF platform data
 */
struct nxp_mp2ts_dev_info {
    int demod_irq_num;
    int demod_rst_num;
    int tuner_rst_num;
};

struct nxp_mp2ts_plat_data {
    struct nxp_mp2ts_dev_info *dev_info;
    int ts_dma_size[3];
};

/*
 *  rfkill platform data
 */
#include <linux/rfkill.h>

#define	RFKILL_INIT_SET		(1<<0)
#define	RFKILL_INIT_ON		(1<<1)
#define	RFKILL_INIT_OFF		(1<<2)

struct rfkill_dev_data {
	/* rfkill config */
	char *supply_name;		/* set regulator name */
	char *module_name;		/* set module driver name */
    int gpio;
    unsigned int initval;
    int invert;				/* 0: high active, 1: low active */
    int delay_time_on;			/* ms */
    int delay_time_off;			/* ms */
};

struct nxp_rfkill_plat_data {
    char *name;             /* the name for the rfkill switch */
    enum rfkill_type type;  /* the type as specified in rfkill.h */
    struct rfkill_dev_data *rf_dev;
    int rf_dev_num;
    int support_suspend;
    int (*init)(void *data);
    int (*set_block)(void *data, bool blocked);
    int (*suspend)(struct platform_device *pdev, pm_message_t state);
    int (*resume)(struct platform_device *pdev);
};

/*
 *  ram console
 */
extern int __init persistent_ram_console_reserve(void);

/*
 *  cpufreq limit
 */
struct nxp_cpufreq_limit_data {
    char **limit_name;
    int limit_num;
    long aval_max_freq;     /* unit Khz */
    long op_max_freq; 		/* unit Khz */
#if defined(CONFIG_ARM_NXP_CPUFREQ_BY_RESOURCE)
	long limit_level0_freq; 	/* unit Khz */
	long limit_level1_freq; 	/* unit Khz */
	long min_max_freq;			/* unit Khz */
	long prev_max_freq;			/* unit Khz */
#endif
    long sched_duration;	/* unit ms */
    long sched_timeout;		/* unit ms */
};

/*
 *  USB HOST (ehci/ohci)
 */
struct nxp_ehci_platdata {
	int (*phy_init)(struct platform_device *pdev, int type);
	int (*phy_exit)(struct platform_device *pdev, int type);
	int (*hsic_phy_pwr_on)(struct platform_device *pdev, bool on);
	int resume_delay_time;	/* unit ms, more than 100 ms */
};

struct nxp_ohci_platdata {
	int (*phy_init)(struct platform_device *pdev, int type);
	int (*phy_exit)(struct platform_device *pdev, int type);
};

/*
 * ADC TMU
 */
struct nxp_adc_tmp_platdata {
	int channel;
	int tmp_offset;
	int duration;				/* default 100ms */
	void (*callback)(int ch, int value, int temp, bool run);
};

#endif    /* __DEVICES_H__ */
