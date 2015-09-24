/*
 *  Copyright (C) 2012, Samsung Electronics Co. Ltd. All Rights Reserved.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/input.h>
#include <linux/input/mt.h>
#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <asm/unaligned.h>
#include <linux/firmware.h>
#include <linux/string.h>
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif
#if defined(CONFIG_PM_RUNTIME)
#include <linux/pm_runtime.h>
#endif

#include <linux/i2c/mxts.h>

#include <linux/regulator/consumer.h>

//static int mxt_power_onoff(struct mxt_data *data, bool enabled);
static int mxt_make_highchg(struct mxt_data *data);

#define TSP_BRING_UP

#if ENABLE_TOUCH_KEY
int tsp_keycodes[NUMOFKEYS] = {
	KEY_MENU,
	KEY_BACK,
};
char *tsp_keyname[NUMOFKEYS] = {
	"Menu",
	"Back",
};
static u16 tsp_keystatus;
#endif

//static u8 power_idle_value;
//static u8 power_active_value;

//static int calibrate_chip(struct mxt_data *data);



static int tsp_request_gpio(struct mxt_data *data)
{
	int ret;
	printk(KERN_INFO "%s\n", __func__);
/*
	ret = gpio_request(data->pdata->tsp_en, "tsp,tsppwr_en");
	if (ret) {
		pr_err("%s: unable to request tsppwer_en [%d]\n",
				__func__, data->pdata->tsp_en);
		return ret;
	}
	gpio_tlmm_config(GPIO_CFG(data->pdata->tsp_en, 0,
		GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), 1);

	ret = gpio_request(data->pdata->tsp_en1, "tsp,tsppwr_en1");
	if (ret) {
		pr_err("%s: unable to request tsppwer_en1 [%d]\n",
				__func__, data->pdata->tsp_en1);
		return ret;
	}
	gpio_tlmm_config(GPIO_CFG(data->pdata->tsp_en1, 0,
		GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), 1);
*/
	ret = gpio_request(data->pdata->tsp_int, "tsp,tsp_int");
	if (ret) {
		pr_err("%s: unable to request tsp_int [%d]\n",
				__func__, data->pdata->tsp_int);
		return ret;
	}
	ret = gpio_direction_input(data->pdata->tsp_int);
//	ret = gpio_tlmm_config(GPIO_CFG(data->pdata->tsp_int,
//				0, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL,
//					GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	if (ret < 0)
		dev_info(&data->client->dev,
				"%s: wakeup int config return: %d\n",
				__func__, ret);

	ret = data->pdata->power_onoff(true);
	if (ret) {
		printk("mxts: Failed to power on.\n");
		return -EINVAL;
	}

// when 50khz
	msleep(100);

	return ret;
}

#if 0
static int mxt_power_onoff(struct mxt_data *data, bool enable)
{
	int ret = 0;

	dev_err(&data->client->dev,
			"%s enable(%d)\n", __func__, enable);
	ret = gpio_direction_output(data->pdata->tsp_en, enable);
	if (ret) {
		dev_err(&data->client->dev,
			"[TKEY]%s: unable to set_direction for vdd_led [%d]\n",
			 __func__, data->pdata->tsp_en);
		return -EINVAL;
	}
	ret = gpio_direction_output(data->pdata->tsp_en1, enable);
	if (ret) {
		dev_err(&data->client->dev,
			"[TKEY]%s: unable to set_direction for vdd_led [%d]\n",
			 __func__, data->pdata->tsp_en1);
		return -EINVAL;
	}

	msleep(30);
#if 0
	ret = gpio_direction_output(data->pdata->tsp_rst, enable);
	if (ret) {
		dev_err(&data->client->dev,
			"%s: unable to set_direction for tsp_rst [%d]\n",
			 __func__, data->pdata->tsp_en1);
		return -EINVAL;
	}
#endif
	msleep(100);

	return 0;
}
#endif
static int mxt_read_mem(struct mxt_data *data, u16 reg, u8 len, void *buf)
{
	int ret = 0, i = 0;
	u16 le_reg = cpu_to_le16(reg);
	struct i2c_msg msg[2] = {
		{
			.addr = data->client->addr,
			.flags = 0,
			.len = 2,
			.buf = (u8 *)&le_reg,
		},
		{
			.addr = data->client->addr,
			.flags = I2C_M_RD,
			.len = len,
			.buf = buf,
		},
	};

#if TSP_USE_ATMELDBG
	if (data->atmeldbg.block_access)
		return 0;
#endif

	for (i = 0; i < 3 ; i++) {
		ret = i2c_transfer(data->client->adapter, &msg[0], 1);
		if (ret < 0)
			dev_err(&data->client->dev, "%s fail[%d] address[0x%x]\n",
					__func__, ret, le_reg);
		else
			break;
	}
	udelay(4);
	for (i = 0; i < 3 ; i++) {
		ret = i2c_transfer(data->client->adapter, &msg[1], 1);
		if (ret < 0)
			dev_err(&data->client->dev, "%s fail[%d] address[0x%x]\n",
					__func__, ret, le_reg);
		else
			break;
	}

	return (ret == 1) ? 0 : -EIO;
}

static int mxt_write_mem(struct mxt_data *data, u16 reg, u8 len, const u8 *buf)
{
	int ret = 0, i = 0;
	u8 tmp[len + 2];

#if TSP_USE_ATMELDBG
	if (data->atmeldbg.block_access)
		return 0;
#endif

	put_unaligned_le16(cpu_to_le16(reg), tmp);
	memcpy(tmp + 2, buf, len);

	for (i = 0; i < 3 ; i++) {
		ret = i2c_master_send(data->client, tmp, sizeof(tmp));
		if (ret < 0)
			dev_err(&data->client->dev,
					"%s %d times write error on address[0x%x,0x%x]\n",
					__func__, i, tmp[1], tmp[0]);
		else
			break;
	}

	return (ret == sizeof(tmp)) ? 0 : -EIO;
}

static struct mxt_object *mxt_get_object(struct mxt_data *data, u8 type)
{
	struct mxt_object *object;
	int i;

	if (!data->objects)
		return NULL;

	for (i = 0; i < data->info.object_num; i++) {
		object = data->objects + i;
		if (object->type == type)
			return object;
	}

	dev_err(&data->client->dev, "Invalid object type T%d\n", type);

	return NULL;
}

static int mxt_read_message(struct mxt_data *data, struct mxt_message *message)
{
	struct mxt_object *object;

	object = mxt_get_object(data, MXT_GEN_MESSAGEPROCESSOR_T5);
	if (!object)
		return -EINVAL;

	return mxt_read_mem(data, object->start_address,
			sizeof(struct mxt_message), message);
}

static int mxt_read_message_reportid(struct mxt_data *data,
				struct mxt_message *message, u8 reportid)
{
	int try = 0;
	int error;
	int fail_count;

	fail_count = data->max_reportid * 2;

	while (++try < fail_count) {
		error = mxt_read_message(data, message);
		if (error)
			return error;

		if (message->reportid == 0xff)
			continue;

		if (message->reportid == reportid)
			return 0;
	}

	return -EINVAL;
}

static int mxt_read_object(struct mxt_data *data, u8 type, u8 offset, u8 *val)
{
	struct mxt_object *object;
	int error = 0;

	object = mxt_get_object(data, type);
	if (!object)
		return -EINVAL;

	error = mxt_read_mem(data, object->start_address + offset, 1, val);
	if (error)
		dev_err(&data->client->dev, "Error to read T[%d] offset[%d] val[%d]\n",
				type, offset, *val);

	return error;
}

static int mxt_write_object(struct mxt_data *data,
				 u8 type, u8 offset, u8 val)
{
	struct mxt_object *object;
	int error = 0;
	u16 reg;

	object = mxt_get_object(data, type);
	if (!object)
		return -EINVAL;

	if (offset >= object->size * object->instances) {
		dev_err(&data->client->dev,
				"Tried to write outside object T%d offset:%d, size:%d\n",
				type, offset, object->size);

		return -EINVAL;
	}

	reg = object->start_address;
	error = mxt_write_mem(data, reg + offset, 1, &val);
	if (error)
		dev_err(&data->client->dev,
				"Error to write T[%d] offset[%d] val[%d]\n",
				type, offset, val);
	return error;
}

static u32 mxt_make_crc24(u32 crc, u8 byte1, u8 byte2)
{
	static const u32 crcpoly = 0x80001B;
	u32 res;
	u16 data_word;

	data_word = (((u16)byte2) << 8) | byte1;
	res = (crc << 1) ^ (u32)data_word;

	if (res & 0x1000000)
		res ^= crcpoly;

	return res;
}

static int mxt_calculate_infoblock_crc(struct mxt_data *data, u32 *crc_pointer)
{
	u32 crc = 0;
	u8 mem[7 + data->info.object_num * 6];
	int ret;
	int i,j;


    for (j=0; j < 20 ; j ++)
	{
		ret = mxt_read_mem(data, 0, sizeof(mem), mem);
		crc =0;
		for (i = 0; i < sizeof(mem) - 1; i += 2)
		{
#if TSP_USE_ATMELDBG
			dev_info(&data->client->dev,
					"read info for CRC i=%d i+1=%d : %d %d\n",i,i+1, mem[i], mem[i + 1]);
#endif
			crc = mxt_make_crc24(crc, mem[i], mem[i + 1]);
		}

		if ( mem[0] != 255 )
		{
			break;
		}
	}
	*crc_pointer = mxt_make_crc24(crc, mem[i], 0) & 0x00FFFFFF;

	return 0;
}

static int mxt_read_info_crc(struct mxt_data *data, u32 *crc_pointer)
{
	u16 crc_address;
	u8 msg[3];
	int ret;

	/* Read Info block CRC address */
	crc_address = MXT_OBJECT_TABLE_START_ADDRESS +
			data->info.object_num * MXT_OBJECT_TABLE_ELEMENT_SIZE;

	ret = mxt_read_mem(data, crc_address, 3, msg);
	if (ret)
		return ret;

	*crc_pointer = msg[0] | (msg[1] << 8) | (msg[2] << 16);

	return 0;
}
static int mxt_read_config_crc(struct mxt_data *data, u32 *crc)
{
	struct device *dev = &data->client->dev;
	struct mxt_message message;
	struct mxt_object *object;
	int error;

	object = mxt_get_object(data, MXT_GEN_COMMANDPROCESSOR_T6);
	if (!object)
		return -EIO;

	/* Try to read the config checksum of the existing cfg */
	mxt_write_object(data, MXT_GEN_COMMANDPROCESSOR_T6,
			MXT_COMMAND_REPORTALL, 1);

	/* Read message from command processor, which only has one report ID */
	error = mxt_read_message_reportid(data, &message, object->max_reportid);
	if (error) {
		dev_err(dev, "Failed to retrieve CRC\n");
		return error;
	}

	/* Bytes 1-3 are the checksum. */
	*crc = message.message[1] | (message.message[2] << 8) |
		(message.message[3] << 16);

	return 0;
}

static int mxt_check_instance(struct mxt_data *data, u8 type)
{
	int i;

	for (i = 0; i < data->info.object_num; i++) {
		if (data->objects[i].type == type)
			return data->objects[i].instances;
	}

	return 0;
}

static int mxt_init_write_config(struct mxt_data *data, u8 type, const u8 *cfg)
{
	struct mxt_object *object;
	u8 *temp;
	int ret;

	object = mxt_get_object(data, type);
	if (!object)
		return -EINVAL;

	if ((object->size == 0) || (object->start_address == 0)) {
		dev_err(&data->client->dev,	"%s error T%d\n",
			 __func__, type);
		return -ENODEV;
	}

	ret = mxt_write_mem(data, object->start_address, object->size, cfg);
	if (ret) {
		dev_err(&data->client->dev,	"%s write error T%d address[0x%x]\n",
			__func__, type, object->start_address);
		return ret;
	}

	if (mxt_check_instance(data, type)) {
		temp = kzalloc(object->size, GFP_KERNEL);

		if (temp == NULL)
			return -ENOMEM;

		ret |= mxt_write_mem(data, object->start_address + object->size,
			object->size, temp);
		kfree(temp);
	}

	return ret;
}

static int mxt_write_config_from_pdata(struct mxt_data *data)
{
	struct device *dev = &data->client->dev;
	u8 **tsp_config = (u8 **)data->pdata->config;
	u8 i;
	int ret = 0;

	if (!tsp_config) {
		dev_info(dev, "No cfg data in pdata\n");
		return 0;
	}

	for (i = 0; tsp_config[i][0] != MXT_RESERVED_T255; i++) {
		ret = mxt_init_write_config(data, tsp_config[i][0],
						tsp_config[i] + 1);
		if (ret)
			return ret;
	}

	return ret;
}
#if 1
static int mxt_write_config(struct mxt_fw_info *fw_info)
{
	struct mxt_data *data = fw_info->data;
	struct device *dev = &data->client->dev;
	struct mxt_object *object;
	struct mxt_cfg_data *cfg_data;
	u32 current_crc;
	u8 i, val = 0;
	u16 reg, index;
	int ret;

	if (!fw_info->cfg_raw_data) {
		dev_info(dev, "No cfg data in file\n");
		ret = mxt_write_config_from_pdata(data);
		return ret;
	}

	/* Get config CRC from device */
	ret = mxt_read_config_crc(data, &current_crc);
	if (ret)
		return ret;

	/* Check Version information */
	if (fw_info->fw_ver != data->info.version) {
		dev_err(dev, "Warning: version mismatch! %s\n", __func__);
		return 0;
	}
	if (fw_info->build_ver != data->info.build) {
		dev_err(dev, "Warning: build num mismatch! %s\n", __func__);
		return 0;
	}

	/* Check config CRC */
	if (current_crc == fw_info->cfg_crc) {
		dev_info(dev, "Skip writing Config:[CRC 0x%06X]\n", current_crc);
		return 0;
	}

	dev_info(dev, "Writing Config:[CRC 0x%06X!=0x%06X]\n", current_crc,
			fw_info->cfg_crc);

	/* Write config info */
	for (index = 0; index < fw_info->cfg_len;) {

		if (index + sizeof(struct mxt_cfg_data) >= fw_info->cfg_len) {
			dev_err(dev, "index(%d) of cfg_data exceeded total size(%d)!!\n",
					index + sizeof(struct mxt_cfg_data), fw_info->cfg_len);
			return -EINVAL;
		}

		/* Get the info about each object */
		cfg_data = (struct mxt_cfg_data *)
					(&fw_info->cfg_raw_data[index]);

		index += sizeof(struct mxt_cfg_data) + cfg_data->size;
		if (index > fw_info->cfg_len) {
			dev_err(dev, "index(%d) of cfg_data exceeded total size(%d)" \
					" in T%d object!!\n", index, fw_info->cfg_len,
					cfg_data->type);
			return -EINVAL;
		}

		object = mxt_get_object(data, cfg_data->type);
		if (!object) {
			dev_err(dev, "T%d is Invalid object type\n", cfg_data->type);
			return -EINVAL;
		}

		/* Check and compare the size, instance of each object */
		if (cfg_data->size > object->size) {
			dev_err(dev, "T%d Object length exceeded!\n", cfg_data->type);
			return -EINVAL;
		}
		if (cfg_data->instance >= object->instances) {
			dev_err(dev, "T%d Object instances exceeded!\n", cfg_data->type);
			return -EINVAL;
		}

#if TSP_USE_ATMELDBG
		dev_err(dev, "Writing config for obj %d len %d instance %d (%d/%d)\n",
				cfg_data->type, object->size,
				cfg_data->instance, index, fw_info->cfg_len);
#endif

		reg = object->start_address + object->size * cfg_data->instance;

		/* Write register values of each object */
		ret = mxt_write_mem(data, reg, cfg_data->size, cfg_data->register_val);
		if (ret) {
			dev_err(dev, "Write T%d Object failed\n", object->type);

			return ret;
		}

		/*
		 * If firmware is upgraded, new bytes may be added to end of
		 * objects. It is generally forward compatible to zero these
		 * bytes - previous behaviour will be retained. However
		 * this does invalidate the CRC and will force a config
		 * download every time until the configuration is updated.
		 */
		if (cfg_data->size < object->size) {
			dev_err(dev, "Warning: zeroing %d byte(s) in T%d\n",
					object->size - cfg_data->size, cfg_data->type);

			for (i = cfg_data->size + 1; i < object->size; i++) {
				ret = mxt_write_mem(data, reg + i, 1, &val);
				if (ret)
					return ret;
			}
		}
	}

	dev_info(dev, "Updated configuration\n");

	return ret;
}
#endif
#if TSP_PATCH
#include "mxts_patch.c"
#endif

#if TSP_INFORM_CHARGER
extern void mxt_tsp_register_callback(struct mxts_callback *cb);

static void set_charger_config(struct mxt_data *data)
{
	struct device *dev = &data->client->dev;
//	struct mxt_object *object;
//	struct mxt_cfg_data *cfg_data;
//	u8 i = 0;
//	u16 reg;

	dev_info(&data->client->dev, "Current state is %s",
		data->charging_mode ? "Charging mode" : "Battery mode");

/* if you need to change configuration depend on chager detection,
 * please insert below line.
 */

	dev_dbg(dev, "set_charger_config data->cfg_len = %d\n", data->cfg_len);

#if TSP_PATCH
	if (data->charging_mode) {
		if (data->patch.event_cnt)
			mxt_patch_test_event(data, 1);
	} else {
		if (data->patch.event_cnt)
			mxt_patch_test_event(data, 0);
	}
#endif
}

static void inform_charger(struct mxts_callback *cb, bool en)
{
	struct mxt_data *data = container_of(cb, struct mxt_data, callback);

	cancel_delayed_work_sync(&data->noti_dwork);
	data->charging_mode = en;
	schedule_delayed_work(&data->noti_dwork, HZ / 5);
}

static void charger_noti_dwork(struct work_struct *work)
{
	struct mxt_data *data = container_of(work, struct mxt_data,
								noti_dwork.work);
	if (!data->mxt_enabled) {
		dev_info(&data->client->dev, "delaye work !!!\n");
		schedule_delayed_work(&data->noti_dwork, HZ / 5);
		return ;
	}

	dev_info(&data->client->dev, "%s mode\n",
		data->charging_mode ? "charging" : "battery");

	set_charger_config(data);
}

static void inform_charger_init(struct mxt_data *data)
{
	INIT_DELAYED_WORK(&data->noti_dwork, charger_noti_dwork);
}
#endif

static void mxt_report_input_data(struct mxt_data *data)
{
	int i;
	int count = 0;
	int report_count = 0;

	for (i = 0; i < MXT_MAX_FINGER; i++) {
		if (data->fingers[i].state == MXT_STATE_INACTIVE)
			continue;

		input_mt_slot(data->input_dev, i);
		if (data->fingers[i].state == MXT_STATE_RELEASE) {
			input_mt_report_slot_state(data->input_dev, MT_TOOL_FINGER, false);
		} else {
			input_mt_report_slot_state(data->input_dev, MT_TOOL_FINGER, true);
			input_report_abs(data->input_dev, ABS_MT_POSITION_X, data->fingers[i].x);
			input_report_abs(data->input_dev, ABS_MT_POSITION_Y, data->fingers[i].y);
			input_report_abs(data->input_dev, ABS_MT_TOUCH_MAJOR, data->fingers[i].w);
			input_report_abs(data->input_dev, ABS_MT_PRESSURE, data->fingers[i].z);
#if TSP_USE_SHAPETOUCH
			/* Currently revision G firmware do not support it */
			if (data->pdata->revision == MXT_REVISION_I) {
				input_report_abs(data->input_dev, ABS_MT_COMPONENT,
					data->fingers[i].component);
				input_report_abs(data->input_dev, ABS_MT_SUMSIZE,
					data->sumsize);
			}
#endif
			input_report_key(data->input_dev, BTN_TOOL_FINGER, 1);

			if (data->fingers[i].type == MXT_T100_TYPE_HOVERING_FINGER)
				/* hover is reported */
				input_report_key(data->input_dev, BTN_TOUCH, 0);
			else
				/* finger or passive stylus are reported */
				input_report_key(data->input_dev, BTN_TOUCH, 1);
		}
		report_count++;

#if !defined(CONFIG_SAMSUNG_PRODUCT_SHIP)
		if (data->fingers[i].state == MXT_STATE_PRESS) {
#if TSP_USE_ATMELDBG
			dev_info(&data->client->dev, "[P][%d]: T[%d][%d] X[%d],Y[%d],W[%d],Z[%d]\n",
				i, data->fingers[i].type,
				data->fingers[i].event,	data->fingers[i].x, data->fingers[i].y,
				data->fingers[i].w, data->fingers[i].z);
#endif
#if TSP_USE_SHAPETOUCH
			pr_cont(",PALM[%d],COMP[%d],SUM[%d],AREA[%d]\n",
				data->palm, data->fingers[i].component,
					data->sumsize, data->fingers[i].w);
#else
			pr_cont("\n");
#endif
		}
#else
		if (data->fingers[i].state == MXT_STATE_PRESS)
			dev_info(&data->client->dev, "[P][%d]: T[%d][%d]\n",
				i, data->fingers[i].type,
				data->fingers[i].event);
#endif
		else if (data->fingers[i].state == MXT_STATE_RELEASE)
			dev_info(&data->client->dev, "[R][%d]: T[%d][%d] M[%d]\n",
				i, data->fingers[i].type,
				data->fingers[i].event,
				data->fingers[i].mcount);

		if (data->fingers[i].state == MXT_STATE_RELEASE) {
			data->fingers[i].state = MXT_STATE_INACTIVE;
			data->fingers[i].mcount = 0;
		} else {
			data->fingers[i].state = MXT_STATE_MOVE;
			count++;
		}
	}

	if (count == 0) {
//		input_report_abs(data->input_dev, ABS_MT_TOUCH_MAJOR, 0);	/* release press */
		input_report_key(data->input_dev, BTN_TOUCH, 0);
		input_report_key(data->input_dev, BTN_TOOL_FINGER, 0);
	}

	if (report_count > 0) {
#if TSP_USE_ATMELDBG
		if (!data->atmeldbg.stop_sync)
#endif
			input_sync(data->input_dev);
	}

#if (TSP_USE_SHAPETOUCH || TSP_BOOSTER)
	/* all fingers are released */
	if (count == 0) {
#if TSP_USE_SHAPETOUCH
		data->sumsize = 0;
#endif
#if TSP_BOOSTER
		mxt_set_dvfs_on(data, false);
#endif
	}
#endif

	data->finger_mask = 0;
}

static void mxt_release_all_finger(struct mxt_data *data)
{
	int i;
	int count = 0;

	for (i = 0; i < MXT_MAX_FINGER; i++) {
		if (data->fingers[i].state == MXT_STATE_INACTIVE)
			continue;
		data->fingers[i].z = 0;
		data->fingers[i].state = MXT_STATE_RELEASE;
		count++;
	}

	if (count) {
		dev_err(&data->client->dev, "%s\n", __func__);
		mxt_report_input_data(data);
	}
}

#if TSP_HOVER_WORKAROUND
static void mxt_current_calibration(struct mxt_data *data)
{
	dev_info(&data->client->dev, "%s\n", __func__);

	mxt_write_object(data, MXT_SPT_SELFCAPHOVERCTECONFIG_T102, 1, 1);
}
#endif


static void mxt_treat_T6_object(struct mxt_data *data,
		struct mxt_message *message)
{
	/* Normal mode */
	if (message->message[0] == 0x00) {
		dev_info(&data->client->dev, "Normal mode\n");
	if (data->cal_busy)
		data->cal_busy = 0;
#if TSP_HOVER_WORKAROUND
/* TODO HOVER : Below commands should be removed. */
		if (data->pdata->revision == MXT_REVISION_I
			&& data->cur_cal_status) {
			mxt_current_calibration(data);
			data->cur_cal_status = false;
		}
#endif
	}
	/* I2C checksum error */
	if (message->message[0] & 0x04)
		dev_err(&data->client->dev, "I2C checksum error\n");
	/* Config error */
	if (message->message[0] & 0x08)
		dev_err(&data->client->dev, "Config error\n");
	/* Calibration */
	if (message->message[0] & 0x10) {
		dev_info(&data->client->dev, "Calibration is on going !!\n");
    }
	/* Signal error */
	if (message->message[0] & 0x20)
		dev_err(&data->client->dev, "Signal error\n");
	/* Overflow */
	if (message->message[0] & 0x40)
		dev_err(&data->client->dev, "Overflow detected\n");
	/* Reset */
	if (message->message[0] & 0x80) {
		dev_info(&data->client->dev, "Reset is ongoing\n");
#if TSP_INFORM_CHARGER
		if (data->charging_mode)
			set_charger_config(data);
#endif

#if TSP_HOVER_WORKAROUND
/* TODO HOVER : Below commands should be removed.
 * it added just for hover. Current firmware shoud set the acqusition mode
 * with free-run and run current calibration after receive reset command
 * to support hover functionality.
 * it is bug of firmware. and it will be fixed in firmware level.
 */
		if (data->pdata->revision == MXT_REVISION_I) {
			int error = 0;
			u8 value = 0;

			error = mxt_read_object(data,
				MXT_SPT_TOUCHSCREENHOVER_T101, 0, &value);

			if (error) {
				dev_err(&data->client->dev, "Error read hover enable status[%d]\n"
					, error);
			} else {
				if (value)
					data->cur_cal_status = true;
			}
		}
#endif
	}
}

#if ENABLE_TOUCH_KEY
static void mxt_release_all_keys(struct mxt_data *data)
{
	if (tsp_keystatus != TOUCH_KEY_NULL) {
		switch (tsp_keystatus) {
		case TOUCH_KEY_MENU:
			input_report_key(data->input_dev, KEY_MENU,
								KEY_RELEASE);
			break;
		case TOUCH_KEY_BACK:
			input_report_key(data->input_dev, KEY_BACK,
								KEY_RELEASE);
			break;
		default:
			break;
		}
		dev_info(&data->client->dev, "[TSP_KEY] r %s\n",
						tsp_keyname[tsp_keystatus - 1]);
		tsp_keystatus = TOUCH_KEY_NULL;
	}
}


static void mxt_treat_T15_object(struct mxt_data *data,
						struct mxt_message *message)
{
	struct	input_dev *input;
	input = data->input_dev;

	/* single key configuration*/
	if (message->message[MXT_MSG_T15_STATUS] & MXT_MSGB_T15_DETECT) {

		/* defence code, if there is any Pressed key, force release!! */
		if (tsp_keystatus != TOUCH_KEY_NULL)
			mxt_release_all_keys(data);

		switch (message->message[MXT_MSG_T15_KEYSTATE]) {
		case TOUCH_KEY_MENU:
			input_report_key(data->input_dev, KEY_MENU, KEY_PRESS);
			tsp_keystatus = TOUCH_KEY_MENU;
			break;
		case TOUCH_KEY_BACK:
			input_report_key(data->input_dev, KEY_BACK, KEY_PRESS);
			tsp_keystatus = TOUCH_KEY_BACK;
			break;
		default:
			dev_err(&data->client->dev, "[TSP_KEY] abnormal P [%d %d]\n",
				message->message[0], message->message[1]);
			return;
		}

		dev_info(&data->client->dev, "[TSP_KEY] P %s\n",
						tsp_keyname[tsp_keystatus - 1]);
	} else {
		switch (tsp_keystatus) {
		case TOUCH_KEY_MENU:
			input_report_key(data->input_dev, KEY_MENU,
								KEY_RELEASE);
			break;
		case TOUCH_KEY_BACK:
			input_report_key(data->input_dev, KEY_BACK,
								KEY_RELEASE);
			break;
		default:
			dev_err(&data->client->dev, "[TSP_KEY] abnormal R [%d %d]\n",
				message->message[0], message->message[1]);
			return;
		}
		dev_info(&data->client->dev, "[TSP_KEY] R %s\n",
						tsp_keyname[tsp_keystatus - 1]);
		tsp_keystatus = TOUCH_KEY_NULL;
	}
	input_sync(data->input_dev);
	return;
}
#endif

static void mxt_treat_T9_object(struct mxt_data *data,
		struct mxt_message *message)
{
	int id;
	u8 *msg = message->message;

	id = data->reportids[message->reportid].index;

	/* If not a touch event, return */
	if (id >= MXT_MAX_FINGER) {
		dev_err(&data->client->dev, "MAX_FINGER exceeded!\n");
		return;
	}

	if (data->finger_mask & (1U << id)) //1106
		mxt_report_input_data(data); //1106

	if (msg[0] & MXT_RELEASE_MSG_MASK) {
		data->fingers[id].z = 0;
		data->fingers[id].w = msg[4];
		data->fingers[id].state = MXT_STATE_RELEASE;
		mxt_report_input_data(data);//1106
	} else if ((msg[0] & MXT_DETECT_MSG_MASK)
		&& (msg[0] & (MXT_PRESS_MSG_MASK | MXT_MOVE_MSG_MASK))) {
		data->fingers[id].x = (msg[1] << 4) | (msg[3] >> 4);
		data->fingers[id].y = (msg[2] << 4) | (msg[3] & 0xF);
		data->fingers[id].w = msg[4];
		data->fingers[id].z = msg[5];
#if TSP_USE_SHAPETOUCH
		data->fingers[id].component = msg[6];
#endif

		if (data->pdata->max_x < 1024)
			data->fingers[id].x = data->fingers[id].x >> 2;
		if (data->pdata->max_y < 1024)
			data->fingers[id].y = data->fingers[id].y >> 2;

//1106		data->finger_mask |= 1U << id;

		if (msg[0] & MXT_PRESS_MSG_MASK) {
			data->fingers[id].state = MXT_STATE_PRESS;
			data->fingers[id].mcount = 0;
		} else if (msg[0] & MXT_MOVE_MSG_MASK) {
			data->fingers[id].mcount += 1;
		}

#if TSP_BOOSTER
		mxt_set_dvfs_on(data, true);
#endif
	} else if ((msg[0] & MXT_SUPPRESS_MSG_MASK)
		&& (data->fingers[id].state != MXT_STATE_INACTIVE)) {
		data->fingers[id].z = 0;
		data->fingers[id].w = msg[4];
		data->fingers[id].state = MXT_STATE_RELEASE;
//1106		data->finger_mask |= 1U << id;
		mxt_report_input_data(data);//1106
		dev_info(&data->client->dev, "%s: Only Suppress w/o Detect\n",
				__func__);//1106
	} else {
		/* ignore changed amplitude and vector messsage */
		if (!((msg[0] & MXT_DETECT_MSG_MASK)
				&& (msg[0] & MXT_AMPLITUDE_MSG_MASK
				|| msg[0] & MXT_VECTOR_MSG_MASK)))
			dev_err(&data->client->dev, "Unknown state %#02x %#02x\n",
				msg[0], msg[1]);
	}
	data->finger_mask |= 1U << id;//1106
}

static void mxt_treat_T42_object(struct mxt_data *data,
		struct mxt_message *message)
{
	if (message->message[0] & 0x01)	/* Palm Press */
		dev_info(&data->client->dev, "palm touch detected\n");
	else	/* Palm release */
		dev_info(&data->client->dev, "palm touch released\n");
}

static void mxt_treat_T57_object(struct mxt_data *data,
		struct mxt_message *message)
{

#if TSP_USE_SHAPETOUCH
	data->sumsize = message->message[0] + (message->message[1] << 8);
#endif	/* TSP_USE_SHAPETOUCH */

}
#if 0 //cha_temp
static void mxt_treat_T61_object(struct mxt_data *data,
		struct mxt_message *message)
{

}
#endif
static void mxt_treat_T100_object(struct mxt_data *data,
		struct mxt_message *message)
{
	u8 id, index;
	u8 *msg = message->message;
	u8 touch_type = 0, touch_event = 0, touch_detect = 0;

	index = data->reportids[message->reportid].index;

	/* Treate screen messages */
	if (index < MXT_T100_SCREEN_MESSAGE_NUM_RPT_ID) {
		if (index == MXT_T100_SCREEN_MSG_FIRST_RPT_ID)
			/* TODO: Need to be implemeted after fixed protocol
			 * This messages will indicate TCHAREA, ATCHAREA
			 */
			dev_dbg(&data->client->dev, "SCRSTATUS:[%02X] %02X %04X %04X %04X\n",
				 msg[0], msg[1], (msg[3] << 8) | msg[2],
				 (msg[5] << 8) | msg[4],
				 (msg[7] << 8) | msg[6]);
#if TSP_USE_SHAPETOUCH
			data->sumsize = (msg[3] << 8) | msg[2];
#endif	/* TSP_USE_SHAPETOUCH */
		return;
	}

	/* Treate touch status messages */
	id = index - MXT_T100_SCREEN_MESSAGE_NUM_RPT_ID;
	touch_detect = msg[0] >> MXT_T100_DETECT_MSG_MASK;
	touch_type = (msg[0] & 0x70) >> 4;
	touch_event = msg[0] & 0x0F;

	dev_dbg(&data->client->dev, "TCHSTATUS [%d] : DETECT[%d] TYPE[%d] EVENT[%d] %d,%d,%d,%d,%d\n",
		id, touch_detect, touch_type, touch_event,
		msg[1] | (msg[2] << 8),	msg[3] | (msg[4] << 8),
		msg[5], msg[6], msg[7]);

	switch (touch_type)	{
	case MXT_T100_TYPE_FINGER:
	case MXT_T100_TYPE_PASSIVE_STYLUS:
	case MXT_T100_TYPE_HOVERING_FINGER:
		/* There are no touch on the screen */
		if (!touch_detect) {
			if (touch_event == MXT_T100_EVENT_UP
				|| touch_event == MXT_T100_EVENT_SUPPESS) {

				data->fingers[id].w = 0;
				data->fingers[id].z = 0;
				data->fingers[id].state = MXT_STATE_RELEASE;
				data->fingers[id].type = touch_type;
				data->fingers[id].event = touch_event;

				mxt_report_input_data(data);
			} else {
				dev_err(&data->client->dev, "Untreated Undetectd touch : type[%d], event[%d]\n",
					touch_type, touch_event);
			}
			break;
		}

		/* There are touch on the screen */
		if (touch_event == MXT_T100_EVENT_DOWN
			|| touch_event == MXT_T100_EVENT_UNSUPPRESS
			|| touch_event == MXT_T100_EVENT_MOVE
			|| touch_event == MXT_T100_EVENT_NONE) {
#if 0
			data->fingers[id].x = ((msg[1] | (msg[2] << 8)) * 1280)/4095; //  screen_x = 1280, x_max = 4096;
			data->fingers[id].y = ((msg[3] | (msg[4] << 8)) * 800)/4095; //  screen_y = 800, y_max = 4096;
#else
			data->fingers[id].x = ((4095 - (msg[1] | (msg[2] << 8))) * 1280)/4095; //  screen_x = 1280, x_max = 4096;
			data->fingers[id].y = ((msg[3] | (msg[4] << 8)) * 800)/4095; //  screen_y = 800, y_max = 4096;
//			data->fingers[id].y = msg[1] | (msg[2] << 8); //  screen_x = 1280, x_max = 4096;
//			data->fingers[id].x = msg[3] | (msg[4] << 8); //  screen_y = 800, y_max = 4096;

#endif
			/* AUXDATA[n]'s order is depended on which values are
			 * enabled or not.
			 */
#if TSP_USE_SHAPETOUCH
			data->fingers[id].component = msg[5];
#endif
#if 1
			data->fingers[id].z = msg[6];
			data->fingers[id].w = msg[7];
#else
			data->fingers[id].w = 15;
			data->fingers[id].z = 125;
#endif

			if (touch_type == MXT_T100_TYPE_HOVERING_FINGER) {
				data->fingers[id].w = 0;
				data->fingers[id].z = 0;
			}

			if (touch_event == MXT_T100_EVENT_DOWN
				|| touch_event == MXT_T100_EVENT_UNSUPPRESS) {
				data->fingers[id].state = MXT_STATE_PRESS;
				data->fingers[id].mcount = 0;
			} else {
				data->fingers[id].state = MXT_STATE_MOVE;
				data->fingers[id].mcount += 1;
			}
			data->fingers[id].type = touch_type;
			data->fingers[id].event = touch_event;

			mxt_report_input_data(data);
		} else {
			dev_err(&data->client->dev, "Untreated Detectd touch : type[%d], event[%d]\n",
				touch_type, touch_event);
		}
		break;
	case MXT_T100_TYPE_ACTIVE_STYLUS:
		break;
	}
}

static irqreturn_t mxt_irq_thread(int irq, void *ptr)
{
	struct mxt_data *data = ptr;
	struct mxt_message message;
	struct device *dev = &data->client->dev;
	u8 reportid, type;

//	printk("========== preempt_count=%d, in interrupt= %s =================\n",
//		preempt_count(), in_interrupt()?"O":"X");

#if TSP_USE_ATMELDBG
	dev_info(dev, "mxt_irq_thread start\n");
#endif

	do {
		if (mxt_read_message(data, &message)) {
			dev_err(dev, "Failed to read message\n");
			goto end;
		}

#if TSP_USE_ATMELDBG
	//if (data->atmeldbg.display_log) {
			print_hex_dump(KERN_INFO, "MXT MSG:",
				DUMP_PREFIX_NONE, 16, 1,
				&message,
				sizeof(struct mxt_message), false);
	//}
#endif
		reportid = message.reportid;

		if (reportid > data->max_reportid){
			dev_err(dev, "mxt_irq_thread reportid %d Failed (max %d) \n",
				reportid, data->max_reportid);
			mdelay(2000);
			//goto end;
			break;
		}
		type = data->reportids[reportid].type;

		switch (type) {
		case MXT_RESERVED_T0:
		{
			dev_err(dev, "mxt_irq_thread MXT_RESERVED_T0  \n");
			goto end;
		}

		case MXT_GEN_COMMANDPROCESSOR_T6:
			mxt_treat_T6_object(data, &message);
			break;
		case MXT_TOUCH_MULTITOUCHSCREEN_T9:
			mxt_treat_T9_object(data, &message);
			break;
#if ENABLE_TOUCH_KEY
		case MXT_TOUCH_KEYARRAY_T15:
			mxt_treat_T15_object(data, &message);
			break;
#endif
		case MXT_SPT_SELFTEST_T25:
			dev_err(dev, "Self test fail [0x%x 0x%x 0x%x 0x%x]\n",
				message.message[0], message.message[1],
				message.message[2], message.message[3]);
			break;
		case MXT_PROCI_TOUCHSUPPRESSION_T42:
			mxt_treat_T42_object(data, &message);
			break;
		case MXT_PROCI_EXTRATOUCHSCREENDATA_T57:
			mxt_treat_T57_object(data, &message);
			break;
//		case MXT_PROCG_NOISESUPPRESSION_T62:
//			break;
//		case MXT_PROCG_NOISESUPPRESSION_T72:
//			break;
		case MXT_TOUCH_MULTITOUCHSCREEN_T100:
			mxt_treat_T100_object(data, &message);
			break;

		default:
			dev_err(dev, "Untreated Object type[%d]\tmessage[0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x]\n",
				type, message.message[0],
				message.message[1], message.message[2],
				message.message[3], message.message[4],
				message.message[5], message.message[6]);
			break;
		}
#if TSP_PATCH
		mxt_patch_message(data, &message);
#endif
//	} while (!gpio_get_value(data->pdata->tsp_int));
	} while (0);

	if (data->finger_mask)
		mxt_report_input_data(data);
end:
#if TSP_USE_ATMELDBG
	dev_info(dev, "mxt_irq_thread end\n");
#endif
	return IRQ_HANDLED;
}

static int mxt_get_bootloader_version(struct i2c_client *client, u8 val)
{
	u8 buf[3];

	if (val & MXT_BOOT_EXTENDED_ID) {
		if (i2c_master_recv(client, buf, sizeof(buf)) != sizeof(buf)) {
			dev_err(&client->dev, "%s: i2c recv failed\n",
				 __func__);
			return -EIO;
		}
		dev_info(&client->dev, "Bootloader ID:%d Version:%d",
			 buf[1], buf[2]);
	} else {
		dev_info(&client->dev, "Bootloader ID:%d",
			 val & MXT_BOOT_ID_MASK);
	}
	return 0;
}

static int mxt_check_bootloader(struct i2c_client *client,
				     unsigned int state)
{
	u8 val;

recheck:
	if (i2c_master_recv(client, &val, 1) != 1) {
		dev_err(&client->dev, "%s: i2c recv failed\n", __func__);
		return -EIO;
	}

	switch (state) {
	case MXT_WAITING_BOOTLOAD_CMD:
		if (mxt_get_bootloader_version(client, val))
			return -EIO;
		val &= ~MXT_BOOT_STATUS_MASK;
		break;
	case MXT_WAITING_FRAME_DATA:
	case MXT_APP_CRC_FAIL:
		val &= ~MXT_BOOT_STATUS_MASK;
		break;
	case MXT_FRAME_CRC_PASS:
		if (val == MXT_FRAME_CRC_CHECK)
			goto recheck;
		if (val == MXT_FRAME_CRC_FAIL) {
			dev_err(&client->dev, "Bootloader CRC fail\n");
			return -EINVAL;
		}
		break;
	default:
		return -EINVAL;
	}

	if (val != state) {
		dev_err(&client->dev,
			 "Invalid bootloader mode state 0x%X\n", val);
		return -EINVAL;
	}

	return 0;
}

static int mxt_unlock_bootloader(struct i2c_client *client)
{
	u8 buf[2] = {MXT_UNLOCK_CMD_LSB, MXT_UNLOCK_CMD_MSB};

	if (i2c_master_send(client, buf, 2) != 2) {
		dev_err(&client->dev, "%s: i2c send failed\n", __func__);

		return -EIO;
	}

	return 0;
}

static int mxt_probe_bootloader(struct i2c_client *client)
{
	u8 val;

	if (i2c_master_recv(client, &val, 1) != 1) {
		dev_err(&client->dev, "%s: i2c recv failed\n", __func__);
		return -EIO;
	}

	if (val & (~MXT_BOOT_STATUS_MASK)) {
		if (val & MXT_APP_CRC_FAIL)
			dev_err(&client->dev, "Application CRC failure\n");
		else
			dev_err(&client->dev, "Device in bootloader mode\n");
	} else {
		dev_err(&client->dev, "%s: Unknow status\n", __func__);
		return -EIO;
	}
	return 0;
}

static int mxt_fw_write(struct i2c_client *client,
				const u8 *frame_data, unsigned int frame_size)
{
	if (i2c_master_send(client, frame_data, frame_size) != frame_size) {
		dev_err(&client->dev, "%s: i2c send failed\n", __func__);
		return -EIO;
	}

	return 0;
}

int mxt_verify_fw(struct mxt_fw_info *fw_info, const struct firmware *fw)
{
	struct mxt_data *data = fw_info->data;
	struct device *dev = &data->client->dev;
	struct mxt_fw_image *fw_img;

	if (!fw) {
		dev_err(dev, "could not find firmware file\n");
		return -ENOENT;
	}

	fw_img = (struct mxt_fw_image *)fw->data;

	if (le32_to_cpu(fw_img->magic_code) != MXT_FW_MAGIC) {
		/* In case, firmware file only consist of firmware */
		dev_dbg(dev, "Firmware file only consist of raw firmware\n");
		fw_info->fw_len = fw->size;
		fw_info->fw_raw_data = fw->data;
	} else {
		/*
		 * In case, firmware file consist of header,
		 * configuration, firmware.
		 */
		dev_info(dev, "Firmware file consist of header, configuration, firmware\n");
		fw_info->fw_ver = fw_img->fw_ver;
		fw_info->build_ver = fw_img->build_ver;
		fw_info->hdr_len = le32_to_cpu(fw_img->hdr_len);
		fw_info->cfg_len = le32_to_cpu(fw_img->cfg_len);
		fw_info->fw_len = le32_to_cpu(fw_img->fw_len);
		fw_info->cfg_crc = le32_to_cpu(fw_img->cfg_crc);

		/* Check the firmware file with header */
		if (fw_info->hdr_len != sizeof(struct mxt_fw_image)
			|| fw_info->hdr_len + fw_info->cfg_len
				+ fw_info->fw_len != fw->size) {
#if TSP_PATCH
			struct patch_header* ppheader;
			u32 ppos = fw_info->hdr_len + fw_info->cfg_len + fw_info->fw_len;
			ppheader = (struct patch_header*)(fw->data + ppos);
			if(ppheader->magic == MXT_PATCH_MAGIC){
				dev_info(dev, "Firmware file has patch size: %d\n", ppheader->size);
				if(ppheader->size){
					u8* patch=NULL;
					if(!data->patch.patch){
						kfree(data->patch.patch);
					}
					patch = kzalloc(ppheader->size, GFP_KERNEL);
					memcpy(patch, (u8*)ppheader, ppheader->size);
					data->patch.patch = patch;
				}
			}
			else
#endif
			{
				dev_err(dev, "Firmware file is invaild !!hdr size[%d] cfg,fw size[%d,%d] filesize[%d]\n",
				fw_info->hdr_len, fw_info->cfg_len,
				fw_info->fw_len, fw->size);
				return -EINVAL;
			}
		}

		if (!fw_info->cfg_len) {
			dev_err(dev, "Firmware file dose not include configuration data\n");
			return -EINVAL;
		}
		if (!fw_info->fw_len) {
			dev_err(dev, "Firmware file dose not include raw firmware data\n");
			return -EINVAL;
		}

		/* Get the address of configuration data */
		fw_info->cfg_raw_data = fw_img->data;

		/* Get the address of firmware data */
		fw_info->fw_raw_data = fw_img->data + fw_info->cfg_len;

#if TSP_SEC_FACTORY
		data->fdata->fw_ver = fw_info->fw_ver;
		data->fdata->build_ver = fw_info->build_ver;
#endif
	}

	return 0;
}

static int mxt_wait_for_chg(struct mxt_data *data, u16 time)
{
	int timeout_counter = 0;

	msleep(time);
	if (data->pdata->tsp_int) {
		while (gpio_get_value(data->pdata->tsp_int)
			&& timeout_counter++ <= 20) {

			msleep(MXT_RESET_INTEVAL_TIME);
			dev_err(&data->client->dev, "Spend %d time waiting for chg_high\n",
				(MXT_RESET_INTEVAL_TIME * timeout_counter)
				 + time);
		}
	}

	return 0;
}

static int mxt_command_reset(struct mxt_data *data, u8 value)
{
	int error;

	mxt_write_object(data, MXT_GEN_COMMANDPROCESSOR_T6,
			MXT_COMMAND_RESET, value);

	error = mxt_wait_for_chg(data, MXT_SW_RESET_TIME);
	if (error)
		dev_err(&data->client->dev, "Not respond after reset command[%d]\n",
			value);

	return error;
}

static int mxt_command_backup(struct mxt_data *data, u8 value)
{
	mxt_write_object(data, MXT_GEN_COMMANDPROCESSOR_T6,
			MXT_COMMAND_BACKUPNV, value);

	msleep(MXT_BACKUP_TIME);
	return 0;
}

static int mxt_flash_fw(struct mxt_fw_info *fw_info)
{
	struct mxt_data *data = fw_info->data;
	struct i2c_client *client = data->client_boot;
	struct device *dev = &data->client->dev;
	const u8 *fw_data = fw_info->fw_raw_data;
	size_t fw_size = fw_info->fw_len;
	unsigned int frame_size;
	unsigned int pos = 0;
	int ret;

	if (!fw_data) {
		dev_err(dev, "firmware data is Null\n");
		return -ENOMEM;
	}

	ret = mxt_check_bootloader(client, MXT_WAITING_BOOTLOAD_CMD);
	if (ret) {
		/*may still be unlocked from previous update attempt */
		ret = mxt_check_bootloader(client, MXT_WAITING_FRAME_DATA);
		if (ret)
			goto out;
	} else {
		dev_info(dev, "Unlocking bootloader\n");
		/* Unlock bootloader */
		mxt_unlock_bootloader(client);
	}
	while (pos < fw_size) {
		ret = mxt_check_bootloader(client,
					MXT_WAITING_FRAME_DATA);
		if (ret) {
			dev_err(dev, "Fail updating firmware. wating_frame_data err\n");
			goto out;
		}

		frame_size = ((*(fw_data + pos) << 8) | *(fw_data + pos + 1));

		/*
		* We should add 2 at frame size as the the firmware data is not
		* included the CRC bytes.
		*/

		frame_size += 2;

		/* Write one frame to device */
		mxt_fw_write(client, fw_data + pos, frame_size);

		ret = mxt_check_bootloader(client,
						MXT_FRAME_CRC_PASS);
		if (ret) {
			dev_err(dev, "Fail updating firmware. frame_crc err\n");
			goto out;
		}

		pos += frame_size;

		dev_dbg(dev, "Updated %d bytes / %zd bytes\n",
				pos, fw_size);

		msleep(20);
	}

	ret = mxt_wait_for_chg(data, MXT_SW_RESET_TIME);
	if (ret) {
		dev_err(dev, "Not respond after F/W  finish reset\n");
		goto out;
	}

	dev_info(dev, "success updating firmware\n");
out:
	return ret;
}

static void mxt_handle_T72_object(struct mxt_data *data)
{
	int ret;
	u8 value;

	ret = mxt_read_object(data, MXT_PROCG_NOISESUPPRESSION_T72, 0, &value);
	if (ret) {
		dev_err(&data->client->dev, "%s: failed to read T72 object.\n",
				__func__);
		return;
	}

	value &= ~(0x02);

	ret = mxt_write_object(data, MXT_PROCG_NOISESUPPRESSION_T72,
						0, value);
	if (ret)
		dev_err(&data->client->dev, "%s: failed to write T72 object.\n",
				__func__);
	else
		dev_err(&data->client->dev, "%s: Setting T72 report disable.\n",
				__func__);
}
static void mxt_handle_init_data(struct mxt_data *data)
{
/*
 * Caution : This function is called before backup NV.
 * So If you write register vaules directly without config file
 * in this function, it can be a cause of that configuration
 * CRC mismatch or unintended values are stored in Non-volatile
 * memory in IC.
 * So I would recommed do not use this function except for bring
 * up case. Please keep this in your mind.
 */

/* disable T62 report bit. */
	mxt_handle_T72_object(data);

	return;
}

static int mxt_read_id_info(struct mxt_data *data)
{
	int ret = 0;
	u8 id[MXT_INFOMATION_BLOCK_SIZE];

	/* Read IC information */
	ret = mxt_read_mem(data, 0, MXT_INFOMATION_BLOCK_SIZE, id);
	if (ret) {
		dev_err(&data->client->dev, "Read fail. IC information\n");
		goto out;
	} else {
		dev_info(&data->client->dev,
			"family: 0x%x variant: 0x%x version: 0x%x"
			" build: 0x%x matrix X,Y size:  %d,%d"
			" number of obect: %d\n"
			, id[0], id[1], id[2], id[3], id[4], id[5], id[6]);
		data->info.family_id = id[0];
		data->info.variant_id = id[1];
		data->info.version = id[2];
		data->info.build = id[3];
		data->info.matrix_xsize = id[4];
		data->info.matrix_ysize = id[5];
		data->info.object_num = id[6];
#if 1 //bitpark
		if ( id[0] != 0xa4)
		{
			dev_err(&data->client->dev, "Read data fail. IC information\n");
			ret = -EIO;
		}
#endif
	}

out:
	return ret;
}

static int mxt_get_object_table(struct mxt_data *data)
{
	int error;
	int i;
	u16 reg;
	u8 reportid = 0;
	u8 buf[MXT_OBJECT_TABLE_ELEMENT_SIZE];

	for (i = 0; i < data->info.object_num; i++) {
		struct mxt_object *object = data->objects + i;

		reg = MXT_OBJECT_TABLE_START_ADDRESS +
				MXT_OBJECT_TABLE_ELEMENT_SIZE * i;
		error = mxt_read_mem(data, reg,
			MXT_OBJECT_TABLE_ELEMENT_SIZE, buf);
		if (error)
			return error;

		object->type = buf[0];
		object->start_address = (buf[2] << 8) | buf[1];
		/* the real size of object is buf[3]+1 */
		object->size = buf[3] + 1;
		/* the real instances of object is buf[4]+1 */
		object->instances = buf[4] + 1;
		object->num_report_ids = buf[5];

		dev_dbg(&data->client->dev,
			"Object:T%d\t\t\t Address:0x%x\tSize:%d\tInstance:%d\tReport Id's:%d\n",
			object->type, object->start_address, object->size,
			object->instances, object->num_report_ids);

		if (object->num_report_ids) {
			reportid += object->num_report_ids * object->instances;
			object->max_reportid = reportid;
		}
	}

	/* Store maximum reportid */
	data->max_reportid = reportid;
	dev_err(&data->client->dev, "maXTouch: %d report ID\n",
			data->max_reportid);

	return 0;
}

static void mxt_make_reportid_table(struct mxt_data *data)
{
	struct mxt_object *objects = data->objects;
	struct mxt_reportid *reportids = data->reportids;
	int i, j;
	int id = 0;

	for (i = 0; i < data->info.object_num; i++) {
		for (j = 0; j < objects[i].num_report_ids *
				objects[i].instances; j++) {
			id++;

			reportids[id].type = objects[i].type;
			reportids[id].index = j;

#if TSP_USE_ATMELDBG
			dev_info(&data->client->dev, "Report_id[%d]:\tT%d\tIndex[%d]\n",
				id, reportids[id].type, reportids[id].index);
#endif
		}
	}
}

static int mxt_initialize(struct mxt_data *data)
{
	struct i2c_client *client = data->client;

	u32 read_info_crc, calc_info_crc;
	int ret;

	ret = mxt_read_id_info(data);

#if 1 //bitpark
	if (ret){
		printk("mxt_initialize mxt_read_id_info Failed try after SW RESET\n");
		/* Soft reset */
		ret = mxt_command_reset(data, MXT_RESET_VALUE);
		if (ret) {
			printk("Failed Reset IC before mxt_read_id_info\n");
		}
		ret = mxt_read_id_info(data);
	}
#endif

	if (ret)
		return ret;

	data->objects = kcalloc(data->info.object_num,
				sizeof(struct mxt_object),
				GFP_KERNEL);
	if (!data->objects) {
		dev_err(&client->dev, "Failed to allocate memory\n");
		ret = -ENOMEM;
		goto out;
	}

	/* Get object table infomation */
	ret = mxt_get_object_table(data);
	if (ret)
		goto out;

	data->reportids = kcalloc(data->max_reportid + 1,
			sizeof(struct mxt_reportid),
			GFP_KERNEL);
	if (!data->reportids) {
		dev_err(&client->dev, "Failed to allocate memory\n");
		ret = -ENOMEM;
		goto out;
	}

	/* Make report id table */
	mxt_make_reportid_table(data);

	/* Verify the info CRC */
	ret = mxt_read_info_crc(data, &read_info_crc);
	if (ret)
		goto out;

	ret = mxt_calculate_infoblock_crc(data, &calc_info_crc);
	if (ret)
		goto out;

	if (read_info_crc != calc_info_crc) {
		dev_err(&data->client->dev, "Infomation CRC error :[CRC 0x%06X!=0x%06X]\n",
				read_info_crc, calc_info_crc);
		ret = -EFAULT;
		goto out;
	}
	return 0;

out:
	return ret;
}

static int  mxt_rest_initialize(struct mxt_fw_info *fw_info)
{
	struct mxt_data *data = fw_info->data;
	struct device *dev = &data->client->dev;
	int ret;

	/* Restore memory and stop event handing */
	ret = mxt_command_backup(data, MXT_DISALEEVT_VALUE);
	if (ret) {
		dev_err(dev, "Failed Restore NV and stop event\n");
		goto out;
	}

	/* Write config */
	ret = mxt_write_config(fw_info);
	if (ret) {
		dev_err(dev, "Failed to write config from file\n");
		goto out;
	}
/*
	if (system_rev != 0x00)
		mxt_write_object(data, MXT_TOUCH_MULTITOUCHSCREEN_T9,
			MXT_TOUCH_ORIENT, 1);
*/
	/* Handle data for init */
	mxt_handle_init_data(data);

	/* Backup to memory */
	ret = mxt_command_backup(data, MXT_BACKUP_VALUE);
	if (ret) {
		dev_err(dev, "Failed backup NV data\n");
		goto out;
	}

	/* Soft reset */
	ret = mxt_command_reset(data, MXT_RESET_VALUE);
	if (ret) {
		dev_err(dev, "Failed Reset IC\n");
		goto out;
	}

#if TSP_PATCH
	if (data->patch.patch)
		ret = mxt_patch_init(data, data->patch.patch);
#endif
out:
	return ret;
}

#if 0
static int mxt_power_on(struct mxt_data *data)
{
/*
 * If do not turn off the power during suspend, you can use deep sleep
 * or disable scan to use T7, T9 Object. But to turn on/off the power
 * is better.
 */
	int ret = 0;

	if (data->mxt_enabled)
		return 0;

	if (!data->pdata->power_onoff) {
		dev_warn(&data->client->dev, "Power on function is not defined\n");
		ret = -EINVAL;
		goto out;
	}

	ret = data->pdata->power_onoff(true);
	if (ret) {
		dev_err(&data->client->dev, "Failed to power on\n");
		goto out;
	}

	ret = mxt_wait_for_chg(data, MXT_HW_RESET_TIME);
	if (ret)
		dev_err(&data->client->dev, "Not respond after H/W reset\n");

	data->mxt_enabled = true;

out:
	return ret;
}

static int mxt_power_off(struct mxt_data *data)
{
	int ret = 0;

	if (!data->mxt_enabled)
		return 0;

	if (!data->pdata->power_onoff) {
		dev_warn(&data->client->dev, "Power off function is not defined\n");
		ret = -EINVAL;
		goto out;
	}

	ret = data->pdata->power_onoff(false);
	if (ret) {
		dev_err(&data->client->dev, "Failed to power off\n");
		goto out;
	}


	data->mxt_enabled = false;

out:
	return ret;
}
#endif

/* Need to be called by function that is blocked with mutex */
static int mxt_start(struct mxt_data *data)
{
	int ret = 0;

	if (data->mxt_enabled) {
		dev_err(&data->client->dev,
			"%s. but touch already on\n", __func__);

#if 1 //hmink
		mxt_command_reset(data, MXT_RESET_VALUE);
#endif

		return ret;
	}

	//ret = data->pdata->power_onoff(true);
	if (ret)
		dev_err(&data->client->dev, "Fail to start touch\n");


	data->patch.cal_flag = 1; //1112 for patch start

	enable_irq(data->client->irq);

#if 1 //hmink
	ret = mxt_make_highchg(data);
	if (ret) {
		dev_err(&data->client->dev, "Failed to clear CHG pin\n");
		//goto err_req_irq;
	}
#endif

	dev_info(&data->client->dev, "mxt_resume\n");
	return ret;
}

/* Need to be called by function that is blocked with mutex */
static int mxt_stop(struct mxt_data *data)
{
	int ret = 0;

	if (!data->mxt_enabled) {
		dev_err(&data->client->dev,
			"%s. but touch already off\n", __func__);
		return ret;
	}

#if 1 //20140305 hmink
	disable_irq(data->client->irq);
#endif

	//ret = data->pdata->power_onoff(false);
	if (ret) {
		dev_err(&data->client->dev, "Fail to stop touch\n");
		goto err_power_off;
	}
	mxt_release_all_finger(data);

#if ENABLE_TOUCH_KEY
	mxt_release_all_keys(data);
#endif

#if TSP_BOOSTER
	mxt_set_dvfs_on(data, false);
#endif
	dev_info(&data->client->dev, "mxt_suspend\n");
	return 0;

err_power_off:
	enable_irq(data->client->irq);
	return ret;
}
#if 0
static int mxt_input_open(struct input_dev *dev)
{
	struct mxt_data *data = input_get_drvdata(dev);
	int ret;

	ret = wait_for_completion_interruptible_timeout(&data->init_done,
			msecs_to_jiffies(90 * MSEC_PER_SEC));

	if (ret <= 0) {
		dev_err(&data->client->dev,
			"error while waiting for device to init (%d)\n", ret);
		ret = -ENXIO;
		goto err_open;
	}
	if (ret == 0) {
		dev_err(&data->client->dev,
			"timedout while waiting for device to init\n");
		ret = -ENXIO;
		goto err_open;
	}

	ret = mxt_start(data);
	if (ret)
		goto err_open;

	dev_dbg(&data->client->dev, "%s\n", __func__);

	return 0;

err_open:
	return ret;
}

static void mxt_input_close(struct input_dev *dev)
{
	struct mxt_data *data = input_get_drvdata(dev);

	mxt_stop(data);

	dev_dbg(&data->client->dev, "%s\n", __func__);
}
#endif
static int mxt_make_highchg(struct mxt_data *data)
{
	struct device *dev = &data->client->dev;
	struct mxt_message message;
	int count = data->max_reportid * 2;
	int error, cnt = 0;

	/* Read dummy message to make high CHG pin */
	do {
		error = mxt_read_message(data, &message);
		dev_err(dev, "mxt_read_message reportid =%x, cnt=%d\n",message.reportid, cnt++);
		if (error)
			return error;
		msleep(1);
	} while (!gpio_get_value(data->pdata->tsp_int));
//	} while (message.reportid != 0xff && --count);

	if (!count) {
		dev_err(dev, "CHG pin isn't cleared\n");
		return -EBUSY;
	}
	return 0;
}

static int mxt_touch_finish_init(struct mxt_data *data)
{
	struct i2c_client *client = data->client;
	int ret;

	client->irq = gpio_to_irq(data->pdata->tsp_int);

	ret = request_threaded_irq(client->irq, NULL, mxt_irq_thread,
		 IRQF_TRIGGER_FALLING | IRQF_DISABLED, client->dev.driver->name, data);

	if (ret) {
		dev_err(&client->dev, "Failed to register interrupt\n");
		goto err_req_irq;
	}
#if 0 //hmink
	ret = mxt_make_highchg(data);
	if (ret) {
		dev_err(&client->dev, "Failed to clear CHG pin\n");
		goto err_req_irq;
	}

//	ret = mxt_wait_for_chg(data, MXT_SW_RESET_TIME);
//	if (ret)
//		dev_err(&data->client->dev, "CHG pin Not respond after mxt_make_highchg[%d]\n");
#endif
#if TSP_BOOSTER
	ret = mxt_init_dvfs(data);
	if (ret < 0) {
		dev_err(&client->dev, "Fail get dvfs level for touch booster\n");
		goto err_req_irq;
	}
#endif

	dev_info(&client->dev,  "Mxt touch controller initialized\n");

	/*
	* to prevent unnecessary report of touch event
	* it will be enabled in open function
	*/
	/* mxt_stop(data);*/

	/* for blocking to be excuted open function untile finishing ts init */
	/* complete_all(&data->init_done); */
	return 0;

err_req_irq:
	return ret;
}

static int mxt_touch_rest_init(struct mxt_fw_info *fw_info)
{
	struct mxt_data *data = fw_info->data;
	struct device *dev = &data->client->dev;
	int ret;

	ret = mxt_initialize(data);
	if (ret) {
		dev_err(dev, "Failed to initialize\n");
		goto err_free_mem;
	}

	ret = mxt_rest_initialize(fw_info);
	if (ret) {
		dev_err(dev, "Failed to rest initialize\n");
		goto err_free_mem;
	}

	ret = mxt_touch_finish_init(data);
	if (ret)
		goto err_free_mem;

	return 0;

err_free_mem:
	kfree(data->objects);
	data->objects = NULL;
	kfree(data->reportids);
	data->reportids = NULL;
	return ret;
}

#ifdef TSP_BRING_UP
static int mxt_enter_bootloader(struct mxt_data *data)
{
	struct device *dev = &data->client->dev;
	int error;

	data->objects = kcalloc(data->info.object_num,
				     sizeof(struct mxt_object),
				     GFP_KERNEL);
	if (!data->objects) {
		dev_err(dev, "%s Failed to allocate memory\n",
			__func__);
		error = -ENOMEM;
		goto out;
	}

	/* Get object table information*/
	error = mxt_get_object_table(data);
	if (error)
		goto err_free_mem;

	/* Change to the bootloader mode */
	error = mxt_command_reset(data, MXT_BOOT_VALUE);
	if (error)
		goto err_free_mem;

err_free_mem:
	kfree(data->objects);
	data->objects = NULL;

out:
	return error;
}

static int mxt_flash_fw_on_probe(struct mxt_fw_info *fw_info)
{
	struct mxt_data *data = fw_info->data;
	struct device *dev = &data->client->dev;
	int error;

#if 0 // ATMEL TEST

    fw_info->fw_ver = 255;

    return 0;

#endif
	error = mxt_read_id_info(data);

#if 1 //hmink
	if (error) {
		dev_err(dev, "mxt_flash_fw_on_probe: mxt_read_id_info Failed\n");
		error = mxt_read_id_info(data);
	}
#endif

	if (error) {
		/* need to check IC is in boot mode */
		error = mxt_probe_bootloader(data->client_boot);
		if (error) {
			dev_err(dev, "Failed to verify bootloader's status\n");
			goto out;
	}

		dev_info(dev, "Updating firmware from boot-mode\n");
		goto load_fw;
	}

	dev_info(dev, "Updating firmware from app-mode : IC:0x%x,0x%x =! FW:0x%x,0x%x\n",
			data->info.version, data->info.build,
			fw_info->fw_ver, fw_info->build_ver);

	/* compare the version to verify necessity of firmware updating */
	if (data->info.version == fw_info->fw_ver
			&& data->info.build == fw_info->build_ver) {
		dev_info(dev, "Firmware version is same with in IC\n");
		goto out;
	}
	dev_info(dev, "Updating firmware from app-mode : IC:0x%x,0x%x =! FW:0x%x,0x%x\n",
			data->info.version, data->info.build,
			fw_info->fw_ver, fw_info->build_ver);

	error = mxt_enter_bootloader(data);
	if (error) {
		dev_err(dev, "Failed updating firmware\n");
		goto out;
	}

load_fw:
	error = mxt_flash_fw(fw_info);
	if (error)
		dev_err(dev, "Failed updating firmware\n");
	else
		dev_info(dev, "succeeded updating firmware\n");
out:
	return error;
}
#endif

static int mxt_touch_init_firmware(const struct firmware *fw, void *context)
{
	struct mxt_data *data = context;
	struct mxt_fw_info fw_info;
	int ret;

	memset(&fw_info, 0, sizeof(struct mxt_fw_info));
	fw_info.data = data;

#if FW_UPDATE_ENABLE
	ret = mxt_verify_fw(&fw_info, fw);
	if (ret)
		goto ts_rest_init;

	/* Skip update on boot up if firmware file does not have a header */
	if (!fw_info.hdr_len)
		goto ts_rest_init;

	ret = mxt_flash_fw_on_probe(&fw_info);
	if (ret)
		goto out;
#endif

ts_rest_init:
	ret = mxt_touch_rest_init(&fw_info);
out:
#if FW_UPDATE_ENABLE
	release_firmware(fw);
#endif
	return ret;
}

static void mxt_request_firmware_work(const struct firmware *fw,
		void *context)
{
	struct mxt_data *data = context;
	mxt_touch_init_firmware(fw, data);
}

static int __devinit mxt_touch_init(struct mxt_data *data, bool nowait)
{
	struct i2c_client *client = data->client;
	const char *firmware_name =
		 data->pdata->firmware_name ?: MXT_DEFAULT_FIRMWARE_NAME;
	int ret = 0;

#if TSP_INFORM_CHARGER
	/* Register callbacks */
	/* To inform tsp, charger connection status*/
	data->pdata->register_cb = mxt_tsp_register_callback;
	data->callback.inform_charger = inform_charger;

	if (data->pdata->register_cb) {
		data->pdata->register_cb(&data->callback);
		inform_charger_init(data);
	}
#endif

	if (nowait) {
		const struct firmware *fw;
		char fw_path[MXT_MAX_FW_PATH];

		memset(&fw_path, 0, MXT_MAX_FW_PATH);

		snprintf(fw_path, MXT_MAX_FW_PATH, "%s/%s",
			MXT_FIRMWARE_INKERNEL_PATH, firmware_name);

		dev_err(&client->dev, "%s\n", fw_path);

#if FW_UPDATE_ENABLE


		ret = request_firmware(&fw, fw_path, &client->dev);
		if (ret) {
			dev_err(&client->dev,
				"error requesting built-in firmware\n");
#if 0	// BITPARK for PXD
			/* Soft reset */
			ret = mxt_command_reset(data, MXT_RESET_VALUE);
			if (ret) {
				printk("Failed Reset IC in request_firmware\n");
				goto out;
			}
			ret = request_firmware(&fw, fw_path, &client->dev);
			if (ret) {
				dev_err(&client->dev, "error requesting built-in firmware after SOFT RESET\n");
				goto  out;
			}
#endif
		}
#endif
		ret = mxt_touch_init_firmware(fw, data);
	} else {
		ret = request_firmware_nowait(THIS_MODULE, true, firmware_name,
				      &client->dev, GFP_KERNEL,
				      data, mxt_request_firmware_work);
		if (ret)
			dev_err(&client->dev,
				"cannot schedule firmware update (%d)\n", ret);
	}
	return ret;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
#define mxt_suspend	NULL
#define mxt_resume	NULL

static void mxt_early_suspend(struct early_suspend *h)
{
	struct mxt_data *data = container_of(h, struct mxt_data,
								early_suspend);
#if TSP_INFORM_CHARGER
	cancel_delayed_work_sync(&data->noti_dwork);
#endif

	mutex_lock(&data->input_dev->mutex);

	mxt_stop(data);

	mutex_unlock(&data->input_dev->mutex);
}

static void mxt_late_resume(struct early_suspend *h)
{
	struct mxt_data *data = container_of(h, struct mxt_data,
								early_suspend);
	mutex_lock(&data->input_dev->mutex);

	mxt_start(data);

	mutex_unlock(&data->input_dev->mutex);
}
#else
static int mxt_suspend(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct mxt_data *data = i2c_get_clientdata(client);

	mutex_lock(&data->input_dev->mutex);

	mxt_stop(data);

	mutex_unlock(&data->input_dev->mutex);
	return 0;
}

static int mxt_resume(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct mxt_data *data = i2c_get_clientdata(client);

	mutex_lock(&data->input_dev->mutex);

	mxt_start(data);

	mutex_unlock(&data->input_dev->mutex);
	return 0;
}
#endif

/* Added for samsung dependent codes such as Factory test,
 * Touch booster, Related debug sysfs.
 */
//#include "mxts_sec.c"

static int __devinit mxt_probe(struct i2c_client *client,
					const struct i2c_device_id *id)
{
	struct mxt_platform_data *pdata = client->dev.platform_data;
	struct mxt_data *data;
	struct input_dev *input_dev;
	u16 boot_address;
	int ret = 0;

	printk(KERN_ERR "%s", __func__);

	if (!pdata) {
		dev_err(&client->dev,
				"%s: Platform data is not proper\n", __func__);
		return -EINVAL;
	}

	data = kzalloc(sizeof(struct mxt_data), GFP_KERNEL);
	if (!data) {
		dev_err(&client->dev, "Failed to allocate memory\n");
		return -ENOMEM;
	}

	data->pdata = pdata;

	tsp_request_gpio(data);

	input_dev = input_allocate_device();
	if (unlikely(!input_dev)) {
		ret = -ENOMEM;
		dev_err(&client->dev, "Failed to allocate input device.\n");
		goto err_allocate_input_device;
	}

	input_dev->name = "px_touchscreen";
	input_dev->id.bustype = BUS_I2C;
	input_dev->dev.parent = &client->dev;
/*	input_dev->open = mxt_input_open;*/
/*	input_dev->close = mxt_input_close;*/

	data->client = client;
	data->input_dev = input_dev;
	data->pdata = pdata;

/*	init_completion(&data->init_done);*/

	set_bit(EV_ABS, input_dev->evbit);
	set_bit(EV_KEY, input_dev->evbit);
	set_bit(INPUT_PROP_DIRECT, input_dev->propbit);
	set_bit(BTN_TOUCH, input_dev->keybit);
	set_bit(BTN_TOOL_FINGER, input_dev->keybit);

#if ENABLE_TOUCH_KEY
	set_bit(KEY_MENU, input_dev->keybit);
	set_bit(KEY_BACK, input_dev->keybit);
#endif

	input_mt_init_slots(input_dev, MXT_MAX_FINGER);
	input_set_abs_params(input_dev, ABS_MT_POSITION_X, 0, pdata->max_x, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_POSITION_Y, 0, pdata->max_y, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_TOUCH_MAJOR, 0, MXT_AREA_MAX, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_PRESSURE, 0, MXT_AMPLITUDE_MAX, 0, 0);
#if TSP_USE_SHAPETOUCH
	input_set_abs_params(input_dev, ABS_MT_COMPONENT, 0, MXT_COMPONENT_MAX, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_SUMSIZE, 0, MXT_SUMSIZE_MAX, 0, 0);
#endif

	input_set_drvdata(input_dev, data);
	i2c_set_clientdata(client, data);

	if (data->pdata->boot_address) {
		boot_address = data->pdata->boot_address;
	} else {
		if (client->addr == MXT_APP_LOW)
			boot_address = MXT_BOOT_LOW;
		else
			boot_address = MXT_BOOT_HIGH;
	}

	data->client_boot = i2c_new_dummy(client->adapter, boot_address);
	if (!data->client_boot) {
		dev_err(&client->dev, "Failed to register sub client[0x%x]\n",
			 boot_address);
		ret = -ENODEV;
		goto err_create_sub_client;
	}

	/* regist input device */
	ret = input_register_device(input_dev);
	if (ret)
		goto err_register_input_device;
/*
	ret = mxt_sysfs_init(client);
	if (ret < 0) {
		dev_err(&client->dev, "Failed to create sysfs.\n");
		goto err_sysfs_init;
	}
*/
#if 0
	/* Soft reset */ // BITPARK for PXD
	ret = mxt_command_reset(data, MXT_RESET_VALUE);
	if (ret) {
		printk("Failed Reset IC in request_firmware\n");
		//goto out;
	}
#endif
	//mxt_read_id_info(data);

	data->patch.cal_flag = 1; //1112 for patch start
	ret = mxt_touch_init(data, MXT_FIRMWARE_UPDATE_TYPE);
	if (ret) {
		dev_err(&client->dev, "Failed to init driver\n");
		goto err_touch_init;
	}
	data->mxt_enabled =true;
#ifdef CONFIG_HAS_EARLYSUSPEND
	data->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
	data->early_suspend.suspend = mxt_early_suspend;
	data->early_suspend.resume = mxt_late_resume;
	register_early_suspend(&data->early_suspend);
#endif

#ifdef CONFIG_PM_RUNTIME
	pm_runtime_enable(&client->dev);
#endif

#if 1 //hmink
	ret = mxt_make_highchg(data);
	if (ret) {
		dev_err(&client->dev, "Failed to clear CHG pin\n");
		//goto err_req_irq;
	}
//	ret = mxt_wait_for_chg(data, MXT_SW_RESET_TIME);
//	if (ret)
//		dev_err(&data->client->dev, "CHG pin Not respond after mxt_make_highchg[%d]\n");
#endif

	return 0;

err_touch_init:
	data->pdata->power_onoff(false);

err_power_on:
	//mxt_sysfs_remove(data);
//err_sysfs_init:
	input_unregister_device(input_dev);
	input_dev = NULL;
err_register_input_device:
	i2c_unregister_device(data->client_boot);
err_create_sub_client:
	input_free_device(input_dev);
err_allocate_input_device:
	kfree(data);

	return ret;
}

static int __devexit mxt_remove(struct i2c_client *client)
{
	struct mxt_data *data = i2c_get_clientdata(client);

#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&data->early_suspend);
#endif
	free_irq(client->irq, data);
	kfree(data->objects);
	kfree(data->reportids);
	input_unregister_device(data->input_dev);
	i2c_unregister_device(data->client_boot);
//	mxt_sysfs_remove(data);
	data->pdata->power_onoff(false);
	kfree(data);

	return 0;
}

static struct i2c_device_id mxt_idtable[] = {
	{MXT_DEV_NAME, 0},
};

MODULE_DEVICE_TABLE(i2c, mxt_idtable);

static const struct dev_pm_ops mxt_pm_ops = {
#if defined(CONFIG_PM_RUNTIME)
		SET_RUNTIME_PM_OPS(mxt_suspend, mxt_resume, NULL)
#else
		SET_SYSTEM_SLEEP_PM_OPS(mxt_suspend, mxt_resume)
#endif
	//.suspend = mxt_suspend,
	//.resume = mxt_resume,
};

static struct i2c_driver mxt_i2c_driver = {
	.id_table = mxt_idtable,
	.probe = mxt_probe,
	.remove = __devexit_p(mxt_remove),
/*	.shutdown = mxt_shutdown,*/
	.driver = {
		.owner	= THIS_MODULE,
		.name	= MXT_DEV_NAME,
#ifdef CONFIG_PM
		.pm	= &mxt_pm_ops,
#endif
	},
};

static int __init mxt_i2c_init(void)
{
	return i2c_add_driver(&mxt_i2c_driver);
}

static void __exit mxt_i2c_exit(void)
{
	i2c_del_driver(&mxt_i2c_driver);
}

module_init(mxt_i2c_init);
module_exit(mxt_i2c_exit);

MODULE_DESCRIPTION("Atmel MaXTouch driver");
MODULE_AUTHOR("bumwoo.lee<bw365.lee@samsung.com>");
MODULE_LICENSE("GPL");
