/****************************************************************

Siano Mobile Silicon, Inc.
MDTV receiver kernel modules.
Copyright (C) 2006-2008, Uri Shkolnik

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

#define __DVB_CORE__

#include <linux/module.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/list.h>

#include "dmxdev.h"
#include "dvbdev.h"
#include "dvb_demux.h"
#include "dvb_frontend.h"

#include "smscoreapi.h"
#include "smsendian.h"
#include "sms-cards.h"

DVB_DEFINE_MOD_OPT_ADAPTER_NR(adapter_nr);
#define MAX_NUM_OF_TUNERS 4
struct smsdvb_client_t;
int hif_task_of_device[] = {HIF_TASK, HIF_TASK_SLAVE, HIF_TASK_SLAVE2, HIF_TASK_SLAVE3};

struct smsdvb_frontend_t{
	struct list_head entry;
	struct dvb_frontend fe;
	struct dvb_demux demux;
	struct dmxdev dmxdev;
	fe_status_t fe_status;
	struct smsdvb_client_t* client;
	int tuner_id;
	struct RECEPTION_STATISTICS_S reception_data;
	struct completion       tune_done;
	struct completion	get_stats_done;
	int event_fe_state;
	int event_unc_state;
};

struct smsdvb_client_t {
	struct list_head entry;
	void *coredev;
	struct smscore_client_t *smsclient;
	struct dvb_adapter      adapter;
	struct list_head 	fe_list;
	/* todo: save freq/band instead whole struct */
	struct dvb_frontend_parameters fe_params;

};

enum SMS_DVB3_EVENTS {
	DVB3_EVENT_INIT = 0,
	DVB3_EVENT_SLEEP,
	DVB3_EVENT_HOTPLUG,
	DVB3_EVENT_FE_LOCK,
	DVB3_EVENT_FE_UNLOCK,
	DVB3_EVENT_UNC_OK,
	DVB3_EVENT_UNC_ERR
};

#define CORRECT_STAT_RSSI(_stat) (_stat).RSSI *= -1


static struct list_head g_smsdvb_clients;
static struct mutex g_smsdvb_clientslock;


/* Events that may come from DVB v3 adapter */
static void sms_board_dvb3_event(struct smsdvb_frontend_t *sms_frontend, enum SMS_DVB3_EVENTS event) 
{

	void *coredev = sms_frontend->client->coredev;
	switch (event) {
	case DVB3_EVENT_INIT:
		sms_info("DVB3_EVENT_INIT");
		sms_board_event(coredev, BOARD_EVENT_BIND);
		break;
	case DVB3_EVENT_SLEEP:
		sms_info("DVB3_EVENT_SLEEP");
		sms_board_event(coredev, BOARD_EVENT_POWER_SUSPEND);
		break;
	case DVB3_EVENT_HOTPLUG:
		sms_info("DVB3_EVENT_HOTPLUG");
		sms_board_event(coredev, BOARD_EVENT_POWER_INIT);
		break;
	case DVB3_EVENT_FE_LOCK:
		if (sms_frontend->event_fe_state != DVB3_EVENT_FE_LOCK) {
			sms_frontend->event_fe_state = DVB3_EVENT_FE_LOCK;
			sms_info("DVB3_EVENT_FE_LOCK");
			sms_board_event(coredev, BOARD_EVENT_FE_LOCK);
		}
		break;
	case DVB3_EVENT_FE_UNLOCK:
		if (sms_frontend->event_fe_state != DVB3_EVENT_FE_UNLOCK) {
			sms_frontend->event_fe_state = DVB3_EVENT_FE_UNLOCK;
			sms_info("DVB3_EVENT_FE_UNLOCK");
			sms_board_event(coredev, BOARD_EVENT_FE_UNLOCK);
		}
		break;
	case DVB3_EVENT_UNC_OK:
		if (sms_frontend->event_unc_state != DVB3_EVENT_UNC_OK) {
			sms_frontend->event_unc_state = DVB3_EVENT_UNC_OK;
			sms_info("DVB3_EVENT_UNC_OK");
			sms_board_event(coredev, BOARD_EVENT_MULTIPLEX_OK);
		}
		break;
	case DVB3_EVENT_UNC_ERR:
		if (sms_frontend->event_unc_state != DVB3_EVENT_UNC_ERR) {
			sms_frontend->event_unc_state = DVB3_EVENT_UNC_ERR;
			sms_info("DVB3_EVENT_UNC_ERR");
			sms_board_event(coredev, BOARD_EVENT_MULTIPLEX_ERRORS);
		}
		break;

	default:
		sms_err("Unknown dvb3 api event");
		break;
	}
}


static void smsdvb_update_dvb_stats(struct RECEPTION_STATISTICS_S *pReceptionData,
				   struct SMSHOSTLIB_STATISTICS_S *p)
{
	sms_debug("IsRfLocked = %d", p->IsRfLocked);
	sms_debug("IsDemodLocked = %d", p->IsDemodLocked);
	sms_debug("IsExternalLNAOn = %d", p->IsExternalLNAOn);
	sms_debug("SNR = %d", p->SNR);
	sms_debug("BER = %d", p->BER);
	sms_debug("TS_PER = %d", p->TS_PER);
	sms_debug("MFER = %d", p->MFER);
	sms_debug("RSSI = %d", p->RSSI);
	sms_debug("InBandPwr = %d", p->InBandPwr);
	sms_debug("CarrierOffset = %d", p->CarrierOffset);
	sms_debug("ModemState = %d", p->ModemState);
	sms_debug("Frequency = %d", p->Frequency);
	sms_debug("Bandwidth = %d", p->Bandwidth);
	sms_debug("TransmissionMode = %d", p->TransmissionMode);
	sms_debug("GuardInterval = %d", p->GuardInterval);
	sms_debug("CodeRate = %d", p->CodeRate);
	sms_debug("LPCodeRate = %d", p->LPCodeRate);
	sms_debug("Hierarchy = %d", p->Hierarchy);
	sms_debug("Constellation = %d", p->Constellation);

	/* update reception data */
	pReceptionData->IsRfLocked = p->IsRfLocked;
	pReceptionData->IsDemodLocked = p->IsDemodLocked;
	pReceptionData->IsExternalLNAOn = p->IsExternalLNAOn;
	pReceptionData->ModemState = p->ModemState;
	pReceptionData->SNR = p->SNR;
	pReceptionData->BER = p->BER;
	pReceptionData->BERErrorCount = p->BERErrorCount;
	pReceptionData->BERBitCount = p->BERBitCount;
	pReceptionData->RSSI = p->RSSI;
	CORRECT_STAT_RSSI(*pReceptionData);
	pReceptionData->InBandPwr = p->InBandPwr;
	pReceptionData->CarrierOffset = p->CarrierOffset;
	pReceptionData->ErrorTSPackets = p->ErrorTSPackets;
	pReceptionData->TotalTSPackets = p->TotalTSPackets;

	/* TS PER */
	if ((p->TotalTSPackets + p->ErrorTSPackets) > 0) 
	{
		pReceptionData->TS_PER = (p->ErrorTSPackets
				* 100) / (p->TotalTSPackets
				+ p->ErrorTSPackets);
	} 
	else {
		pReceptionData->TS_PER = 0;
	}

	pReceptionData->MFER = 0;
};


static void smsdvb_update_isdbt_stats(struct RECEPTION_STATISTICS_S *pReceptionData,
				    struct SMSHOSTLIB_STATISTICS_ISDBT_S *p)
{
	int i;
	sms_debug("IsRfLocked = %d", p->IsRfLocked);
	sms_debug("IsDemodLocked = %d", p->IsDemodLocked);
	sms_debug("IsExternalLNAOn = %d", p->IsExternalLNAOn);
	sms_debug("SNR = %d", p->SNR);
	sms_debug("RSSI = %d", p->RSSI);
	sms_debug("InBandPwr = %d", p->InBandPwr);
	sms_debug("CarrierOffset = %d", p->CarrierOffset);
	sms_debug("Frequency = %d", p->Frequency);
	sms_debug("Bandwidth = %d", p->Bandwidth);
	sms_debug("TransmissionMode = %d", p->TransmissionMode);
	sms_debug("ModemState = %d", p->ModemState);
	sms_debug("GuardInterval = %d", p->GuardInterval);
	sms_debug("SystemType = %d", p->SystemType);
	sms_debug("PartialReception = %d", p->PartialReception);
	sms_debug("NumOfLayers = %d", p->NumOfLayers);
	sms_debug("SegmentNumber = %d", p->SegmentNumber);
	sms_debug("TuneBW = %d", p->TuneBW);

	for (i = 0; i < p->NumOfLayers && i < 3 ; i++)
	{
		sms_debug("");
		sms_debug("Layer[%d].CodeRate = %d", i, p->LayerInfo[i].CodeRate);
		sms_debug("Layer[%d].Constellation = %d", i, p->LayerInfo[i].Constellation);
		sms_debug("Layer[%d].BER = %d", i, p->LayerInfo[i].BER);
		sms_debug("Layer[%d].BERErrorCount = %d", i, p->LayerInfo[i].BERErrorCount);
		sms_debug("Layer[%d].BERBitCount = %d", i, p->LayerInfo[i].BERBitCount);
		sms_debug("Layer[%d].PreBER = %d", i, p->LayerInfo[i].PreBER);
		sms_debug("Layer[%d].TS_PER = %d", i, p->LayerInfo[i].TS_PER);
		sms_debug("Layer[%d].ErrorTSPackets = %d", i, p->LayerInfo[i].ErrorTSPackets);
		sms_debug("Layer[%d].TotalTSPackets = %d", i, p->LayerInfo[i].TotalTSPackets);
		sms_debug("Layer[%d].TILdepthI = %d", i, p->LayerInfo[i].TILdepthI);
		sms_debug("Layer[%d].NumberOfSegments = %d", i, p->LayerInfo[i].NumberOfSegments);
		sms_debug("Layer[%d].TMCCErrors = %d", i, p->LayerInfo[i].TMCCErrors);
	}


	/* update reception data */
	pReceptionData->IsRfLocked = p->IsRfLocked;
	pReceptionData->IsDemodLocked = p->IsDemodLocked;
	pReceptionData->IsExternalLNAOn = p->IsExternalLNAOn;
	pReceptionData->ModemState = p->ModemState;
	pReceptionData->SNR = p->SNR;
	pReceptionData->BER = p->LayerInfo[0].BER;
	pReceptionData->BERErrorCount = p->LayerInfo[0].BERErrorCount;
	pReceptionData->BERBitCount = p->LayerInfo[0].BERBitCount;
	pReceptionData->RSSI = p->RSSI;
	CORRECT_STAT_RSSI(*pReceptionData);
	pReceptionData->InBandPwr = p->InBandPwr;
	pReceptionData->CarrierOffset = p->CarrierOffset;
	pReceptionData->ErrorTSPackets = p->LayerInfo[0].ErrorTSPackets;
	pReceptionData->TotalTSPackets = p->LayerInfo[0].TotalTSPackets;
	pReceptionData->MFER = 0;


	/* TS PER */
	if ((p->LayerInfo[0].TotalTSPackets + 
		 p->LayerInfo[0].ErrorTSPackets) > 0) 
	{
		pReceptionData->TS_PER = (p->LayerInfo[0].ErrorTSPackets
				* 100) / (p->LayerInfo[0].TotalTSPackets
				+ p->LayerInfo[0].ErrorTSPackets);
	} else {
		pReceptionData->TS_PER = 0;
	}
}

static int smsdvb_onresponse(void *context, struct smscore_buffer_t *cb)
{
	struct smsdvb_client_t *client = (struct smsdvb_client_t *) context;
	struct SmsMsgHdr_S *phdr = (struct SmsMsgHdr_S *) (((u8 *) cb->p)
			+ cb->offset);
	u32 *pMsgData = (u32 *) (phdr + 1);
	bool is_status_update = false;
	int tuner_id;
	struct list_head *next, *first;
	struct smsdvb_frontend_t *sms_frontend = NULL;

	smsendian_handle_rx_message((struct SmsMsgData_S *) phdr);

	switch (phdr->msgSrcId)
	{
		case HIF_TASK:
		case DLU_TASK:
			tuner_id = 0;
			break;
		case HIF_TASK_SLAVE:
			tuner_id = 1;
			break;
		case HIF_TASK_SLAVE2:
			tuner_id = 2;
			break;
		case HIF_TASK_SLAVE3:
			tuner_id = 3;
			break;
		default:
			sms_err("Recieved message from unknown src %d", phdr->msgSrcId);
			tuner_id = -1;
			break;
	}

	first = &client->fe_list;
	for (next = first->next; next != first;next = next->next) 
	{
		sms_frontend = (struct smsdvb_frontend_t*) next;
		if (sms_frontend->tuner_id == tuner_id)
			break;		
	}
	if ((void*)next == (void*)first)
	{
		sms_err("Recieved message from unknown tuner_id (src=%d). Discarding message.", phdr->msgSrcId);
		smscore_putbuffer(client->coredev, cb);
		return 0;
	}


	switch (phdr->msgType) {
	case MSG_SMS_DVBT_BDA_DATA:
		dvb_dmx_swfilter(&sms_frontend->demux, (u8 *)(phdr + 1),
				 cb->size - sizeof(struct SmsMsgHdr_S));
		break;

	case MSG_SMS_RF_TUNE_RES:
	case MSG_SMS_ISDBT_TUNE_RES:
		sms_info("MSG_SMS_RF_TUNE_RES");
		complete(&sms_frontend->tune_done);
		break;

	case MSG_SMS_SIGNAL_DETECTED_IND:
		sms_info("MSG_SMS_SIGNAL_DETECTED_IND");
		sms_frontend->reception_data.IsDemodLocked = true;
		is_status_update = true;
		break;

	case MSG_SMS_NO_SIGNAL_IND:
		sms_info("MSG_SMS_NO_SIGNAL_IND");
		sms_frontend->reception_data.IsDemodLocked = false;
		is_status_update = true;
		break;

	case MSG_SMS_TRANSMISSION_IND: {
		struct RECEPTION_STATISTICS_S *pReceptionData =
				&sms_frontend->reception_data;
		struct TRANSMISSION_STATISTICS_S *pTrnsInd = 
				(struct TRANSMISSION_STATISTICS_S*)pMsgData;
		sms_info("MSG_SMS_TRANSMISSION_IND");

		/* update reception data */
		pReceptionData->IsDemodLocked = pTrnsInd->IsDemodLocked;
		is_status_update = true;
		break;
	}
	case MSG_SMS_HO_PER_SLICES_IND: {
		struct RECEPTION_STATISTICS_S *pReceptionData =
				&sms_frontend->reception_data;
		/*sms_info("MSG_SMS_HO_PER_SLICES_IND");*/
		pReceptionData->IsRfLocked = pMsgData[16];
		pReceptionData->IsDemodLocked = pMsgData[17];
		pReceptionData->ModemState = pMsgData[12];
		pReceptionData->SNR = pMsgData[1];
		pReceptionData->BER = pMsgData[13];
		pReceptionData->RSSI = pMsgData[14];
		CORRECT_STAT_RSSI(sms_frontend->reception_data);

		pReceptionData->InBandPwr = (s32) pMsgData[2];
		pReceptionData->CarrierOffset = (s32) pMsgData[15];
		pReceptionData->TotalTSPackets = pMsgData[3];
		pReceptionData->ErrorTSPackets = pMsgData[4];

		/* TS PER */
		if ((pMsgData[3] + pMsgData[4])	> 0) {
			pReceptionData->TS_PER = (pMsgData[4] * 100) / (pMsgData[3] + pMsgData[4]);
		} else {
			pReceptionData->TS_PER = 0;
		}

		pReceptionData->BERBitCount = pMsgData[18];
		pReceptionData->BERErrorCount = pMsgData[19];

		pReceptionData->MRC_SNR = pMsgData[20];
		pReceptionData->MRC_InBandPwr = pMsgData[21];
		pReceptionData->MRC_RSSI = pMsgData[22];

		is_status_update = true;
		break;
	}
	case MSG_SMS_GET_STATISTICS_RES: {
		struct SMSHOSTLIB_STATISTICS_S *p_dvb_stats = (void *)(pMsgData+1);
		struct RECEPTION_STATISTICS_S *pReceptionData =	&sms_frontend->reception_data;
		sms_info("MSG_SMS_GET_STATISTICS_RES");
		is_status_update = true;
		smsdvb_update_dvb_stats(pReceptionData, p_dvb_stats);
		complete(&sms_frontend->get_stats_done);
		break;
	}
	case MSG_SMS_GET_STATISTICS_EX_RES: {
		struct SMSHOSTLIB_STATISTICS_ISDBT_S  *p_isdbt_stats = (void *)pMsgData+1;
		struct RECEPTION_STATISTICS_S *pReceptionData =	&sms_frontend->reception_data;
		sms_info("MSG_SMS_GET_STATISTICS_EX_RES");
		is_status_update = true;
		smsdvb_update_isdbt_stats(pReceptionData, p_isdbt_stats);
		complete(&sms_frontend->get_stats_done);
		break;
	}
	case MSG_SMS_INIT_DEVICE_RES:
		sms_err("MSG_SMS_INIT_DEVICE_RES");
		break;
	case MSG_SMS_SLAVE_DEVICE_DETECTED:
		sms_err("MSG_SMS_SLAVE_DEVICE_DETECTED");
		break;

	default:
		sms_info("Unhandled message %d", phdr->msgType);

	}
	smscore_putbuffer(client->coredev, cb);

	if (is_status_update) {
		if (sms_frontend->reception_data.IsDemodLocked) {
			sms_frontend->fe_status = FE_HAS_SIGNAL | FE_HAS_CARRIER
				| FE_HAS_VITERBI | FE_HAS_SYNC | FE_HAS_LOCK;
			sms_board_dvb3_event(sms_frontend, DVB3_EVENT_FE_LOCK);
			if (sms_frontend->reception_data.ErrorTSPackets == 0)
				sms_board_dvb3_event(sms_frontend, DVB3_EVENT_UNC_OK);
			else
				sms_board_dvb3_event(sms_frontend, DVB3_EVENT_UNC_ERR);

		} else {
			if (sms_frontend->reception_data.IsRfLocked)
				sms_frontend->fe_status = FE_HAS_SIGNAL | FE_HAS_CARRIER;
			else
				sms_frontend->fe_status = 0;
			sms_board_dvb3_event(sms_frontend, DVB3_EVENT_FE_UNLOCK);
		}
	}

	return 0;
}

static void smsdvb_destroy_client(struct smsdvb_client_t *client)
{
	/* must be called under clientslock */
	list_del(&client->entry);
	while (!list_empty(&client->fe_list))
	{
		struct smsdvb_frontend_t *frontend;
		frontend = (struct smsdvb_frontend_t*)client->fe_list.next;
		sms_info("Releasing dmxdev 0x%p",  (void*)&frontend->dmxdev);
		dvb_dmxdev_release(&frontend->dmxdev);
		sms_info("now Releasing demux 0x%p", (void*)&frontend->demux);
		dvb_dmx_release(&frontend->demux);
		sms_info("Removing FE 0x%p", &frontend->fe);
		dvb_unregister_frontend(&frontend->fe);
		dvb_frontend_detach(&frontend->fe);
		list_del (&frontend->entry);
		sms_info("Frontend removed.");
		kfree (frontend);
	}
	sms_info("Removing the adapter 0x%p", &client->adapter);
	dvb_unregister_adapter(&client->adapter);
	sms_info("Adapter removed.");
	kfree(client);
}

static void smsdvb_unregister_client(struct smsdvb_client_t *client)
{
	sms_info("unregistering the sms client");
	if (client->smsclient)
	{
		smscore_unregister_client(client->smsclient);
	}
	client->smsclient = NULL;
	client->coredev = NULL;
}

static void smsdvb_onremove(void *context)
{
	kmutex_lock(&g_smsdvb_clientslock);
	sms_info("unregistering the client");
	smsdvb_unregister_client((struct smsdvb_client_t *) context);
	kmutex_unlock(&g_smsdvb_clientslock);
}

static int smsdvb_start_feed(struct dvb_demux_feed *feed)
{
	struct smsdvb_frontend_t* sms_frontend = container_of(feed->demux, struct smsdvb_frontend_t, demux);
	struct smsdvb_client_t *client = sms_frontend->client;
	struct SmsMsgData_S PidMsg;
	if (!client->smsclient)
	{
		sms_err("device does not exists");
		return -ENODEV;
	}

	sms_info("add pid %d, tuner %d (dst=%d)", feed->pid, sms_frontend->tuner_id,
					hif_task_of_device[sms_frontend->tuner_id]);

	PidMsg.xMsgHeader.msgSrcId = DVBT_BDA_CONTROL_MSG_ID;
	PidMsg.xMsgHeader.msgDstId = hif_task_of_device[sms_frontend->tuner_id];
	PidMsg.xMsgHeader.msgFlags = 0;
	PidMsg.xMsgHeader.msgType  = MSG_SMS_ADD_PID_FILTER_REQ;
	PidMsg.xMsgHeader.msgLength = sizeof(PidMsg);
	PidMsg.msgData[0] = feed->pid;

	smsendian_handle_tx_message((struct SmsMsgHdr_S *)&PidMsg);
	return smsclient_sendrequest(client->smsclient,
				     &PidMsg, sizeof(PidMsg));
}

static int smsdvb_stop_feed(struct dvb_demux_feed *feed)
{
	struct smsdvb_frontend_t* sms_frontend = container_of(feed->demux, struct smsdvb_frontend_t, demux);
	struct smsdvb_client_t *client = sms_frontend->client;
	struct SmsMsgData_S PidMsg;
	if (!client->smsclient)
	{
		sms_err("device does not exists");
		return -ENODEV;
	}
	PidMsg.xMsgHeader.msgSrcId = DVBT_BDA_CONTROL_MSG_ID;
	PidMsg.xMsgHeader.msgDstId = hif_task_of_device[sms_frontend->tuner_id];
	PidMsg.xMsgHeader.msgFlags = 0;
	PidMsg.xMsgHeader.msgType  = MSG_SMS_REMOVE_PID_FILTER_REQ;
	PidMsg.xMsgHeader.msgLength = sizeof(PidMsg);
	PidMsg.msgData[0] = feed->pid;

	smsendian_handle_tx_message((struct SmsMsgHdr_S *)&PidMsg);
	return smsclient_sendrequest(client->smsclient,
				     &PidMsg, sizeof(PidMsg));
}

static int smsdvb_sendrequest_and_wait(struct smsdvb_client_t *client,
					void *buffer, size_t size,
					struct completion *completion)
{
	int rc;
	if (!client->smsclient)
	{
		sms_err("device does not exists");
		return -ENODEV;
	}
	smsendian_handle_tx_message((struct SmsMsgHdr_S *)buffer);
	rc = smsclient_sendrequest(client->smsclient, buffer, size);
	if (rc < 0)
		return rc;
	return wait_for_completion_timeout(completion,
					   msecs_to_jiffies(200)) ?
						0 : -ETIME;
}

static int smsdvb_get_statistics_ex(struct dvb_frontend *fe) 
{

	struct SmsMsgHdr_S Msg;
	struct sms_properties_t properties;
	struct smsdvb_frontend_t* sms_frontend = container_of(fe, struct smsdvb_frontend_t, fe);
	struct smsdvb_client_t *client = sms_frontend->client;

	Msg.msgSrcId = DVBT_BDA_CONTROL_MSG_ID;
	Msg.msgDstId = hif_task_of_device[sms_frontend->tuner_id];
	Msg.msgFlags = 0;
	Msg.msgLength = sizeof(Msg);

	smscore_get_device_properties(client->coredev, &properties);
	if (properties.mode == SMSHOSTLIB_DEVMD_DVBT || 
	    properties.mode == SMSHOSTLIB_DEVMD_DVBT_BDA ||
	    properties.mode == SMSHOSTLIB_DEVMD_DVBH)
	{
		Msg.msgType = MSG_SMS_GET_STATISTICS_REQ;
	}
	else
	{
		Msg.msgType = MSG_SMS_GET_STATISTICS_EX_REQ;
	}
	sms_info("get statistics, tuner %d (dst=%d)", sms_frontend->tuner_id,
					hif_task_of_device[sms_frontend->tuner_id]);

	smsendian_handle_tx_message((struct SmsMsgHdr_S *)&Msg);
	return smsdvb_sendrequest_and_wait(client, &Msg, sizeof(Msg), &sms_frontend->get_stats_done);

}

static inline int led_feedback(struct smsdvb_frontend_t* sms_frontend)
{
	if (sms_frontend->fe_status & FE_HAS_LOCK)
	{
		return sms_board_led_feedback(sms_frontend->client->coredev,
				(sms_frontend->reception_data.BER== 0) ? SMS_LED_HI : SMS_LED_LO);
	}
	return sms_board_led_feedback(sms_frontend->client->coredev, SMS_LED_OFF);
}

static int smsdvb_read_status(struct dvb_frontend *fe, fe_status_t *stat)
{
	int rc;
	struct smsdvb_frontend_t* sms_frontend = container_of(fe, struct smsdvb_frontend_t, fe);
	sms_debug("Reading FE status");
	if (!sms_frontend->client->smsclient)
	{
		sms_debug("tried to access device which was plugged out");
		return -ENODEV;
	}
	rc = smsdvb_get_statistics_ex(fe);
	*stat = sms_frontend->fe_status;
	led_feedback(sms_frontend);
	sms_debug("status = %d, rc=%d", *stat, rc);
	return rc;
}

static int smsdvb_read_ber(struct dvb_frontend *fe, u32 *ber)
{
	int rc;
	struct smsdvb_frontend_t* sms_frontend = container_of(fe, struct smsdvb_frontend_t, fe);
	if (!sms_frontend->client->smsclient)
	{
		sms_debug("tried to access device which was plugged out");
		return -ENODEV;
	}
	rc = smsdvb_get_statistics_ex(fe);
	*ber = sms_frontend->reception_data.BER;
	led_feedback(sms_frontend);
	return rc;
}

static int smsdvb_read_signal_strength(struct dvb_frontend *fe, u16 *strength)
{
	int rc;

	struct smsdvb_frontend_t* sms_frontend = container_of(fe, struct smsdvb_frontend_t, fe);
	if (!sms_frontend->client->smsclient)
	{
		sms_debug("tried to access device which was plugged out");
		return -ENODEV;
	}
	rc = smsdvb_get_statistics_ex(fe);
	if (sms_frontend->reception_data.InBandPwr < -95)
		*strength = 0;
		else if (sms_frontend->reception_data.InBandPwr > -29)
			*strength = 100;
		else
			*strength =
				(sms_frontend->reception_data.InBandPwr
				+ 95) * 3 / 2;
	led_feedback(sms_frontend);
	return rc;
}

static int smsdvb_read_snr(struct dvb_frontend *fe, u16 *snr)
{
	int rc;
	struct smsdvb_frontend_t* sms_frontend = container_of(fe, struct smsdvb_frontend_t, fe);
	if (!sms_frontend->client->smsclient)
	{
		sms_debug("tried to access device which was plugged out");
		return -ENODEV;
	}
	rc = smsdvb_get_statistics_ex(fe);
	*snr = sms_frontend->reception_data.SNR;
	led_feedback(sms_frontend);
	return rc;
}

static int smsdvb_read_ucblocks(struct dvb_frontend *fe, u32 *ucblocks)
{
	int rc;
	struct smsdvb_frontend_t* sms_frontend = container_of(fe, struct smsdvb_frontend_t, fe);
	if (!sms_frontend->client->smsclient)
	{
		sms_debug("tried to access device which was plugged out");
		return -ENODEV;
	}
	rc = smsdvb_get_statistics_ex(fe);
	*ucblocks = sms_frontend->reception_data.ErrorTSPackets;
	led_feedback(sms_frontend);
	return rc;
}

static int smsdvb_get_tune_settings(struct dvb_frontend *fe,
				    struct dvb_frontend_tune_settings *tune)
{
	struct smsdvb_frontend_t* sms_frontend = container_of(fe, struct smsdvb_frontend_t, fe);
	if (!sms_frontend->client->smsclient)
	{
		sms_debug("tried to access device which was plugged out");
		return -ENODEV;
	}
	sms_info("");
	tune->min_delay_ms = 400;
	tune->step_size = 250000;
	tune->max_drift = 0;
	return 0;
}

static int smsdvb_dvbt_set_frontend(struct dvb_frontend *fe,
				    struct dvb_frontend_parameters *p)
{
	struct dtv_frontend_properties *c = &fe->dtv_property_cache;
	struct smsdvb_frontend_t* sms_frontend = container_of(fe, struct smsdvb_frontend_t, fe);
	struct smsdvb_client_t *client = sms_frontend->client;

	struct SmsMsgData4Args_S Msg, Msg2;
	int ret;
	if (!sms_frontend->client->smsclient)
	{
		sms_debug("tried to access device which was plugged out");
		return -ENODEV;
	}
	sms_info("setting DVB freq to %d on tuner %d", c->frequency, sms_frontend->tuner_id);

	sms_frontend->fe_status = FE_HAS_SIGNAL;
	sms_frontend->event_fe_state = -1;
	sms_frontend->event_unc_state = -1;
	fe->dtv_property_cache.delivery_system = SYS_DVBT;

	Msg.xMsgHeader.msgSrcId = DVBT_BDA_CONTROL_MSG_ID;
	Msg.xMsgHeader.msgDstId = hif_task_of_device[sms_frontend->tuner_id];
	Msg.xMsgHeader.msgFlags = 0;
	Msg.xMsgHeader.msgType = MSG_SMS_RF_TUNE_REQ;
	Msg.xMsgHeader.msgLength = sizeof(Msg);
	Msg.msgData[0] = c->frequency;
	Msg.msgData[2] = 12000000;
	Msg.msgData[3] = 0;

	switch (c->bandwidth_hz / 1000000) {
	case 8:
		Msg.msgData[1] = BW_8_MHZ;
		break;
	case 7:
		Msg.msgData[1] = BW_7_MHZ;
		break;
	case 6:
		Msg.msgData[1] = BW_6_MHZ;
		break;
	case 0:
		return -EOPNOTSUPP;
	default:
		return -EINVAL;
	}
	memcpy (&Msg2, &Msg, sizeof(Msg2));
	/* Disable LNA, if any. An error is returned if no LNA is present */
	sms_info("setting LNA");

	ret = sms_board_lna_control(client->coredev, 0);
	if (ret == 0) {
		fe_status_t status;

		sms_info("set LNA, tuner %d (dst=%d)", sms_frontend->tuner_id,
					hif_task_of_device[sms_frontend->tuner_id]);
		/* tune with LNA off at first */
		ret = smsdvb_sendrequest_and_wait(client, &Msg, sizeof(Msg),
						  &sms_frontend->tune_done);
		sms_info("Tune done. returned err=%d", ret);
		smsdvb_read_status(fe, &status);

		if (status & FE_HAS_LOCK)
		{
			sms_info("Aquired lock.");
			return ret;
		}

		/* previous tune didn't lock - enable LNA and tune again */
		sms_info("No lock. set LNA again");
		sms_board_lna_control(client->coredev, 1);
	}
	sms_info("Sending message");
	sms_info("set LNA2, tuner %d (dst=%d)", sms_frontend->tuner_id,
					hif_task_of_device[sms_frontend->tuner_id]);
	ret = smsdvb_sendrequest_and_wait(client, &Msg2, sizeof(Msg2),
					   &sms_frontend->tune_done);
	sms_info("Tune done. returned err=%d", ret);
	return ret;
}

static int smsdvb_isdbt_set_frontend(struct dvb_frontend *fe,
				     struct dvb_frontend_parameters *p)
{
	struct dtv_frontend_properties *c = &fe->dtv_property_cache;
	struct smsdvb_frontend_t* sms_frontend = container_of(fe, struct smsdvb_frontend_t, fe);
	struct smsdvb_client_t *client = sms_frontend->client;
	struct SmsMsgData4Args_S Msg;


	if (fe->dtv_property_cache.delivery_system != SYS_ISDBT)
	{
		fe->dtv_property_cache.delivery_system = SYS_ISDBT;
		c->isdbt_sb_segment_count = 0;
		c->isdbt_sb_segment_idx = 1;
	}

	Msg.xMsgHeader.msgSrcId = DVBT_BDA_CONTROL_MSG_ID;
	Msg.xMsgHeader.msgDstId = hif_task_of_device[sms_frontend->tuner_id];
	Msg.xMsgHeader.msgFlags = 0;
	Msg.xMsgHeader.msgType   = MSG_SMS_ISDBT_TUNE_REQ;
	Msg.xMsgHeader.msgLength = sizeof(Msg);
	Msg.msgData[0] = c->frequency;
	sms_info("freq %d seg %d",
		  c->frequency, c->isdbt_sb_segment_count);

	switch (c->isdbt_sb_segment_count) {
	case 3:
		Msg.msgData[1] = BW_ISDBT_3SEG;
		break;
	case 1:
		Msg.msgData[1] = BW_ISDBT_1SEG;
		break;
	case 0:	/* AUTO */
	case -1:	
		switch (c->bandwidth_hz / 1000000) {
		case 8:
		case 7:
			c->isdbt_sb_segment_count = 13;
			Msg.msgData[1] = BW_ISDBT_13SEG;
			break;
		case 6:
			c->isdbt_sb_segment_idx = 0;
			Msg.msgData[1] = BW_ISDBT_1SEG;
			break;
		default: /* Assumes 6 MHZ bw */
			c->isdbt_sb_segment_count = 1;
			c->bandwidth_hz = 6000000;
			Msg.msgData[1] = BW_ISDBT_3SEG;
			break;
		}
		break;
	default:
		sms_info("Segment count %d not supported", c->isdbt_sb_segment_count);
		return -EINVAL;
	}
        Msg.msgData[0] = c->frequency;
	Msg.msgData[2] = 12000000;
	Msg.msgData[3] = c->isdbt_sb_segment_idx;

	sms_info("%s: freq %d bwwidth %d crystal %d, segindex %d\n", __func__,
		 Msg.msgData[0], Msg.msgData[1], Msg.msgData[2],
		 Msg.msgData[3]);
	return smsdvb_sendrequest_and_wait(client, &Msg, sizeof(Msg),
					   &sms_frontend->tune_done);
}

static int smsdvb_set_frontend(struct dvb_frontend *fe,
			       struct dvb_frontend_parameters *fep)
{
	struct sms_properties_t properties;

	struct smsdvb_frontend_t* sms_frontend = container_of(fe, struct smsdvb_frontend_t, fe);
	struct smsdvb_client_t *client = sms_frontend->client;
	if (!sms_frontend->client->smsclient)
	{
		sms_debug("tried to access device which was plugged out");
		return -ENODEV;
	}
	sms_info("setting the front end");

	sms_frontend->fe_status = FE_HAS_SIGNAL;
	sms_frontend->event_fe_state = -1;
	sms_frontend->event_unc_state = -1;

	smscore_get_device_properties(client->coredev, &properties);
	switch (properties.mode) {
	case SMSHOSTLIB_DEVMD_DVBT:
	case SMSHOSTLIB_DEVMD_DVBT_BDA:
		return smsdvb_dvbt_set_frontend(fe, fep);
	case SMSHOSTLIB_DEVMD_ISDBT:
	case SMSHOSTLIB_DEVMD_ISDBT_BDA:
		return smsdvb_isdbt_set_frontend(fe, fep);
	default:
		sms_err("SMS Device mode is not set for DVB operation.");
		return -EINVAL;
	}
}

static int smsdvb_get_frontend(struct dvb_frontend *fe,
			       struct dvb_frontend_parameters *fep)
{
	struct smsdvb_frontend_t* sms_frontend = container_of(fe, struct smsdvb_frontend_t, fe);
	struct smsdvb_client_t *client = sms_frontend->client;

	if (!sms_frontend->client->smsclient)
	{
		sms_debug("tried to access device which was plugged out");
		return -ENODEV;
	}
	memcpy(fep, &client->fe_params,
	       sizeof(struct dvb_frontend_parameters));


//	fep->frequency = 
//	fep->inversion = INVERSION_AUTO;

//	fep->u.qpsk.symbol_rate
//	fep->u.qpsk.fec_inner
//
//	fep->u.qam.symbol_rate
//	fep->u.qam.fec_inner
//	fep->u.qam.modulation
//
//	fep->u.vsb.modulation
//
//	fep->u.ofdm.bandwidth
//	fep->u.ofdm.code_rate_HP
//	fep->u.ofdm.code_rate_LP
//	fep->u.ofdm.constellation
//	fep->u.ofdm.transmission_mode
//	fep->u.ofdm.guard_interval
//	fep->u.ofdm.hierarchy_information
//
	return 0;
}

static int smsdvb_init(struct dvb_frontend *fe)
{
	struct smsdvb_frontend_t* sms_frontend = container_of(fe, struct smsdvb_frontend_t, fe);
	struct smsdvb_client_t *client = sms_frontend->client;
	if (!sms_frontend->client->smsclient)
	{
		sms_debug("tried to access device which was plugged out");
		return -ENODEV;
	}
	sms_board_power(client->coredev, 1);

	sms_board_dvb3_event(sms_frontend, DVB3_EVENT_INIT);
	return 0;
}

static int smsdvb_sleep(struct dvb_frontend *fe)
{
	struct smsdvb_frontend_t* sms_frontend = container_of(fe, struct smsdvb_frontend_t, fe);
	struct smsdvb_client_t *client = sms_frontend->client;
	if (!sms_frontend->client->smsclient)
	{
		sms_debug("tried to access device which was plugged out");
		return -ENODEV;
	}
	sms_board_led_feedback(client->coredev, SMS_LED_OFF);
	sms_board_power(client->coredev, 0);

	sms_board_dvb3_event(sms_frontend, DVB3_EVENT_SLEEP);
	return 0;
}

static void smsdvb_release(struct dvb_frontend *fe)
{
	/* Do nothing*/
}


int smsdvb_get_property(struct dvb_frontend *fe, struct dtv_property *tvp)
{

	struct smsdvb_frontend_t* sms_frontend = container_of(fe, struct smsdvb_frontend_t, fe);
	struct smsdvb_client_t *client = sms_frontend->client;
	if (!sms_frontend->client->smsclient)
	{
		sms_debug("tried to access device which was plugged out");
		return -ENODEV;
	}

	sms_debug ("smsdvb_get_property: (before) tvp->cmd=%d tvp->data=%d", tvp->cmd, tvp->u.data);
	if (tvp->cmd ==  DTV_DELIVERY_SYSTEM)
	{
		struct sms_properties_t props;
		smscore_get_device_properties(client->coredev, &props);
		switch (props.mode)
		{
		case SMSHOSTLIB_DEVMD_DVBT_BDA:
			tvp->u.data = SYS_DVBT;
			break;
		case SMSHOSTLIB_DEVMD_DVBH:
			tvp->u.data = SYS_DVBH;
			break;
		case SMSHOSTLIB_DEVMD_ISDBT_BDA:
			tvp->u.data = SYS_ISDBT;
			break;
		case SMSHOSTLIB_DEVMD_ATSC:
			tvp->u.data = SYS_ATSCMH;	
			break;
		case SMSHOSTLIB_DEVMD_CMMB:
			tvp->u.data = SYS_CMMB;
			break;
		case SMSHOSTLIB_DEVMD_DAB_TDMB:
			tvp->u.data = SYS_DAB;
			break;
		default:
			tvp->u.data = SYS_UNDEFINED;
			break;
		}
	}
	sms_debug ("smsdvb_get_property: (after) tvp->cmd=%d tvp->data=%d", tvp->cmd, tvp->u.data);
	return 0;
}

int smsdvb_set_property(struct dvb_frontend *fe, struct dtv_property *tvp) 
{
	struct smsdvb_frontend_t* sms_frontend = container_of(fe, struct smsdvb_frontend_t, fe);
#if 0
	struct smsdvb_client_t *client = sms_frontend->client;
#endif
	int request_mode=SMSHOSTLIB_DEVMD_NONE;
	if (!sms_frontend->client->smsclient)
	{
		sms_debug("tried to access device which was plugged out");
		return -ENODEV;
	}
	sms_debug ("smsdvb_set_property: tvp->cmd=%d tvp->data=%d", tvp->cmd, tvp->u.data);
	if (tvp->cmd ==  DTV_DELIVERY_SYSTEM)
	{
		switch (tvp->u.data)
		{
		case SYS_DVBT:
			request_mode = SMSHOSTLIB_DEVMD_DVBT_BDA;
			break;
		case SYS_DVBH:
			request_mode = SMSHOSTLIB_DEVMD_DVBH;
			break;
		case SYS_ISDBT:
			request_mode = SMSHOSTLIB_DEVMD_ISDBT_BDA;
			break;
		case SYS_ATSCMH:
			request_mode = SMSHOSTLIB_DEVMD_ATSC;
			break;
		case SYS_CMMB:
			request_mode = SMSHOSTLIB_DEVMD_CMMB;
			break;
		case SYS_DAB:
			request_mode = SMSHOSTLIB_DEVMD_DAB_TDMB;
			break;
		default:
			request_mode = SMSHOSTLIB_DEVMD_NONE;
			break;
		}
		if (request_mode == SMSHOSTLIB_DEVMD_NONE)
			return -EOPNOTSUPP;
		sms_debug ("Setting device mode to = %d", request_mode);

/* This section is currently canceled since all of known players doesn't support this feature*/
/*Therefore player is configured as DVB-T to show ISDBT and causes mode change in the SMS device*/
/*Mode change is currently nots supported until players will configure the mode properly*/
#if 0
		if (smscore_set_device_mode(client->coredev, request_mode))
		{
			sms_err ("Setting device mode to %d failed!", request_mode);
			return -EOPNOTSUPP;
		}
#endif
		return 0;
			
	}
	return 0;
}

static struct dvb_frontend_ops smsdvb_fe_ops = {
	.info = {
		.name			= "Siano Mobile Digital MDTV Receiver",
		.type			= FE_OFDM,
	        .frequency_min = 164000000,
		.frequency_max		= 867250000,
		.frequency_stepsize	= 250000,
		.caps = FE_CAN_INVERSION_AUTO |
			FE_CAN_FEC_1_2 | FE_CAN_FEC_2_3 | FE_CAN_FEC_3_4 |
			FE_CAN_FEC_5_6 | FE_CAN_FEC_7_8 | FE_CAN_FEC_AUTO |
			FE_CAN_QPSK | FE_CAN_QAM_16 | FE_CAN_QAM_64 |
			FE_CAN_QAM_AUTO | FE_CAN_TRANSMISSION_MODE_AUTO |
			FE_CAN_GUARD_INTERVAL_AUTO |
			FE_CAN_RECOVER |
			FE_CAN_HIERARCHY_AUTO,
	},

	.release = smsdvb_release,

	.set_frontend = smsdvb_set_frontend,
	.get_frontend = smsdvb_get_frontend,
	.get_tune_settings = smsdvb_get_tune_settings,

	.read_status = smsdvb_read_status,
	.read_ber = smsdvb_read_ber,
	.read_signal_strength = smsdvb_read_signal_strength,
	.read_snr = smsdvb_read_snr,
	.read_ucblocks = smsdvb_read_ucblocks,
	.get_property = smsdvb_get_property,
	.set_property = smsdvb_set_property,


	.init = smsdvb_init,
	.sleep = smsdvb_sleep,
};

static int smsdvb_hotplug(void *coredev, struct device *device, int arrival)
{
	struct smsclient_params_t params;
	int rc, i;
	struct sms_properties_t properties;
	struct list_head *next;
	struct smsdvb_frontend_t *sms_frontend;
	struct smsdvb_client_t *client = NULL;

	smscore_get_device_properties(coredev, &properties);


	/* device removal handled by onremove callback */
	if (!arrival)
	{
		sms_err("device was removed.");
		return 0;
	}

	if ( (properties.mode != SMSHOSTLIB_DEVMD_DVBT_BDA) &&
	     (properties.mode != SMSHOSTLIB_DEVMD_ISDBT_BDA) ) {
		sms_err("SMS Device mode is not set for "
			"DVB operation.");
		return 0;
	}

	/*Look for unused client struct if exists*/
	kmutex_lock(&g_smsdvb_clientslock);
	list_for_each(next,  &g_smsdvb_clients) 
	{
		client = (struct smsdvb_client_t *)next;
		if (!client->smsclient)
		{
			client->coredev = coredev;
			sms_debug("Found old empty client - will be used.");
			break;
		}
	}
	kmutex_unlock(&g_smsdvb_clientslock);
	
	if (!client || client->smsclient) 
	{
		sms_debug("Did not find old empty client - create new.");
		client = kzalloc(sizeof(struct smsdvb_client_t), GFP_KERNEL);
		if (!client) {
			sms_err("kmalloc() failed");
			return -ENOMEM;
		}
		client->smsclient = NULL;
		client->coredev = coredev;
		INIT_LIST_HEAD(&client->fe_list);
		kmutex_lock(&g_smsdvb_clientslock);
		list_add(&client->entry, &g_smsdvb_clients);
		kmutex_unlock(&g_smsdvb_clientslock);
		/* register dvb adapter */
		rc = dvb_register_adapter(&client->adapter,
					  sms_get_board(properties.board_id)->name,
					  THIS_MODULE, device, adapter_nr);
		if (rc < 0) {
			sms_err("dvb_register_adapter() failed %d", rc);
			goto adapter_error;
		}

		sms_err("adapter 0x%p registered", &client->adapter);
		/* init and register frontend */
	}



	i = 0;	
	list_for_each(next,  &client->fe_list) 
	{ /*Count how many frontends in current client.*/
		i++;
	}

	sms_debug("client struct has already %d frontends, out of %d. creating rest of required nodes.", i, properties.num_of_tuners);
	/*Now create frontends for each additional tuner in new device*/
	for (; i < properties.num_of_tuners ; i++)
	{

		sms_frontend = kmalloc(sizeof(struct smsdvb_frontend_t), GFP_KERNEL);
		if (!sms_frontend)
		{
			rc = -ENOMEM;
			sms_err("Not enough memory for frontend allocation");
			goto frontend_error;
		}
		else
		{			
			INIT_LIST_HEAD(&sms_frontend->entry);
			init_completion(&sms_frontend->tune_done);
			init_completion(&sms_frontend->get_stats_done);
			sms_frontend->event_fe_state = -1;
			sms_frontend->event_unc_state = -1;
			sms_frontend->client = client;

			/* init dvb demux */
			sms_frontend->demux.dmx.capabilities = DMX_TS_FILTERING;
			sms_frontend->demux.filternum = 32;
			sms_frontend->demux.feednum = 32;
			sms_frontend->demux.start_feed = smsdvb_start_feed;
			sms_frontend->demux.stop_feed = smsdvb_stop_feed;
			sms_debug ("Initializing dmx 0x%p", &sms_frontend->demux);
			rc = dvb_dmx_init(&sms_frontend->demux);	
			if (rc < 0) {	
				sms_err("dvb_dmx_init failed %d", rc);
				kfree (sms_frontend);
				goto frontend_error;
			}

			/* init dmxdev */
			sms_frontend->dmxdev.filternum = 32;
			sms_frontend->dmxdev.demux = &sms_frontend->demux.dmx;
			sms_frontend->dmxdev.capabilities = 0;

			rc = dvb_dmxdev_init(&sms_frontend->dmxdev, &client->adapter);
			if (rc < 0) {
				sms_err("dvb_dmxdev_init failed %d", rc);
				dvb_dmx_release(&sms_frontend->demux);
				kfree (sms_frontend);
				goto frontend_error;
			}

			memcpy(&sms_frontend->fe.ops, &smsdvb_fe_ops, sizeof(struct dvb_frontend_ops));
			sms_frontend->fe.frontend_priv = sms_frontend;
			sms_frontend->client = client;
			sms_frontend->tuner_id = i;
			sms_debug ("registering frontend 0x%p", &sms_frontend->fe);
			rc = dvb_register_frontend(&client->adapter, &sms_frontend->fe);
			if (rc < 0) {
				dvb_dmxdev_release(&sms_frontend->dmxdev);
				dvb_dmx_release(&sms_frontend->demux);
				kfree (sms_frontend);
				sms_err("frontend registration failed %d", rc);
				goto frontend_error;
			}
			list_add(&sms_frontend->entry, &client->fe_list);
			sms_board_dvb3_event(sms_frontend, DVB3_EVENT_HOTPLUG);
		}
	}

	params.initial_id = 1;
	params.data_type = MSG_SMS_DVBT_BDA_DATA;
	params.onresponse_handler = smsdvb_onresponse;
	params.onremove_handler = smsdvb_onremove;
	params.context = client;


	rc = smscore_register_client(coredev, &params, &client->smsclient);
	if (rc < 0) {
		sms_err("smscore_register_client() failed %d", rc);
		goto client_error;
	}



	sms_info("success");
	sms_board_setup(coredev);

	return 0;

client_error:
frontend_error:
	list_for_each(next,  &client->fe_list) 
	{
		sms_frontend = (struct smsdvb_frontend_t *)next;
		dvb_unregister_frontend(&sms_frontend->fe);
		dvb_dmxdev_release(&sms_frontend->dmxdev);
		dvb_dmx_release(&sms_frontend->demux);
		list_del (&sms_frontend->entry);
		kfree (sms_frontend);
		
	}
	dvb_unregister_adapter(&client->adapter);

adapter_error:
	list_del(&client->entry);
	kfree(client);
	return rc;
}

int smsdvb_register(void)
{
	int rc;
	INIT_LIST_HEAD(&g_smsdvb_clients);
	kmutex_init(&g_smsdvb_clientslock);
	rc = smscore_register_hotplug(smsdvb_hotplug);

	return rc;
}

void smsdvb_unregister(void)
{
	smscore_unregister_hotplug(smsdvb_hotplug);

	kmutex_lock(&g_smsdvb_clientslock);

	while (!list_empty(&g_smsdvb_clients))
	{
	       	smsdvb_unregister_client((struct smsdvb_client_t *) g_smsdvb_clients.next);
		smsdvb_destroy_client((struct smsdvb_client_t *) g_smsdvb_clients.next);
	}

	kmutex_unlock(&g_smsdvb_clientslock);
}


