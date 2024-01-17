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

#ifndef __EDP_LOWLEVEL_H__
#define __EDP_LOWLEVEL_H__

#include <linux/types.h>
#include <video/sunxi_edp.h>
#include "../edp_core/edp_core.h"
#include "../../include/disp_edid.h"

#define SETMASK(width, shift)   ((width?((-1U) >> (32-width)):0)  << (shift))
#define CLRMASK(width, shift)   (~(SETMASK(width, shift)))
#define GET_BITS(shift, width, reg)     \
	(((reg) & SETMASK(width, shift)) >> (shift))
#define SET_BITS(shift, width, reg, val) \
	(((reg) & CLRMASK(width, shift)) | (val << (shift)))

#define NATIVE_READ   0b1001
#define NATIVE_WRITE  0b1000
#define AUX_I2C_READ  0b0001
#define AUX_I2C_WRITE 0b0000

#define AUX_REPLY_NACK 0b001
#define AUX_REPLY_ACK  0b000

#define EDP_EDID_ADDR 0x50

bool edp_hal_get_hpd_status(u32 sel);
bool edp_hal_ssc_is_enabled(u32 sel);
bool edp_hal_psr_is_enabled(u32 sel);
bool edp_hal_audio_is_enabled(u32 sel);

void edp_hal_set_reg_base(u32 sel, uintptr_t base);
void edp_hal_lane_config(u32 sel, struct edp_tx_core *edp_core);
void edp_hal_set_video_mode(u32 sel, struct edp_tx_core *edp_core);
void edp_hal_set_misc(u32 sel, s32 misc0_val, s32 misc1_val);
void edp_hal_clean_hpd_interrupt_status(u32 sel);
void edp_hal_show_builtin_patten(u32 sel, u32 pattern);

s32 edp_hal_phy_init(u32 sel, struct edp_tx_core *edp_core);
s32 edp_hal_enable(u32 sel, struct edp_tx_core *edp_core);
s32 edp_hal_disable(u32 sel, struct edp_tx_core *edp_core);
s32 edp_hal_link_training1(u32 sel, struct edp_tx_core *edp_core, bool bypass_1st_train);
s32 edp_hal_link_training2(u32 sel, struct edp_tx_core *edp_core, bool bypass_1st_train);
s32 edp_hal_link_training3(u32 sel, struct edp_tx_core *edp_core, bool bypass_1st_train);
s32 edp_hal_get_training_info(u32 sel, struct edp_lane_para *para);
s32 edp_hal_read_edid(u32 sel, struct edid *edid);
s32 edp_hal_read_edid_ext(u32 sel, u32 block_cnt, struct edid *edid);
s32 edp_hal_low_power_mode_enable(u32 sel, bool en);
s32 edp_hal_read_dpcd(u32 sel, char *dpcd_rx_buf);
s32 edp_hal_sink_init(u32 sel, u32 bit_rate, u32 lane_cnt);
s32 edp_hal_irq_enable(u32 sel, u32 irq_id);
s32 edp_hal_irq_disable(u32 sel, u32 irq_id);
s32 edp_hal_irq_query(u32 sel);
s32 edp_hal_irq_clear(u32 sel);
s32 edp_hal_get_cur_line(u32 sel);
s32 edp_hal_get_start_dly(u32 sel);
s32 edp_hal_aux_read(u32 sel, s32 addr, s32 len, char *buf);
s32 edp_hal_aux_write(u32 sel, s32 addr, s32 len, char *buf);
s32 edp_hal_aux_i2c_read(u32 sel, s32 addr, s32 len, char *buf);
s32 edp_hal_aux_i2c_write(u32 sel, s32 addr, s32 len, char *buf);
s32 edp_hal_link_start(u32 sel);
s32 edp_hal_audio_set_para(u32 sel, edp_audio_t *para);
s32 edp_hal_audio_enable(u32 sel);
s32 edp_hal_audio_disable(u32 sel);
s32 edp_hal_ssc_enable(u32 sel, bool enable);
s32 edp_hal_ssc_set_mode(u32 sel, u32 mode);
s32 edp_hal_ssc_get_mode(u32 sel);
s32 edp_hal_psr_enable(u32 sel, bool enable);
s32 edp_hal_get_color_fmt(u32 sel);
s32 edp_hal_get_pixclk(u32 sel);
s32 edp_hal_get_train_pattern(u32 sel);
s32 edp_hal_get_lane_para(u32 sel, struct edp_lane_para *tmp_lane_para);
s32 edp_hal_get_tu_size(u32 sel);
s32 edp_hal_get_symbol_rate(u32 sel);
s32 edp_hal_get_audio_if(u32 sel);
s32 edp_hal_audio_is_mute(u32 sel);
s32 edp_hal_get_audio_chn_cnt(u32 sel);
s32 edp_hal_get_audio_date_width(u32 sel);

#endif
