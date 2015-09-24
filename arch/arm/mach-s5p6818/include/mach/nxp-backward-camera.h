#ifndef _NXP_BACKWARD_CAMERA_H_
#define _NXP_BACKWARD_CAMERA_H_ __FILE__

#define END_MARKER {0xff, 0xff}

struct reg_val {
    uint8_t reg;
    uint8_t val;
};

struct nxp_backward_camera_platform_data {
    int backgear_gpio_num;
    bool active_high; /* if turn on backgear and gpio goto high, this field is true */
    int vip_module_num;
    int mlc_module_num;

    // for suspend/resume
    // sensor
    int i2c_bus;
    int chip_addr;
    int clk_rate;
    struct reg_val *reg_val;
    int (*power_enable)(bool);
    int (*set_clock)(int);
    void (*setup_io)(void);

    // vip
    int port;
    bool external_sync;
    bool is_mipi;
    u32  h_active;
    u32  h_frontporch;
    u32  h_syncwidth;
    u32  h_backporch;
    u32  v_active;
    u32  v_frontporch;
    u32  v_syncwidth;
    u32  v_backporch;
    u32  data_order;
    bool interlace;

    u32 lu_addr;
    u32 cb_addr;
    u32 cr_addr;
    u32 lu_stride;
    u32 cb_stride;
    u32 cr_stride;

    u32 rgb_format;
    u32 width;
    u32 height;
    u32 rgb_addr;

    void (*draw_rgb_overlay)(struct nxp_backward_camera_platform_data *);
};

#endif
