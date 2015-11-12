/****************************************************************

 Siano Mobile Silicon, Inc.
 MDTV receiver kernel modules.
 Copyright (C) 2006-2009, Uri Shkolnik

 Copyright (c) 2010 - Mauro Carvalho Chehab
	- Ported the driver to use rc-core
	- IR raw event decoding is now done at rc-core
	- Code almost re-written

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.

 ****************************************************************/


#include <linux/types.h>
#include <linux/input.h>

#include "smscoreapi.h"
#include "smsir.h"
#include "sms-cards.h"

#ifdef SMS_RC_SUPPORT_SUBSYS
#define MODULE_NAME "smsmdtv"

extern int sms_dbg;

void sms_ir_event(struct ir_t *ir, const char *buf, int len)
{
	int i;
	const s32 *samples = (const void *)buf;

	for (i = 0; i < len >> 2; i++) {
		DEFINE_IR_RAW_EVENT(ev);

		ev.duration = abs(samples[i]) * 1000; /* Convert to ns */
		ev.pulse = (samples[i] > 0) ? false : true;

		ir_raw_event_store(ir->rc_dev, &ev);
	}
	ir_raw_event_handle(ir->rc_dev);

}

int sms_ir_init(struct ir_t *ir, void* coredev, struct device *device)
{
	int err;
	struct rc_dev *rc_dev;
	struct sms_properties_t properties;

	smscore_get_device_properties(coredev, &properties);

	sms_info("Allocating input device");
	rc_dev = rc_allocate_device();
	if (!rc_dev)	{
		sms_err("Not enough memory");
		return -ENOMEM;
	}

	ir->controller = 0;	/* Todo: vega/nova SPI number */
	ir->timeout = IR_DEFAULT_TIMEOUT;
	sms_info("IR port %d, timeout %d ms", ir->controller, ir->timeout);

	snprintf(ir->name, sizeof(ir->name),
		 "SMS IR (%s)", sms_get_board(properties.board_id)->name);

//	strlcpy(ir->phys, coredev->devpath, sizeof(ir->phys));
	strlcat(ir->phys, "/ir0", sizeof(ir->phys));

	rc_dev->input_name = ir->name;
	rc_dev->input_phys = ir->phys;
	rc_dev->dev.parent = device;
	rc_dev->priv = ir;
	rc_dev->driver_type = RC_DRIVER_IR_RAW;
	rc_dev->allowed_protos = RC_TYPE_ALL;
	rc_dev->map_name = sms_get_board(properties.board_id)->rc_codes;
	rc_dev->driver_name = MODULE_NAME;

	sms_info("Input device (IR) %s is set for key events", rc_dev->input_name);

	err = rc_register_device(rc_dev);
	if (err < 0) {
		sms_err("Failed to register device");
		rc_free_device(rc_dev);
		return err;
	}

	ir->rc_dev = rc_dev;
	return 0;
}

void sms_ir_exit(struct ir_t *ir)
{
	if (ir->rc_dev)
		rc_unregister_device(ir->rc_dev);

	sms_info("");
}
#endif /*SMS_RC_SUPPORT_SUBSYS*/


