 /*
 * Copyright (c) 2007-2022 Allwinnertech Co., Ltd.
 * Author: huangyongxing <huangyongxing@allwinnertech.com>
 *
 * core function of edp driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include "edp_core.h"
#include "../edp_configs.h"
#include "../lowlevel/edp_lowlevel.h"


/*edp_hal_xxx means xxx function is from lowlevel*/

s32 edp_core_phy_init(u32 sel, struct edp_tx_core *edp_core)
{
	return edp_hal_phy_init(sel, edp_core);
}

void edp_core_set_reg_base(u32 sel, uintptr_t base)
{
	edp_hal_set_reg_base(sel, base);
}

bool edp_core_get_hpd_status(u32 sel)
{
	return edp_hal_get_hpd_status(sel);
}

s32 edp_core_read_dpcd(u32 sel, char *dpcd_rx_buf)
{
	return edp_hal_read_dpcd(sel, dpcd_rx_buf);
}

void edp_core_set_video_mode(u32 sel, struct edp_tx_core *edp_core)
{
	edp_hal_set_video_mode(sel, edp_core);
}

s32 edp_core_enable(u32 sel, struct edp_tx_core *edp_core)
{
	return edp_hal_enable(sel, edp_core);
}

s32 edp_core_disable(u32 sel, struct edp_tx_core *edp_core)
{
	return edp_hal_disable(sel, edp_core);
}

s32 edp_core_low_power_mode_enable(u32 sel, bool en)
{
	return edp_hal_low_power_mode_enable(sel, en);
}

s32 edp_core_link_training1(u32 sel, struct edp_tx_core *edp_core, bool bypass_1st_train)
{
	return edp_hal_link_training1(sel, edp_core, bypass_1st_train);
}

s32 edp_core_link_training2(u32 sel, struct edp_tx_core *edp_core, bool bypass_1st_train)
{
	return edp_hal_link_training2(sel, edp_core, bypass_1st_train);
}

s32 edp_core_link_training3(u32 sel, struct edp_tx_core *edp_core, bool bypass_1st_train)
{
	return edp_hal_link_training3(sel, edp_core, bypass_1st_train);
}

s32 edp_core_link_start(u32 sel)
{
	return edp_hal_link_start(sel);
}

s32 edp_core_get_training_info(u32 sel, struct edp_tx_core *edp_core)
{
	struct edp_lane_para *recom_lane_para;

	recom_lane_para = &edp_core->recom_lane_para;

//	memset(recom_lane_para, 0, sizeof(edp_lane_para))

	return edp_hal_get_training_info(sel, recom_lane_para);
}

s32 edp_core_irq_enable(u32 sel, u32 irq_id, bool en)
{
	if (en)
		return edp_hal_irq_enable(sel, irq_id);
	else
		return edp_hal_irq_disable(sel, irq_id);
}

s32 edp_core_irq_query(u32 sel)
{
	return edp_hal_irq_query(sel);
}

s32 edp_core_irq_clear(u32 sel)
{
	return edp_hal_irq_clear(sel);
}

s32 edp_core_get_cur_line(u32 sel)
{
	return edp_hal_get_cur_line(sel);
}

s32 edp_core_get_start_dly(u32 sel)
{
	return edp_hal_get_start_dly(sel);
}

void edp_core_show_builtin_patten(u32 sel, u32 pattern)
{
	edp_hal_show_builtin_patten(sel, pattern);
}

s32 edp_core_aux_read(u32 sel, s32 addr, s32 lenth, char *buf)
{
	return edp_hal_aux_read(sel, addr, lenth, buf);
}

s32 edp_core_aux_write(u32 sel, s32 addr, s32 lenth, char *buf)
{
	return edp_hal_aux_write(sel, addr, lenth, buf);
}

s32 edp_core_audio_set_para(u32 sel, edp_audio_t *para)
{
	return edp_hal_audio_set_para(sel, para);
}

s32 edp_core_audio_enable(u32 sel)
{
	return edp_hal_audio_enable(sel);
}

s32 edp_core_audio_disable(u32 sel)
{
	return edp_hal_audio_disable(sel);
}

s32 edp_core_ssc_enable(u32 sel, bool enable)
{
	return edp_hal_ssc_enable(sel, enable);
}

bool edp_core_ssc_is_enabled(u32 sel)
{
	return edp_hal_ssc_is_enabled(sel);
}

s32 edp_core_ssc_get_mode(u32 sel)
{
	return edp_hal_ssc_get_mode(sel);
}

s32 edp_core_ssc_set_mode(u32 sel, u32 mode)
{
	return edp_hal_ssc_set_mode(sel, mode);
}

s32 edp_core_psr_enable(u32 sel, bool enable)
{
	return edp_hal_psr_enable(sel, enable);
}

bool edp_core_psr_is_enabled(u32 sel)
{
	return edp_hal_psr_is_enabled(sel);
}

s32 edp_core_get_color_fmt(u32 sel)
{
	return edp_hal_get_color_fmt(sel);
}

s32 edp_core_get_pixclk(u32 sel)
{
	return edp_hal_get_pixclk(sel);
}

s32 edp_core_get_train_pattern(u32 sel)
{
	return edp_hal_get_train_pattern(sel);
}

s32 edp_core_get_lane_para(u32 sel, struct edp_lane_para *tmp_lane_para)
{
	return edp_hal_get_lane_para(sel, tmp_lane_para);
}

s32 edp_core_get_tu_size(u32 sel)
{
	return edp_hal_get_tu_size(sel);
}

s32 edp_core_get_symbol_rate(u32 sel)
{
	return edp_core_get_symbol_rate(sel);
}

bool edp_core_audio_is_enabled(u32 sel)
{
	return edp_hal_audio_is_enabled(sel);
}

s32 edp_core_get_audio_if(u32 sel)
{
	return edp_hal_get_audio_if(sel);
}

s32 edp_core_audio_is_mute(u32 sel)
{
	return edp_hal_audio_is_mute(sel);
}

s32 edp_core_get_audio_chn_cnt(u32 sel)
{
	return edp_hal_get_audio_chn_cnt(sel);
}

s32 edp_core_get_audio_date_width(u32 sel)
{
	return edp_core_get_audio_date_width(sel);
}
