
#if TSP_PATCH

/*
	Header
	2013-11-07 오후 13:51:52 - by sateyang@eplus.co.kr
	Modified the start position of stage can select between T6 normal message or T6 cal message
	Refer to the define MXT_PATCH_START_FROM_NORMAL

	2013-11-05 오후 13:51:52 - by wj.hong@atmel.com
	Added T9 status message to ITEM for checking the  'PRESS' and 'RELEASE' condition .
	Refer to the define  MXT_PATCH_T9STATUS_CHK
*/

#define MXT_PATCH_LOCK_CHECK		1
#define MXT_PATCH_SUPP_CHECK		1
#define MXT_PATCH_STAGE_RESET		0 // BITPARK 'struct mxt_patch' has no member named 'stage_timestamp'
#define MXT_PATCH_TOUCH_SLOT		1
#define MXT_PATCH_START_FROM_NORMAL	1

#define TIME_WRAP_AROUND(x, y)		(((y)>(x)) ? (y)-(x) : (0-(x))+(y))

#define MXT_PATCH_MAGIC				0x52296416
#define MXT_PATCH_VERSION			1
#define MXT_PATCH_MAX_STAGE			255
#define MXT_PATCH_MAX_TLINE			255
#define MXT_PATCH_MAX_TRIG			255
#define MXT_PATCH_MAX_ITEM			255
#define MXT_PATCH_MAX_TYPE			255
#define MXT_PATCH_MAX_CON			255
#define MXT_PATCH_MAX_EVENT			255
#define MXT_PATCH_MAX_MSG_SIZE		10

#define MXT_XML_CON_NONE		"0"			//0
#define MXT_XML_CON_EQUAL		"="			//1
#define MXT_XML_CON_BELOW		"<"			//2
#define MXT_XML_CON_ABOVE		">"			//3
#define MXT_XML_CON_PLUS		"+"			//4
#define MXT_XML_CON_MINUS		"-"			//5
#define MXT_XML_CON_MUL			"*"			//6
#define MXT_XML_CON_DIV			"/"			//7
#define MXT_XML_CON_MASK		"&"			//8

#define MXT_XML_SRC_NONE		"0"			//0
#define MXT_XML_SRC_CHRG		"TA"		//1
#define MXT_XML_SRC_FCNT		"FCNT"		//2
#define MXT_XML_SRC_AREA		"AREA"		//3
#define MXT_XML_SRC_AMP			"AMP"		//4
#define MXT_XML_SRC_SUM			"SUM"		//5
#define MXT_XML_SRC_TCH			"TCH"		//6
#define MXT_XML_SRC_ATCH		"ATCH"		//7
#define MXT_XML_SRC_KCNT		"KCNT"		//8
#define MXT_XML_SRC_KVAL		"KVAL"		//9
#define MXT_XML_SRC_EVT			"EVT"		//10	//MXT_PATCH_TOUCH_SLOT
#define MXT_XML_SRC_SCNT		"SCNT"		//11	//MXT_PATCH_TOUCH_SLOT

#define MXT_XML_ACT_NONE		"0"			//0
#define MXT_XML_ACT_CAL			"CAL"		//1
#define MXT_XML_ACT_EXTMR		"EXTMR"		//2

enum{
	MXT_PATCH_CON_NONE = 0,
	MXT_PATCH_CON_EQUAL,		//1
	MXT_PATCH_CON_BELOW,		//2
	MXT_PATCH_CON_ABOVE,		//3
	MXT_PATCH_CON_PLUS,			//4
	MXT_PATCH_CON_MINUS,		//5
	MXT_PATCH_CON_MUL,			//6
	MXT_PATCH_CON_DIV,			//7
	MXT_PATCH_CON_MASK,			//8
	//...
	MXT_PATCH_CON_END
};

enum {
	MXT_PATCH_ITEM_NONE	= 0,
	MXT_PATCH_ITEM_CHARG,		//1
	MXT_PATCH_ITEM_FCNT,		//2
	MXT_PATCH_ITEM_AREA,		//3
	MXT_PATCH_ITEM_AMP,			//4
	MXT_PATCH_ITEM_SUM,			//5
	MXT_PATCH_ITEM_TCH,			//6
	MXT_PATCH_ITEM_ATCH,		//7
	MXT_PATCH_ITEM_KCNT,		//8
	MXT_PATCH_ITEM_KVAL,		//9
	MXT_PATCH_ITEM_EVT,			//10 //MXT_PATCH_TOUCH_SLOT
	MXT_PATCH_ITEM_SCNT,		//11 //MXT_PATCH_TOUCH_SLOT
	//...
	MXT_PATCH_ITEM_END
};

enum {
	MXT_PATCH_ACTION_NONE = 0,
	MXT_PATCH_ACTION_CAL,
	MXT_PATCH_ACTION_EXTEND_TIMER,
	MXT_PATCH_ACTION_GOTO_STAGE,
	//...
	MXT_PATCH_ACTION_END
};

struct patch_header{ // 32b
	u32	magic;
	u32	size;
	u32 date;
	u16	version;
	u8	option;
	u8  debug;
	u8	timer_id;
	u8	stage_cnt;
	u8	trigger_cnt;
	u8  event_cnt;
	u8	reserved[12];
};

struct stage_def{	// 8b
	u8	stage_id;
	u8  option;
	u16 stage_period;
	u8	cfg_cnt;
	u8	test_cnt;
#if MXT_PATCH_STAGE_RESET
	u16	reset_period;
#endif
};

struct stage_cfg{	// 4b
	u8	obj_type;
	u8	option;
	u8	offset;
	u8	val;
};

struct test_line{	// 12b
	u8	test_id;
	u8	item_cnt;
	u8	cfg_cnt;
	u8	act_id;
	u16	act_val;
	u16	option;
	u16	check_cnt;
	u8	reserved[2];
};

struct action_cfg{	// 4b
	u8	obj_type;
	u8	option;
	u8	offset;
	u8	val;
};

struct item_val{	// 4b
	u8	val_id;
	u8	val_eq;
	u16 val;
};

struct test_item{	// 8b
	u8	src_id;
	u8	cond;
	u8	reserved[2];
	struct item_val ival;
};

// Message Trigger
struct trigger{		// 12b
	u8 tid;
	u8 option;
	u8 object;
	u8 index;
	u8 match_cnt;
	u8 cfg_cnt;
	u8 reserved[3];
	u8 act_id;
	u16 act_val;
};

struct match{		//8b
	u8 offset;
	u8 cond;
	u16 mask;
	u8 reserved[2];
	u16 val;
};

struct trigger_cfg{	// 4b
	u8	obj_type;
	u8	reserved;
	u8	offset;
	u8	val;
};

// Event
struct user_event{	// 8b
	u8 eid;
	u8 option;
	u8 cfg_cnt;
	u8 reserved[5];
};

struct event_cfg{	// 4b
	u8	obj_type;
	u8	reserved;
	u8	offset;
	u8	val;
};

/*
	Source
*/
#if MXT_PATCH_TOUCH_SLOT
struct touch_slot{
	u8 detect[MXT_MAX_FINGER];
	u8 type[MXT_MAX_FINGER];
	u8 event[MXT_MAX_FINGER];
};
struct touch_slot tch_slot;
#endif

struct test_src{
	int charger;
	int finger_cnt;
	int area;
	int amp;
	int sum_size;
	int tch_ch;
	int atch_ch;
	int key_cnt;
	int key_val;
	int tch_evt;	//MXT_PATCH_TOUCH_SLOT
	int stylus_cnt; //MXT_PATCH_TOUCH_SLOT
};

#if MXT_PATCH_LOCK_CHECK
struct touch_pos{
	u8	tcount[MXT_MAX_FINGER];
	u16	initx[MXT_MAX_FINGER];
	u16	inity[MXT_MAX_FINGER];
	u16	oldx[MXT_MAX_FINGER];
	u16	oldy[MXT_MAX_FINGER];
	u8	locked_id;
	u8	moved_cnt;
	u8	option;
	u8	cal_enable;
	u8	reset_cnt;
	u8	distance;
	u8	maxdiff;
	u8	locked_cnt;
	u8	jitter;
};
#endif

#ifndef __mxt_patch_debug
#define __mxt_patch_debug(_data, ...)	if(data->patch.debug) \
	dev_info(&(_data)->client->dev, __VA_ARGS__);
#endif

#ifndef __mxt_patch_ddebug
#define __mxt_patch_ddebug(_data, ...)	if(data->patch.debug>1) \
	dev_info(&(_data)->client->dev, __VA_ARGS__);
#endif

/* Function Define */
static void mxt_patch_dump_source(struct mxt_data *data,
				bool do_action);
static int mxt_patch_run_stage(struct mxt_data *data);

/* Porting */
static void mxt_patch_calibration(struct mxt_data *data)
{
	mxt_write_object(data, MXT_GEN_COMMANDPROCESSOR_T6,
		MXT_COMMAND_CALIBRATE, 1);
}

static int mxt_patch_start_timer(struct mxt_data *data, u16 period)
{
	struct mxt_object* object;
	int ret = 0;
	u8 t61_reg[5] = {3, 1, 0, 0, 0};

	object = mxt_get_object(data, MXT_SPT_TIMER_T61);
	t61_reg[3] = period & 0xFF;
	t61_reg[4] = (period >> 8) & 0xFF;

	ret = mxt_write_mem(data, object->start_address+
			5*data->patch.timer_id, 5, t61_reg);
	if (!ret) {
		__mxt_patch_debug(data, "START STAGE: %d TIMER[%d] %dms\n",
			data->patch.cur_stage, data->patch.timer_id, period);
	}
	return ret;
}

static int mxt_patch_stop_timer(struct mxt_data *data)
{
	struct mxt_object* object;
	int ret = 0;
	u8 t61_reg[5] = {3, 2, 0, 0, 0};

	object = mxt_get_object(data, MXT_SPT_TIMER_T61);
	ret = mxt_write_mem(data, object->start_address+
			5*data->patch.timer_id, 5, t61_reg);
	if (!ret) {
		__mxt_patch_debug(data, "STOP TIMER[%d]\n",
			data->patch.timer_id);
	}
	return ret;
}

/*
 OPT= 0:ALL, 2:BATT ONLY, 3:TA ONLY
*/
#if TSP_INFORM_CHARGER
static int mxt_patch_check_tacfg(struct mxt_data *data,
	u8 option, u8 ta_mode)
{
	if(option&0x02){
		if(ta_mode){//TA
			if((option&0x01)==0){
				__mxt_patch_ddebug(data, "|- SCFG BATT SKIP");
				return 1;
			}
		}
		else{// BATT
			if((option&0x01)==1){
				__mxt_patch_ddebug(data, "|- SCFG TA SKIP");
				return 1;
			}
		}
	}
	return 0;
}
#endif

static int mxt_patch_write_stage_cfg(struct mxt_data *data,
		struct stage_cfg* pscfg, bool do_action)
{
	if(do_action){
#if TSP_INFORM_CHARGER
		if(mxt_patch_check_tacfg(data, pscfg->option,
			data->charging_mode)){
			return 0;
		}
#endif
		__mxt_patch_debug(data, "|- SCFG_WRITE: OBJECT_TYPE:%d"
			" OFFSET:%d VAL:%d OPT:%d\n",
			pscfg->obj_type, pscfg->offset,
			pscfg->val, pscfg->option);
		mxt_write_object(data, pscfg->obj_type,
			pscfg->offset, pscfg->val);
	}
	return 0;
}

static int mxt_patch_write_action_cfg(struct mxt_data *data,
		struct action_cfg* pacfg, bool do_action)
{
	if(do_action){
#if TSP_INFORM_CHARGER
		if(mxt_patch_check_tacfg(data, pacfg->option,
			data->charging_mode)){
			return 0;
		}
#endif
		__mxt_patch_debug(data, "|-- ACFG_WRITE: OBJECT_TYPE:%d"
			" OFFSET:%d VAL:%d OPT:%d\n",
			pacfg->obj_type, pacfg->offset,
			pacfg->val, pacfg->option);
		mxt_write_object(data, pacfg->obj_type,
			pacfg->offset, pacfg->val);
	}
	return 0;
}

static int mxt_patch_write_trigger_cfg(struct mxt_data *data,
		struct trigger_cfg* ptcfg, bool do_action)
{
	if(do_action){
		__mxt_patch_debug(data, "|-- TCFG_WRITE: OBJECT_TYPE:%d"
			" OFFSET:%d VAL:%d\n",
			ptcfg->obj_type, ptcfg->offset, ptcfg->val);
		mxt_write_object(data, ptcfg->obj_type,
			ptcfg->offset, ptcfg->val);
	}
	return 0;
}

static int mxt_patch_write_event_cfg(struct mxt_data *data,
		struct event_cfg* pecfg, bool do_action)
{
	if(do_action){
		__mxt_patch_debug(data, "|-- ECFG_WRITE: OBJECT_TYPE:%d"
			" OFFSET:%d VAL:%d\n",
			pecfg->obj_type, pecfg->offset, pecfg->val);
		mxt_write_object(data, pecfg->obj_type,
			pecfg->offset, pecfg->val);
	}
	return 0;
}

static int mxt_patch_predefined_action(struct mxt_data *data,
		u8 action_id, u16 action_val, bool do_action)
{
	if(do_action){
		switch(action_id){
			case MXT_PATCH_ACTION_NONE:
				__mxt_patch_debug(data, "|-- ACTION NONE\n");
				break;
			case MXT_PATCH_ACTION_CAL:
				__mxt_patch_debug(data, "|-- ACTION CALIBRATE: %d\n",
					action_val);
				mxt_patch_calibration(data);
				data->patch.start = false; // Wait Restart
				data->patch.start_stage = action_val;
				break;
			case MXT_PATCH_ACTION_EXTEND_TIMER:
				__mxt_patch_debug(data, "|-- ACTION EXTEND TIMER: %d\n",
					action_val);
				mxt_patch_start_timer(data, action_val);
				break;
			case MXT_PATCH_ACTION_GOTO_STAGE:
				__mxt_patch_debug(data, "|-- ACTION GOTO STAGE: %d\n",
					action_val);
				data->patch.skip_test = 1;
				data->patch.cur_stage = action_val;
				data->patch.run_stage = false;
				break;
			default:
				__mxt_patch_debug(data, "@@ INVALID ACTION ID=%d !!\n",
					action_id);
				return -1;
		}
	}
	return 0;
}

#if MXT_PATCH_LOCK_CHECK

#define MXT_PATCH_T71_PTN_OPT	1
#define MXT_PATCH_T71_PTN_CAL	2

struct touch_pos tpos_data;

static void mxt_patch_init_tpos(struct mxt_data *data,
				struct touch_pos* tpos)
{
	int i;
	for(i=0;i<MXT_MAX_FINGER;i++){
		tpos->tcount[i] = 0;
		tpos->initx[i] = 0;
		tpos->inity[i] = 0;
		tpos->oldx[i] = 0;
		tpos->oldy[i] = 0;
	}
	tpos->locked_id = 0xff;
	tpos->moved_cnt = 0;
}

static bool mxt_patch_check_locked(struct mxt_data *data,
			struct touch_pos* tpos, u8 tid, u16 x, u16 y)
{
	s16 diffx, diffy;
	u32 distance;

	//OLD DIFF
	diffx = x - tpos->oldx[tid];
	diffy = y - tpos->oldy[tid];
	distance = abs(diffx) + abs(diffy);

	// INIT DIFF
	if((tpos->initx[tid] != 0)&&(tpos->inity[tid] != 0)){
		diffx = x - tpos->initx[tid];
		diffy = y - tpos->inity[tid];
		__mxt_patch_ddebug(data,
			"[TPOS] INITDIFF[%d] ABS X=%d, ABS Y=%d\n",
			tid, (int)abs(diffx), (int)abs(diffy));
	}
	if((tpos->initx[tid] == 0)&&(tpos->inity[tid] == 0)){
		__mxt_patch_ddebug(data,
			"[TPOS] INITSET[%d] X=%d, Y=%d\n", tid, x, y);
		tpos->initx[tid] = x;
		tpos->inity[tid] = y;
		tpos->moved_cnt = 0;
	}else{
		// OLD DIFF vs INIT DIFF
		if((distance < tpos->jitter)&&
			((abs(diffx) > tpos->maxdiff)||
			(abs(diffy) > tpos->maxdiff))){
			tpos->moved_cnt++;
		}
	}
	if(tpos->moved_cnt > tpos->reset_cnt){
		__mxt_patch_ddebug(data,
			"[TPOS] RESET[%d] X=%d, Y=%d\n", tid, x, y);
		tpos->initx[tid] = x;
		tpos->inity[tid] = y;
		tpos->moved_cnt = 0;
	}
	//__mxt_patch_ddebug(data, "[TPOS] %d, %d, %d, %d, DISTANCE %d\n",
	//	x, y, tpos->oldx[tid], tpos->oldy[tid], distance);

	if((distance < tpos->distance)&&
		(abs(diffx) < tpos->maxdiff)&&
		(abs(diffy) < tpos->maxdiff)){
		return true;
	}
	else
		return false;
	return false;
}

static void mxt_patch_check_pattern(struct mxt_data *data,
			struct touch_pos* tpos, u8 tid, u16 x, u16 y, u8 finger_cnt)
{
	bool cal_condition=false;

	if(!finger_cnt){
		return;
	}
	if(mxt_patch_check_locked(data, tpos, tid, x, y)){
		tpos->tcount[tid] = tpos->tcount[tid]+1;
		//__mxt_patch_debug(data, "[TPOS] INC TCOUNT[%d]=%d\n",
		//	tid, tpos->tcount[tid]);
	}else{
		tpos->tcount[tid] = 0;
		//__mxt_patch_debug(data, "[TPOS] CLEAR TCOUNT[%d]=%d\n",
		//	tid, tpos->tcount[tid]);
	}
	tpos->oldx[tid] = x;
	tpos->oldy[tid] = y;

	if(finger_cnt == 1){
		if(tpos->tcount[tid] > tpos->locked_cnt){
			__mxt_patch_debug(data, "[TPOS] ONE TOUCH LOCKED\n");
			mxt_patch_init_tpos(data, tpos);
			cal_condition = true;
		}
	}else{
		if((tpos->tcount[tid] > tpos->locked_cnt) &&
			tpos->locked_id != tid && tpos->locked_id != 0xff){
			__mxt_patch_debug(data, "[TPOS] TWO TOUCH LOCKED [%d, %d]\n",
				tid, tpos->locked_id);
			mxt_patch_init_tpos(data, tpos);
			cal_condition = true;
		}
		if(tpos->tcount[tid] > tpos->locked_cnt){
			//__mxt_patch_debug(data, "[TSP] LOCKED FOUND %d\n",
			//	tpos->locked_id);
			tpos->locked_id = tid;
			if(tpos->tcount[tid] > 200){
				__mxt_patch_debug(data, "[TPOS] OVER LOCKED\n");
				mxt_patch_init_tpos(data, tpos);//1127
				cal_condition = true;
			}
		}
	}

	if(cal_condition){
	    int error = mxt_read_object(data,
	    			MXT_SPT_DYNAMICCONFIGURATIONCONTAINER_T71,
					MXT_PATCH_T71_PTN_CAL, &tpos->cal_enable);

	    if (error) {
		    dev_err(&data->client->dev, "%s: Error read T71 [%d]\n",
				    __func__, error);
	    } else {
				if(tpos->cal_enable){
					__mxt_patch_debug(data, "[TPOS] CAL\n");
					mxt_patch_calibration(data);

					error = mxt_read_object(data,
			    			MXT_SPT_DYNAMICCONFIGURATIONCONTAINER_T71,
							MXT_PATCH_T71_PTN_OPT, &tpos->option);

					if(!error){
						if(tpos->option&0x01){ // Onetime
							mxt_write_object(data,
								MXT_SPT_DYNAMICCONFIGURATIONCONTAINER_T71,
								MXT_PATCH_T71_PTN_CAL, 0);
							__mxt_patch_debug(data, "[TPOS] DISABLE T71[2]\n");
						}
					}
				}
				else{
					__mxt_patch_debug(data, "[TPOS] SKIP CAL T71[2]=0\n");
				}
		}
	}
}
#endif //MXT_PATCH_LOCK_CHECK

#if MXT_PATCH_SUPP_CHECK
struct touch_supp{
	u32 old_time;
	u8 repeat_cnt;
	u8 time_gap;
	u8 repeat_max;
};

struct touch_supp tsupp_data;

static void mxt_patch_init_supp(struct mxt_data *data,
				struct touch_supp* tsup)
{
	tsup->old_time = jiffies_to_msecs(jiffies);
	tsup->repeat_cnt = 0;
}

static void mxt_patch_check_supp(struct mxt_data *data, struct touch_supp* tsup)
{
	u32 curr_time = jiffies_to_msecs(jiffies);
	u32 time_diff;
	time_diff = TIME_WRAP_AROUND(tsup->old_time, curr_time);

	if(time_diff < tsup->time_gap*100){
		__mxt_patch_debug(data, "[TSUP] Abnormal suppress %d\n",
			tsup->repeat_cnt);
		if(tsup->repeat_cnt++ > tsup->repeat_max){
			__mxt_patch_debug(data, "[TSUP] Abnormal suppress detected\n");
			mxt_patch_calibration(data);
		}
	}
	else{
		tsup->repeat_cnt = 0;
		__mxt_patch_debug(data, "[TSUP] Normal suppress\n");
	}
	tsup->old_time = curr_time;
	mxt_patch_dump_source(data, true);
}

#endif //MXT_PATCH_SUPP_CHECK

#define MXT_PATCH_T71_DATA_MAX	10

static void mxt_patch_load_t71data(struct mxt_data *data)
{
	struct mxt_object *obj;
	u8 buf[MXT_PATCH_T71_DATA_MAX];
	int error;

	obj = mxt_get_object(data,
			MXT_SPT_DYNAMICCONFIGURATIONCONTAINER_T71);
	if(obj){
	    error = mxt_read_mem(data, obj->start_address,
				    MXT_PATCH_T71_DATA_MAX, buf);

	    if(!error){
#if MXT_PATCH_LOCK_CHECK
			struct touch_pos* tpos = &tpos_data;
			tpos->option = buf[MXT_PATCH_T71_PTN_OPT]; //1
			tpos->cal_enable = buf[MXT_PATCH_T71_PTN_CAL];	//1
			tpos->reset_cnt = buf[3];	//5
			tpos->distance = buf[4];	//10
			tpos->maxdiff = buf[5];		//10
			tpos->locked_cnt = buf[6];	//40
			tpos->jitter = buf[7];		//1

			__mxt_patch_debug(data,
				"PTN CAL %d RST %d DST %d DIF %d CNT %d JIT %d\n",
				tpos->cal_enable, tpos->reset_cnt,
				tpos->distance, tpos->maxdiff,
				tpos->locked_cnt, tpos->jitter);
#endif

#if MXT_PATCH_SUPP_CHECK
			tsupp_data.time_gap = buf[8];	//10
			tsupp_data.repeat_max = buf[9]; //2

			__mxt_patch_debug(data,
				"SUPP GAP %d*100ms CNT %d\n",
				tsupp_data.time_gap, tsupp_data.repeat_max);
#endif
		}
	}
}

/* Patch */

const char* mxt_patch_src_item_name(u8 src_id)
{
	const char* src_item_name[MXT_PATCH_MAX_TYPE]={
		MXT_XML_SRC_NONE,	//MXT_PATCH_ITEM_NONE		0
		MXT_XML_SRC_CHRG,	//MXT_PATCH_ITEM_CHARGER	1
		MXT_XML_SRC_FCNT,	//MXT_PATCH_ITEM_FINGER_CNT	2
		MXT_XML_SRC_AREA,	//MXT_PATCH_ITEM_T9_AREA	3
		MXT_XML_SRC_AMP,	//MXT_PATCH_ITEM_T9_AMP		4
		MXT_XML_SRC_SUM,	//MXT_PATCH_ITEM_T57_SUM	5
		MXT_XML_SRC_TCH,	//MXT_PATCH_ITEM_T57_TCH	6
		MXT_XML_SRC_ATCH,	//MXT_PATCH_ITEM_T57_ATCH	7
		MXT_XML_SRC_KCNT,	//MXT_PATCH_ITEM_KCNT		8
		MXT_XML_SRC_KVAL,	//MXT_PATCH_ITEM_KVAL		9
		MXT_XML_SRC_EVT,	//MXT_PATCH_ITEM_EVT		10//MXT_PATCH_TOUCH_SLOT
		MXT_XML_SRC_SCNT,	//MXT_PATCH_ITEM_SCNT		11//MXT_PATCH_TOUCH_SLOT
	};
	if(MXT_PATCH_ITEM_NONE <= src_id &&
		src_id < MXT_PATCH_ITEM_END){
		return src_item_name[src_id];
	}
	return "ERR";
}

const char* mxt_patch_cond_name(u8 con_id)
{
	const char* cond_name[MXT_PATCH_MAX_CON]={
		MXT_XML_CON_NONE,	//MXT_PATCH_CON_NONE	0
		MXT_XML_CON_EQUAL,	//MXT_PATCH_CON_EQUAL	1
		MXT_XML_CON_BELOW,	//MXT_PATCH_CON_BELOW	2
		MXT_XML_CON_ABOVE,	//MXT_PATCH_CON_ABOVE	3
		MXT_XML_CON_PLUS,	//MXT_PATCH_CON_PLUS	4
		MXT_XML_CON_MINUS,	//MXT_PATCH_CON_MINUS	5
		MXT_XML_CON_MUL,	//MXT_PATCH_CON_MUL		6
		MXT_XML_CON_DIV,	//MXT_PATCH_CON_DIV		7
		MXT_XML_CON_MASK,	//MXT_PATCH_CON_MASK	8
	};
	if(MXT_PATCH_CON_NONE <= con_id &&
		con_id < MXT_PATCH_CON_END){
		return cond_name[con_id];
	}
	return "ERR";
}

static int mxt_patch_item_lval(struct mxt_data *data,
		u16* psrc_item, u8 src_id)
{
	if(psrc_item != NULL){
		if(MXT_PATCH_ITEM_NONE <= src_id &&
			src_id < MXT_PATCH_ITEM_END){
			return psrc_item[src_id];
		}
		else{
			__mxt_patch_debug(data, "@@ INVALID ITEM ID=%d !!\n", src_id);
		}
	}
	return 0;
}

static int mxt_patch_item_rval(struct mxt_data *data,
		u16* psrc_item, struct item_val ival)
{
	int lval = mxt_patch_item_lval(data, psrc_item, ival.val_id);
	int rval = ival.val;

	switch(ival.val_eq){
		case MXT_PATCH_CON_NONE:
			return lval ? lval : rval;
		case MXT_PATCH_CON_PLUS:
			lval += rval;
			break;
		case MXT_PATCH_CON_MINUS:
			lval -= rval;
			break;
		case MXT_PATCH_CON_MUL:
			lval *= rval;
			break;
		case MXT_PATCH_CON_DIV:
			lval /= rval;
			break;
		default:
			if(psrc_item){
				__mxt_patch_debug(data, "@@ INVALID VAL_EQ=%d"
					" (LVAL=%d) => RVAL=%d !!\n",
					ival.val_eq, lval, rval);
			}
			return rval;
	}
	return lval;
}

static int mxt_patch_item_check(struct mxt_data *data,
		u16* psrc_item, struct test_item* ptitem, bool do_action)
{
	int lval = mxt_patch_item_lval(data, psrc_item, ptitem->src_id);
	int rval = mxt_patch_item_rval(data, psrc_item, ptitem->ival);

	if(!do_action){
		__mxt_patch_debug(data, "|-- ITEM SRC_ID:%s COND:%s"
			" VAL_ID:%s EQ:%s VAL:%d\n",
			mxt_patch_src_item_name(ptitem->src_id),
			mxt_patch_cond_name(ptitem->cond),
			mxt_patch_src_item_name(ptitem->ival.val_id),
			mxt_patch_cond_name(ptitem->ival.val_eq),
			ptitem->ival.val);
	}
	if(psrc_item){
		switch(ptitem->cond){
			case MXT_PATCH_CON_EQUAL:
				__mxt_patch_ddebug(data, "|--- IF %s: %d == %d = %d\n",
					mxt_patch_src_item_name(ptitem->src_id),
					lval, rval, lval == rval ? 1 : 0);
				return lval == rval ? 1 : 0;
			case MXT_PATCH_CON_BELOW:
				__mxt_patch_ddebug(data, "|--- IF %s: %d < %d = %d\n",
					mxt_patch_src_item_name(ptitem->src_id),
					lval, rval, lval < rval ? 1 : 0);
				return lval < rval ? 1 : 0;
			case MXT_PATCH_CON_ABOVE:
				__mxt_patch_ddebug(data, "|--- IF %s: %d > %d = %d\n",
					mxt_patch_src_item_name(ptitem->src_id),
					lval, rval, lval > rval ? 1 : 0);
				return lval > rval ? 1 : 0;
			case MXT_PATCH_CON_MASK:
				__mxt_patch_ddebug(data, "|--- IF %s: %d & %d = %d\n",
					mxt_patch_src_item_name(ptitem->src_id),
					lval, rval, lval & rval ? 1 : 0);
				return lval & rval ? 1 : 0;
			default:
				__mxt_patch_debug(data, "@@ INVALID TEST COND=%d !!\n",
					ptitem->cond);
				return -1;
		}
	}
	return -1;
}

static int mxt_patch_stage_timer(struct mxt_data *data,
		u16 period, bool do_action)
{
	if(do_action){
		int ret = 0;
		u32 time = period * 10;

		//__mxt_patch_debug(data, "STAGE[%d] TIMER: %dx10ms\n",
		//	data->patch.cur_stage, period);

		ret = mxt_patch_start_timer(data, time);
		if (!ret) {
			data->patch.period = period;
		}
	}
	return 0;
}

static void mxt_patch_dump_source(struct mxt_data *data,
				bool do_action)
{
	if(do_action){
		__mxt_patch_debug(data, "TA:%d FC:%d AR:%d AP:%d"
			" SM:%d TC:%d AT:%d KC:%d KV:%d EV:%d SC:%d\n",
			data->patch.src_item[1], data->patch.src_item[2],
			data->patch.src_item[3], data->patch.src_item[4],
			data->patch.src_item[5], data->patch.src_item[6],
			data->patch.src_item[7], data->patch.src_item[8],
			data->patch.src_item[9], data->patch.src_item[10],
			data->patch.src_item[11]);	//MXT_PATCH_TOUCH_SLOT
	}
}

static int mxt_patch_parse_test_line(struct mxt_data *data,
		u8* ppatch, u16* psrc_item, u16* check_cnt, bool do_action)
{
	struct test_line*	ptline;
	struct test_item*	ptitem;
	struct action_cfg*	pacfg;
	u32 i=0, ulpos=0;
	u8 test_result=0;
	bool test_action=false;

	ptline = (struct test_line*)ppatch;

	if(!do_action){
		__mxt_patch_debug(data, "|- TEST_LINE:%X OPT:%d CHK_CNT:%d"
			" ITEM_CNT:%d CFG_CNT:%d ACTION:%d VAL:%d \n",
			ptline->test_id, ptline->option, ptline->check_cnt,
			ptline->item_cnt, ptline->cfg_cnt, ptline->act_id,
			ptline->act_val);
	}
	ulpos += sizeof(struct test_line);

	test_result = 0;
	test_action = false;
	for(i=0; i < ptline->item_cnt; i++){ /* Test Item Parsing */
		ptitem = (struct test_item*)(ppatch+ulpos);

		if(mxt_patch_item_check(data, psrc_item,
			ptitem, do_action) > 0){
			test_result++;

			if(test_result == ptline->item_cnt){
				if(check_cnt != NULL){
					*check_cnt = *check_cnt+1;
					if(*check_cnt == ptline->check_cnt){
						test_action = true;
						__mxt_patch_debug(data, "STAGE:%d TEST %d MATCHED",
							data->patch.cur_stage, ptline->test_id);
						mxt_patch_dump_source(data, test_action);

						if(ptline->option&0x01){
							*check_cnt=0;
							__mxt_patch_ddebug(data, "CHEK CNT CLEAR\n");
						}
					}
				}
			}
		}
		else{
			if(data->patch.option&0x04){
			    if(do_action&&psrc_item){// Skip if any item was failed
				    __mxt_patch_ddebug(data, "SKIP REMAINED ITEMS %d\n", i);
				    return 0;
			    }
			}
		}
		ulpos += sizeof(struct test_item);
	}

	for(i=0; i <ptline->cfg_cnt; i++){ /* Test Line Action config */
		pacfg = (struct action_cfg*)(ppatch+ulpos);
		if(!do_action){
			__mxt_patch_debug(data, "|-- ACTION_CFG: OBJ:%d"
				" OFFSET:%d VAL:%d OPT:%d\n",
				pacfg->obj_type, pacfg->offset,
				pacfg->val, pacfg->option);
		}
		mxt_patch_write_action_cfg(data, pacfg, test_action);
		ulpos += sizeof(struct action_cfg);
	}
	mxt_patch_predefined_action(data, ptline->act_id,
		ptline->act_val, test_action);

	return ulpos;
}

static int mxt_patch_parse_stage(struct mxt_data *data,
		u8* ppatch, u16* ptline_addr, u8* ptline_cnt, bool do_action)
{
	struct stage_def* psdef;
	struct stage_cfg* pscfg;
	u32 i=0, ulpos=0;

	psdef = (struct stage_def*)ppatch;

	if(!do_action){
#if MXT_PATCH_STAGE_RESET
		__mxt_patch_debug(data,
			"STAGE_ID:%d OPT:%d PERIOD:%d CFG_CNT:%d TST_CNT:%d RESET:%d\n",
			psdef->stage_id, psdef->option, psdef->stage_period,
			psdef->cfg_cnt, psdef->test_cnt, psdef->reset_period);//MXT_PATCH_STAGE_RESET
#endif
	;
	}
	mxt_patch_stage_timer(data, psdef->stage_period, do_action);
	ulpos += sizeof(struct stage_def);

	for(i=0; i < psdef->cfg_cnt; i++){ /* Stage Config Parsing */
		pscfg = (struct stage_cfg*)(ppatch+ulpos);
		if(!do_action){
			__mxt_patch_debug(data,
				"|- STAGE_CFG: OBJ:%d OFFSET:%d VAL:%d OPT:%d\n",
				pscfg->obj_type, pscfg->offset,
				pscfg->val, pscfg->option);
		}
		mxt_patch_write_stage_cfg(data, pscfg, do_action);
		ulpos += sizeof(struct stage_cfg);
	}

	for(i=0; i < psdef->test_cnt; i++){ /* Test Line Parsing */
		if(ptline_addr != NULL){
			ptline_addr[i] = (u16)ulpos;
			//__mxt_patch_debug(data, "|- TEST LINE_ADDR: %d\n", ulpos);
		}
		ulpos += mxt_patch_parse_test_line(data,
					ppatch+ulpos, NULL, NULL, do_action);
	}
	if(ptline_cnt != NULL)
		*ptline_cnt = psdef->test_cnt;

	return ulpos;
}

static u16 mxt_patch_match_lval(struct mxt_data *data,
			u8* pmsg, u8 offset, u16 mask)
{
	u16 lval=0;
	u8 msg[MXT_PATCH_MAX_MSG_SIZE+1];
	memset(msg, 0, MXT_PATCH_MAX_MSG_SIZE+1);

	if(pmsg){
		memcpy(msg, pmsg, MXT_PATCH_MAX_MSG_SIZE);
		if(0 <= offset && offset < MXT_PATCH_MAX_MSG_SIZE){
			lval = msg[offset] | (msg[offset+1] << 8);
			return mask ? lval & mask : lval;
		}
		else{
			__mxt_patch_debug(data, "@@ INVALID OFFSET=%d !!\n",
				offset);
		}
	}
	return 0;
}

static int mxt_patch_match_check(struct mxt_data *data,
			u8* pmsg, struct match* pmatch, bool do_action)
{
	u16 lval = mxt_patch_match_lval(data, pmsg,
				pmatch->offset, pmatch->mask);
	u16 rval = pmatch->val;

	if(pmsg){
		switch(pmatch->cond){
			case MXT_PATCH_CON_EQUAL:
				__mxt_patch_ddebug(data, "|--- IF %d == %d = %d\n",
					lval, rval, lval == rval ? 1 : 0);
				return lval == rval ? 1 : 0;
			case MXT_PATCH_CON_BELOW:
				__mxt_patch_ddebug(data, "|--- IF %d < %d = %d\n",
					lval, rval, lval < rval ? 1 : 0);
				return lval < rval ? 1 : 0;
			case MXT_PATCH_CON_ABOVE:
				__mxt_patch_ddebug(data, "|--- IF %d > %d = %d\n",
					lval, rval, lval > rval ? 1 : 0);
				return lval > rval ? 1 : 0;
			default:
				__mxt_patch_debug(data, "@@ INVALID MATCH COND=%d !!\n",
					pmatch->cond);
				return -1;
		}
	}
	return -1;
}

static int mxt_patch_trigger_check(struct mxt_data *data,
			u8 object, u8 index, u8* pmsg)
{
	u8 reportid= pmsg[0];
	u8 type, id;


	type = data->reportids[reportid].type;
	id = data->reportids[reportid].index;

	if((type == object)&&(id == index))
		return 0;

	return 1;
}

static int mxt_patch_parse_trigger(struct mxt_data *data,
			u8* ppatch, u8* pmsg, bool do_action, u8 option)
{
	struct trigger* ptrgg;
	struct match* pmatch;
	struct trigger_cfg* ptcfg;
	u32 i=0, ulpos=0;
	u8 match_result=0;
	u8 trigger_action=0;

	ptrgg = (struct trigger*)ppatch;

	if(!do_action){
		__mxt_patch_debug(data, "TRIGGER ID:%d OPT:%d OBJ:%d IDX:%d "
			"MATCH:%d CFG:%d ACT:%d VAL:%d\n",
			ptrgg->tid, ptrgg->option, ptrgg->object,
			ptrgg->index, ptrgg->match_cnt, ptrgg->cfg_cnt,
			ptrgg->act_id, ptrgg->act_val);
	}
	ulpos += sizeof(struct trigger);

	// Message Filter
	if(do_action){
		if(mxt_patch_trigger_check(data,
			ptrgg->object, ptrgg->index, pmsg))
			return 1;
	}

	// Match Parsing
	match_result=0;
	trigger_action=false;
	for(i=0; i < ptrgg->match_cnt; i++){
		pmatch = (struct match*)(ppatch+ulpos);
		if(!do_action){
			__mxt_patch_debug(data, "|- MATCH:%d OFFSET:%d MASK:%d"
				" COND:%s VAL:%d\n", i,
				pmatch->offset, pmatch->mask,
				mxt_patch_cond_name(pmatch->cond), pmatch->val);
		}
		if(mxt_patch_match_check(data, pmsg, pmatch, do_action) > 0){
			match_result++;
			if(match_result == ptrgg->match_cnt){
				if(option == ptrgg->option)
				trigger_action = true;
			}
		}
		ulpos += sizeof(struct match);
	}

	// Trigger Config Parsing
	for(i=0; i < ptrgg->cfg_cnt; i++){
		ptcfg = (struct trigger_cfg*)(ppatch+ulpos);
		if(!do_action){
			__mxt_patch_debug(data, "|- TRIGGER_CFG: OBJECT_TYPE:%d"
				" OFFSET:%d VAL:%d\n",
				ptcfg->obj_type, ptcfg->offset, ptcfg->val);
		}
		mxt_patch_write_trigger_cfg(data, ptcfg, trigger_action);
		ulpos += sizeof(struct trigger_cfg);
	}
	// Predefined Action
	mxt_patch_predefined_action(data, ptrgg->act_id,
		ptrgg->act_val, trigger_action);

	return ulpos;
}

int mxt_patch_parse_event(struct mxt_data *data,
		u8* ppatch, bool do_action)
{
	struct user_event* pevent;
	struct event_cfg* pecfg;
	u32 i=0, ulpos=0;

	pevent = (struct user_event*)ppatch;

	if(!do_action){
		__mxt_patch_debug(data, "EVENT ID:%d OPT:%d CFG:%d\n",
			pevent->eid, pevent->option, pevent->cfg_cnt);
	}
	ulpos += sizeof(struct user_event);

	// Event Config Parsing
	for(i=0; i < pevent->cfg_cnt; i++){
		pecfg = (struct event_cfg*)(ppatch+ulpos);
		if(!do_action){
			__mxt_patch_debug(data, "|- EVENT_CFG: "
				"OBJECT_TYPE:%d OFFSET:%d VAL:%d\n",
				pecfg->obj_type, pecfg->offset, pecfg->val);
		}
		mxt_patch_write_event_cfg(data, pecfg, do_action);
		ulpos += sizeof(struct event_cfg);
	}

	return ulpos;
}

static int mxt_patch_parse_header(struct mxt_data *data,
		u8* ppatch, u16* pstage_addr, u16* ptrigger_addr,
		u16* pevent_addr)
{
	struct patch_header* ppheader;
	u32 i=0, ulpos=0;

	ppheader = (struct patch_header*)ppatch;

	dev_info(&data->client->dev, "PATCH MAGIC:%X SIZE:%d DATE:%d"
		" VER:%d OPT:%d DBG:%d TMR:%d STG:%d TRG:%d EVT:%d\n",
		ppheader->magic, ppheader->size, ppheader->date,
		ppheader->version, ppheader->option, ppheader->debug,
		ppheader->timer_id, ppheader->stage_cnt, ppheader->trigger_cnt,
		ppheader->event_cnt);

	if(ppheader->version != MXT_PATCH_VERSION){
		dev_err(&data->client->dev, "MXT_PATCH_VERSION ERR\n");
	}

	ulpos = sizeof(struct patch_header);

	for(i=0; i < ppheader->stage_cnt; i++){ /* Stage Def Parsing */
		if(pstage_addr != NULL){
			pstage_addr[i] = (u16)ulpos;
			//__mxt_patch_debug(data, "STAGE_ADDR: %d\n", ulpos);
		}
		ulpos += mxt_patch_parse_stage(data, ppatch+ulpos,
					NULL, NULL, false);
	}

	for(i=0; i < ppheader->trigger_cnt; i++){ /* Trigger Parsing */
		if(ptrigger_addr != NULL){
			ptrigger_addr[i] = (u16)ulpos;
			//__mxt_patch_debug(data, "TRIGGER_ADDR: %d\n", ulpos);
		}
		ulpos += mxt_patch_parse_trigger(data, ppatch+ulpos,
					NULL, false, 0);
	}

	for(i=0; i < ppheader->event_cnt; i++){ /* Event */
		if(pevent_addr != NULL){
			pevent_addr[i] = (u16)ulpos;
			//__mxt_patch_debug(data, "EVENT_ADDR: %d\n", ulpos);
		}
		ulpos += mxt_patch_parse_event(data, ppatch+ulpos, false);
	}

	if(ppheader->size != ulpos){ /* Patch Size Check */
		__mxt_patch_debug(data, "PATCH SIZE ERROR %d != %d\n\n",
			ppheader->size, ulpos);
		return 0;
	}
	else{
		__mxt_patch_debug(data, "PATCH SIZE OK= %d\n\n", ulpos);
	}
	return ulpos;
}

static int mxt_patch_run_stage(struct mxt_data *data)
{
	struct stage_def* psdef=NULL;
	u8* ppatch = data->patch.patch;
	u16* pstage_addr = data->patch.stage_addr;
	u16 tline_addr[MXT_PATCH_MAX_TLINE];
	u8  tline_cnt;
	u8	cur_stage = data->patch.cur_stage;

	//__mxt_patch_debug(data, "RUN STAGE:%d\n", cur_stage);

	if(!ppatch || !pstage_addr){
		dev_err(&data->client->dev, "%s pstage_addr is null\n", __func__);
		return 1;
	}
	psdef = (struct stage_def*)(ppatch+pstage_addr[cur_stage]);
	data->patch.cur_stage_opt = psdef->option;

	mxt_patch_parse_stage(data, (u8*)psdef, tline_addr, &tline_cnt, true);

	if(!data->patch.tline_addr){
		kfree(data->patch.tline_addr);
	}
	if(!data->patch.check_cnt){
		kfree(data->patch.check_cnt);
	}
	data->patch.tline_addr = kzalloc(tline_cnt*sizeof(u16), GFP_KERNEL);
	data->patch.check_cnt  = kzalloc(tline_cnt*sizeof(u16), GFP_KERNEL);
	if (!data->patch.tline_addr || !data->patch.check_cnt){
		dev_err(&data->client->dev, "tline_addr alloc error\n");
		return 1;
	}

	memcpy(data->patch.tline_addr, tline_addr, tline_cnt*sizeof(u16));
	memset(data->patch.check_cnt, 0, tline_cnt*sizeof(u16));
	data->patch.tline_cnt = tline_cnt;
	data->patch.run_stage = 1;
	data->patch.skip_test = 0;
#if MXT_PATCH_STAGE_RESET
	data->patch.stage_timestamp = jiffies_to_msecs(jiffies);
	__mxt_patch_ddebug(data, "Stage[%d] %d\n",
		cur_stage, data->patch.stage_timestamp);
#endif
	return 0;
}

static int mxt_patch_test_source(struct mxt_data *data, u16* psrc_item)
{
	int i;
	u8* ppatch = data->patch.patch;
	u16* pstage_addr = data->patch.stage_addr;
	u8	cur_stage = data->patch.cur_stage;
#if MXT_PATCH_STAGE_RESET
	u32 curr_time = jiffies_to_msecs(jiffies);
	u32 time_diff = TIME_WRAP_AROUND(data->patch.stage_timestamp, curr_time);
	struct stage_def* psdef=NULL;
#endif

	if(!ppatch || !pstage_addr){
		dev_err(&data->client->dev, "%s pstage_addr is null\n", __func__);
		return 1;
	}
	if(!data->patch.run_stage){
		mxt_patch_run_stage(data);
	}
	if(data->patch.run_stage){
		for(i=0; i< data->patch.tline_cnt; i++){
			u16* ptline_addr = data->patch.tline_addr;
			u16* pcheck_cnt = data->patch.check_cnt;
			if(!ptline_addr || !pcheck_cnt){
				dev_err(&data->client->dev, "ptline_addr is null\n");
				return 1;
			}
			__mxt_patch_ddebug(data, "STAGE:%d, TEST:%d\n", cur_stage, i);

			mxt_patch_parse_test_line(data,
				ppatch+pstage_addr[cur_stage]+ptline_addr[i],
				psrc_item, &pcheck_cnt[i], true);

#if MXT_PATCH_STAGE_RESET
			psdef = (struct stage_def*)(ppatch+pstage_addr[cur_stage]);
			if(psdef->reset_period){
				if(time_diff > psdef->reset_period*10){
					pcheck_cnt[i] = 0;
					__mxt_patch_ddebug(data,
						"RESET CNT STAGE:%d, TEST:%d RESET:%d DIF:%d\n",
						cur_stage, i,
						psdef->reset_period, time_diff);
					data->patch.stage_timestamp = jiffies_to_msecs(jiffies);
				}
			}
#endif

			if(data->patch.skip_test){
				__mxt_patch_debug(data, "REMAINED TEST SKIP\n");
				return 0;
			}
		}
	}
	return 0;
}

#if MXT_PATCH_TOUCH_SLOT
static void mxt_patch_init_tslot(void)
{
	memset(&tch_slot, 0, sizeof(struct touch_slot));
}
#endif

static void mxt_patch_init_tsrc(struct test_src* tsrc)
{
	tsrc->charger=-1;
	tsrc->finger_cnt=-1;
	tsrc->area=-1;
	tsrc->amp=-1;
	tsrc->sum_size=-1;
	tsrc->tch_ch=-1;
	tsrc->atch_ch=-1;
	tsrc->key_cnt=-1;
	tsrc->key_val=-1;
	tsrc->tch_evt=-1;		//MXT_PATCH_TOUCH_SLOT
	tsrc->stylus_cnt=-1;	//MXT_PATCH_TOUCH_SLOT
}

static int mxt_patch_make_source(struct mxt_data *data,
			struct test_src* tsrc)
{
	if(tsrc->charger >= 0)
		data->patch.src_item[MXT_PATCH_ITEM_CHARG]= tsrc->charger;
	if(tsrc->finger_cnt >= 0)
		data->patch.src_item[MXT_PATCH_ITEM_FCNT]= tsrc->finger_cnt;
	if(tsrc->area >= 0)
		data->patch.src_item[MXT_PATCH_ITEM_AREA]= tsrc->area;
	if(tsrc->amp >= 0)
		data->patch.src_item[MXT_PATCH_ITEM_AMP]= tsrc->amp;
	if(tsrc->sum_size >= 0)
		data->patch.src_item[MXT_PATCH_ITEM_SUM]= tsrc->sum_size;
	if(tsrc->tch_ch >= 0)
		data->patch.src_item[MXT_PATCH_ITEM_TCH]= tsrc->tch_ch;
	if(tsrc->atch_ch >= 0)
		data->patch.src_item[MXT_PATCH_ITEM_ATCH]= tsrc->atch_ch;
	if(tsrc->key_cnt >= 0)
		data->patch.src_item[MXT_PATCH_ITEM_KCNT]= tsrc->key_cnt;
	if(tsrc->key_val >= 0)
		data->patch.src_item[MXT_PATCH_ITEM_KVAL]= tsrc->key_val;
	if(tsrc->tch_evt >= 0) //MXT_PATCH_TOUCH_SLOT
		data->patch.src_item[MXT_PATCH_ITEM_EVT]= tsrc->tch_evt;
	if(tsrc->stylus_cnt >= 0) //MXT_PATCH_TOUCH_SLOT
		data->patch.src_item[MXT_PATCH_ITEM_SCNT]= tsrc->stylus_cnt;

	//mxt_patch_dump_source(data, true);
	return 0;
}

static int mxt_patch_start_stage(struct mxt_data *data)
{
	if(data->patch.patch){
		mxt_patch_stop_timer(data);
		data->patch.start = true;
		data->patch.cur_stage = 0;
		data->patch.run_stage = false;

		if(data->patch.start_stage){
			data->patch.cur_stage = data->patch.start_stage;
		}
		__mxt_patch_debug(data, "PATCH: START STAGE %d\n",
			data->patch.cur_stage);

#if MXT_PATCH_LOCK_CHECK
		mxt_patch_init_tpos(data, &tpos_data);
#endif
#if MXT_PATCH_SUPP_CHECK
		mxt_patch_init_supp(data, &tsupp_data);
#endif
#if MXT_PATCH_TOUCH_SLOT
		mxt_patch_init_tslot();
#endif
		return 0;
	}
	return 1;
}

static int mxt_patch_test_trigger(struct mxt_data *data,
			struct mxt_message *message, u8 option)
{
	int i;
	u8* ppatch = data->patch.patch;
	u16* ptrigger_addr = data->patch.trigger_addr;
	u8	trigger_cnt = data->patch.trigger_cnt;
	u8	tmsg[MXT_PATCH_MAX_MSG_SIZE];

	if(!ppatch || !ptrigger_addr){
		dev_err(&data->client->dev, "%s ptrigger_addr is null\n",
			__func__);
		return 1;
	}
	memset(tmsg, 0, MXT_PATCH_MAX_MSG_SIZE);
	tmsg[0] =  message->reportid;
	memcpy(&tmsg[1], message->message, 8);

	for(i=0; i< trigger_cnt; i++){
		//__mxt_patch_debug(data, "TRIGGER:%d\n", i);
		mxt_patch_parse_trigger(data, ppatch+ptrigger_addr[i],
			tmsg, true, option);
	}
	return 0;
}

#if TSP_INFORM_CHARGER
static int mxt_patch_test_event(struct mxt_data *data,
	u8 event_id)
{
	u8* ppatch = data->patch.patch;
	u16* pevent_addr = data->patch.event_addr;

	if(!ppatch || !pevent_addr){
		dev_err(&data->client->dev, "%s pevent_addr is null\n",
			__func__);
		return 1;
	}
	if(event_id < data->patch.event_cnt){
		mxt_patch_parse_event(data, ppatch+pevent_addr[event_id],
			true);
	}
	return 0;
}
#endif

static void mxt_patch_T6_object(struct mxt_data *data,
		struct mxt_message *message)
{
	/* Normal mode */
	if (message->message[0] == 0x00) {
		__mxt_patch_debug(data, "PATCH: NORMAL\n");
#if MXT_PATCH_START_FROM_NORMAL
		if(data->patch.cal_flag == 1) {//1107
			mxt_patch_start_stage(data);
			data->patch.cal_flag = 0;
		}
#endif

#if 0 //Event Test
		if(data->patch.event_cnt)
			mxt_patch_test_event(data, 0);
#endif
	}
	/* Calibration */
	if (message->message[0] & 0x10){
		__mxt_patch_debug(data, "PATCH: CAL\n");

#if MXT_PATCH_START_FROM_NORMAL
		data->patch.cal_flag = 1;		 //1107
#else
		mxt_patch_start_stage(data);     //1107
#endif
	}
	/* Reset */
	if (message->message[0] & 0x80) {
		__mxt_patch_debug(data, "PATCH: RESET\n");
		data->patch.start_stage = 0;
#if MXT_PATCH_START_FROM_NORMAL
		data->patch.cal_flag = 1;//1107
#else
		mxt_patch_start_stage(data);
#endif
	}
}

static void mxt_patch_T9_object(struct mxt_data *data,
		struct mxt_message *message)
{
	int id;
	u8 *msg = message->message;
	struct test_src tsrc;


	id = data->reportids[message->reportid].index;

	mxt_patch_init_tsrc(&tsrc);

	tsrc.area = msg[4];
	tsrc.amp = msg[5];
	tsrc.tch_evt = msg[0];
	if(data->patch.start){
		mxt_patch_make_source(data, &tsrc);

#if MXT_PATCH_SUPP_CHECK
		if(data->patch.cur_stage_opt&0x02){
			if((msg[0] & MXT_DETECT_MSG_MASK) != MXT_DETECT_MSG_MASK){
				if (msg[0] & MXT_SUPPRESS_MSG_MASK){
					mxt_patch_check_supp(data, &tsupp_data);
				}
			}
		}
#endif
	}
}

#if ENABLE_TOUCH_KEY
static void mxt_patch_T15_object(struct mxt_data *data,
		struct mxt_message *message)
{
	struct test_src tsrc;
	unsigned long keystates = message->message[MXT_MSG_T15_KEYSTATE];
	u8 key_cnt=0;
	int i;

	for(i=0; i < 8; i++){
		if(test_bit(i, &keystates)){
			key_cnt++;
		}
	}
	mxt_patch_init_tsrc(&tsrc);
	tsrc.key_cnt = key_cnt;
	tsrc.key_val = keystates;

	if(data->patch.start){
		mxt_patch_make_source(data, &tsrc);
		if(data->patch.option&0x02) //0905#2
			mxt_patch_test_source(data, data->patch.src_item);
	}
}
#endif

static void mxt_patch_T57_object(struct mxt_data *data,
		struct mxt_message *message)
{
	struct test_src tsrc;
	u8 *msg = message->message;
	u8 finger_cnt = 0;
	int i;

	mxt_patch_init_tsrc(&tsrc);

	for (i = 0; i < MXT_MAX_FINGER; i++) {
		if ((data->fingers[i].state != MXT_STATE_INACTIVE) &&
			(data->fingers[i].state != MXT_STATE_RELEASE))
			finger_cnt++;
	}
#if TSP_INFORM_CHARGER
	tsrc.charger = data->charging_mode;
#endif
	tsrc.finger_cnt = finger_cnt;

	tsrc.sum_size = msg[0] | (msg[1] << 8);
	tsrc.tch_ch = msg[2] | (msg[3] << 8);
	tsrc.atch_ch = msg[4] | (msg[5] << 8);

	if(data->patch.start){
		if((data->patch.option & 0x01)== 0x01 && !finger_cnt)
			return;

		mxt_patch_make_source(data, &tsrc);
		mxt_patch_test_source(data, data->patch.src_item);
	}
#if MXT_PATCH_LOCK_CHECK
	if(data->patch.cur_stage_opt&0x01){
		if(finger_cnt){
			for (i = 0; i < MXT_MAX_FINGER; i++) {
				if ((data->fingers[i].state != MXT_STATE_INACTIVE) &&
					(data->fingers[i].state != MXT_STATE_RELEASE)){
					mxt_patch_check_pattern(data, &tpos_data, i,
						data->fingers[i].x, data->fingers[i].y, finger_cnt);
				}
			}
		}
		else{
			mxt_patch_init_tpos(data, &tpos_data);
		}
	}
#endif

}

static void mxt_patch_T61_object(struct mxt_data *data,
		struct mxt_message *message)
{
	int id;
	u8 *msg = message->message;
	id = data->reportids[message->reportid].index;

	if ((id != data->patch.timer_id) || ((msg[0] & 0xa0) != 0xa0))
		return;

	__mxt_patch_debug(data, "END STAGE %d TIMER\n",
		data->patch.cur_stage);

	if((data->patch.cur_stage+1) == data->patch.stage_cnt){
		if(data->patch.period == 0){
			__mxt_patch_debug(data, "EX-STAGE\n");
		}
		else{
			data->patch.start = false;
			__mxt_patch_debug(data, "END ALL STAGE\n");
		}
	}
	else{
		data->patch.cur_stage++;
		data->patch.run_stage = false;
	}
	if(!data->patch.run_stage){
		mxt_patch_run_stage(data);
	}
}

static void mxt_patch_T100_object(struct mxt_data *data,
		struct mxt_message *message)
{
	u8 id, index;
	u8 *msg = message->message;
	u8 touch_type = 0, touch_event = 0, touch_detect = 0;
	u16 x, y;
	struct test_src tsrc;

	index = data->reportids[message->reportid].index;

	mxt_patch_init_tsrc(&tsrc);

	/* Treate screen messages */
	if (index < MXT_T100_SCREEN_MESSAGE_NUM_RPT_ID) {
		if (index == MXT_T100_SCREEN_MSG_FIRST_RPT_ID){
			data->patch.finger_cnt = msg[1];
			tsrc.finger_cnt = data->patch.finger_cnt;
			tsrc.tch_ch = (msg[3] << 8) | msg[2];
			tsrc.atch_ch = (msg[5] << 8) | msg[4];
			tsrc.sum_size = (msg[7] << 8) | msg[6];

			if(data->patch.start){
				mxt_patch_make_source(data, &tsrc);
			}
#if MXT_PATCH_LOCK_CHECK
			if(data->patch.cur_stage_opt &&
				data->patch.finger_cnt==0){
				mxt_patch_init_tpos(data, &tpos_data);
			}
#endif
			return;
		}
	}

	if(index >= MXT_T100_SCREEN_MESSAGE_NUM_RPT_ID){
		u8 i=0, stylus_cnt=0;

		/* Treate touch status messages */
		id = index - MXT_T100_SCREEN_MESSAGE_NUM_RPT_ID;
		touch_detect = msg[0] >> MXT_T100_DETECT_MSG_MASK;
		touch_type = (msg[0] & 0x70) >> 4;
		touch_event = msg[0] & 0x0F;

#if MXT_PATCH_TOUCH_SLOT
		tch_slot.detect[id] = touch_detect;
		tch_slot.type[id] = touch_type;
		tch_slot.event[id] = touch_event;

		for(i=0; i< MXT_MAX_FINGER; i++){
			if(tch_slot.detect[i]){
				if(tch_slot.type[i] == MXT_T100_TYPE_PASSIVE_STYLUS){
					stylus_cnt++;
				}
			}
		}
		tsrc.stylus_cnt = stylus_cnt;
#endif

		switch (touch_type)	{
			case MXT_T100_TYPE_FINGER:
			case MXT_T100_TYPE_PASSIVE_STYLUS:
				x = msg[1] | (msg[2] << 8);
				y = msg[3] | (msg[4] << 8);

				tsrc.amp = msg[6];
				tsrc.area = msg[7];

				if(data->patch.start){
					if((data->patch.option & 0x01)== 0x01 && !touch_detect)
						return;

					mxt_patch_make_source(data, &tsrc);
					mxt_patch_test_source(data, data->patch.src_item);
				}
#if MXT_PATCH_LOCK_CHECK
				if(data->patch.cur_stage_opt&0x01 &&
					data->patch.finger_cnt){
					mxt_patch_check_pattern(data, &tpos_data, id,
						x, y, data->patch.finger_cnt);
				}
#endif

			break;
		}
	}
}

static void mxt_patch_message(struct mxt_data *data,
		struct mxt_message *message)
{
	u8 reportid, type;
	reportid = message->reportid;

	if (reportid > data->max_reportid)
		return;

	type = data->reportids[reportid].type;
	switch (type) {
		case MXT_GEN_COMMANDPROCESSOR_T6:
			mxt_patch_T6_object(data, message);
			break;
		case MXT_TOUCH_MULTITOUCHSCREEN_T9:
			mxt_patch_T9_object(data, message);
			break;
#if ENABLE_TOUCH_KEY
		case MXT_TOUCH_KEYARRAY_T15:
			mxt_patch_T15_object(data, message);
			break;
#endif
		case MXT_PROCI_EXTRATOUCHSCREENDATA_T57:
			mxt_patch_T57_object(data, message);
			break;
		case MXT_SPT_TIMER_T61:
			mxt_patch_T61_object(data, message);
			break;
		case MXT_TOUCH_MULTITOUCHSCREEN_T100:
			mxt_patch_T100_object(data, message);
			break;
	}
	if(data->patch.trigger_cnt && type){
		u8 option=0;
#if TSP_INFORM_CHARGER
		option = data->charging_mode;
#endif
		mxt_patch_test_trigger(data, message, option);
	}
}

static int mxt_patch_init(struct mxt_data *data, u8* ppatch)
{
	struct mxt_patch *patch_info = &data->patch;
	struct patch_header *ppheader;
	u16 stage_addr[32];
	u16 trigger_addr[32];
	u16 event_addr[32];
	u32 patch_size=0;

	if(!ppatch){
		dev_info(&data->client->dev, "%s patch file error\n", __func__);
		return 1;
	}

	patch_size = mxt_patch_parse_header(data,
					ppatch, stage_addr, trigger_addr, event_addr);
	if(!patch_size){
		dev_info(&data->client->dev, "%s patch_size error\n", __func__);
		return 1;
	}
	ppheader = (struct patch_header*)ppatch;
	patch_info->timer_id = ppheader->timer_id;
	patch_info->option = ppheader->option;
	patch_info->debug = ppheader->debug;
	patch_info->stage_cnt = ppheader->stage_cnt;
	patch_info->trigger_cnt = ppheader->trigger_cnt;
	patch_info->event_cnt = ppheader->event_cnt;

	if(!data->patch.src_item){
		kfree(data->patch.src_item);
	}
	patch_info->src_item = kzalloc(MXT_PATCH_ITEM_END*sizeof(u16),
							GFP_KERNEL);

	if(patch_info->stage_cnt){
		if(!patch_info->stage_addr){
			kfree(patch_info->stage_addr);
		}
		patch_info->stage_addr =
			kzalloc(patch_info->stage_cnt*sizeof(u16), GFP_KERNEL);
		if (!patch_info->stage_addr) {
			dev_err(&data->client->dev, "stage_addr alloc error\n");
			return 1;
		}
		memcpy(patch_info->stage_addr, stage_addr,
			patch_info->stage_cnt*sizeof(u16));
	}
	if(patch_info->trigger_cnt){
		if(!patch_info->trigger_addr){
			kfree(patch_info->trigger_addr);
		}
		patch_info->trigger_addr =
			kzalloc(patch_info->trigger_cnt*sizeof(u16), GFP_KERNEL);
		if (!patch_info->trigger_addr) {
			dev_err(&data->client->dev, "trigger_addr alloc error\n");
			return 1;
		}
		memcpy(patch_info->trigger_addr, trigger_addr,
			patch_info->trigger_cnt*sizeof(u16));
	}
	if(patch_info->event_cnt){
		if(!patch_info->event_addr){
			kfree(patch_info->event_addr);
		}
		patch_info->event_addr =
			kzalloc(patch_info->event_cnt*sizeof(u16), GFP_KERNEL);
		if (!patch_info->event_addr) {
			dev_err(&data->client->dev, "event_addr alloc error\n");
			return 1;
		}
		memcpy(patch_info->event_addr, event_addr,
			patch_info->event_cnt*sizeof(u16));
	}

	mxt_patch_load_t71data(data);
	return 0;
}
#endif
