/* This driver is for nexell nxp mpeg ts interface */
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/file.h>
#include <linux/miscdevice.h>
#include <linux/mm.h>
#include <linux/poll.h>
#include <linux/sched.h>
#include <linux/uaccess.h>
#include <linux/dma-mapping.h>
#include <linux/wait.h>
#include <linux/irq.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/interrupt.h>
#include <linux/ioctl.h>
#include <linux/kernel.h>
#include <linux/timer.h>
#include <linux/gpio.h>
#include <linux/syscalls.h>

#include <mach/platform.h>
#include <mach/nxp_mp2ts.h>


#define MP2TS_DBG_HEADER "[NXP-TS]"

//#define NEXELL_MPEGTS_DEBUG

#ifdef NEXELL_MPEGTS_DEBUG
#define MP2TS_DBG(args...) printk(MP2TS_DBG_HEADER ":" args)
#else
#define MP2TS_DBG(args...) do{}while(0)
#endif


#define ALLOC_ALIGN(size)   ALIGN(size, 16)


static struct ts_drv_context *s_ctx = NULL;
static int  one_sec_ticks = 100;

#if 0
static u16 NX_CAP_PIDs[] = {
    0x0000 | NX_CAP_VALID_PID,  /* PAT  */
    0x0001 | NX_CAP_VALID_PID,  /* CAT  */

    0x0c11 | NX_CAP_VALID_PID,  /* MBC */
    0x0011 | NX_CAP_VALID_PID,
    0x0014 | NX_CAP_VALID_PID,

    0x0411 | NX_CAP_VALID_PID,  /* KBS */
    0x0021 | NX_CAP_VALID_PID,
    0x0024 | NX_CAP_VALID_PID,
    0x0034 | NX_CAP_VALID_PID,

//    0x0001 | NX_CAP_VALID_PID,  /* SBS */
    0x0011 | NX_CAP_VALID_PID,
    0x0014 | NX_CAP_VALID_PID,
};
#endif

static int _prepare_dma_submit(
	u8 ch_num,
	struct ts_drv_context *ctx
	);


static inline int _w_able(struct ts_channel_info *buf)
{
    if(buf->cnt < buf->page_num)
    {
        return 1;
    }
    return 0;
}

static inline int _r_able(struct ts_channel_info *buf)
{
    if(buf->cnt)
    {
        return 1;
    }
    return 0;
}

static inline int _rw_able(struct ts_channel_info *buf)
{
    if( buf->is_first )
    {
        return 1;
    }
    else
    {
        if( buf->r_pos && (buf->w_pos == (buf->r_pos-1)) )
        {
            return 0;
        }
        else if( !buf->r_pos && (buf->w_pos == (buf->page_num-1)) )
        {
            return 0;
        }

        return 1;
    }

    return 0;
}

static inline void _w_buf(struct ts_channel_info *buf)
{
    if(buf->cnt < buf->page_num)
    {
        buf->cnt++;
        buf->w_pos = (buf->w_pos + 1) % buf->page_num;
    }
}

static inline void _r_buf(struct ts_channel_info *buf)
{
    if(buf->cnt)
    {
        buf->cnt--;
        buf->r_pos = (buf->r_pos + 1) % buf->page_num;
    }
}

static inline int _init_dma(u8 ch_num, struct ts_drv_context *ctx)
{
#if (CFG_MPEGTS_IDMA_MODE == 0)
	dma_filter_fn filter_fn;
	dma_cap_mask_t mask;

	MP2TS_DBG(" %s ++\n", __func__);

	filter_fn	= pl08x_filter_id;

	switch(ch_num)
	{
	case NXP_MP2TS_ID_CAP0:
		ctx->ch_info[ch_num].filter_data	= DMA_PERIPHERAL_NAME_MPEGTSI0;
		break;
	case NXP_MP2TS_ID_CAP1:
		ctx->ch_info[ch_num].filter_data	= DMA_PERIPHERAL_NAME_MPEGTSI1;
		break;
	case NXP_MP2TS_ID_CORE:
		ctx->ch_info[ch_num].filter_data	= DMA_PERIPHERAL_NAME_MPEGTSI2;
		break;
	case NXP_MP2TS_ID_MAX:
		ctx->ch_info[ch_num].filter_data	= DMA_PERIPHERAL_NAME_MPEGTSI3;
		break;
	}

	dma_cap_zero(mask);
	dma_cap_set(DMA_SLAVE , mask);
	dma_cap_set(DMA_CYCLIC, mask);

onemore_request:
	ctx->ch_info[ch_num].dma_chan = dma_request_channel(mask, filter_fn, ctx->ch_info[ch_num].filter_data);
	if (!ctx->ch_info[ch_num].dma_chan)
	{
		MP2TS_DBG("Error: dma '%s'\n", (char*)ctx->ch_info[ch_num].filter_data);
		goto error_request;
	}

    if (ch_num == NXP_MP2TS_ID_CORE)
    {
        ch_num++;
        goto onemore_request;
    }

    return 0;

error_request:

    if (ch_num == NXP_MP2TS_ID_MAX)
    {
		dma_release_channel(ctx->ch_info[ch_num-1].dma_chan);
    }

	return -EINVAL;
#endif
}

static inline void _deinit_dma(u8 ch_num, struct ts_drv_context *ctx)
{
	MP2TS_DBG("%s ++\n", __func__);

#if (CFG_MPEGTS_IDMA_MODE == 1)
	NX_MPEGTSI_SetIDMAIntMaskClear(ch_num, CFALSE);
	NX_MPEGTSI_SetIDMAIntClear(ch_num);
	NX_MPEGTSI_SetIDMAEnable(ch_num, CFALSE);
#else
	if (ctx->ch_info[ch_num].dma_chan) {
		dma_release_channel(ctx->ch_info[ch_num].dma_chan);

		ctx->ch_info[ch_num].dma_chan = NULL;
	}
#endif

	ctx->ch_info[ch_num].cnt = 0;
}

static inline void _start_dma(u8 ch_num, struct ts_drv_context *ctx)
{
	u8	tmp_chnum, tx_mode;

	MP2TS_DBG("%s ++\n", __func__);

	tx_mode = ctx->ch_info[ch_num].tx_mode; 

	tmp_chnum = ch_num;
repeat_set_dma:

#if (CFG_MPEGTS_IDMA_MODE == 1)
	NX_MPEGTSI_SetIDMABaseAddr(tmp_chnum, ctx->ch_info[tmp_chnum].dma_phy);
	NX_MPEGTSI_SetIDMALength(tmp_chnum, ctx->ch_info[tmp_chnum].page_size);

	NX_MPEGTSI_SetIDMAEnable(tmp_chnum, CTRUE);
	NX_MPEGTSI_SetIDMAIntEnable(tmp_chnum, CTRUE);

	if( (tmp_chnum != NXP_MP2TS_ID_CORE) && (tx_mode != 1) )
	{
		NX_MPEGTSI_RunIDMA(tmp_chnum);
	}
	else
	{
		ctx->ch_info[tmp_chnum].do_continue = 1;
	}

	NX_MPEGTSI_SetIDMAIntMaskClear(tmp_chnum, CTRUE);
#else

	if (ctx->ch_info[tmp_chnum].dma_chan)
	{
		_prepare_dma_submit(tmp_chnum, ctx);

	    if( (tmp_chnum != NXP_MP2TS_ID_CORE) && (tx_mode != 1) )
	    {
			dma_async_issue_pending(ctx->ch_info[tmp_chnum].dma_chan);
	    }
	    else
	    {
	        ctx->ch_info[tmp_chnum].do_continue = 1;
	    }
	}
#endif

	if( tmp_chnum == NXP_MP2TS_ID_CORE )
	{
		tmp_chnum++;
		goto repeat_set_dma;
	}
}

static inline void _stop_dma(u8 ch_num, struct ts_drv_context *ctx)
{
	u8	tmp_chnum;

	MP2TS_DBG("%s ++\n", __func__);

	tmp_chnum = ch_num;

repeat_stop_dma:
	
#if (CFG_MPEGTS_IDMA_MODE == 1)
	NX_MPEGTSI_SetIDMAIntMaskClear(tmp_chnum, CFALSE);
	NX_MPEGTSI_StopIDMA(tmp_chnum);
	NX_MPEGTSI_SetIDMAIntEnable(tmp_chnum, CFALSE);
	NX_MPEGTSI_SetIDMAIntClear(tmp_chnum);
	NX_MPEGTSI_SetIDMAEnable(tmp_chnum, CFALSE);
#else

	if (ctx->ch_info[tmp_chnum].dma_chan) {
		dmaengine_terminate_all(ctx->ch_info[tmp_chnum].dma_chan);
	}
#endif
	ctx->ch_info[tmp_chnum].cnt = 0;

	if( tmp_chnum == NXP_MP2TS_ID_CORE )
	{
		tmp_chnum++;
		goto repeat_stop_dma;
	}
}

static inline int _init_buf(struct ts_drv_context *ctx, struct ts_buf_init_info *init_info)
{
    int ret = 0;
    u8  ch_num;

    ch_num = init_info->ch_num;

    ctx->dev->coherent_dma_mask = 0xffffffff;

onemore_alloc:
    ctx->ch_info[ch_num].cnt    = 0;
    ctx->ch_info[ch_num].w_pos  = 0;
    ctx->ch_info[ch_num].r_pos  = 0;

    ctx->ch_info[ch_num].page_size      = init_info->page_size;
    ctx->ch_info[ch_num].page_num       = init_info->page_num;
    ctx->ch_info[ch_num].alloc_align    = init_info->page_size;
    ctx->ch_info[ch_num].alloc_size     = (ctx->ch_info[ch_num].alloc_align * init_info->page_num);


    ctx->ch_info[ch_num].dma_virt = (unsigned int)dma_alloc_writecombine(ctx->dev, ctx->ch_info[ch_num].alloc_size, &ctx->ch_info[ch_num].dma_phy, GFP_ATOMIC);
    if (!ctx->ch_info[ch_num].dma_virt)
    {
        printk(KERN_ERR "can't alloc packet buffer...\n");
        ret = -ETS_ALLOC;
        goto fail_dma_buf;
    }

    if (ch_num == NXP_MP2TS_ID_CORE)
    {
        ch_num++;
        goto onemore_alloc;
    }

    return 0;

fail_dma_buf:
    ch_num = init_info->ch_num;

onemore_free:
    ctx->ch_info[ch_num].alloc_align  = 0;
    ctx->ch_info[ch_num].alloc_size   = 0;
    ctx->ch_info[ch_num].page_size    = 0;
    ctx->ch_info[ch_num].page_num     = 0;
    ctx->ch_info[ch_num].dma_virt     = 0;
    ctx->ch_info[ch_num].dma_phy      = 0;

    if (ch_num == NXP_MP2TS_ID_CORE)
    {
        ch_num++;
        goto onemore_free;
    }

    return ret;
}

static inline void _deinit_buf(u8 ch_num, struct ts_drv_context *ctx)
{
    MP2TS_DBG(" %s ++\n", __func__);

    if (ctx->ch_info[ch_num].dma_virt)
    {
        dma_free_coherent(ctx->dev,
                ctx->ch_info[ch_num].alloc_size,
                (void *)ctx->ch_info[ch_num].dma_virt,
                ctx->ch_info[ch_num].dma_phy);

        ctx->ch_info[ch_num].alloc_align = 0;
        ctx->ch_info[ch_num].alloc_size = 0;
        ctx->ch_info[ch_num].page_size = 0;
        ctx->ch_info[ch_num].page_num = 0;
        ctx->ch_info[ch_num].dma_virt = 0;
        ctx->ch_info[ch_num].dma_phy = 0;
    }
}

static int _init_context(struct ts_drv_context *ctx, struct nxp_mp2ts_plat_data *pdata)
{
    u8 ch_num;

    MP2TS_DBG(" %s ++\n", __func__);

    for (ch_num = 0; ch_num < 2; ch_num++)
    {
        // Reset
        if (pdata->dev_info[ch_num].demod_rst_num > -1)
        {
            gpio_set_value(pdata->dev_info[ch_num].demod_rst_num,   1);
            mdelay(1);
            gpio_set_value(pdata->dev_info[ch_num].demod_rst_num,   0);
            mdelay(1);
            gpio_set_value(pdata->dev_info[ch_num].demod_rst_num,   1);
        }
        mdelay(1);

        // Interrup pin
        if (pdata->dev_info[ch_num].demod_irq_num > -1)
        {
            nxp_soc_gpio_set_int_mode(pdata->dev_info[ch_num].demod_irq_num,    1);             // High Level
        }
    }

    ctx->baseaddr = (void __iomem *)IO_ADDRESS(PHY_BASEADDR_MPEGTSI);

    for (ch_num = 0; ch_num < NXP_IDMA_MAX; ch_num++)
    {
        init_waitqueue_head(&ctx->ch_info[ch_num].wait);
    }

    return 0;
}

static void _deinit_context(struct ts_drv_context *ctx)
{
    u8 ch_num;

    MP2TS_DBG(" %s ++\n", __func__);

    for (ch_num = 0; ch_num < NXP_IDMA_MAX; ch_num++)
    {
        wake_up(&ctx->ch_info[ch_num].wait);
        _stop_dma(ch_num, ctx);
        _deinit_dma(ch_num, ctx);
        _deinit_buf(ch_num, ctx);
    }
}

static int _init_device(struct ts_drv_context *ctx)
{
    struct clk *mp2ts_clk;
    u32 addr;

    MP2TS_DBG(" %s ++\n", __func__);

    // Clock control
    mp2ts_clk = clk_get(ctx->dev, DEV_NAME_MPEGTSI);
    clk_enable(mp2ts_clk);

    // MP2TS Reset control
    NX_RSTCON_Initialize();
    addr = NX_RSTCON_GetPhysicalAddress();
    printk("NX_RSTCON_GetPhysicalAddress = 0x%08x\n", addr);
    NX_RSTCON_SetBaseAddress( (void*)IO_ADDRESS(addr) );
#if defined(CONFIG_ARCH_S5P4418)
    NX_RSTCON_SetnRST(RESETINDEX_OF_MPEGTSI_MODULE_i_nRST, RSTCON_DISABLE);
    udelay(100);
    NX_RSTCON_SetnRST(RESETINDEX_OF_MPEGTSI_MODULE_i_nRST, RSTCON_ENABLE);
    udelay(100);
#elif defined(CONFIG_ARCH_S5P6818)
    NX_RSTCON_SetRST(RESETINDEX_OF_MPEGTSI_MODULE_i_nRST, RSTCON_ASSERT);
    udelay(100);
    NX_RSTCON_SetRST(RESETINDEX_OF_MPEGTSI_MODULE_i_nRST, RSTCON_NEGATE);
    udelay(100);
#endif

    // MPEGTSI Initialize
    NX_MPEGTSI_Initialize();
    addr = NX_MPEGTSI_GetPhysicalAddress();
    printk("NX_MPEGTSI_GetPhysicalAddress = 0x%08x\n", addr);
    NX_MPEGTSI_SetBaseAddress( (void*)IO_ADDRESS(addr) );

    return 0;
}

static void _deinit_device(void)
{
#if 0   // by kook
    NX_MPEGTSI_CloseModule();
#endif
}

static int _power_on_device(u8 ch_num)
{
    int i;

    if (ch_num == NXP_MP2TS_ID_CORE)
    {
        NX_MPEGTSI_SetTsiEnable(CFALSE);

        NX_MPEGTSI_SetTsiSramPowerEnable(CTRUE);
        NX_MPEGTSI_SetTsiSramWakeUp(CTRUE);
    }
    else
    {
        if( ch_num == NXP_MP2TS_ID_CAP0 )
        {
            /* Set pad capture ch0 */
            nxp_soc_gpio_set_io_pull_enb(PAD_GPIO_B+24, CFALSE);        // DATA[0]
            nxp_soc_gpio_set_io_pull_enb(PAD_GPIO_B+25, CFALSE);        // DATA[1]
            nxp_soc_gpio_set_io_pull_enb(PAD_GPIO_B+26, CFALSE);        // DATA[2]
            nxp_soc_gpio_set_io_pull_enb(PAD_GPIO_B+27, CFALSE);        // DATA[3]
            nxp_soc_gpio_set_io_pull_enb(PAD_GPIO_B+28, CFALSE);        // DATA[4]
            nxp_soc_gpio_set_io_pull_enb(PAD_GPIO_B+29, CFALSE);        // DATA[5]
            nxp_soc_gpio_set_io_pull_enb(PAD_GPIO_B+30, CFALSE);        // DATA[6]
            nxp_soc_gpio_set_io_pull_enb(PAD_GPIO_B+31, CFALSE);        // DATA[7]
            nxp_soc_gpio_set_io_pull_enb(PAD_GPIO_C+15, CFALSE);        // CLk
            nxp_soc_gpio_set_io_pull_enb(PAD_GPIO_C+16, CFALSE);        // SYNC
            nxp_soc_gpio_set_io_pull_enb(PAD_GPIO_C+17, CFALSE);        // VALID

            nxp_soc_gpio_set_io_func(PAD_GPIO_B+24, NX_GPIO_PADFUNC_2); // DATA[0]
            nxp_soc_gpio_set_io_func(PAD_GPIO_B+25, NX_GPIO_PADFUNC_2); // DATA[1]
            nxp_soc_gpio_set_io_func(PAD_GPIO_B+26, NX_GPIO_PADFUNC_2); // DATA[2]
            nxp_soc_gpio_set_io_func(PAD_GPIO_B+27, NX_GPIO_PADFUNC_2); // DATA[3]
            nxp_soc_gpio_set_io_func(PAD_GPIO_B+28, NX_GPIO_PADFUNC_2); // DATA[4]
            nxp_soc_gpio_set_io_func(PAD_GPIO_B+29, NX_GPIO_PADFUNC_2); // DATA[5]
            nxp_soc_gpio_set_io_func(PAD_GPIO_B+30, NX_GPIO_PADFUNC_2); // DATA[6]
            nxp_soc_gpio_set_io_func(PAD_GPIO_B+31, NX_GPIO_PADFUNC_2); // DATA[7]
            nxp_soc_gpio_set_io_func(PAD_GPIO_C+15, NX_GPIO_PADFUNC_2); // CLk
            nxp_soc_gpio_set_io_func(PAD_GPIO_C+16, NX_GPIO_PADFUNC_2); // SYNC
            nxp_soc_gpio_set_io_func(PAD_GPIO_C+17, NX_GPIO_PADFUNC_2); // VALID
        }
        else
        {
            /* Set pad capture ch1 */
            nxp_soc_gpio_set_io_pull_enb(PAD_GPIO_D+28, CFALSE);        // DATA[0]
            nxp_soc_gpio_set_io_pull_enb(PAD_GPIO_D+29, CFALSE);        // DATA[1]
            nxp_soc_gpio_set_io_pull_enb(PAD_GPIO_D+30, CFALSE);        // DATA[2]
            nxp_soc_gpio_set_io_pull_enb(PAD_GPIO_D+31, CFALSE);        // DATA[3]
            nxp_soc_gpio_set_io_pull_enb(PAD_GPIO_E+0,  CFALSE);        // DATA[4]
            nxp_soc_gpio_set_io_pull_enb(PAD_GPIO_E+1,  CFALSE);        // DATA[5]
            nxp_soc_gpio_set_io_pull_enb(PAD_GPIO_E+2,  CFALSE);        // DATA[6]
            nxp_soc_gpio_set_io_pull_enb(PAD_GPIO_E+3,  CFALSE);        // DATA[7]
            nxp_soc_gpio_set_io_pull_enb(PAD_GPIO_E+4,  CFALSE);        // CLk
            nxp_soc_gpio_set_io_pull_enb(PAD_GPIO_E+5,  CFALSE);        // SYNC
            nxp_soc_gpio_set_io_pull_enb(PAD_GPIO_E+6,  CFALSE);        // VALID

            nxp_soc_gpio_set_io_func(PAD_GPIO_D+28, NX_GPIO_PADFUNC_2); // DATA[0]
            nxp_soc_gpio_set_io_func(PAD_GPIO_D+29, NX_GPIO_PADFUNC_2); // DATA[1]
            nxp_soc_gpio_set_io_func(PAD_GPIO_D+30, NX_GPIO_PADFUNC_2); // DATA[2]
            nxp_soc_gpio_set_io_func(PAD_GPIO_D+31, NX_GPIO_PADFUNC_2); // DATA[3]
            nxp_soc_gpio_set_io_func(PAD_GPIO_E+0,  NX_GPIO_PADFUNC_2); // DATA[4]
            nxp_soc_gpio_set_io_func(PAD_GPIO_E+1,  NX_GPIO_PADFUNC_2); // DATA[5]
            nxp_soc_gpio_set_io_func(PAD_GPIO_E+2,  NX_GPIO_PADFUNC_2); // DATA[6]
            nxp_soc_gpio_set_io_func(PAD_GPIO_E+3,  NX_GPIO_PADFUNC_2); // DATA[7]
            nxp_soc_gpio_set_io_func(PAD_GPIO_E+4,  NX_GPIO_PADFUNC_2); // CLk
            nxp_soc_gpio_set_io_func(PAD_GPIO_E+5,  NX_GPIO_PADFUNC_2); // SYNC
            nxp_soc_gpio_set_io_func(PAD_GPIO_E+6,  NX_GPIO_PADFUNC_2); // VALID
        }

        NX_MPEGTSI_SetCapEnable(ch_num, CFALSE);

        NX_MPEGTSI_SetCapSramPowerEnable(ch_num, CTRUE);
        NX_MPEGTSI_SetCapSramWakeUp(ch_num, CTRUE);
    }

    if( ch_num == NXP_MP2TS_ID_CORE )
    {
        // Clear Core PID Filter
        for( i = 0; i < TS_CORE_PID_MAX; i++ )
        {
            NX_MPEGTSI_WritePID( 2, NX_PARAM_TYPE_PID, i, 0x1fff1fff );   // clear 128 PIDs
        }

        for( i = 0; i < TS_CORE_PID_MAX; i++ )
        {
            NX_MPEGTSI_WriteAESKEYIV( i, 0, 0, 0, 0, 0, 0, 0, 0 );
        }
    }
    else
    {
        // Clear Capture Port PID Filter
        for( i = 0; i < TS_CAPx_PID_MAX; i++ )
        {
            NX_MPEGTSI_WritePID( ch_num, NX_PARAM_TYPE_PID, i, 0x1fff1fff );   // clear 128 PIDs
        }
    }

    return 0;
}

static int _power_off_device(u8 ch_num)
{
    if (ch_num == NXP_MP2TS_ID_CORE)
    {
        NX_MPEGTSI_SetTsiEnable(CFALSE);

        NX_MPEGTSI_SetTsiSramPowerEnable(CFALSE);
        NX_MPEGTSI_SetTsiSramWakeUp(CFALSE);
    }
    else
    {
        NX_MPEGTSI_SetCapEnable(ch_num, CFALSE);

        NX_MPEGTSI_SetCapSramPowerEnable(ch_num, CFALSE);
        NX_MPEGTSI_SetCapSramWakeUp(ch_num, CFALSE);
    }

    return 0;
}

static int _set_config(struct ts_config_descr *config_descr)
{
    if (config_descr->ch_num == NXP_MP2TS_ID_CORE)
    {
        NX_MPEGTSI_SetTsiEncrypt(config_descr->un.bits.encry_on);
    }
    else
    {
        NX_MPEGTSI_SetTCLKPolarityEnable(config_descr->ch_num, config_descr->un.bits.clock_pol);
        NX_MPEGTSI_SetTDPPolarityEnable(config_descr->ch_num, config_descr->un.bits.valid_pol);
        NX_MPEGTSI_SetTSYNCPolarityEnable(config_descr->ch_num, config_descr->un.bits.sync_pol);
        NX_MPEGTSI_SetTERRPolarityEnable(config_descr->ch_num, config_descr->un.bits.err_pol);
        NX_MPEGTSI_SetSerialEnable(config_descr->ch_num, config_descr->un.bits.data_width1);
        NX_MPEGTSI_SetBypassEnable(config_descr->ch_num, config_descr->un.bits.bypass_enb);

        if (config_descr->ch_num == NXP_MP2TS_ID_CAP1)
        {
            if (config_descr->un.bits.xfer_mode)
            {
                s_ctx->ch_info[NXP_MP2TS_ID_CAP1].tx_mode  = 1;
                NX_MPEGTSI_SetCap1OutputEnable(CTRUE);
                NX_MPEGTSI_SetCap1OutTCLKPolarityEnable(config_descr->un.bits.xfer_clk_pol);
            }
            else
            {
                s_ctx->ch_info[NXP_MP2TS_ID_CAP1].tx_mode  = 0;
                NX_MPEGTSI_SetCap1OutputEnable(CFALSE);
            }
        }
    }

    return 0;
}

static int _get_config(struct ts_config_descr *config_descr)
{
    u8 ch_num = config_descr->ch_num;

    memset((void *)config_descr, 0x00, sizeof(config_descr) );
    config_descr->ch_num = ch_num;

    if (config_descr->ch_num == NXP_MP2TS_ID_CORE)
    {
        config_descr->un.bits.encry_on      = NX_MPEGTSI_GetTsiEncrypt();
    }
    else
    {
        config_descr->un.bits.clock_pol     = NX_MPEGTSI_GetTCLKPolarityEnable(config_descr->ch_num);
        config_descr->un.bits.valid_pol     = NX_MPEGTSI_GetTDPPolarityEnable(config_descr->ch_num);
        config_descr->un.bits.sync_pol      = NX_MPEGTSI_GetTSYNCPolarityEnable(config_descr->ch_num);
        config_descr->un.bits.err_pol       = NX_MPEGTSI_GetTERRPolarityEnable(config_descr->ch_num);
        config_descr->un.bits.data_width1   = NX_MPEGTSI_GetSerialEnable(config_descr->ch_num);
        config_descr->un.bits.bypass_enb    = NX_MPEGTSI_GetBypassEnable(config_descr->ch_num);

        if (config_descr->ch_num == NXP_MP2TS_ID_CAP1)
        {
            config_descr->un.bits.xfer_clk_pol  = NX_MPEGTSI_GetCap1OutTCLKPolarityEnable();
            config_descr->un.bits.xfer_mode     = NX_MPEGTSI_GetCap1OutputEnable();
        }
    }

    return 0;
}

static int _set_param(struct ts_param_descr *param_descr)
{
    int idx, cnt, max_cnt;
    u8  ch_num;

    ch_num = param_descr->info.un.bits.ch_num;

    if (param_descr->info.un.bits.type == NXP_MP2TS_PARAM_TYPE_PID)
    {
        u16 *PID_buf16;
        u32 temp;

        PID_buf16 = (u16 *)param_descr->buf;

        if (param_descr->info.un.bits.ch_num == NXP_MP2TS_ID_CORE)
        {
            cnt = TS_CORE_PID_MAX;
        }
        else
        {
            cnt = TS_CAPx_PID_MAX;
        }

        max_cnt = min( (param_descr->buf_size/2), cnt );
        for( idx = 0, cnt = 0; cnt < (max_cnt/2); cnt++ )
        {
            temp = ( (PID_buf16[idx+1] << 16) | PID_buf16[idx] );
            if( temp == 0x3fff3fff )
            {
                NX_MPEGTSI_WritePID( ch_num, NX_PARAM_TYPE_PID, cnt, 0x1fff1fff ); // clear
            }
            else
            {
                NX_MPEGTSI_WritePID( ch_num, NX_PARAM_TYPE_PID, cnt, temp );
            }

            idx += 2;
        }

        if( max_cnt & 1 )
        {
            temp = ( (NX_NO_PID << 16) | PID_buf16[idx] );
            if( temp == 0x3fff3fff )
            {
                NX_MPEGTSI_WritePID( ch_num, NX_PARAM_TYPE_PID, cnt, 0x1fff1fff ); // clear
            }
            else
            {
                NX_MPEGTSI_WritePID( ch_num, NX_PARAM_TYPE_PID, cnt, temp );
            }
        }
    }
    else if (param_descr->info.un.bits.ch_num == NXP_MP2TS_ID_CORE)
    {
        u32 *CAS_buf32;

        CAS_buf32 = (u32 *)param_descr->buf;

        cnt = TS_CORE_PID_MAX * 8;
        max_cnt = min( (param_descr->buf_size/4), cnt );
        for( cnt = 0; cnt < max_cnt; cnt++ )
        {
            NX_MPEGTSI_WritePID( ch_num, NX_PARAM_TYPE_CAS, cnt, CAS_buf32[cnt] );
        }
    }

    return 0;
}

static int _clear_param(struct ts_param_descr *param_descr)
{
	u8 ch_num;
	int i=0;

	ch_num = param_descr->info.un.bits.ch_num;

	if (param_descr->info.un.bits.type == NXP_MP2TS_PARAM_TYPE_PID)
	{
		if( ch_num == NXP_MP2TS_ID_CORE )
		{
			// Clear Core PID Filter
			for( i = 0; i < TS_CORE_PID_MAX; i++ )
			{
				NX_MPEGTSI_WritePID( 2, NX_PARAM_TYPE_PID, i, 0x1fff1fff );   // clear 128 PIDs
			}

			for( i = 0; i < TS_CORE_PID_MAX; i++ )
			{
				NX_MPEGTSI_WriteAESKEYIV( i, 0, 0, 0, 0, 0, 0, 0, 0 );
			}
		}
		else
		{
			// Clear Capture Port PID Filter
			for( i = 0; i < TS_CAPx_PID_MAX; i++ )
			{
				NX_MPEGTSI_WritePID( ch_num, NX_PARAM_TYPE_PID, i, 0x1fff1fff );   // clear 128 PIDs
			}
		}
	}

	return 0;
}

static int _get_param(struct ts_param_descr *param_descr)
{
#if 0
    struct ts_param_reg param_reg;
    u16 *SRAM16, *SBUF16;
    u32 *SRAM32, *SBUF32;
    int cnt, max_cnt;

    param_reg.un.bits.ch_num = param_descr->ch_num;

#if 1
    for (cnt = 0; cnt < max_cnt; cnt++)
    {
        param_reg.un.bits.index = (cnt & 0x7F);
        NX_MPEGTSI_SetCPUWrAddr(param_reg.data);
        NX_MPEGTSI_SetCPUWrData(SBUF16[cnt]);
printk("PID Val = 0x%04x\n", SBUF16[cnt]);
    }
#endif
#endif

    return 0;
}

static int _enable_device(struct ts_op_mode *ts_op)
{
    u8  ch_num, tx_mode;

    ch_num  = ts_op->ch_num;
    tx_mode = s_ctx->ch_info[ch_num].tx_mode; 

    if (!s_ctx->ch_info[ch_num].dma_phy)
    {
        return -1;
    }

    if (ch_num == NXP_MP2TS_ID_CORE)
    {
        NX_MPEGTSI_SetTsiIntClear();
        NX_MPEGTSI_SetTsiIntMaskClear(CTRUE);
        NX_MPEGTSI_SetTsiEnable(CTRUE);
    }
    else
    {
#if (CFG_MPEGTS_IDMA_MODE == 1)
        NX_MPEGTSI_SetCapEnable(ch_num, CFALSE);
        NX_MPEGTSI_SetCapIntMaskClear(ch_num, CFALSE);
        NX_MPEGTSI_SetCapIntClear(ch_num);
#endif

        NX_MPEGTSI_SetCapIntLockEnable(ch_num, CTRUE);
//        NX_MPEGTSI_SetCapIntEnable(ch_num, CTRUE);

        NX_MPEGTSI_SetCapIntMaskClear(ch_num, CTRUE);

        NX_MPEGTSI_SetCapEnable(ch_num, CTRUE);
    }

    return 0;
}

#if (0)
static void  _mpegts_clear(void)
{

    NX_MPEGTSI_SetTDPPolarityEnable(1);
    msleep(1);
    //NX_MPEGTSI_SetTDPPolarityEnable(0);

}
#endif

static int _disable_device(u8 ch_num, struct ts_drv_context *ctx)
{
    MP2TS_DBG("%s ++\n", __func__);

    if( ch_num == NXP_MP2TS_ID_CORE )
    {
        NX_MPEGTSI_SetTsiIntMaskClear(CFALSE);
        NX_MPEGTSI_SetTsiIntEnable(CFALSE);
        NX_MPEGTSI_SetTsiIntClear();

        NX_MPEGTSI_SetTsiEnable(CFALSE);

        NX_MPEGTSI_SetTsiSramPowerEnable(CFALSE);
        NX_MPEGTSI_SetTsiSramWakeUp(CFALSE);
    }
    else
    {
        NX_MPEGTSI_SetCapIntMaskClear(ch_num, CFALSE);
        NX_MPEGTSI_SetCapIntEnable(ch_num, CFALSE);
        NX_MPEGTSI_SetCapIntClear(ch_num);

        NX_MPEGTSI_SetCapEnable(ch_num, CFALSE);

        NX_MPEGTSI_SetCapSramWakeUp(ch_num, CFALSE);
        NX_MPEGTSI_SetCapSramPowerEnable(ch_num, CFALSE);
    }

    msleep(1);

    return 0;
}

static int _prepare_dma(int ch_num, struct ts_drv_context *ctx, struct ts_buf_init_info *buf_info)
{
    int ret;

#if (CFG_MPEGTS_IDMA_MODE == 1)
    ret = _init_buf(ctx, buf_info);
    if ( ret != 0 )
        goto exit_cleanup;
#else

    ret = _init_dma(ch_num, ctx);
    if ( ret != 0 )
        goto exit_cleanup;

    ret = _init_buf(ctx, buf_info);
    if ( ret != 0 )
    {
        _deinit_dma(ch_num, ctx);
        goto exit_cleanup;
    }
#endif

    ctx->ch_info[ch_num].is_malloced  = 1;

exit_cleanup:
    return ret;
}

/* application interface */
static int mpegts_open(struct inode *inode, struct file *filp)
{
    int ret = 0;

    MP2TS_DBG("%s ++\n", __func__);
    if (s_ctx->is_opened) {
        printk("MPEG TS Device is already opened!!!\n");
        return -EBUSY;
    }
    s_ctx->is_opened = 1;
    s_ctx->swich_ch = 0xFF;

    MP2TS_DBG("%s --\n", __func__);
    return ret;
}

static int mpegts_close(struct inode *inode, struct file *filp)
{
    u8  ch_num;
    int ret = 0;

    MP2TS_DBG("%s ++\n", __func__);
    if (!s_ctx->is_opened) {
        printk("MPEG TS Device is not opened!!!\n");
        return -EINVAL;
    }

    for (ch_num = 0; ch_num < NXP_MP2TS_ID_MAX; ch_num++)
    {
        if( s_ctx->ch_info[ch_num].is_running )
        {
            s_ctx->ch_info[ch_num].is_running = 0;
            _stop_dma(ch_num, s_ctx);
            _disable_device(ch_num, s_ctx);
        }
    }

    for (ch_num = 0; ch_num < NXP_MP2TS_ID_MAX; ch_num++)
    {
        s_ctx->ch_info[ch_num].is_first     = 1;
        s_ctx->ch_info[ch_num].do_continue  = 0;
    }

    s_ctx->is_opened = 0;

    MP2TS_DBG("%s --\n", __func__);
    return ret;
}

static ssize_t mpegts_read(struct file *filp, char *buf, size_t count, loff_t *pos)
{
    return 0;
}

static ssize_t mpegts_read_buf(u8 ch_num, struct ts_param_descr *param_descr)
{
	u32         src_addr;
	char        *buf        = param_descr->buf;
	size_t      want_size   = param_descr->buf_size;
	signed long wait_time   = msecs_to_jiffies(s_ctx->ch_info[ch_num].wait_time);
	size_t      temp_size;
	int         ret = 0;

	if( !_r_able(&s_ctx->ch_info[ch_num]) )
	{
		if (wait_time)
		{
			ret = interruptible_sleep_on_timeout(&s_ctx->ch_info[ch_num].wait, wait_time);
			if (ret == 0)
			{
				MP2TS_DBG("time out \n");
			}

			if( !_r_able(&s_ctx->ch_info[ch_num]) )
				return -ETS_READBUF;
		}
		else
		{
			return -ETS_READBUF;
		}
	}

	temp_size = (s_ctx->ch_info[ch_num].cnt * s_ctx->ch_info[ch_num].alloc_align);
	if (want_size > temp_size)
		want_size = temp_size;

	src_addr = s_ctx->ch_info[ch_num].dma_virt + (s_ctx->ch_info[ch_num].alloc_align * s_ctx->ch_info[ch_num].r_pos);
	if (copy_to_user((void *)buf, (const void *)src_addr, want_size))
	{
		return -ETS_FAULT;
	}

	_r_buf(&s_ctx->ch_info[ch_num]);

	MP2TS_DBG("read_buf : cnt = %d\n", s_ctx->ch_info[ch_num].cnt);

	return want_size;
}

static ssize_t mpegts_write_buf(u8 ch_num, struct ts_param_descr *param_descr)
{
	u32         dst_addr;
	char        *buf        = param_descr->buf;
	size_t      want_size   = param_descr->buf_size;
	signed long wait_time   = msecs_to_jiffies(s_ctx->ch_info[ch_num].wait_time);
	size_t      temp_size;
	int         ret = 0;

	if( !_w_able(&s_ctx->ch_info[ch_num]) )
	{
		if (wait_time)
		{
			ret = interruptible_sleep_on_timeout(&s_ctx->ch_info[ch_num].wait, wait_time);
			if (ret == 0)
			{
				MP2TS_DBG("time out : cnt = %d\n", s_ctx->ch_info[ch_num].cnt);
			}

			if( !_w_able(&s_ctx->ch_info[ch_num]) )
				return -ETS_WRITEBUF;
		}
		else
		{
			return -ETS_WRITEBUF;
		}
	}

	temp_size = (s_ctx->ch_info[ch_num].cnt * s_ctx->ch_info[ch_num].alloc_align);
	if (want_size > temp_size)
		want_size = temp_size;

	dst_addr = s_ctx->ch_info[ch_num].dma_virt + (s_ctx->ch_info[ch_num].alloc_align * s_ctx->ch_info[ch_num].w_pos);
	if (copy_from_user((void *)dst_addr, (const void *)buf, want_size))
	{
		return -ETS_FAULT;
	}

	_w_buf(&s_ctx->ch_info[ch_num]);

	return want_size;
}

static long mpegts_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    u8 ch_num = 0;
    long ret = -1;

    switch(cmd)
    {
        case IOCTL_MPEGTS_POWER_ON:     // init
        {
            if(copy_from_user((void *)&ch_num, (const void *)arg, sizeof(ch_num)))
                return false;

            printk("IOCTL : [MP2TS] Capture IF Power On : ch num %d\n", ch_num);

            _power_on_device(ch_num);

            ret = 0;
        }
            break;

        case IOCTL_MPEGTS_POWER_OFF:    // deinit
        {
            if(copy_from_user((void *)&ch_num, (const void *)arg, sizeof(ch_num)))
                return false;

            printk("IOCTL : [MP2TS] Capture IF Power Off\n");

            _power_off_device(ch_num);
        }

            ret = 0;
            break;

        case IOCTL_MPEGTS_RUN:
        {
            struct ts_op_mode   ts_op;
            u8 temp_chnum;

            if(copy_from_user((void *)&ts_op, (const void *)arg, sizeof(struct ts_op_mode)))
            {
                ret = -EFAULT;
                goto exit_ioctl;
            }

            ch_num = ts_op.ch_num;

            if( !s_ctx->ch_info[ch_num].is_malloced )
                break;
            if( s_ctx->ch_info[ch_num].is_running )
                break;

            temp_chnum = ch_num;
repeat_init:
//            s_ctx->ch_info[temp_chnum].tx_mode  = ts_op.tx_mode;
            s_ctx->ch_info[temp_chnum].cnt      = 0;
            s_ctx->ch_info[temp_chnum].w_pos    = 0;
            s_ctx->ch_info[temp_chnum].r_pos    = 0;

            if (temp_chnum == NXP_MP2TS_ID_CORE)
            {
                temp_chnum++;
                goto repeat_init;
            }

            msleep(1);
			_start_dma(ch_num, s_ctx);
            ret = _enable_device(&ts_op);
            if( ret == 0 )
            {
                s_ctx->ch_info[ch_num].is_running = 1;
                ret = 0;
            }
            else
            {
                s_ctx->ch_info[ch_num].is_running = 0;
                _stop_dma(ch_num, s_ctx);
            }
        }
            break;

        case IOCTL_MPEGTS_STOP:
            if(copy_from_user((void *)&ch_num, (const void *)arg, sizeof(ch_num)))
            {
                ret = -EFAULT;
                goto exit_ioctl;
            }

            if( s_ctx->ch_info[ch_num].is_running )
            {
                s_ctx->ch_info[ch_num].is_running = 0;

                _stop_dma(ch_num, s_ctx);
                _disable_device(ch_num, s_ctx);
            }

            ret = 0;
            break;

        case IOCTL_MPEGTS_DO_ALLOC:
        {
            struct ts_buf_init_info buf_info;

            if(copy_from_user((void *)&buf_info, (const void *)arg, sizeof(struct ts_buf_init_info)))
                goto exit_ioctl;

            ch_num  = buf_info.ch_num;

            if( s_ctx->ch_info[ch_num].is_running || s_ctx->ch_info[ch_num].is_malloced )
            {
                ret = -ENOMEM;
                break;
            }

#if (CFG_MPEGTS_IDMA_MODE == 1)
            ret = _init_buf(s_ctx, &buf_info);
            if ( ret == 0 )
                s_ctx->ch_info[ch_num].is_malloced  = 1;
#else

            ret = _init_dma(ch_num, s_ctx);
            if ( ret < 0 )
                goto exit_ioctl;

            ret = _init_buf(s_ctx, &buf_info);
            if ( ret < 0 )
            {
                _deinit_dma(ch_num, s_ctx);
                goto exit_ioctl;
            }

            s_ctx->ch_info[ch_num].is_malloced	= 1;
#endif

            printk("IOCTL : [MP2TS] Memory Alloc Step.2\n");
        }
            break;

        case IOCTL_MPEGTS_DO_DEALLOC:
            if (( s_ctx->ch_info[ch_num].is_running ) || ( !s_ctx->ch_info[ch_num].is_malloced ))
            {
                ret = -EBUSY;
                break;
            }

            {
                if(copy_from_user((void *)&ch_num, (const void *)arg, sizeof(ch_num)))
                    goto exit_ioctl;

                printk("IOCTL : [MP2TS] Memory Dealloc Step.0\n");

                _deinit_dma(ch_num, s_ctx);
                _deinit_buf(ch_num, s_ctx);

                s_ctx->ch_info[ch_num].is_malloced  = 0;
                
                ret = 0;
            }
            break;

        case IOCTL_MPEGTS_READ_BUF:
        {
            struct ts_param_descr param_descr;

            if(copy_from_user((void *)&param_descr, (const void *)arg, sizeof(struct ts_param_descr)))
            {
                ret = -EFAULT;
                goto exit_ioctl;
            }

            ch_num  = (u8)param_descr.info.un.bits.ch_num;

            if (!s_ctx->ch_info[ch_num].is_running)
            {
                printk(KERN_ERR "Error: Invalid operation.(is not running)\n");
                ret = -EPERM;
                goto exit_ioctl;
            }

            if (param_descr.info.un.bits.type != NXP_MP2TS_PARAM_TYPE_BUF)
            {
                ret = -EFAULT;
                goto exit_ioctl;
            }

            if (s_ctx->ch_info[ch_num].tx_mode == 1)
            {
                ret = -EFAULT;
                goto exit_ioctl;
            }

            s_ctx->ch_info[ch_num].wait_time = param_descr.wait_time;

            ret = (int)mpegts_read_buf(ch_num, &param_descr);
            if (ret > 0)
                ret = 0;
        }
            break;

        case IOCTL_MPEGTS_WRITE_BUF:
        {
            struct ts_param_descr param_descr;

            if(copy_from_user((void *)&param_descr, (const void *)arg, sizeof(struct ts_param_descr)))
            {
                ret = -EFAULT;
                goto exit_ioctl;
            }

            ch_num      = (u8)param_descr.info.un.bits.ch_num;

            if (!s_ctx->ch_info[ch_num].is_running)
            {
                printk(KERN_ERR "Error: Invalid operation.(is not running)\n");
                ret = -EPERM;
                goto exit_ioctl;
            }

            if (param_descr.info.un.bits.type != NXP_MP2TS_PARAM_TYPE_BUF)
            {
                ret = -EFAULT;
                goto exit_ioctl;
            }

            if (s_ctx->ch_info[ch_num].tx_mode == 0)
            {
                ret = -EFAULT;
                goto exit_ioctl;
            }

            s_ctx->ch_info[ch_num].wait_time = param_descr.wait_time;

            ret = (int)mpegts_write_buf(ch_num, &param_descr);
            if (ret > 0)
                ret = 0;
        }
            break;

        case IOCTL_MPEGTS_DECRY_TEST:
        {
            struct ts_param_descr param_descr;
            int     rCode;

            if(copy_from_user((void *)&param_descr, (const void *)arg, sizeof(struct ts_param_descr)))
            {
                ret = -ETS_FAULT;
                goto exit_ioctl;
            }

            ch_num      = (u8)param_descr.info.un.bits.ch_num;

            param_descr.ret_value  = ((1<<ETS_WRITEBUF) | (1<<ETS_READBUF));

            if (!s_ctx->ch_info[ch_num].is_running)
            {
                printk(KERN_ERR "Error: Invalid operation.(is not running)\n");
                ret = -ETS_RUNNING;
                goto exit_ioctl;
            }

            if (param_descr.info.un.bits.type != NXP_MP2TS_PARAM_TYPE_BUF)
            {
                ret = -ETS_TYPE;
                goto exit_ioctl;
            }

            param_descr.ret_value = 0;

            rCode = (int)mpegts_write_buf(NXP_IDMA_SRC, &param_descr);
            if( rCode != param_descr.buf_size )
                param_descr.ret_value  = (1<<ETS_WRITEBUF);

            rCode = (int)mpegts_read_buf(NXP_IDMA_DST, &param_descr);
            if( rCode != param_descr.buf_size )
                param_descr.ret_value |= (1<<ETS_READBUF);

            ret = 0;

            if (copy_to_user((void *)arg, (const void *)&param_descr, sizeof(struct ts_param_descr)))
            {
                ret = -ETS_FAULT;
                goto exit_ioctl;
            }
        }
            break;


        case IOCTL_MPEGTS_SET_CONFIG:
//            if( s_ctx->ch_info[ch_num].is_running )
            {
                struct ts_config_descr config_descr;

                if(copy_from_user((void *)&config_descr, (const void *)arg, sizeof(struct ts_config_descr)))
                {
                    ret = -ETS_FAULT;
                    goto exit_ioctl;
                }

                ret = _set_config(&config_descr);
            }
            break;

        case IOCTL_MPEGTS_GET_CONFIG:
//            if( s_ctx->ch_info[ch_num].is_running )
            {
                struct ts_config_descr config_descr;
#if (CFG_MPEGTS_IDMA_MODE == 1)
                u32 temp;
#endif

                if(copy_from_user((void *)&config_descr, (const void *)arg, sizeof(struct ts_config_descr)))
                {
                    ret = -EFAULT;
                    goto exit_ioctl;
                }

#if (CFG_MPEGTS_IDMA_MODE == 1)
                temp = NX_MPEGTSI_GetIDMABaseAddr(config_descr.ch_num);
#endif

                ret = _get_config(&config_descr);
                if(copy_to_user((void *)arg, (const void *)&config_descr, sizeof(struct ts_config_descr)))
                {
                    ret = -EFAULT;
                    goto exit_ioctl;
                }
            }
            break;

        case IOCTL_MPEGTS_SET_PARAM:
            {
                struct ts_param_descr param_descr;

                if(copy_from_user((void *)&param_descr, (const void *)arg, sizeof(struct ts_param_descr)))
                {
                    ret = -EFAULT;
                    goto exit_ioctl;
                }

#if 0
                ch_num = param_descr.info.un.bits.ch_num;
                if( s_ctx->ch_info[ch_num].is_malloced )
                {
                    printk(KERN_ERR "Error: Invalid operation.(Do not malloc)\n");
                    ret = -ETS_RUNNING;
                    goto exit_ioctl;
                }
#endif
                ret = _set_param(&param_descr);
            }
            break;

        case IOCTL_MPEGTS_CLR_PARAM:
            {
                struct ts_param_descr param_descr;

                if(copy_from_user((void *)&param_descr, (const void *)arg, sizeof(struct ts_param_descr)))
                {
                    ret = -EFAULT;
                    goto exit_ioctl;
                }

                ret = _clear_param(&param_descr);
            }
            break;

        case IOCTL_MPEGTS_GET_PARAM:
            {
                struct ts_param_descr param_descr;

                if(copy_from_user((void *)&param_descr, (const void *)arg, sizeof(struct ts_param_descr)))
                {
                    ret = -EFAULT;
                    goto exit_ioctl;
                }

                ret = _get_param(&param_descr);

                if(copy_to_user((void *)&param_descr, (const void *)arg, sizeof(struct ts_param_descr)))
                {
                    ret = -EFAULT;
                    goto exit_ioctl;
                }
            }
            break;

        case IOCTL_MPEGTS_GET_LOCK_STATUS:
            {
#if (CFG_MPEGTS_IDMA_MODE == 1)
                u32 data;
#endif

                if(copy_from_user((void *)&ch_num, (const void *)arg, sizeof(ch_num)))
                    goto exit_ioctl;

printk("IOCTL : [MP2TS] get Lock status Step.0\n");

#if (CFG_MPEGTS_IDMA_MODE == 1)
                data = NX_MPEGTSI_GetIDMAEnable(ch_num);
printk("IOCTL : [MP2TS] 0. GetIDMAEnable       = 0x%08X\n", data);

                data = NX_MPEGTSI_GetIDMAIntEnable();
printk("IOCTL : [MP2TS] 0. GetIDMAIntEnable    = 0x%08X\n", data);

                NX_MPEGTSI_RunIDMA(ch_num);

                data = NX_MPEGTSI_GetIDMAEnable(ch_num);
printk("IOCTL : [MP2TS] 1. GetIDMAEnable       = 0x%08X\n", data);

                data = NX_MPEGTSI_GetIDMAIntEnable();
printk("IOCTL : [MP2TS] 1. GetIDMAIntEnable    = 0x%08X\n", data);
#endif

                ret = 0;
            }
            break;

        default:
            break;
    }

exit_ioctl:
    return ret;
}

static struct file_operations mpegts_fops = {
    .owner          = THIS_MODULE,
    .open           = mpegts_open,
    .release        = mpegts_close,
    .unlocked_ioctl = mpegts_ioctl,
    .read           = mpegts_read,
};

#if 1
static struct miscdevice mpegts_misc_device = {
    .minor      = MISC_DYNAMIC_MINOR,
    .name       = DEV_NAME_MPEGTSI,
    .fops       = &mpegts_fops,
};
#else
static struct dvb_device mpegts_misc_device = {
    .priv        = NULL,
    .users        = 1,
    .writers    = 1,
    .fops        = &mpegts_fops,
//    .kernel_ioctl    = dvb_osd_ioctl,
};
#endif

/* irq handler */
#if (CFG_MPEGTS_IDMA_MODE == 1)
static irqreturn_t mpegts_irq(int irq, void *param)
{
    struct ts_drv_context *ctx = (struct ts_drv_context *)param;
    u32 dma_int_status;
    u32 temp;

    dma_int_status = NX_MPEGTSI_GetIDMAIntRawStatus();

    if ( NX_MPEGTSI_GetTsiIntStatus() || (dma_int_status & (3<<2)) )
    {
        NX_MPEGTSI_SetTsiIntMaskClear(CFALSE);

        if ( dma_int_status & (1<<NXP_IDMA_DST) )
        {
            NX_MPEGTSI_SetIDMAIntClear(NXP_IDMA_DST);

            if( !_w_able(&ctx->ch_info[NXP_IDMA_DST]) || !_rw_able(&ctx->ch_info[NXP_IDMA_DST]) )
            {
                ctx->ch_info[NXP_IDMA_DST].do_continue = 1;
                goto exit_mpegts_irq;
            }

            _w_buf(&ctx->ch_info[NXP_IDMA_DST]);
            temp = (ctx->ch_info[NXP_IDMA_DST].alloc_align * ctx->ch_info[NXP_IDMA_DST].w_pos);
            NX_MPEGTSI_SetIDMABaseAddr(NXP_IDMA_DST, (u32)(ctx->ch_info[NXP_IDMA_DST].dma_phy + temp) );
            NX_MPEGTSI_SetIDMALength(NXP_IDMA_DST, ctx->ch_info[NXP_IDMA_DST].page_size);

            NX_MPEGTSI_RunIDMA(NXP_IDMA_DST);
            wake_up(&ctx->ch_info[NXP_IDMA_DST].wait);
        }
        if ( dma_int_status & (1<<NXP_IDMA_SRC) )
        {
            NX_MPEGTSI_SetIDMAIntClear(NXP_IDMA_SRC);

            if( !_r_able(&ctx->ch_info[NXP_IDMA_SRC]) || !_w_able(&ctx->ch_info[NXP_IDMA_DST]) )
            {
                ctx->ch_info[NXP_IDMA_SRC].do_continue = 1;
                goto exit_mpegts_irq;
            }

            ctx->ch_info[NXP_IDMA_SRC].is_first = 0;

            temp = (ctx->ch_info[NXP_IDMA_SRC].alloc_align * ctx->ch_info[NXP_IDMA_SRC].r_pos);
            _r_buf(&ctx->ch_info[NXP_IDMA_SRC]);

            NX_MPEGTSI_SetIDMABaseAddr(NXP_IDMA_SRC, (u32)(ctx->ch_info[NXP_IDMA_SRC].dma_phy + temp) );
            NX_MPEGTSI_SetIDMALength(NXP_IDMA_SRC, ctx->ch_info[NXP_IDMA_SRC].page_size);

            NX_MPEGTSI_RunIDMA(NXP_IDMA_SRC);
            wake_up(&ctx->ch_info[NXP_IDMA_SRC].wait);
        }

        NX_MPEGTSI_SetTsiIntMaskClear(CTRUE);
    }
    if ( NX_MPEGTSI_GetCapIntStatus(0) || (dma_int_status & (1<<0)) )
    {
        NX_MPEGTSI_SetIDMAIntClear(NXP_IDMA_CH0);

        NX_MPEGTSI_SetCapIntMaskClear(NXP_IDMA_CH0, CFALSE);

//        if ( !_w_able(&ctx->ch_info[NXP_IDMA_CH0]) || !_rw_able(&ctx->ch_info[NXP_IDMA_CH0]) )
        if ( !_rw_able(&ctx->ch_info[NXP_IDMA_CH0]) )
        {
            ctx->ch_info[NXP_IDMA_CH0].do_continue = 1;
            goto exit_mpegts_irq;
        }

        ctx->ch_info[NXP_IDMA_CH0].is_first = 0;

        _w_buf(&ctx->ch_info[NXP_IDMA_CH0]);
        temp = (ctx->ch_info[NXP_IDMA_CH0].alloc_align * ctx->ch_info[NXP_IDMA_CH0].w_pos);

        NX_MPEGTSI_SetIDMABaseAddr(NXP_IDMA_CH0, (u32)(ctx->ch_info[NXP_IDMA_CH0].dma_phy + temp) );
        NX_MPEGTSI_SetIDMALength(NXP_IDMA_CH0, ctx->ch_info[NXP_IDMA_CH0].page_size);

        NX_MPEGTSI_RunIDMA(NXP_IDMA_CH0);
        wake_up(&ctx->ch_info[NXP_IDMA_CH0].wait);
    }
    if ( NX_MPEGTSI_GetCapIntStatus(1) || (dma_int_status & (1<<1))  )
    {
        NX_MPEGTSI_SetIDMAIntClear(NXP_IDMA_CH1);

        NX_MPEGTSI_SetCapIntMaskClear(NXP_IDMA_CH1, CFALSE);

        if ( ctx->ch_info[NXP_IDMA_CH1].tx_mode )
        {
            if ( !_r_able(&ctx->ch_info[NXP_IDMA_CH1]) )
            {
                ctx->ch_info[NXP_IDMA_CH1].do_continue = 1;
                goto exit_mpegts_irq;
            }

            temp = (ctx->ch_info[NXP_IDMA_CH1].alloc_align * ctx->ch_info[NXP_IDMA_CH1].r_pos);
            _r_buf(&ctx->ch_info[NXP_IDMA_CH1]);
        }
        else
        {
//            if ( !_w_able(&ctx->ch_info[NXP_IDMA_CH1]) || !_rw_able(&ctx->ch_info[NXP_IDMA_CH1]) )
            if ( !_rw_able(&ctx->ch_info[NXP_IDMA_CH1]) )
            {
                ctx->ch_info[NXP_IDMA_CH1].do_continue = 1;
                goto exit_mpegts_irq;
            }

            _w_buf(&ctx->ch_info[NXP_IDMA_CH1]);
            temp = (ctx->ch_info[NXP_IDMA_CH1].alloc_align * ctx->ch_info[NXP_IDMA_CH1].w_pos);
        }

        ctx->ch_info[NXP_IDMA_CH1].is_first = 0;

        NX_MPEGTSI_SetIDMABaseAddr(NXP_IDMA_CH1, (ctx->ch_info[NXP_IDMA_CH1].dma_phy + temp) );
        NX_MPEGTSI_SetIDMALength(NXP_IDMA_CH1, ctx->ch_info[NXP_IDMA_CH1].page_size);

        NX_MPEGTSI_RunIDMA(NXP_IDMA_CH1);
        wake_up(&ctx->ch_info[NXP_IDMA_CH1].wait);
    }

exit_mpegts_irq:
    return IRQ_HANDLED;
}
#else	// #if (CFG_MPEGTS_IDMA_MODE == 1)

static void mpegts_capture0_dma_irq(void *arg)
{
	struct ts_drv_context *ctx = arg;

	_w_buf(&ctx->ch_info[NXP_IDMA_CH0]);
	if (ctx->ch_info[NXP_IDMA_CH0].wait_time)
		wake_up(&ctx->ch_info[NXP_IDMA_CH0].wait);
}

static void mpegts_capture1_dma_irq(void *arg)
{
	struct ts_drv_context *ctx = arg;

	if (ctx->ch_info[NXP_IDMA_CH1].tx_mode)
		_r_buf(&ctx->ch_info[NXP_IDMA_CH1]);
	else
		_w_buf(&ctx->ch_info[NXP_IDMA_CH1]);

	if (ctx->ch_info[NXP_IDMA_CH1].wait_time)
		wake_up(&ctx->ch_info[NXP_IDMA_CH1].wait);
}

static void mpegts_core_input_dma_irq(void *arg)
{
	struct ts_drv_context *ctx = arg;

	_r_buf(&ctx->ch_info[NXP_IDMA_SRC]);
	if (ctx->ch_info[NXP_IDMA_SRC].wait_time)
		wake_up(&ctx->ch_info[NXP_IDMA_SRC].wait);
}

static void mpegts_core_output_dma_irq(void *arg)
{
	struct ts_drv_context *ctx = arg;

	_w_buf(&ctx->ch_info[NXP_IDMA_DST]);
	if (ctx->ch_info[NXP_IDMA_DST].wait_time)
		wake_up(&ctx->ch_info[NXP_IDMA_DST].wait);
}

static int _prepare_dma_submit(u8 ch_num, struct ts_drv_context *ctx)
{
	struct dma_chan *dma_chan;
	struct dma_slave_config slave_config = { 0, };
	size_t	total_buf_len;
	int ret;

	NX_MPEGTSI_SetIDMAEnable(ch_num, CFALSE);
	NX_MPEGTSI_RunIDMA(ch_num);

	dma_chan = ctx->ch_info[ch_num].dma_chan;

    if (ctx->ch_info[ch_num].tx_mode == 1)
	{
		slave_config.direction 		= DMA_MEM_TO_DEV;
		slave_config.dst_addr 		= (PHY_BASEADDR_MPEGTSI + 0x10) + (ch_num * 0x04);
		slave_config.dst_addr_width = DMA_SLAVE_BUSWIDTH_4_BYTES;
		slave_config.dst_maxburst 	= 8;	/* peri burst dword unit */
		slave_config.src_addr_width = DMA_SLAVE_BUSWIDTH_4_BYTES;
		slave_config.src_maxburst 	= 8;	/* memory burst */
		slave_config.device_fc 		= false;
	}
    else
	{
		slave_config.direction 		= DMA_DEV_TO_MEM;
		slave_config.src_addr 		= (PHY_BASEADDR_MPEGTSI + 0x10) + (ch_num * 0x04);
		slave_config.src_addr_width = DMA_SLAVE_BUSWIDTH_4_BYTES;
		slave_config.src_maxburst 	= 8;	/* peri burst dword unit */
		slave_config.dst_addr_width = DMA_SLAVE_BUSWIDTH_4_BYTES;
		slave_config.dst_maxburst 	= 8;	/* memory burst */
		slave_config.device_fc 		= false;
	}

	ret = dmaengine_slave_config(dma_chan, &slave_config);
	if (ret < 0)
		return -EINVAL;

	total_buf_len = ctx->ch_info[ch_num].page_num * ctx->ch_info[ch_num].page_size;

	ctx->ch_info[ch_num].desc = dmaengine_prep_dma_cyclic(dma_chan,
										ctx->ch_info[ch_num].dma_phy,
										total_buf_len,
										ctx->ch_info[ch_num].page_size,
										slave_config.direction);

	if (!ctx->ch_info[ch_num].desc) {
		MP2TS_DBG("%s: cannot prepare slave %s dma\n",
			__func__, (char *)(ctx->ch_info[ch_num].filter_data) );
		goto error_prepare;
	}

	switch(ch_num)
	{
	case NXP_MP2TS_ID_CAP0:
		ctx->ch_info[ch_num].desc->callback	= mpegts_capture0_dma_irq;
		break;
	case NXP_MP2TS_ID_CAP1:
		ctx->ch_info[ch_num].desc->callback	= mpegts_capture1_dma_irq;
		break;
	case NXP_MP2TS_ID_CORE:
		ctx->ch_info[ch_num].desc->callback	= mpegts_core_input_dma_irq;
		break;
	case NXP_MP2TS_ID_MAX:
		ctx->ch_info[ch_num].desc->callback	= mpegts_core_output_dma_irq;
		break;
	}
	ctx->ch_info[ch_num].desc->callback_param = ctx;
	dmaengine_submit(ctx->ch_info[ch_num].desc);

	return 0;

error_prepare:

    if (ch_num == NXP_MP2TS_ID_MAX)
    {
		dma_release_channel(ctx->ch_info[ch_num-1].dma_chan);
    }

	return -EINVAL;

}
#endif	// #if (CFG_MPEGTS_IDMA_MODE == 1)


/* register/remove driver */
static int nexell_mpegts_probe(struct platform_device *pdev)
{
    int i, dma_alloc_count;
    int ret;
    struct nxp_mp2ts_plat_data *mp2ts_plat = pdev->dev.platform_data;
    struct ts_buf_init_info     buf_info;

    one_sec_ticks = msecs_to_jiffies( 1000 );

    s_ctx = (struct ts_drv_context *)kzalloc(sizeof(struct ts_drv_context), GFP_KERNEL);
    if (!s_ctx) {
        return -ENOMEM;
    }

#if (MULTI_TS_DRV == 1)
    platform_set_drvdata(pdev, s_ctx);

    if ((mp2ts_plat->cap_ch_num != -1) && (mp2ts_plat->cap_ch_num < 2)) {
        snprintf(s_ctx->drv_name, sizeof(DEV_NAME_MPEGTSI),
             "mp2ts-cap%d", mp2ts_plat->cap_ch_num);
    } else {
        snprintf(s_ctx->drv_name, sizeof(DEV_NAME_MPEGTSI),
             "mp2ts-cap");
    }

    s_ctx->mp2ts_miscdev.name   = s_ctx->drv_name;
    s_ctx->mp2ts_miscdev.fops   = &mpegts_fops;
    s_ctx->mp2ts_miscdev.minor  = MISC_DYNAMIC_MINOR;
    s_ctx->mp2ts_miscdev.parent = &pdev->dev;

    s_ctx->cap_ch_num = mp2ts_plat->cap_ch_num;
#endif

    s_ctx->dev = &pdev->dev;

    s_ctx->is_opened = 0;
    for (i = 0; i < NXP_MP2TS_ID_MAX; i++)
    {
        s_ctx->ch_info[i].alloc_align   = 0;
        s_ctx->ch_info[i].alloc_size    = 0;
        s_ctx->ch_info[i].page_size     = 0;
        s_ctx->ch_info[i].page_num      = 0;
        s_ctx->ch_info[i].dma_virt      = 0;
        s_ctx->ch_info[i].dma_phy       = 0;

        s_ctx->ch_info[i].is_running    = 0;
        s_ctx->ch_info[i].is_malloced   = 0;
        s_ctx->ch_info[i].is_first      = 1;
        s_ctx->ch_info[i].do_continue   = 0;
    }

    dma_alloc_count = 0;
    for (i = 0; i < 3; i++)
    {
        if (mp2ts_plat->ts_dma_size[i] < 0)
            continue;

        buf_info.ch_num     = i;
        buf_info.page_size  = TS_PAGE_SIZE;
        buf_info.page_num   = mp2ts_plat->ts_dma_size[i] / TS_PAGE_SIZE;

        ret = _prepare_dma(i, s_ctx, &buf_info);
        if (ret == 0) {
            dma_alloc_count++;
        }
    }

    if (dma_alloc_count == 0) {
        ret = -EINVAL;
        goto fail_init_context;
    }
    ret = _init_context(s_ctx, mp2ts_plat);
    if (ret < 0) {
        goto fail_init_context;
    }

    ret = _init_device(s_ctx);
    if (ret < 0) {
        goto fail_init_device;
    }
#if (CFG_MPEGTS_IDMA_MODE == 1)
    s_ctx->irq_dma = IRQ_PHY_MPEGTSI;
    ret = request_irq(s_ctx->irq_dma, mpegts_irq,
                    IRQF_DISABLED, DEV_NAME_MPEGTSI, s_ctx);
    if (ret) {
		MP2TS_DBG(&s_ctx->dev, "Failed to request irq, err: %d\n", ret);
        goto fail_irq;
    }
#endif

#if (MULTI_TS_DRV == 1)
    ret = misc_register(&s_ctx->mp2ts_miscdev);
#else
    ret = misc_register(&mpegts_misc_device);
#endif
    if (ret) {
        goto fail_misc_register;
    }

    return 0;

fail_misc_register:
    free_irq(s_ctx->irq_dma, s_ctx);

#if (CFG_MPEGTS_IDMA_MODE == 1)
fail_irq:
    _deinit_device();
#endif

fail_init_device:
    _deinit_context(s_ctx);

fail_init_context:
    kfree(s_ctx);
    s_ctx = NULL;

    return ret;
}

static int nexell_mpegts_remove(struct platform_device *pdev)
{
#if (CFG_MPEGTS_IDMA_MODE == 1)
	if (s_ctx->irq_dma)
	    free_irq(s_ctx->irq_dma, s_ctx);
#endif

    _deinit_device();
    _deinit_context(s_ctx);
    kfree(s_ctx);
    s_ctx = NULL;

    return 0;
}

static struct platform_driver nexell_mpegts_driver = {
    .probe      = nexell_mpegts_probe,
    .remove     = nexell_mpegts_remove,
    .driver     = {
        .owner  = THIS_MODULE,
        .name   = DEV_NAME_MPEGTSI,
    },
};

static int __init nexell_mpegts_init(void)
{
    return platform_driver_register(&nexell_mpegts_driver);
}

static void __exit nexell_mpegts_exit(void)
{
    MP2TS_DBG("%s\n", __func__);
    return platform_driver_unregister(&nexell_mpegts_driver);
}

module_init(nexell_mpegts_init);
module_exit(nexell_mpegts_exit);

MODULE_DESCRIPTION("Nexell MPEGTS Interface driver");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nexell swpark@nexell.co.kr");
MODULE_ALIAS("platform:nexell-mpegts");
