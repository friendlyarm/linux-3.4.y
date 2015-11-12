#ifndef _NXP_HDCP_H
#define _NXP_HDCP_H

#include <linux/interrupt.h>

struct work_struct;
struct workqueue_struct;
struct i2c_client;
struct mutex;
struct nxp_hdmi;
struct nxp_v4l2_i2c_board_info;

enum hdcp_event {
	HDCP_EVENT_STOP			= 1 << 0,
	HDCP_EVENT_START		= 1 << 1,
	HDCP_EVENT_READ_BKSV_START	= 1 << 2,
	HDCP_EVENT_WRITE_AKSV_START	= 1 << 4,
	HDCP_EVENT_CHECK_RI_START	= 1 << 8,
	HDCP_EVENT_SECOND_AUTH_START	= 1 << 16
};

enum hdcp_auth_state {
	NOT_AUTHENTICATED,
	RECEIVER_READ_READY,
	BCAPS_READ_DONE,
	BKSV_READ_DONE,
	AN_WRITE_DONE,
	AKSV_WRITE_DONE,
	FIRST_AUTHENTICATION_DONE,
	SECOND_AUTHENTICATION_RDY,
	SECOND_AUTHENTICATION_DONE,
};

struct nxp_hdcp {
    bool is_repeater;
    bool is_start;

    enum hdcp_event event;
    enum hdcp_auth_state auth_state;

    struct work_struct work;
    struct workqueue_struct *wq;
    
    irqreturn_t (*irq_handler)(struct nxp_hdcp *);
    int (*prepare)(struct nxp_hdcp *);
    int (*start)(struct nxp_hdcp *);
    int (*stop)(struct nxp_hdcp *);
    int (*suspend)(struct nxp_hdcp *);
    int (*resume)(struct nxp_hdcp *);

    int bus_id; /* i2c adapter id */ 
    struct i2c_client *client;

    struct mutex mutex;
};

/**
 * public api
 */
int  nxp_hdcp_init(struct nxp_hdcp *, struct nxp_v4l2_i2c_board_info *);
void nxp_hdcp_cleanup(struct nxp_hdcp *);

#endif
