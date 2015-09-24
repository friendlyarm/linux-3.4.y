#ifndef _NXP_TVOUT_H
#define _NXP_TVOUT_H

struct media_pad;
struct v4l2_subdev;

struct nxp_tvout {
    struct v4l2_subdev sd;
    struct media_pad pad;
};

/**
 * publi api
 */
struct nxp_tvout *create_nxp_tvout(void);
void release_nxp_tvout(struct nxp_tvout *);
int register_nxp_tvout(struct nxp_tvout *);
void unregister_nxp_tvout(struct nxp_tvout *);
int suspend_nxp_tvout(struct nxp_tvout *);
int resume_nxp_tvout(struct nxp_tvout *);

#endif
