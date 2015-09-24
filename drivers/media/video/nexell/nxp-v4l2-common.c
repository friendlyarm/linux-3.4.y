#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/i2c.h>

#include <mach/nxp-v4l2-platformdata.h>

#include "nxp-v4l2-common.h"

struct i2c_client *
nxp_v4l2_get_i2c_client(struct nxp_v4l2_i2c_board_info *board_info)
{
    struct i2c_adapter *adapter;
    struct i2c_client *client = NULL;
    struct i2c_board_info *info = board_info->board_info;

    if (!info)
        return NULL;

    adapter = i2c_get_adapter(board_info->i2c_adapter_id);
    if (!adapter) {
        pr_err("%s: unable to get i2c adapter %d for device %s\n",
                __func__,
                board_info->i2c_adapter_id,
                board_info->board_info->type);
        return NULL;
    }    

    request_module(I2C_MODULE_PREFIX "%s", info->type);

    client = i2c_new_device(adapter, info);
    if (client == NULL || client->driver == NULL) {
        pr_err("%s: failed to i2c_new_device()\n", __func__);
        goto error_i2c_new;
    }    

    return client;

error_i2c_new:
    if (client)
        i2c_unregister_device(client);
    return NULL;
}
