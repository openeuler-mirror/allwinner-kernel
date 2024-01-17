/* edp_core.h
 *
 * Copyright (c) 2007-2022 Allwinnertech Co., Ltd.
 * Author: huangyongxing <huangyongxing@allwinnertech.com>
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/
#ifndef __EDP_CORE_H__
#define __EDP_CORE_H__

#include <linux/types.h>
#include <video/sunxi_display2.h>
#include <video/sunxi_edp.h>
#include "../../include/disp_edid.h"

#define RET_OK (0)
#define RET_FAIL (-1)

#define __DEBUG_ON
#ifdef __DEBUG_ON
#define EDP_PRINT(fmt, ...)			printk("%s-<%d>:" fmt, __func__, __LINE__, ##__VA_ARGS__);
#define EDP_DBG(fmt, ...)			pr_debug("[EDP]: "fmt, ##__VA_ARGS__)
#define EDP_ERR(fmt, ...)			pr_err("[EDP]: "fmt, ##__VA_ARGS__)
#define EDP_WRN(fmt, ...)			pr_warn("[EDP]: "fmt, ##__VA_ARGS__)
#define EDP_INFO(fmt, ...)			pr_info("[EDP]: "fmt, ##__VA_ARGS__)
#define EDP_DEV_ERR(dev, fmt, ...)	dev_err(dev, "[EDP]: "fmt, ##__VA_ARGS__)
#else
#define EDP_PRINT(fmt, ...)
#define EDP_DBG(fmt, ...)
#define EDP_ERR(fmt, ...)
#define EDP_WRN(fmt, ...)
#define EDP_INFO(fmt, ...)
#define EDP_DEV_ERR(dev, fmt, ...)
#endif

struct edp_lane_para {
	u64 bit_rate;
	u32 lane_cnt;
	u32 fps;
	u32 training_func;
	u32 sramble_seed;
	u32 lane0_sw;
	u32 lane1_sw;
	u32 lane2_sw;
	u32 lane3_sw;
	u32 lane0_pre;
	u32 lane1_pre;
	u32 lane2_pre;
	u32 lane3_pre;
	u32 support_hbr2;
	u32 colordepth;
};

struct edp_rx_cap {
	/*parse from dpcd*/
	u32 dpcd_rev;
	u64 max_rate;
	u32 max_lane;
	bool tps3_support;
	bool fast_train_support;
	bool downstream_port_support;
	u32 downstream_port_type;
	u32 downstream_port_cnt;
	bool local_edid_support;
	bool is_edp_device;
	bool assr_support;
	bool enhance_frame_support;

	/*parse from edid*/
	u32 mfg_weak;
	u32 mfg_year;
	u32 edid_ver;
	u32 edid_rev;
	u32 input_type;
	u32 bit_depth;
	u32 video_interface;
	u32 width_cm;
	u32 height_cm;

	/*parse from edid_ext*/
	bool Ycc444_support;
	bool Ycc422_support;
	bool Ycc420_support;
	bool audio_support;
};

struct edp_tx_core {
	s32 mode;
	u32 audio_en;
	u32 ssc_en;
	s32 ssc_mode;
	u32 psr_en;
	struct edp_lane_para def_lane_para;
	struct edp_lane_para recom_lane_para;
	struct edp_lane_para lane_para;

	struct disp_video_timings def_timings;
	struct disp_video_timings edid_timings;
	struct disp_video_timings timings;

	struct edid *edid;

	struct pinctrl *rst_pin;
};

bool edp_core_get_hpd_status(u32 sel);
bool edp_core_ssc_is_enabled(u32 sel);
bool edp_core_psr_is_enabled(u32 sel);
bool edp_core_audio_is_enabled(u32 sel);

void edp_core_set_video_mode(u32 sel, struct edp_tx_core *edp_core);
void edp_core_set_reg_base(u32 sel, uintptr_t base);
void edp_core_show_builtin_patten(u32 sel, u32 pattern);

s32 edp_core_link_start(u32 sel);
s32 edp_core_phy_init(u32 sel, struct edp_tx_core *edp_core);
s32 edp_core_read_dpcd(u32 sel, char *dpcd_rx_buf);
s32 edp_core_enable(u32 sel, struct edp_tx_core *edp_core);
s32 edp_core_disable(u32 sel, struct edp_tx_core *edp_core);
s32 edp_core_low_power_mode_enable(u32 sel, bool en);
s32 edp_core_link_training1(u32 sel, struct edp_tx_core *edp_core, bool bypass_1st_train);
s32 edp_core_link_training2(u32 sel, struct edp_tx_core *edp_core, bool bypass_1st_train);
s32 edp_core_link_training3(u32 sel, struct edp_tx_core *edp_core, bool bypass_1st_train);
s32 edp_core_get_training_info(u32 sel, struct edp_tx_core *edp_core);
s32 edp_core_irq_enable(u32 sel, u32 irq_id, bool en);
s32 edp_core_irq_query(u32 sel);
s32 edp_core_irq_clear(u32 sel);
s32 edp_core_get_cur_line(u32 sel);
s32 edp_core_get_start_dly(u32 sel);
s32 edp_core_aux_read(u32 sel, s32 addr, s32 len, char *buf);
s32 edp_core_aux_write(u32 sel, s32 addr, s32 len, char *buf);
s32 edp_core_audio_set_para(u32 sel, edp_audio_t *para);
s32 edp_core_audio_enable(u32 sel);
s32 edp_core_audio_disable(u32 sel);
s32 edp_core_ssc_enable(u32 sel, bool enable);
s32 edp_core_ssc_set_mode(u32 sel, u32 mode);
s32 edp_core_ssc_get_mode(u32 sel);
s32 edp_core_psr_enable(u32 sel, bool enable);
s32 edp_core_get_color_fmt(u32 sel);
s32 edp_core_get_pixclk(u32 sel);
s32 edp_core_get_train_pattern(u32 sel);
s32 edp_core_get_lane_para(u32 sel, struct edp_lane_para *tmp_lane_para);
s32 edp_core_get_tu_size(u32 sel);
s32 edp_core_get_symbol_rate(u32 sel);
s32 edp_core_get_audio_if(u32 sel);
s32 edp_core_audio_is_mute(u32 sel);
s32 edp_core_get_audio_chn_cnt(u32 sel);
s32 edp_core_get_audio_date_width(u32 sel);

#endif /*End of file*/
