#ifndef _NXP_OUT_H
#define _NXP_OUT_H

#define NXP_OUT_MLC_NUM     2

#if defined(CONFIG_NXP_OUT_HDMI)
struct nxp_hdmi;
#endif
#if defined(CONFIG_NXP_OUT_RESOLUTION_CONVERTER)
struct nxp_resc;
#endif
#if defined(CONFIG_NXP_OUT_TVOUT)
struct nxp_tvout;
#endif
struct nxp_mlc;
struct nxp_out_platformdata;

struct nxp_out {
    /* children */
    struct nxp_mlc *mlcs[NXP_OUT_MLC_NUM];

#if defined(CONFIG_NXP_OUT_HDMI)
    struct nxp_hdmi *hdmi;
#endif

#if defined(CONFIG_NXP_OUT_RESOLUTION_CONVERTER)
    struct nxp_resc *resc;
#endif

#if defined(CONFIG_NXP_OUT_TVOUT)
    struct nxp_tvout *tvout;
#endif
};

struct nxp_out *create_nxp_out(struct nxp_out_platformdata *);
void release_nxp_out(struct nxp_out *);
int  register_nxp_out(struct nxp_out *);
void unregister_nxp_out(struct nxp_out *);
int suspend_nxp_out(struct nxp_out *);
int resume_nxp_out(struct nxp_out *);

#endif
