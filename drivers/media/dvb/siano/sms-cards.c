/*
 *  Card-specific functions for the Siano SMS1xxx USB dongle
 *
 *  Copyright (c) 2008 Michael Krufky <mkrufky@linuxtv.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation;
 *
 *  Software distributed under the License is distributed on an "AS IS"
 *  basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.
 *
 *  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "sms-cards.h"
#ifdef SMS_RC_SUPPORT_SUBSYS
#include "smsir.h"
#endif
static struct sms_board sms_boards[] = {
	[SMS_BOARD_UNKNOWN] = {
	/* 0 */
		.name = "Unknown board",
		.type = SMS_UNKNOWN_TYPE,
		.default_mode = SMSHOSTLIB_DEVMD_NONE,
	},
	[SMS1XXX_BOARD_SIANO_STELLAR] = {
	/* 1 */
		.name =	"Siano Stellar Digital Receiver",
		.type = SMS_STELLAR,
		.default_mode = SMSHOSTLIB_DEVMD_DVBT_BDA,
	},
	[SMS1XXX_BOARD_SIANO_NOVA_A] = {
	/* 2 */
		.name = "Siano Nova A Digital Receiver",
		.type = SMS_NOVA_A0,
		.default_mode = SMSHOSTLIB_DEVMD_DVBT_BDA,
	},
	[SMS1XXX_BOARD_SIANO_NOVA_B] = {
	/* 3 */
		.name = "Siano Nova B Digital Receiver",
		.type = SMS_NOVA_B0,
		.default_mode = SMSHOSTLIB_DEVMD_DVBT_BDA,
	},
	[SMS1XXX_BOARD_SIANO_VEGA] = {
	/* 4 */
		.name = "Siano Vega Digital Receiver",
		.type = SMS_VEGA,
		.default_mode = SMSHOSTLIB_DEVMD_CMMB,
	},
	[SMS1XXX_BOARD_HAUPPAUGE_CATAMOUNT] = {
	/* 5 */
		.name = "Hauppauge Catamount",
		.type = SMS_STELLAR,
		.fw[SMSHOSTLIB_DEVMD_DVBT_BDA] = "sms1xxx-stellar-dvbt-01.fw",
		.default_mode = SMSHOSTLIB_DEVMD_DVBT_BDA,
	},
	[SMS1XXX_BOARD_HAUPPAUGE_OKEMO_A] = {
	/* 6 */
		.name = "Hauppauge Okemo-A",
		.type = SMS_NOVA_A0,
		.fw[SMSHOSTLIB_DEVMD_DVBT_BDA] = "sms1xxx-nova-a-dvbt-01.fw",
		.default_mode = SMSHOSTLIB_DEVMD_DVBT_BDA,
	},
	[SMS1XXX_BOARD_HAUPPAUGE_OKEMO_B] = {
	/* 7 */
		.name = "Hauppauge Okemo-B",
		.type = SMS_NOVA_B0,
		.fw[SMSHOSTLIB_DEVMD_DVBT_BDA] = "sms1xxx-nova-b-dvbt-01.fw",
	},
	[SMS1XXX_BOARD_HAUPPAUGE_WINDHAM] = {
	/* 8 */
		.name = "Hauppauge WinTV MiniStick",
		.type = SMS_NOVA_B0,
		.fw[SMSHOSTLIB_DEVMD_DVBT_BDA] = "sms1xxx-hcw-55xxx-dvbt-02.fw",
		.default_mode = SMSHOSTLIB_DEVMD_DVBT_BDA,
		.board_cfg.leds_power = 26,
		.board_cfg.led0 = 27,
		.board_cfg.led1 = 28,
		.board_cfg.ir = 9,
		.led_power = 26,
		.led_lo    = 27,
		.led_hi    = 28,
	},
	[SMS1XXX_BOARD_HAUPPAUGE_TIGER_MINICARD] = {
	/* 9 */
		.name	= "Hauppauge WinTV MiniCard",
		.type	= SMS_NOVA_B0,
		.fw[SMSHOSTLIB_DEVMD_DVBT_BDA] = "sms1xxx-hcw-55xxx-dvbt-02.fw",
		.default_mode = SMSHOSTLIB_DEVMD_DVBT_BDA,
		.lna_ctrl  = 29,
		.board_cfg.foreign_lna0_ctrl = 29,
		.rf_switch = 17,
		.board_cfg.rf_switch_uhf = 17,
	},
	[SMS1XXX_BOARD_HAUPPAUGE_TIGER_MINICARD_R2] = {
	/* 10 */
		.name = "Hauppauge WinTV MiniCard",
		.type = SMS_NOVA_B0,
		.fw[SMSHOSTLIB_DEVMD_DVBT_BDA] = "sms1xxx-hcw-55xxx-dvbt-02.fw",
		.default_mode = SMSHOSTLIB_DEVMD_DVBT_BDA,
		.board_cfg.foreign_lna0_ctrl = 1,
		.lna_ctrl  = -1,
	},
	[SMS1XXX_BOARD_SIANO_NICE] = {
	/* 11 */
		.name = "Siano Nice Digital Receiver",
		.type = SMS_NOVA_B0,
		.default_mode = SMSHOSTLIB_DEVMD_DVBT_BDA,
	},
	[SMS1XXX_BOARD_SIANO_VENICE] = {
	/* 12 */
		.name = "Siano Venice Digital Receiver",
		.type = SMS_VENICE,
		.default_mode = SMSHOSTLIB_DEVMD_CMMB,
	},
	[SMS1XXX_BOARD_SIANO_STELLAR_ROM] = {
	/* 13 */
		.name =
		"Siano Stellar Digital Receiver ROM",
		.type = SMS_STELLAR,
		.default_mode = SMSHOSTLIB_DEVMD_DVBT_BDA,
		.intf_num = 1,
	},
	[SMS1XXX_BOARD_ZTE_DVB_DATA_CARD] = {
	/* 14 */
		.name = "ZTE Data Card Digital Receiver",
		.type = SMS_NOVA_B0,
		.default_mode = SMSHOSTLIB_DEVMD_DVBT_BDA,
		.intf_num = 5,
		.mtu = 15792,
	},
	[SMS1XXX_BOARD_ONDA_MDTV_DATA_CARD] = {
	/* 15 */
		.name = "ONDA Data Card Digital Receiver",
		.type = SMS_NOVA_B0,
		.default_mode = SMSHOSTLIB_DEVMD_DVBT_BDA,
		.intf_num = 6,
		.mtu = 15792,
	},
	[SMS1XXX_BOARD_SIANO_MING] = {
	/* 16 */
		.name = "Siano Ming Digital Receiver",
		.type = SMS_MING,
		.default_mode = SMSHOSTLIB_DEVMD_CMMB,
	},
	[SMS1XXX_BOARD_SIANO_PELE] = {
	/* 17 */
		.name = "Siano Pele Digital Receiver",
		.type = SMS_PELE,
		.default_mode = SMSHOSTLIB_DEVMD_ISDBT_BDA,
	},
	[SMS1XXX_BOARD_SIANO_RIO] = {
	/* 18 */
		.name = "Siano Rio Digital Receiver",
		.type = SMS_RIO,
		.default_mode = SMSHOSTLIB_DEVMD_ISDBT_BDA,
	},
	[SMS1XXX_BOARD_SIANO_DENVER_1530] = {
    /* 19 */
        .name = "Siano Denver (ATSC-M/H) Digital Receiver",
        .type = SMS_DENVER_1530,
        .default_mode = SMSHOSTLIB_DEVMD_ATSC,
	.crystal = 2400,
    },
    [SMS1XXX_BOARD_SIANO_DENVER_2160] = {
    /* 20 */
        .name = "Siano Denver (TDMB) Digital Receiver",
        .type = SMS_DENVER_2160,
        .default_mode = SMSHOSTLIB_DEVMD_DAB_TDMB,
	},
    [SMS1XXX_BOARD_SIANO_QING] = {
    /* 21 */
	.name = "Siano Qing Digital Receiver",
	.type = SMS_QING,
	.default_mode = SMSHOSTLIB_DEVMD_CMMB,
    },
    [SMS1XXX_BOARD_SIANO_ZICO] = {
    /* 22 */
	.name = "Siano Zico Digital Receiver",
	.type = SMS_ZICO,
	.default_mode = SMSHOSTLIB_DEVMD_ISDBT,
    },
    [SMS1XXX_BOARD_SIANO_SANTOS] = {
    /* 23 */
	.name = "Siano Santos Digital Receiver",
	.type = SMS_SANTOS,
	.default_mode = SMSHOSTLIB_DEVMD_ISDBT,
    },

};

struct sms_board *sms_get_board(unsigned id)
{
	BUG_ON(id >= ARRAY_SIZE(sms_boards));

	return &sms_boards[id];
}

static inline void sms_gpio_assign_11xx_default_led_config(
		struct smscore_gpio_config *pGpioConfig) {
	pGpioConfig->direction = SMS_GPIO_DIRECTION_OUTPUT;
	pGpioConfig->input_characteristics = SMS_GPIO_INPUTCHARACTERISTICS_NORMAL;
	pGpioConfig->output_driving = SMS_GPIO_OUTPUTDRIVING_4mA;
	pGpioConfig->output_slew_rate = SMS_GPIO_OUTPUTSLEWRATE_0_45_V_NS;
	pGpioConfig->pull_up_down = SMS_GPIO_PULLUPDOWN_NONE;
}

int sms_board_event(void *coredev,
		enum SMS_BOARD_EVENTS gevent) {

	struct sms_board *board;
	struct smscore_gpio_config MyGpioConfig;
	struct sms_properties_t properties;

	smscore_get_device_properties(coredev, &properties);
	board = sms_get_board(properties.board_id);

	sms_gpio_assign_11xx_default_led_config(&MyGpioConfig);

	switch (gevent) {
	case BOARD_EVENT_POWER_INIT: /* including hotplug */
		switch (properties.board_id) {
		case SMS1XXX_BOARD_HAUPPAUGE_WINDHAM:
			/* set I/O and turn off all LEDs */
			smscore_gpio_configure(coredev,
					board->board_cfg.leds_power,
					&MyGpioConfig);
			smscore_gpio_set_level(coredev,
					board->board_cfg.leds_power, 0);
			smscore_gpio_configure(coredev, board->board_cfg.led0,
					&MyGpioConfig);
			smscore_gpio_set_level(coredev,
					board->board_cfg.led0, 0);
			smscore_gpio_configure(coredev, board->board_cfg.led1,
					&MyGpioConfig);
			smscore_gpio_set_level(coredev,
					board->board_cfg.led1, 0);
			break;
		case SMS1XXX_BOARD_HAUPPAUGE_TIGER_MINICARD_R2:
		case SMS1XXX_BOARD_HAUPPAUGE_TIGER_MINICARD:
			/* set I/O and turn off LNA */
			smscore_gpio_configure(coredev,
					board->board_cfg.foreign_lna0_ctrl,
					&MyGpioConfig);
			smscore_gpio_set_level(coredev,
					board->board_cfg.foreign_lna0_ctrl,
					0);
			break;
		}
		break; /* BOARD_EVENT_BIND */

	case BOARD_EVENT_POWER_SUSPEND:
		switch (properties.board_id) {
		case SMS1XXX_BOARD_HAUPPAUGE_WINDHAM:
			smscore_gpio_set_level(coredev,
						board->board_cfg.leds_power, 0);
			smscore_gpio_set_level(coredev,
						board->board_cfg.led0, 0);
			smscore_gpio_set_level(coredev,
						board->board_cfg.led1, 0);
			break;
		case SMS1XXX_BOARD_HAUPPAUGE_TIGER_MINICARD_R2:
		case SMS1XXX_BOARD_HAUPPAUGE_TIGER_MINICARD:
			smscore_gpio_set_level(coredev,
					board->board_cfg.foreign_lna0_ctrl,
					0);
			break;
		}
		break; /* BOARD_EVENT_POWER_SUSPEND */

	case BOARD_EVENT_POWER_RESUME:
		switch (properties.board_id) {
		case SMS1XXX_BOARD_HAUPPAUGE_WINDHAM:
			smscore_gpio_set_level(coredev,
						board->board_cfg.leds_power, 1);
			smscore_gpio_set_level(coredev,
						board->board_cfg.led0, 1);
			smscore_gpio_set_level(coredev,
						board->board_cfg.led1, 0);
			break;
		case SMS1XXX_BOARD_HAUPPAUGE_TIGER_MINICARD_R2:
		case SMS1XXX_BOARD_HAUPPAUGE_TIGER_MINICARD:
			smscore_gpio_set_level(coredev,
					board->board_cfg.foreign_lna0_ctrl,
					1);
			break;
		}
		break; /* BOARD_EVENT_POWER_RESUME */

	case BOARD_EVENT_BIND:
		switch (properties.board_id) {
		case SMS1XXX_BOARD_HAUPPAUGE_WINDHAM:
			smscore_gpio_set_level(coredev,
				board->board_cfg.leds_power, 1);
			smscore_gpio_set_level(coredev,
				board->board_cfg.led0, 1);
			smscore_gpio_set_level(coredev,
				board->board_cfg.led1, 0);
			break;
		case SMS1XXX_BOARD_HAUPPAUGE_TIGER_MINICARD_R2:
		case SMS1XXX_BOARD_HAUPPAUGE_TIGER_MINICARD:
			smscore_gpio_set_level(coredev,
					board->board_cfg.foreign_lna0_ctrl,
					1);
			break;
		}
		break; /* BOARD_EVENT_BIND */

	case BOARD_EVENT_SCAN_PROG:
		break; /* BOARD_EVENT_SCAN_PROG */
	case BOARD_EVENT_SCAN_COMP:
		break; /* BOARD_EVENT_SCAN_COMP */
	case BOARD_EVENT_EMERGENCY_WARNING_SIGNAL:
		break; /* BOARD_EVENT_EMERGENCY_WARNING_SIGNAL */
	case BOARD_EVENT_FE_LOCK:
		switch (properties.board_id) {
		case SMS1XXX_BOARD_HAUPPAUGE_WINDHAM:
			smscore_gpio_set_level(coredev,
			board->board_cfg.led1, 1);
			break;
		}
		break; /* BOARD_EVENT_FE_LOCK */
	case BOARD_EVENT_FE_UNLOCK:
		switch (properties.board_id) {
		case SMS1XXX_BOARD_HAUPPAUGE_WINDHAM:
			smscore_gpio_set_level(coredev,
						board->board_cfg.led1, 0);
			break;
		}
		break; /* BOARD_EVENT_FE_UNLOCK */
	case BOARD_EVENT_DEMOD_LOCK:
		break; /* BOARD_EVENT_DEMOD_LOCK */
	case BOARD_EVENT_DEMOD_UNLOCK:
		break; /* BOARD_EVENT_DEMOD_UNLOCK */
	case BOARD_EVENT_RECEPTION_MAX_4:
		break; /* BOARD_EVENT_RECEPTION_MAX_4 */
	case BOARD_EVENT_RECEPTION_3:
		break; /* BOARD_EVENT_RECEPTION_3 */
	case BOARD_EVENT_RECEPTION_2:
		break; /* BOARD_EVENT_RECEPTION_2 */
	case BOARD_EVENT_RECEPTION_1:
		break; /* BOARD_EVENT_RECEPTION_1 */
	case BOARD_EVENT_RECEPTION_LOST_0:
		break; /* BOARD_EVENT_RECEPTION_LOST_0 */
	case BOARD_EVENT_MULTIPLEX_OK:
		break; /* BOARD_EVENT_MULTIPLEX_OK */
	case BOARD_EVENT_MULTIPLEX_ERRORS:
		break; /* BOARD_EVENT_MULTIPLEX_ERRORS */

	default:
		sms_err("Unknown SMS board event");
		break;
	}
	return 0;
}

static int sms_set_gpio(void *coredev, int pin, int enable)
{
	int lvl, ret;
	u32 gpio;
	struct smscore_gpio_config gpioconfig = {
		.direction            = SMS_GPIO_DIRECTION_OUTPUT,
		.pull_up_down           = SMS_GPIO_PULLUPDOWN_NONE,
		.input_characteristics = SMS_GPIO_INPUTCHARACTERISTICS_NORMAL,
		.output_slew_rate       = SMS_GPIO_OUTPUTSLEWRATE_FAST,
		.output_driving        = SMS_GPIO_OUTPUTDRIVING_4mA,
	};

	if (pin == 0)
		return -EINVAL;

	if (pin < 0) {
		/* inverted gpio */
		gpio = pin * -1;
		lvl = enable ? 0 : 1;
	} else {
		gpio = pin;
		lvl = enable ? 1 : 0;
	}

	ret = smscore_gpio_configure(coredev, gpio, &gpioconfig);
	if (ret < 0)
		return ret;

	return smscore_gpio_set_level(coredev, gpio, lvl);
}

int sms_board_setup(void *coredev)
{
	struct sms_properties_t properties;

	struct sms_board *board;

	smscore_get_device_properties(coredev, &properties);
	board = sms_get_board(properties.board_id);

	switch (properties.board_id) {
	case SMS1XXX_BOARD_HAUPPAUGE_WINDHAM:
		smscore_gpio_set_level(coredev, board->board_cfg.led1, 1);
		/* turn off all LEDs */
		sms_set_gpio(coredev, board->led_power, 0);
		sms_set_gpio(coredev, board->led_hi, 0);
		sms_set_gpio(coredev, board->led_lo, 0);
		break;
	case SMS1XXX_BOARD_HAUPPAUGE_TIGER_MINICARD_R2:
	case SMS1XXX_BOARD_HAUPPAUGE_TIGER_MINICARD:
		/* turn off LNA */
		sms_set_gpio(coredev, board->lna_ctrl, 0);
		break;
	}
	return 0;
}


int sms_board_power(void *coredev, int onoff)
{
	struct sms_properties_t properties;
	struct sms_board *board;

	smscore_get_device_properties(coredev, &properties);
	board = sms_get_board(properties.board_id);

	switch (properties.board_id) {
	case SMS1XXX_BOARD_HAUPPAUGE_WINDHAM:
		smscore_gpio_set_level(coredev,	board->board_cfg.led1, 0);
		/* power LED */
		sms_set_gpio(coredev,
			     board->led_power, onoff ? 1 : 0);
		break;
	case SMS1XXX_BOARD_HAUPPAUGE_TIGER_MINICARD_R2:
	case SMS1XXX_BOARD_HAUPPAUGE_TIGER_MINICARD:
		/* LNA */
		if (!onoff)
			sms_set_gpio(coredev, board->lna_ctrl, 0);
		break;
	}
	return 0;
}


int sms_board_led_feedback(void *coredev, int led)
{
	struct sms_properties_t properties;
	struct sms_board *board;

	smscore_get_device_properties(coredev, &properties);
	board = sms_get_board(properties.board_id);

	/* dont touch GPIO if LEDs are already set */
	if (smscore_led_state(coredev, -1) == led)
		return 0;

	switch (properties.board_id) {
	case SMS1XXX_BOARD_HAUPPAUGE_WINDHAM:
		sms_set_gpio(coredev,
			     board->led_lo, (led & SMS_LED_LO) ? 1 : 0);
		sms_set_gpio(coredev,
			     board->led_hi, (led & SMS_LED_HI) ? 1 : 0);

		smscore_led_state(coredev, led);
		break;
	}
	return 0;
}


int sms_board_lna_control(void *coredev, int onoff)
{
	struct sms_properties_t properties;
	struct sms_board *board;

	smscore_get_device_properties(coredev, &properties);
	board = sms_get_board(properties.board_id);

	sms_debug("%s: LNA %s", __func__, onoff ? "enabled" : "disabled");

	switch (properties.board_id) {
	case SMS1XXX_BOARD_HAUPPAUGE_TIGER_MINICARD_R2:
	case SMS1XXX_BOARD_HAUPPAUGE_TIGER_MINICARD:
		sms_set_gpio(coredev,
			     board->rf_switch, onoff ? 1 : 0);
		return sms_set_gpio(coredev,
				    board->lna_ctrl, onoff ? 1 : 0);
	}
	return -EINVAL;
}


int sms_board_load_modules(int id)
{
	/* Siano smsmdtv loads all other supported "client" modules*/
	return 0;
}

