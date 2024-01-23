/* drv_edp.h
 *
 * Copyright (c) 2007-2022 Allwinnertech Co., Ltd.
 * Author: huangyongxing <huangyongxing@allwinnertech.com>
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/
#ifndef __DRV_EDP_H__
#define __DRV_EDP_H__


#include <linux/module.h>
#include <linux/uaccess.h>
#include <asm/memory.h>
#include <asm/unistd.h>
#include <asm-generic/int-ll64.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/semaphore.h>
#include <linux/vmalloc.h>
#include <linux/fs.h>
#include <linux/dma-mapping.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/err.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/clk.h>
#include <linux/cdev.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/of_iommu.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>
#include <linux/of_gpio.h>
#include <linux/compat.h>
#include <video/sunxi_display2.h>
#include <video/sunxi_edp.h>
#include <linux/regulator/consumer.h>
#include <linux/pinctrl/consumer.h>
#include <linux/reset.h>
#include "edp_core/edp_core.h"
#include "edp_core/edp_edid.h"
#include "edp_configs.h"
#include "../include/disp_edid.h"

#if defined(CONFIG_EXTCON)
#include <linux/extcon.h>
#endif

#define EDP_NUM_MAX 2
#define EDP_POWER_STR_LEN 32


/**
 * save some info here for every edp module
 */
struct edp_info_t {
	u32 enable;
	uintptr_t base_addr;
	struct device *dev;
	struct clk *clk_bus;
	struct clk *clk;
	struct clk *clk_parent;
	struct regulator *regulator;
	struct reset_control *rst_bus;
	struct pinctrl *rst_pin;
	bool suspend;
	bool active;
	bool training_done;
	bool support_fixed_timings;
	bool support_edid_timings;
	bool use_def_timings;
	bool use_user_timings;
	bool use_edid_timings;

	bool use_recom_para;
	bool use_def_para;

	struct mutex mlock;
	struct edp_tx_core edp_core;
	struct edp_rx_cap sink_cap;
};


extern s32 disp_set_edp_func(struct disp_tv_func *func);
extern unsigned int disp_boot_para_parse(const char *name);

#endif /*End of file*/
