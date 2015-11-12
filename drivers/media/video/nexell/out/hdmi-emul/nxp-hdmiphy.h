#ifndef _HDMIPHY_H
#define _HDMIPHY_H

struct v4l2_dv_preset;

struct hdmiphy_preset {
	u32 preset;
	const u8 *data;
};

struct nxp_hdmiphy {
    int (*s_power)(struct nxp_hdmiphy *, int);
    int (*s_dv_preset)(struct nxp_hdmiphy *, struct v4l2_dv_preset *);
    int (*s_stream)(struct nxp_hdmiphy *, int);
    int (*suspend)(struct nxp_hdmiphy *);
    int (*resume)(struct nxp_hdmiphy *);
};

/**
 * public api
 */
int  nxp_hdmiphy_init(struct nxp_hdmiphy *);
void nxp_hdmiphy_cleanup(struct nxp_hdmiphy *);

#endif
