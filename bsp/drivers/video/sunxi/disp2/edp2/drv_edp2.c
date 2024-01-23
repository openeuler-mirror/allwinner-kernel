/* drv_edp.c
 *
 * Copyright (c) 2007-2022 Allwinnertech Co., Ltd.
 * Author: huangyongxing <huangyongxing@allwinnertech.com>
 *
 * edp driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include "drv_edp2.h"
#include "../disp/disp_sys_intf.h"
#if defined(CONFIG_EXTCON)
#include <linux/extcon.h>
#include <linux/extcon-provider.h>
#include "../drivers/extcon/extcon.h"
#endif


static u32 g_edp_num;
struct edp_info_t g_edp_info[EDP_NUM_MAX];
static struct task_struct *edp_task[EDP_NUM_MAX];
static u32 hpd_state[EDP_NUM_MAX];
static char dpcd_rx_buf[256];

static dev_t devid[EDP_NUM_MAX];
static struct cdev *edp_cdev[EDP_NUM_MAX];

static struct class *edp_class[EDP_NUM_MAX];
static struct device *edp_dev[EDP_NUM_MAX];



static struct disp_video_timings video_timing[] = {
	{
		.vic = 0,
		.tv_mode = DISP_TV_MOD_480I,
		.pixel_clk = 216000000,
		.pixel_repeat = 0,
		.x_res = 720,
		.y_res = 480,
		.hor_total_time = 858,
		.hor_back_porch = 57,
		.hor_front_porch = 62,
		.hor_sync_time = 19,
		.ver_total_time = 525,
		.ver_back_porch = 4,
		.ver_front_porch = 1,
		.ver_sync_time = 3,
		.hor_sync_polarity = 0,/* 0: negative, 1: positive */
		.ver_sync_polarity = 0,/* 0: negative, 1: positive */
		.b_interlace = 1,
		.vactive_space = 0,
		.trd_mode = 0,
	},
	{
		.vic = 0,
		.tv_mode = DISP_TV_MOD_576I,
		.pixel_clk = 216000000,
		.pixel_repeat = 0,
		.x_res = 720,
		.y_res = 576,
		.hor_total_time = 864,
		.hor_back_porch = 69,
		.hor_front_porch = 63,
		.hor_sync_time = 12,
		.ver_total_time = 625,
		.ver_back_porch = 2,
		.ver_front_porch = 44,
		.ver_sync_time = 3,
		.hor_sync_polarity = 0,/* 0: negative, 1: positive */
		.ver_sync_polarity = 0,/* 0: negative, 1: positive */
		.b_interlace = 1,
		.vactive_space = 0,
		.trd_mode = 0,
	},
	{
		.vic = 0,
		.tv_mode = DISP_TV_MOD_480P,
		.pixel_clk = 54000000,
		.pixel_repeat = 0,
		.x_res = 720,
		.y_res = 480,
		.hor_total_time = 858,
		.hor_back_porch = 60,
		.hor_front_porch = 62,
		.hor_sync_time = 16,
		.ver_total_time = 525,
		.ver_back_porch = 9,
		.ver_front_porch = 30,
		.ver_sync_time = 6,
		.hor_sync_polarity = 1,/* 0: negative, 1: positive */
		.ver_sync_polarity = 1,/* 0: negative, 1: positive */
		.b_interlace = 0,
		.vactive_space = 0,
		.trd_mode = 0,
	},
	{
		.vic = 0,
		.tv_mode = DISP_TV_MOD_576P,
		.pixel_clk = 54000000,
		.pixel_repeat = 0,
		.x_res = 720,
		.y_res = 576,
		.hor_total_time = 864,
		.hor_back_porch = 68,
		.hor_front_porch = 64,
		.hor_sync_time = 12,
		.ver_total_time = 625,
		.ver_back_porch = 5,
		.ver_front_porch = 39,
		.ver_sync_time = 5,
		.hor_sync_polarity = 1,/* 0: negative, 1: positive */
		.ver_sync_polarity = 1,/* 0: negative, 1: positive */
		.b_interlace = 0,
		.vactive_space = 0,
		.trd_mode = 0,
	},
	{
		.vic = 0,
		.tv_mode = DISP_TV_MOD_720P_60HZ,
		.pixel_clk = 74250000,
		.pixel_repeat = 0,
		.x_res = 1280,
		.y_res = 720,
		.hor_total_time = 1650,
		.hor_back_porch = 220,
		.hor_front_porch = 40,
		.hor_sync_time = 110,
		.ver_total_time = 750,
		.ver_back_porch = 5,
		.ver_front_porch = 20,
		.ver_sync_time = 5,
		.hor_sync_polarity = 1,/* 0: negative, 1: positive */
		.ver_sync_polarity = 1,/* 0: negative, 1: positive */
		.b_interlace = 0,
		.vactive_space = 0,
		.trd_mode = 0,
	},
	{
		.vic = 0,
		.tv_mode = DISP_TV_MOD_720P_50HZ,
		.pixel_clk = 74250000,
		.pixel_repeat = 0,
		.x_res = 1280,
		.y_res = 720,
		.hor_total_time = 1980,
		.hor_back_porch = 220,
		.hor_front_porch = 40,
		.hor_sync_time = 440,
		.ver_total_time = 750,
		.ver_back_porch = 5,
		.ver_front_porch = 20,
		.ver_sync_time = 5,
		.hor_sync_polarity = 1,/* 0: negative, 1: positive */
		.ver_sync_polarity = 1,/* 0: negative, 1: positive */
		.b_interlace = 0,
		.vactive_space = 0,
		.trd_mode = 0,
	},
	{
		.vic = 0,
		.tv_mode = DISP_TV_MOD_1080I_60HZ,
		.pixel_clk = 74250000,
		.pixel_repeat = 0,
		.x_res = 1920,
		.y_res = 1080,
		.hor_total_time = 2200,
		.hor_back_porch = 148,
		.hor_front_porch = 44,
		.hor_sync_time = 88,
		.ver_total_time = 1125,
		.ver_back_porch = 2,
		.ver_front_porch = 38,
		.ver_sync_time = 5,
		.hor_sync_polarity = 1,/* 0: negative, 1: positive */
		.ver_sync_polarity = 1,/* 0: negative, 1: positive */
		.b_interlace = 1,
		.vactive_space = 0,
		.trd_mode = 0,
	},
	{
		.vic = 0,
		.tv_mode = DISP_TV_MOD_1080I_50HZ,
		.pixel_clk = 74250000,
		.pixel_repeat = 0,
		.x_res = 1920,
		.y_res = 1080,
		.hor_total_time = 2640,
		.hor_back_porch = 148,
		.hor_front_porch = 44,
		.hor_sync_time = 528,
		.ver_total_time = 1125,
		.ver_back_porch = 2,
		.ver_front_porch = 38,
		.ver_sync_time = 5,
		.hor_sync_polarity = 1,/* 0: negative, 1: positive */
		.ver_sync_polarity = 1,/* 0: negative, 1: positive */
		.b_interlace = 1,
		.vactive_space = 0,
		.trd_mode = 0,
	},
	{
		.vic = 0,
		.tv_mode = DISP_TV_MOD_1080P_60HZ,
		.pixel_clk = 148500000,
		.pixel_repeat = 0,
		.x_res = 1920,
		.y_res = 1080,
		.hor_total_time = 2200,
		.hor_back_porch = 148,
		.hor_front_porch = 44,
		.hor_sync_time = 88,
		.ver_total_time = 1125,
		.ver_back_porch = 4,
		.ver_front_porch = 36,
		.ver_sync_time = 5,
		.hor_sync_polarity = 1,/* 0: negative, 1: positive */
		.ver_sync_polarity = 1,/* 0: negative, 1: positive */
		.b_interlace = 0,
		.vactive_space = 0,
		.trd_mode = 0,
	},
	{
		.vic = 0,
		.tv_mode = DISP_TV_MOD_1080P_50HZ,
		.pixel_clk = 148500000,
		.pixel_repeat = 0,
		.x_res = 1920,
		.y_res = 1080,
		.hor_total_time = 2640,
		.hor_back_porch = 148,
		.hor_front_porch = 44,
		.hor_sync_time = 528,
		.ver_total_time = 1125,
		.ver_back_porch = 4,
		.ver_front_porch = 36,
		.ver_sync_time = 5,
		.hor_sync_polarity = 1,/* 0: negative, 1: positive */
		.ver_sync_polarity = 1,/* 0: negative, 1: positive */
		.b_interlace = 0,
		.vactive_space = 0,
		.trd_mode = 0,
	},
};



static int __parse_dump_str(const char *buf, size_t size,
				unsigned long *start, unsigned long *end)
{
	char *ptr = NULL;
	char *ptr2 = (char *)buf;
	int ret = 0, times = 0;

	/* Support single address mode, some time it haven't ',' */
next:

	/*Default dump only one register(*start =*end).
	If ptr is not NULL, we will cover the default value of end.*/
	if (times == 1)
		*start = *end;

	if (!ptr2 || (ptr2 - buf) >= size)
		goto out;

	ptr = ptr2;
	ptr2 = strnchr(ptr, size - (ptr - buf), ',');
	if (ptr2) {
		*ptr2 = '\0';
		ptr2++;
	}

	ptr = strim(ptr);
	if (!strlen(ptr))
		goto next;

	ret = kstrtoul(ptr, 16, end);
	if (!ret) {
		times++;
		goto next;
	} else
	EDP_WRN("String syntax errors: \"%s\"\n", ptr);

out:
	return ret;
}

s32 edp_hardware_reset(u32 sel)
{
	s32 ret = 0;
	struct pinctrl_state *state;

	if (!IS_ERR(g_edp_info[sel].rst_pin)) {
		state = pinctrl_lookup_state(g_edp_info[sel].rst_pin, "rst_assert");
		if (IS_ERR(state)) {
			EDP_WRN("pinctrl_lookup_state for edp reset assert fail\n");
			return RET_FAIL;
		}

		ret = pinctrl_select_state(g_edp_info[sel].rst_pin, state);
		if (ret < 0) {
			EDP_WRN("pinctrl_select_state for edp reset assert fail\n");
			return ret;
		}

		mdelay(5);

		state = pinctrl_lookup_state(g_edp_info[sel].rst_pin, "rst_deassert");
		if (IS_ERR(state)) {
			EDP_WRN("pinctrl_lookup_state for edp reset deassert fail\n");
			return RET_FAIL;
		}

		ret = pinctrl_select_state(g_edp_info[sel].rst_pin, state);
		if (ret < 0) {
			EDP_WRN("pinctrl_select_state for reset deassert fail\n");
			return ret;
		}
	}

	return ret;
}

/**
 * @name       :edp_clk_enable
 * @brief      :enable or disable edp clk
 * @param[IN]  :sel index of edp
 * @param[IN]  :en 1:enable , 0 disable
 * @return     :0 if success
 */
static s32 edp_clk_enable(u32 sel, bool en)
{
	s32 ret = -1;

	if (en) {
		if (!IS_ERR_OR_NULL(g_edp_info[sel].rst_bus))
			ret = reset_control_deassert(g_edp_info[sel].rst_bus);

		if (!IS_ERR_OR_NULL(g_edp_info[sel].clk_parent))
			ret = clk_prepare_enable(g_edp_info[sel].clk_parent);

		if (!IS_ERR_OR_NULL(g_edp_info[sel].clk_bus))
			ret = clk_prepare_enable(g_edp_info[sel].clk_bus);

		if (!IS_ERR_OR_NULL(g_edp_info[sel].clk))
			ret = clk_prepare_enable(g_edp_info[sel].clk);
	} else {
		if (!IS_ERR_OR_NULL(g_edp_info[sel].clk))
			clk_disable_unprepare(g_edp_info[sel].clk);

		if (!IS_ERR_OR_NULL(g_edp_info[sel].clk_bus))
			clk_disable_unprepare(g_edp_info[sel].clk_bus);

		if (!IS_ERR_OR_NULL(g_edp_info[sel].clk_parent))
			clk_disable_unprepare(g_edp_info[sel].clk_parent);

		if (!IS_ERR_OR_NULL(g_edp_info[sel].rst_bus))
			ret = reset_control_assert(g_edp_info[sel].rst_bus);
	}

	return ret;
}

void edp_correct_lane_para(u32 sel)
{
	struct edp_tx_core *edp_core;
	struct edp_rx_cap *sink_cap;
	struct edp_lane_para *recom_lane_para;
	struct edp_lane_para *def_lane_para;
	struct edp_lane_para *lane_para;

	edp_core = &g_edp_info[sel].edp_core;
	recom_lane_para = &edp_core->recom_lane_para;
	def_lane_para = &edp_core->def_lane_para;
	lane_para = &edp_core->lane_para;

	sink_cap = &g_edp_info[sel].sink_cap;

	/*recom para is empty now, need to fill first*/
	memcpy(recom_lane_para, lane_para, sizeof(struct edp_lane_para));

	if (sink_cap->max_rate < lane_para->bit_rate) {
		recom_lane_para->bit_rate = sink_cap->max_rate;
		g_edp_info[sel].use_def_para = false;
		g_edp_info[sel].use_recom_para = true;
		memcpy(lane_para, recom_lane_para, sizeof(struct edp_lane_para));
	}

	if (sink_cap->max_lane < lane_para->lane_cnt) {
		recom_lane_para->lane_cnt = sink_cap->max_lane;
		g_edp_info[sel].use_def_para = false;
		g_edp_info[sel].use_recom_para = true;
		memcpy(lane_para, recom_lane_para, sizeof(struct edp_lane_para));
	}

	if (sink_cap->bit_depth < lane_para->colordepth) {
		recom_lane_para->colordepth = sink_cap->bit_depth;
		g_edp_info[sel].use_def_para = false;
		g_edp_info[sel].use_recom_para = true;
		memcpy(lane_para, recom_lane_para, sizeof(struct edp_lane_para));
	}
}

void edid_cap_reset(u32 sel)
{
	struct edp_rx_cap *sink_cap;

	sink_cap = &g_edp_info[sel].sink_cap;

	memset(sink_cap, 0, sizeof(struct edp_rx_cap));
}


s32 edid_to_sink_info(u32 sel, struct edid *edid)
{
	struct edp_rx_cap *sink_cap;
	u8 *cea;
	s32 start, end;

	sink_cap = &g_edp_info[sel].sink_cap;

	sink_cap->mfg_weak = edid->mfg_week;
	sink_cap->mfg_year = edid->mfg_year;
	sink_cap->edid_ver = edid->version;
	sink_cap->edid_rev = edid->revision;

	/*14H: edid input info*/
	sink_cap->input_type = (edid->input & SUNXI_EDID_INPUT_DIGITAL) >> SUNXI_EDID_INPUT_SHIFT;

	/*digital input*/
	if (sink_cap->input_type == 1) {
		switch (edid->input & SUNXI_EDID_DIGITAL_DEPTH_MASK) {
		case SUNXI_EDID_DIGITAL_DEPTH_6:
			sink_cap->bit_depth = 6;
			break;
		case SUNXI_EDID_DIGITAL_DEPTH_8:
			sink_cap->bit_depth = 8;
			break;
		case SUNXI_EDID_DIGITAL_DEPTH_10:
			sink_cap->bit_depth = 10;
			break;
		case SUNXI_EDID_DIGITAL_DEPTH_12:
			sink_cap->bit_depth = 12;
			break;
		case SUNXI_EDID_DIGITAL_DEPTH_14:
			sink_cap->bit_depth = 14;
			break;
		case SUNXI_EDID_DIGITAL_DEPTH_16:
			sink_cap->bit_depth = 16;
			break;
		case SUNXI_EDID_DIGITAL_DEPTH_UNDEF:
		default:
			sink_cap->bit_depth = 0;
			break;
		}

		sink_cap->video_interface = edid->input & SUNXI_EDID_DIGITAL_TYPE_MASK;

		switch (edid->features & SUNXI_EDID_COLOR_FMT_MASK) {
		case SUNXI_EDID_COLOR_FMT_YCC444:
			sink_cap->Ycc444_support = true;
			sink_cap->Ycc422_support = false;
			break;
		case SUNXI_EDID_COLOR_FMT_YCC422:
			sink_cap->Ycc444_support = false;
			sink_cap->Ycc422_support = true;
			break;
		case SUNXI_EDID_COLOR_FMT_YCC444_422:
			sink_cap->Ycc444_support = true;
			sink_cap->Ycc422_support = true;
			break;
		default:
			sink_cap->Ycc444_support = false;
			sink_cap->Ycc422_support = false;
			break;
		}
	}


	sink_cap->width_cm = edid->width_cm;
	sink_cap->height_cm = edid->height_cm;


	cea = edp_edid_cea_extension(edid);

	if (!cea) {
		EDP_ERR("no CEA Extension found\n");
		return -ENOENT;
	}

	if (edp_edid_cea_tag(cea) != CEA_EXT) {
		EDP_ERR("wrong CEA tag\n");
		return -EOPNOTSUPP;
	}

	if (edp_edid_cea_revision(cea) < 3) {
		EDP_ERR("wrong CEA revision\n");
		return -EOPNOTSUPP;
	}

	if (edp_edid_cea_db_offsets(cea, &start, &end)) {
		EDP_ERR("invalid data block offsets\n");
		return -EPROTO;
	}

	if (cea[3] & SUNXI_CEA_BASIC_AUDIO_MASK)
		sink_cap->audio_support = true;
	else
		sink_cap->audio_support = false;

	if (cea[3] & SUNXI_CEA_YCC444_MASK)
		sink_cap->Ycc444_support = true;
	else
		sink_cap->Ycc444_support = false;

	if (cea[3] & SUNXI_CEA_YCC422_MASK)
		sink_cap->Ycc422_support = true;
	else
		sink_cap->Ycc422_support = false;

	return 0;
}

void edid_to_prefer_timings(u32 sel, struct edid *edid)
{
	struct detailed_timing *dt = edid->detailed_timings;
	struct detailed_pixel_timing *tmgs = &dt->data.pixel_data;
	struct disp_video_timings *edid_tmgs;
	struct edp_tx_core *edp_core;
	u32 hactive, vactive, hblank, vblank;
	u32 hsync_offset, hsync_pulse_width, vsync_offset, vsync_pulse_width;

	hactive = (tmgs->hactive_hblank_hi & 0xf0) << 4 | tmgs->hactive_lo;
	vactive = (tmgs->vactive_vblank_hi & 0xf0) << 4 | tmgs->vactive_lo;
	hblank = (tmgs->hactive_hblank_hi & 0xf) << 8 | tmgs->hblank_lo;
	vblank = (tmgs->vactive_vblank_hi & 0xf) << 8 | tmgs->vblank_lo;
	hsync_offset = (tmgs->hsync_vsync_offset_pulse_width_hi & 0xc0) << 2 | tmgs->hsync_offset_lo;
	hsync_pulse_width = (tmgs->hsync_vsync_offset_pulse_width_hi & 0x30) << 4 | tmgs->hsync_pulse_width_lo;
	vsync_offset = (tmgs->hsync_vsync_offset_pulse_width_hi & 0xc) << 2 | tmgs->vsync_offset_pulse_width_lo >> 4;
	vsync_pulse_width = (tmgs->hsync_vsync_offset_pulse_width_hi & 0x3) << 4 | (tmgs->vsync_offset_pulse_width_lo & 0xf);

	edp_core = &g_edp_info[sel].edp_core;
	edid_tmgs = &edp_core->edid_timings;

	edid_tmgs->vic = 0;
	edid_tmgs->tv_mode = 0;
	edid_tmgs->pixel_clk = le16_to_cpu(dt->pixel_clock) * 10000;
	edid_tmgs->pixel_repeat = 0;
	edid_tmgs->x_res = hactive;
	edid_tmgs->y_res = vactive;
	edid_tmgs->hor_total_time = hactive + hblank;
	edid_tmgs->hor_back_porch = hblank - hsync_offset - hsync_pulse_width;
	edid_tmgs->hor_front_porch = hsync_pulse_width;
	edid_tmgs->hor_sync_time = hsync_offset;
	edid_tmgs->ver_total_time = vactive + vblank;
	edid_tmgs->ver_back_porch = vblank - vsync_offset - vsync_pulse_width;
	edid_tmgs->ver_front_porch = vsync_pulse_width;
	edid_tmgs->ver_sync_time = vsync_offset;

	switch (tmgs->misc & SUNXI_EDID_MISC_HPOLAR_MASK) {
	case SUNXI_EDID_MISC_HPOLAR_NEG:
		edid_tmgs->hor_sync_polarity = 0;
		break;
	case SUNXI_EDID_MISC_HPOLAR_POS:
	default:
		edid_tmgs->hor_sync_polarity = 1;
		break;
	}

	switch (tmgs->misc & SUNXI_EDID_MISC_VPOLAR_MASK) {
	case SUNXI_EDID_MISC_VPOLAR_NEG:
		edid_tmgs->ver_sync_polarity = 0;
		break;
	case SUNXI_EDID_MISC_VPOLAR_POS:
	default:
		edid_tmgs->ver_sync_polarity = 1;
		break;
	}

	if (tmgs->misc & SUNXI_EDID_MISC_INTERLACE_MASK)
		edid_tmgs->b_interlace = true;
	else
		edid_tmgs->b_interlace = false;

	edid_tmgs->vactive_space = 0;
	edid_tmgs->trd_mode = 0;
}

s32 edp_parse_edid(u32 sel, struct edid *edid)
{
	s32 ret;
	ret = edid_to_sink_info(sel, edid);
	if (ret < 0)
		return ret;
	edid_to_prefer_timings(sel, edid);

	return ret;
}

void edp_correct_timings(u32 sel)
{
	struct edp_tx_core *edp_core;

	edp_core = &g_edp_info[sel].edp_core;

	/*set timings by edid*/
	if (!g_edp_info[sel].support_fixed_timings) {
		if (g_edp_info[sel].support_edid_timings) {
			memcpy(&edp_core->timings, &edp_core->edid_timings,
			       sizeof(struct disp_video_timings));

			mutex_lock(&g_edp_info[sel].mlock);
			g_edp_info[sel].use_def_timings = false;
			g_edp_info[sel].use_edid_timings = true;
			g_edp_info[sel].use_user_timings = false;
			g_edp_info[sel].edp_core.mode = DISP_TV_MODE_NUM;
			mutex_unlock(&g_edp_info[sel].mlock);
		}
	}
}

s32 edp_phy_cfg_parse(u32 sel)
{
	s32 ret = -1;
	s32  value = 1;
	char primary_key[20];
	struct edp_tx_core *edp_core;

	edp_core = &g_edp_info[sel].edp_core;

	sprintf(primary_key, "edp%d", sel);

	ret = disp_sys_script_get_item(primary_key, "edp_ssc_en", &value, 1);
	if (ret == 1)
		edp_core->ssc_en = value;

	/*
	 * ssc modulation mode select
	 * 0: center mode
	 * 1: downspread mode
	 *
	 * */
	ret = disp_sys_script_get_item(primary_key, "edp_ssc_mode", &value, 1);
	if (ret == 1)
		edp_core->ssc_mode = value;

	ret = disp_sys_script_get_item(primary_key, "edp_psr_support", &value, 1);
	if (ret == 1)
		edp_core->psr_en = value;

	ret = disp_sys_script_get_item(primary_key, "edp_audio_en", &value, 1);
	if (ret == 1)
		edp_core->audio_en = value;

	return 0;
}


s32 edp_lane_para_parse(u32 sel)
{
	s32 ret = -1;
	s32  value = 1;
	char primary_key[20];
	struct edp_tx_core *edp_core;
	struct edp_lane_para *def_lane_para;

	edp_core = &g_edp_info[sel].edp_core;
	def_lane_para = &edp_core->def_lane_para;

	sprintf(primary_key, "edp%d", sel);

	ret = disp_sys_script_get_item(primary_key, "edp_rate", &value, 1);
	if (ret == 1) {
		switch (value) {
		case 0:
			def_lane_para->bit_rate = BIT_RATE_1G62;
			break;
		case 1:
			def_lane_para->bit_rate = BIT_RATE_2G7;
			break;
		case 2:
			def_lane_para->bit_rate = BIT_RATE_5G4;
			break;
		default:
			EDP_WRN("edp_rate out of range!\n");
			break;
		}
	}

	ret = disp_sys_script_get_item(primary_key, "edp_lane", &value, 1);
	if (ret == 1)
		def_lane_para->lane_cnt = value;

	ret = disp_sys_script_get_item(primary_key, "edp_hbr2", &value, 1);
	if (ret == 1)
		def_lane_para->support_hbr2 = value;

	ret = disp_sys_script_get_item(primary_key, "edp_training_func", &value,
				       1);
	if (ret == 1)
		def_lane_para->training_func = value;

	ret = disp_sys_script_get_item(primary_key, "edp_sramble_seed", &value,
				       1);
	if (ret == 1)
		def_lane_para->sramble_seed = value;

	ret = disp_sys_script_get_item(primary_key, "edp_colordepth", &value, 1);
	if (ret == 1)
		def_lane_para->colordepth = value;


	ret = disp_sys_script_get_item(primary_key, "lane0_sw", &value, 1);
	if (ret == 1)
		def_lane_para->lane0_sw = value;

	ret = disp_sys_script_get_item(primary_key, "lane1_sw", &value, 1);
	if (ret == 1)
		def_lane_para->lane1_sw = value;

	ret = disp_sys_script_get_item(primary_key, "lane2_sw", &value, 1);
	if (ret == 1)
		def_lane_para->lane2_sw = value;

	ret = disp_sys_script_get_item(primary_key, "lane3_sw", &value, 1);
	if (ret == 1)
		def_lane_para->lane3_sw = value;

	ret = disp_sys_script_get_item(primary_key, "lane0_pre", &value, 1);
	if (ret == 1)
		def_lane_para->lane0_pre = value;

	ret = disp_sys_script_get_item(primary_key, "lane1_pre", &value, 1);
	if (ret == 1)
		def_lane_para->lane1_pre = value;

	ret = disp_sys_script_get_item(primary_key, "lane2_pre", &value, 1);
	if (ret == 1)
		def_lane_para->lane2_pre = value;

	ret = disp_sys_script_get_item(primary_key, "lane3_pre", &value, 1);
	if (ret == 1)
		def_lane_para->lane3_pre = value;

	ret = disp_sys_script_get_item(primary_key, "edp_fps", &value, 1);
	if (ret == 1)
		def_lane_para->fps = value;

	edp_core->lane_para = edp_core->def_lane_para;

	g_edp_info[sel].use_def_para = true;
	g_edp_info[sel].use_recom_para = false;

	return 0;
}

s32 edp_default_timming_parse(u32 sel)
{
	s32 ret = -1;
	s32  value = 1;
	char primary_key[20];
	struct edp_tx_core *edp_core;
	struct disp_video_timings *def_timings;

	edp_core = &g_edp_info[sel].edp_core;
	def_timings = &edp_core->def_timings;

	sprintf(primary_key, "edp%d", sel);

	/* edp_timings:
	 *
	 * 0: only support fixed_timings
	 * 1: support fixed_timings and timings set from user*
	 * 2: support fix_timings, timming set from user and timing from
	 * edid(edid is highest priority)
	 *
	 */
	ret = disp_sys_script_get_item(primary_key, "edp_timings", &value, 1);
	if (ret == 1) {
		switch (value) {
		case 0:
			g_edp_info[sel].support_fixed_timings = true;
			g_edp_info[sel].support_edid_timings = false;
			break;
		case 1:
			g_edp_info[sel].support_fixed_timings = false;
			g_edp_info[sel].support_edid_timings = false;
			break;
		case 2:
		default:
			g_edp_info[sel].support_fixed_timings = false;
			g_edp_info[sel].support_edid_timings = true;
			break;
		}
	}

	ret = disp_sys_script_get_item(primary_key, "edp_x", &value, 1);
	if (ret == 1)
		def_timings->x_res = value;

	ret = disp_sys_script_get_item(primary_key, "edp_y", &value, 1);
	if (ret == 1)
		def_timings->y_res = value;

	ret = disp_sys_script_get_item(primary_key, "edp_hbp", &value, 1);
	if (ret == 1)
		def_timings->hor_back_porch = value;

	ret = disp_sys_script_get_item(primary_key, "edp_ht", &value, 1);
	if (ret == 1)
		def_timings->hor_total_time = value;

	ret = disp_sys_script_get_item(primary_key, "edp_hspw", &value, 1);
	if (ret == 1)
		def_timings->hor_sync_time = value;

	ret = disp_sys_script_get_item(primary_key, "edp_vt", &value, 1);
	if (ret == 1)
		def_timings->ver_total_time = value;

	ret = disp_sys_script_get_item(primary_key, "edp_vspw", &value, 1);
	if (ret == 1)
		def_timings->ver_sync_time = value;

	ret = disp_sys_script_get_item(primary_key, "edp_vbp", &value, 1);
	if (ret == 1)
		def_timings->ver_back_porch = value;


	def_timings->pixel_clk = def_timings->hor_total_time * \
		def_timings->ver_total_time * edp_core->def_lane_para.fps;

	edp_core->timings = edp_core->def_timings;

	memcpy(&edp_core->timings, &edp_core->def_timings,
	       sizeof(struct disp_video_timings));

	mutex_lock(&g_edp_info[sel].mlock);
	g_edp_info[sel].use_def_timings = true;
	g_edp_info[sel].use_edid_timings = false;
	g_edp_info[sel].use_user_timings = false;
	g_edp_info[sel].edp_core.mode = DISP_TV_MODE_NUM;
	mutex_unlock(&g_edp_info[sel].mlock);

	return 0;
}

static bool disp_is_edp_boot(void)
{
	u32 output_type0, output_mode0;
	u32 output_type1, output_mode1;
	u32 value;

	value = disp_boot_para_parse("boot_disp");
	output_type0 = (value >> 8) & 0xff;
	output_mode0 = (value)&0xff;

	output_type1 = (value >> 24) & 0xff;
	output_mode1 = (value >> 16) & 0xff;

	if ((output_type0 == DISP_OUTPUT_TYPE_EDP) ||
	    (output_type1 == DISP_OUTPUT_TYPE_EDP))
		return true;
	else
		return false;
}

s32 edp_get_list_num(void)
{
	return sizeof(video_timing)/sizeof(struct disp_video_timings);
}

s32 edp_mode_support(u32 sel, enum disp_tv_mode mode)
{
	u32 i, list_num;
	struct disp_video_timings *info;

	info = video_timing;
	list_num = edp_get_list_num();
	for (i = 0; i < list_num; i++) {
		if (info->tv_mode == mode)
			return 1;
		info++;
	}
	return 0;
}

s32 edp_mode_to_timings_transfer(enum disp_tv_mode mode, struct disp_video_timings *tmgs)
{
	u32 i, list_num;

	list_num = edp_get_list_num();
	for (i = 0; i < list_num; i++) {
		if (video_timing[i].tv_mode == mode) {
			memcpy(tmgs, &video_timing[i], sizeof(struct disp_video_timings));
			return 1;
		}
	}
	return 0;
}

s32 edp_get_mode(u32 sel)
{
	return g_edp_info[sel].edp_core.mode;
}

s32 edp_set_mode(u32 sel, enum disp_tv_mode mode)
{
	if (g_edp_info[sel].support_fixed_timings) {
		EDP_WRN("edp timing is fixed, can not be modify\n");
		return -1;
	}

	if (mode >= DISP_TV_MODE_NUM)
		return -1;

	if (!edp_mode_support(sel, mode))
		return -1;

	EDP_DBG("edp%d mode: %d\n", sel, mode);

	mutex_lock(&g_edp_info[sel].mlock);
	g_edp_info[sel].edp_core.mode = mode;
	mutex_unlock(&g_edp_info[sel].mlock);
	edp_mode_to_timings_transfer(g_edp_info[sel].edp_core.mode,
				     &g_edp_info[sel].edp_core.timings);

	mutex_lock(&g_edp_info[sel].mlock);
	g_edp_info[sel].use_def_timings = false;
	g_edp_info[sel].use_edid_timings = false;
	g_edp_info[sel].use_user_timings = true;
	mutex_unlock(&g_edp_info[sel].mlock);

	return  0;
}



void edp_recommaend_lane_config(u32 sel)
{
	struct edp_tx_core *edp_core;
	struct edp_lane_para *lane_para;
	struct edp_lane_para *recom_lane_para;

	edp_core = &g_edp_info[sel].edp_core;
	lane_para = &edp_core->lane_para;
	recom_lane_para = &edp_core->recom_lane_para;

	lane_para->lane0_sw = recom_lane_para->lane0_sw;
	lane_para->lane0_pre = recom_lane_para->lane0_pre;
	lane_para->lane1_sw = recom_lane_para->lane1_sw;
	lane_para->lane1_pre = recom_lane_para->lane1_pre;
	lane_para->lane2_sw = recom_lane_para->lane2_sw;
	lane_para->lane2_pre = recom_lane_para->lane2_pre;
	lane_para->lane3_sw = recom_lane_para->lane3_sw;
	lane_para->lane3_pre = recom_lane_para->lane3_pre;
}

/**
 * @name       edp_get_video_timing_info
 * @brief      get timing info
 * @param[IN]  sel:index of edp module
 * @param[OUT] video_info:timing info
 * @return     0 if success
 */
static s32 edp_get_video_timing_info(u32 sel,
				      struct disp_video_timings **video_info)
{
	s32 ret = 0;

	*video_info = &g_edp_info[sel].edp_core.timings;
	return ret;
}


s32 edp_read_dpcd(u32 sel, char *dpcd_rx_buf)
{
	return edp_core_read_dpcd(sel, dpcd_rx_buf);
}

void edp_parse_dpcd(u32 sel, char *dpcd_rx_buf)
{
	struct edp_rx_cap *sink_cap;

	sink_cap = &g_edp_info[sel].sink_cap;

	sink_cap->dpcd_rev = 10 + ((dpcd_rx_buf[0] >> 4) & 0x0f);
	EDP_DBG("DPCD version:1.%d\n", sink_cap->dpcd_rev % 10);

	if (dpcd_rx_buf[1] == 0x06) {
		sink_cap->max_rate = BIT_RATE_1G62;
		EDP_DBG("sink max bit rate:1.62Gbps\n");
	} else if (dpcd_rx_buf[1] == 0x0a) {
		sink_cap->max_rate = BIT_RATE_2G7;
		EDP_DBG("sink max bit rate:2.7Gbps\n");
	} else if (dpcd_rx_buf[1] == 0x14) {
		sink_cap->max_rate = BIT_RATE_5G4;
		EDP_DBG("sink max bit rate:5.4Gbps\n");
	}

	sink_cap->max_lane = dpcd_rx_buf[2] & EDP_DPCD_MAX_LANE_MASK;
	EDP_DBG("sink max lane count:%d\n", sink_cap->max_lane);

	if (dpcd_rx_buf[2] & EDP_DPCD_ENHANCE_FRAME_MASK) {
		sink_cap->enhance_frame_support = true;
		EDP_DBG("enhanced mode:support\n");
	} else {
		sink_cap->enhance_frame_support = false;
		EDP_DBG("enhanced mode:not support\n");
	}

	if (dpcd_rx_buf[2] & EDP_DPCD_TPS3_MASK) {
		sink_cap->tps3_support = true;
		EDP_DBG("enhanced mode:support\n");
	} else {
		sink_cap->tps3_support = false;
		EDP_DBG("enhanced mode:not support\n");
	}

	if (dpcd_rx_buf[3] & EDP_DPCD_FAST_TRAIN_MASK) {
		sink_cap->fast_train_support = true;
		EDP_DBG("aux handshake link training:not require\n");
	} else {
		sink_cap->fast_train_support = false;
		EDP_DBG("aux handshake link training:require\n");
	}

	if (dpcd_rx_buf[5] & EDP_DPCD_DOWNSTREAM_PORT_MASK) {
		sink_cap->downstream_port_support = true;
		EDP_DBG("downstream port:present\n");
	} else {
		sink_cap->downstream_port_support = false;
		EDP_DBG("downstream port:not present\n");
	}

	sink_cap->downstream_port_type = (dpcd_rx_buf[5] & EDP_DPCD_DOWNSTREAM_PORT_TYPE_MASK);

	sink_cap->downstream_port_cnt = (dpcd_rx_buf[7] & EDP_DPCD_DOWNSTREAM_PORT_CNT_MASK);

	if (dpcd_rx_buf[8] & EDP_DPCD_LOCAL_EDID_MASK) {
		sink_cap->local_edid_support = true;
		EDP_DBG("ReceiverPort0 Capability_0:Has a local EDID\n");
	} else {
		sink_cap->local_edid_support = false;
		EDP_DBG("ReceiverPort0 Capability_0:Not has a local EDID\n");
	}

	/* eDP_CONFIGURATION_CAP */
	/* Always reads 0x00 for external receivers */
	if (dpcd_rx_buf[0x0d] != 0) {
		sink_cap->is_edp_device = true;
		EDP_DBG("Sink device is eDP receiver!\n");
		if (dpcd_rx_buf[0x0d] & EDP_DPCD_ASSR_MASK)
			sink_cap->assr_support = true;
		else
			sink_cap->assr_support = false;

		if (dpcd_rx_buf[0x0d] & EDP_DPCD_FRAME_CHANGE_MASK)
			sink_cap->enhance_frame_support = true;
		else
			sink_cap->enhance_frame_support = false;

	} else {
		sink_cap->is_edp_device = false;
		EDP_DBG("Sink device is external receiver!\n");
	}
}

bool sink_is_edp(u32 sel)
{
	struct edp_rx_cap *sink_cap;

	sink_cap = &g_edp_info[sel].sink_cap;

	return sink_cap->is_edp_device;
}


#if defined(CONFIG_EXTCON)
static struct extcon_dev *extcon_edp[EDP_NUM_MAX];
static char edp_extcon_name[20];
static const u32 edp_cable[] = {
	EXTCON_DISP_EDP,
	EXTCON_NONE,
};

s32 edp_report_hpd_work(u32 sel, u32 hpd)
{
	if (hpd_state[sel] == hpd)
		return -1;

	switch (hpd) {

	case EDP_HPD_PLUGIN:
		extcon_set_state_sync(extcon_edp[sel], EXTCON_DISP_EDP,
				      EDP_HPD_PLUGIN);
		break;
	case EDP_HPD_PLUGOUT:
	default:
		extcon_set_state_sync(extcon_edp[sel], EXTCON_DISP_EDP,
				      EDP_HPD_PLUGOUT);
		break;
	}
	return 0;
}

#else
s32 edp_report_hpd_work(u32 sel, u32 hpd)
{
	EDP_ERR("You need to enable CONFIG_EXTCON!\n");
	return 0;
}
#endif

static s32 edp_init(struct platform_device *pdev)
{
	s32 ret = -1;
	u32 sel = pdev->id;
	bool boot_edp;

	EDP_DBG("start edp init\n");

	mutex_init(&g_edp_info[sel].mlock);

	pdev->dev.id = pdev->id;

	g_edp_info[sel].dev = &pdev->dev;

	ret = edp_phy_cfg_parse(sel);
	if (ret != 0)
		goto OUT;

	ret = edp_lane_para_parse(sel);
	if (ret != 0)
		goto OUT;

	ret = edp_default_timming_parse(sel);
	if (ret != 0)
		goto OUT;

	boot_edp = disp_is_edp_boot();

	if (boot_edp)
		g_edp_info[sel].enable = 1;
	else {
		//dp_hpd_enable(pdev->id, g_edp_info[pdev->id].para.edp_lane,
		//	      g_edp_info[pdev->id].para.edp_rate);

		edp_core_phy_init(sel, &g_edp_info[sel].edp_core);
		if (g_edp_info[sel].edp_core.ssc_en) {
			edp_core_ssc_enable(sel, g_edp_info[sel].edp_core.ssc_en);
			edp_core_ssc_set_mode(sel, g_edp_info[sel].edp_core.ssc_mode);
		}

		if (g_edp_info[sel].edp_core.psr_en)
			edp_core_psr_enable(sel, true);
	}

#if defined(CONFIG_EXTCON)
	snprintf(edp_extcon_name, sizeof(edp_extcon_name), "edp%d", sel);
	extcon_edp[sel] = devm_extcon_dev_allocate(&pdev->dev, edp_cable);
	if (IS_ERR_OR_NULL(extcon_edp[sel])) {
		EDP_ERR("devm_extcon_dev_allocate fail:%d", sel);
		goto OUT;
	}
	ret = devm_extcon_dev_register(&pdev->dev, extcon_edp[sel]);
	extcon_edp[sel]->name = edp_extcon_name;
#endif
OUT:
	EDP_DBG("end of edp init:%d\n", ret);
	return ret;
}

void edp_show_builtin_patten(u32 sel, u32 patten)
{
	edp_core_show_builtin_patten(sel, patten);
}

u32 edp_get_cur_line(u32 sel)
{
	u32 ret = 0;

	ret = edp_core_get_cur_line(sel);
	return ret;
}

s32 edp_get_start_delay(u32 sel)
{
	s32 ret = -1;

	ret = edp_core_get_start_dly(sel);
	return ret;
}

s32 edp_irq_enable(u32 sel, u32 irq_id, u32 en)
{
	return edp_core_irq_enable(sel, irq_id, (bool)en);
}

s32 edp_irq_query(u32 sel)
{
	s32 ret = -1;

	ret = edp_core_irq_query(sel);
	if (ret == 1)
		ret = edp_core_irq_clear(sel);
	return ret;
}

s32 edp_resource_init(struct platform_device *pdev)
{
	u32 clk_index = 0;
	s32 ret = 0;

	pdev->id = of_alias_get_id(pdev->dev.of_node, "edp");
	if (pdev->id < 0) {
		EDP_DEV_ERR(&pdev->dev, "failed to get alias id\n");
		ret = RET_FAIL;
		goto OUT;
	}

	g_edp_info[g_edp_num].base_addr =
	    (uintptr_t __force)of_iomap(pdev->dev.of_node, 0);
	if (!g_edp_info[g_edp_num].base_addr) {
		EDP_DEV_ERR(&pdev->dev, "fail to get addr for edp%d!\n", pdev->id);
		ret = RET_FAIL;
		goto OUT;
	}

	edp_core_set_reg_base(pdev->id, g_edp_info[pdev->id].base_addr);

	g_edp_info[pdev->id].clk_parent = of_clk_get(pdev->dev.of_node, clk_index);
	if (IS_ERR_OR_NULL(g_edp_info[pdev->id].clk)) {
		EDP_DEV_ERR(&pdev->dev, "fail to get clk parentfor edp%d!\n", pdev->id);
		ret = RET_FAIL;
		goto ERR_IOMAP;
	}

	clk_index++;
	g_edp_info[pdev->id].clk_bus = of_clk_get(pdev->dev.of_node, clk_index);
	if (IS_ERR_OR_NULL(g_edp_info[pdev->id].clk)) {
		EDP_DEV_ERR(&pdev->dev, "fail to get clk bus for edp%d!\n", pdev->id);
		ret = RET_FAIL;
		goto ERR_CLK_PARENT;
	}

	clk_index++;
	g_edp_info[pdev->id].clk = of_clk_get(pdev->dev.of_node, clk_index);
	if (IS_ERR_OR_NULL(g_edp_info[pdev->id].clk)) {
		EDP_DEV_ERR(&pdev->dev, "fail to get clk for edp%d!\n", pdev->id);
		ret = RET_FAIL;
		goto ERR_CLK_BUS;
	}

	g_edp_info[pdev->id].rst_bus = devm_reset_control_get(&pdev->dev, "rst_bus_edp");
	if (IS_ERR_OR_NULL(g_edp_info[pdev->id].rst_bus)) {
		EDP_DEV_ERR(&pdev->dev, "fail to get rst for edp%d!\n", pdev->id);
		ret = RET_FAIL;
		goto ERR_CLK;
	}

	ret = edp_clk_enable(pdev->id, true);
	if (ret) {
		EDP_ERR("edp%d edp_clk_enable fail!!\n", pdev->id);
		goto ERR_CLK;
	}

	/*parse power resource*/
	g_edp_info[pdev->id].regulator = regulator_get(&pdev->dev, "edp");
	if (IS_ERR_OR_NULL(g_edp_info[pdev->id].regulator))
		EDP_DBG("edp power is null or no need!\n");
	if (g_edp_info[pdev->id].regulator)
		ret = regulator_enable(g_edp_info[pdev->id].regulator);

	g_edp_info[pdev->id].rst_pin = pinctrl_get(&pdev->dev);
	if (IS_ERR(g_edp_info[pdev->id].rst_pin))
		EDP_DBG("edp reset pin is null or no need!\n");
	g_edp_info[pdev->id].edp_core.rst_pin = g_edp_info[pdev->id].rst_pin;

	edp_hardware_reset(pdev->id);

	return RET_OK;

ERR_CLK:
	clk_put(g_edp_info[pdev->id].clk);
ERR_CLK_BUS:
	clk_put(g_edp_info[pdev->id].clk_bus);
ERR_CLK_PARENT:
	clk_put(g_edp_info[pdev->id].clk_parent);
ERR_IOMAP:
	if (g_edp_info[g_edp_num].base_addr)
		iounmap((char __iomem *)g_edp_info[g_edp_num].base_addr);
OUT:
	return ret;

}



static ssize_t dpcd_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	u32 count = 0;
	struct platform_device *pdev = dev_get_drvdata(dev);
	u32 sel = pdev->id;
	int i;

	if (hpd_state[sel]) {
		count += sprintf(buf + count, "[EDP] error: sink is plugout!\n");
		return count;
	}

	if (dpcd_rx_buf != NULL) {
		count += sprintf(buf + count, "[EDP] error: dpcd read uncorrectly!\n");
		return count;
	}

	for (i = 0; i < sizeof(dpcd_rx_buf); i++) {
		if ((i % 0x10) == 0)
			count += sprintf(buf + count, "\n%02x:", i);
		count += sprintf(buf + count, "  %02x", dpcd_rx_buf[i]);
	}

	return count;
}

static ssize_t edid_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	struct edp_tx_core *edp_core;
	struct edid *edid;
	struct platform_device *pdev = dev_get_drvdata(dev);
	u32 sel = pdev->id;
	u8 *cea;
	u32 count = 0;
	s32 i, edid_lenth;

	if (hpd_state[sel]) {
		count += sprintf(buf + count, "[EDP] error: sink is plugout!\n");
		return count;
	}

	edp_core = &g_edp_info[sel].edp_core;
	edid = edp_core->edid;
	if (edid == NULL) {
		count += sprintf(buf + count, "[EDP] error: edid read uncorrectly!\n");
		return count;
	}

	cea = edp_edid_cea_extension(edid);
	if (!cea)
		edid_lenth = 128;
	 else
		edid_lenth = 256;


	for (i = 0; i < edid_lenth; i++) {
		if ((i % 0x10) == 0)
			count += sprintf(buf + count, "\n%02x:", i);
		count += sprintf(buf + count, "  %s", (char *)edid + i);
	}

	return count;
}


static ssize_t hotplug_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	u32 count = 0;
	struct platform_device *pdev = dev_get_drvdata(dev);
	u32 sel = pdev->id;

	if (hpd_state[sel])
		count += sprintf(buf + count, "hpd_state: Plugin");
	else
		count += sprintf(buf + count, "hpd_state: Plugout");

	return count;
}

static ssize_t sink_info_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	struct edp_rx_cap *sink_cap;
	struct platform_device *pdev = dev_get_drvdata(dev);
	u32 sel = pdev->id;
	u32 count = 0;

	if (hpd_state[sel]) {
		count += sprintf(buf + count, "[EDP] error: sink is plugout!\n");
		return count;
	}

	sink_cap = &g_edp_info[sel].sink_cap;

	if (!sink_cap) {
		count += sprintf(buf + count, "eDP sink capacity unknown\n");
		return count;
	}

	count += sprintf(buf + count, "eDP Sink capacity info:\n");
	count += sprintf(buf + count, "dpcd_rev: %d", sink_cap->dpcd_rev);
	count += sprintf(buf + count, "\t\tmax_bit_rate: %lld", sink_cap->max_rate);
	count += sprintf(buf + count, "\t\tmax_lane_cnt: %d", sink_cap->max_lane);
	count += sprintf(buf + count, "\t\tmax_lane_cnt: %d\n", sink_cap->max_lane);
	count += sprintf(buf + count, "tps3_support: %s", sink_cap->tps3_support ? "Yes" : "No");
	count += sprintf(buf + count, "\t\tfast_link_train_support: %s\n", sink_cap->fast_train_support ? "Yes" : "No");
	count += sprintf(buf + count, "downstream_port_support: %s", sink_cap->downstream_port_support ? "Yes" : "No");

	if (sink_cap->downstream_port_support) {
		switch (sink_cap->downstream_port_type) {
		case 0:
			count += sprintf(buf + count, "\t\tdownstream_port_type: DP");
			break;
		case 1:
			count += sprintf(buf + count, "\t\tdownstream_port_type: VGA");
			break;
		case 2:
			count += sprintf(buf + count, "\t\tdownstream_port_type: DVI/HDMI/DP++");
			break;
		case 3:
		default:
			count += sprintf(buf + count, "\t\tdownstream_port_type: others");
			break;
		}

		count += sprintf(buf + count, "\t\tdownstream_port_cnt: %d\n", sink_cap->downstream_port_cnt);
	} else {
		count += sprintf(buf + count, "\t\tdownstream_port_type: NULL");
		count += sprintf(buf + count, "\t\tdownstream_port_cnt: NULL\n");
	}

	count += sprintf(buf + count, "\t\tlocal_edid_support: %s", sink_cap->local_edid_support ? "Yes" : "No");
	count += sprintf(buf + count, "\t\tlocal_edid_support: %s", sink_cap->local_edid_support ? "Yes" : "No");
	count += sprintf(buf + count, "\t\tedp_device: %s", sink_cap->is_edp_device ? "Yes" : "No");
	count += sprintf(buf + count, "\t\tassr_support: %s", sink_cap->assr_support ? "Yes" : "No");
	count += sprintf(buf + count, "\t\tenhance_frame_support: %s\n", sink_cap->enhance_frame_support ? "Yes" : "No");

	/*edid info*/
	count += sprintf(buf + count, "mfg_year: %d\n", sink_cap->mfg_year);
	count += sprintf(buf + count, "\t\tmfg_weak: %d", sink_cap->mfg_weak);
	count += sprintf(buf + count, "\t\tedid_ver: %d", sink_cap->edid_ver);
	count += sprintf(buf + count, "\t\tedid_rev: %d", sink_cap->edid_rev);
	count += sprintf(buf + count, "\t\twidth_cm: %d", sink_cap->width_cm);
	count += sprintf(buf + count, "\t\theight_cm: %d", sink_cap->height_cm);
	count += sprintf(buf + count, "\t\tinput_type: %s", sink_cap->input_type ? "Digital" : "Analog");
	count += sprintf(buf + count, "\t\tinput_bit_depth: %d", sink_cap->bit_depth);

	switch (sink_cap->video_interface) {
	case SUNXI_EDID_DIGITAL_TYPE_UNDEF:
		count += sprintf(buf + count, "\t\tsink_video_interface: Undefined\n");
		break;
	case SUNXI_EDID_DIGITAL_TYPE_DVI:
		count += sprintf(buf + count, "\t\tsink_video_interface: DVI\n");
		break;
	case SUNXI_EDID_DIGITAL_TYPE_HDMI_A:
		count += sprintf(buf + count, "\t\tsink_video_interface: HDMIa\n");
		break;
	case SUNXI_EDID_DIGITAL_TYPE_HDMI_B:
		count += sprintf(buf + count, "\t\tsink_video_interface: HDMIb\n");
		break;
	case SUNXI_EDID_DIGITAL_TYPE_MDDI:
		count += sprintf(buf + count, "\t\tsink_video_interface: MDDI\n");
		break;
	case SUNXI_EDID_DIGITAL_TYPE_DP:
		count += sprintf(buf + count, "\t\tsink_video_interface: DP/eDP\n");
		break;
	}

	count += sprintf(buf + count, "\t\tYcc444_support: %s", sink_cap->Ycc444_support ? "Yes" : "No");
	count += sprintf(buf + count, "\t\tYcc422_support: %s", sink_cap->Ycc422_support ? "Yes" : "No");
	count += sprintf(buf + count, "\t\tYcc420_support: %s", sink_cap->Ycc420_support ? "Yes" : "No");
	count += sprintf(buf + count, "\t\taudio_support: %s\n", sink_cap->audio_support ? "Yes" : "No");

	return count;
}

static ssize_t source_info_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	struct edp_tx_core *edp_core;
	struct edp_lane_para tmp_lane_para;
	struct platform_device *pdev = dev_get_drvdata(dev);
	u32 sel = pdev->id;
	u32 count = 0;
	u32 tu_size;
	char color_fmt[10];

	edp_core = &g_edp_info[sel].edp_core;

	switch (edp_core_get_color_fmt(sel)) {
	case RGB_6BIT:
		snprintf(color_fmt, sizeof(color_fmt), "RGB_6BIT");
		break;
	case RGB_8BIT:
		snprintf(color_fmt, sizeof(color_fmt), "RGB_8BIT");
		break;
	case RGB_10BIT:
		snprintf(color_fmt, sizeof(color_fmt), "RGB_10BIT");
		break;
	case RGB_12BIT:
		snprintf(color_fmt, sizeof(color_fmt), "RGB_12BIT");
		break;
	case RGB_16BIT:
		snprintf(color_fmt, sizeof(color_fmt), "RGB_16BIT");
		break;
	case YCBCR444_8BIT:
		snprintf(color_fmt, sizeof(color_fmt), "YCBCR444_8BIT");
		break;
	case YCBCR444_10BIT:
		snprintf(color_fmt, sizeof(color_fmt), "YCBCR444_10BIT");
		break;
	case YCBCR444_12BIT:
		snprintf(color_fmt, sizeof(color_fmt), "YCBCR444_12BIT");
		break;
	case YCBCR444_16BIT:
		snprintf(color_fmt, sizeof(color_fmt), "YCBCR444_16BIT");
		break;
	case YCBCR422_8BIT:
		snprintf(color_fmt, sizeof(color_fmt), "YCBCR422_8BIT");
		break;
	case YCBCR422_10BIT:
		snprintf(color_fmt, sizeof(color_fmt), "YCBCR422_10BIT");
		break;
	case YCBCR422_12BIT:
		snprintf(color_fmt, sizeof(color_fmt), "YCBCR422_12BIT");
		break;
	case YCBCR422_16BIT:
		snprintf(color_fmt, sizeof(color_fmt), "YCBCR422_16BIT");
		break;
	default:
		snprintf(color_fmt, sizeof(color_fmt), "Unknown");
		break;
	}

	count += sprintf(buf + count, "color_fmt: %s", color_fmt);

	if (edp_core->ssc_en) {
		count += sprintf(buf + count, "ssc_enable: %s\n",\
			edp_core_ssc_is_enabled(sel) ? "YES" : "No");
		if (!edp_core_ssc_is_enabled(sel))
			count += sprintf(buf + count, "\t\tssc_mode: Null\n");
		else
			count += sprintf(buf + count, "\t\tssc_mode: %s\n", \
				 edp_core_ssc_get_mode(sel) ? "Downspread" : "Center");
	} else
		count += sprintf(buf + count, "ssc is not support!\n");


	if (edp_core->psr_en) {
		count += sprintf(buf + count, "psr_enable: %s",\
				 edp_core_psr_is_enabled(sel) ? "Yes" : "No");
	} else
		count += sprintf(buf + count, "psr is not support!\n");

	count += sprintf(buf + count, "\t\tpixel_clock: %d",\
			 edp_core_get_pixclk(sel));
	count += sprintf(buf + count, "\t\ttrain_pattern: TPS%d\n",\
			 edp_core_get_train_pattern(sel));

	edp_core_get_lane_para(sel, &tmp_lane_para);
	count += sprintf(buf + count, "eDP lane para:\n");
	count += sprintf(buf + count, "bit_rate: %lld",\
			 tmp_lane_para.bit_rate);
	count += sprintf(buf + count, "\t\tlane_cnt: %d\n",\
			 tmp_lane_para.lane_cnt);
	count += sprintf(buf + count, "Voltage Swing0: Level%d",\
			 tmp_lane_para.lane0_sw);
	count += sprintf(buf + count, "\t\tPre_emphasis0: Level%d\n",\
			 tmp_lane_para.lane0_pre);
	count += sprintf(buf + count, "Voltage Swing1: Level%d",\
			 tmp_lane_para.lane1_sw);
	count += sprintf(buf + count, "\t\tPre_emphasis1: Level%d\n",\
			 tmp_lane_para.lane1_pre);
	count += sprintf(buf + count, "Voltage Swing2: Level%d",\
			 tmp_lane_para.lane2_sw);
	count += sprintf(buf + count, "\t\tPre_emphasis2: Level%d\n",\
			 tmp_lane_para.lane2_pre);
	count += sprintf(buf + count, "Voltage Swing3: Level%d",\
			 tmp_lane_para.lane3_sw);
	count += sprintf(buf + count, "\t\tPre_emphasis3: Level%d\n",\
			 tmp_lane_para.lane3_pre);

	count += sprintf(buf + count, "Transfer_Unit:\n");
	tu_size = edp_core_get_tu_size(sel);
	count += sprintf(buf + count, "tu_size: %d.%d",\
			 tu_size / 10, tu_size % 10);
	count += sprintf(buf + count, "\t\tlink_symbol_rate: %d",\
			 edp_core_get_symbol_rate(sel));

	count += sprintf(buf + count, "eDP Audio:\n");
	if (!edp_core_audio_is_enabled(sel)) {
		count += sprintf(buf + count, "eDP audio disabled!\n");
		return count;
	}

	count += sprintf(buf + count, "interface: %s", edp_core_get_audio_if(sel) ?\
				 "I2S" : "SPDIF");
	count += sprintf(buf + count, "\t\tmute: %s", edp_core_audio_is_mute(sel) ?\
				 "Yes" : "No");
	count += sprintf(buf + count, "\t\tchannel_cnt: %d",\
			 edp_core_get_audio_chn_cnt(sel));
	count += sprintf(buf + count, "\t\tdata_width: %d bits\n",\
			 edp_core_get_audio_date_width(sel));

	return count;
}

static ssize_t aux_read_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	return sprintf(buf, "%s\n", "echo [0x(address), 0x(length)] > aux_read");
}

static ssize_t aux_read_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	unsigned long start_reg = 0;
	unsigned long end_reg = 0;
	unsigned long len;
	struct platform_device *pdev = dev_get_drvdata(dev);
	u32 sel = pdev->id;
	u32 i, times, show_cnt = 0;
	char tmp_rx_buf[256];
	char *show_buf = NULL;

	if (__parse_dump_str(buf, count, &start_reg, &end_reg)) {
		EDP_ERR("%s,%d err, invalid para!\n", __func__, __LINE__);
		return count;
	}

	if (end_reg < start_reg) {
		EDP_ERR("%s,%d err, addresss syntax error!\n", __func__, __LINE__);
		return count;
	}

	if (start_reg > 0x70000) {
		EDP_ERR("%s,%d err, addresss out of range define in eDP spec!\n", __func__, __LINE__);
		return count;
	}

	EDP_DBG("start_reg=0x%lx end_reg=0x%lx\n", start_reg, end_reg);

	len = end_reg - start_reg;
	if (len > 256)
		EDP_ERR("%s,%d err, out of length, length should < 256!\n", __func__, __LINE__);

	memset(tmp_rx_buf, 0, sizeof(tmp_rx_buf));
	if (edp_core_aux_read(sel, start_reg, len, tmp_rx_buf) < 0) {
		EDP_ERR("aux read fail!\n");
		return count;
	}

	if ((start_reg % 0x10) == 0)
		times = 1;

	for (i = 0; i < len; i++) {
		if ((times == 0) && (((start_reg + i) % 0x10) != 0)) {
			show_cnt += sprintf(show_buf + show_cnt, "\n0x%08lx:", (start_reg + i));
			times = 1;
		}

		if (((start_reg+ i) % 0x10) == 0)
			show_cnt += sprintf(show_buf + show_cnt, "\n0x%08lx:", (start_reg + i));
		show_cnt += sprintf(show_buf + show_cnt, "  %x", tmp_rx_buf[i]);
	}

	show_cnt += sprintf(show_buf + show_cnt, "\n");

	printk("%s\n", show_buf);

	return count;
}

static ssize_t aux_write_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	return sprintf(buf, "%s\n", "echo [0x(address) 0x(value)] > aux_write");
}

static ssize_t aux_write_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	struct platform_device *pdev = dev_get_drvdata(dev);
	u32 sel = pdev->id;
	u8 regval_before;
	u8 regval_after;
	u8 value = 0;
	u8 *separator = NULL;
	char tmp_tx_buf[2];
	char tmp_rx_buf[2];
	unsigned long reg_addr = 0;

	separator = strchr(buf, ' ');
	if (separator == NULL) {
		EDP_ERR("%s,%d err, syntax error!\n", __func__, __LINE__);
	} else {
		reg_addr = simple_strtoul(buf, NULL, 0);
		value = simple_strtoul(separator + 1, NULL, 0);
		tmp_tx_buf[0] = value;

		EDP_DBG("reg_addr=0x%lx  write_value=0x%x\n", reg_addr, value);

		memset(tmp_rx_buf, 0, sizeof(tmp_rx_buf));
		edp_core_aux_read(sel, reg_addr, 1, tmp_rx_buf);
		regval_before = tmp_rx_buf[0];

		mdelay(1);
		edp_core_aux_write(sel, reg_addr, 1, tmp_tx_buf);

		mdelay(1);
		memset(tmp_rx_buf, 0, sizeof(tmp_rx_buf));
		edp_core_aux_read(sel, reg_addr, 1, tmp_rx_buf);
		regval_after = tmp_rx_buf[0];

		printk("reg[0x%lx]: before_write:0x%x  wants_write:0x%x  after_write:0x%x\n",\
				reg_addr, regval_before, value, regval_after);
	}

	return count;

}

static ssize_t lane_config_now_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	struct edp_tx_core *edp_core;
	struct edp_lane_para *lane_para;
	struct platform_device *pdev = dev_get_drvdata(dev);
	u32 sel = pdev->id;
	u32 count = 0;

	edp_core = &g_edp_info[sel].edp_core;
	lane_para = &edp_core->lane_para;

	if (g_edp_info[sel].use_def_para)
		count += sprintf(buf + count, "lane_para_use: default\n");
	else if (g_edp_info[sel].use_recom_para)
		count += sprintf(buf + count, "lane_para_use: recommand\n");
	else
		count += sprintf(buf + count, "lane_para_use: unknown\n");

	count += sprintf(buf + count, "bit_rate: %lld", lane_para->bit_rate);
	count += sprintf(buf + count, "\t\tbit_rate: %d", lane_para->lane_cnt);
	count += sprintf(buf + count, "\t\tfps: %d", lane_para->fps);
	count += sprintf(buf + count, "\t\tcolordepth: %d\n", lane_para->colordepth);
	count += sprintf(buf + count, "lane0_sw: %d", lane_para->lane0_sw);
	count += sprintf(buf + count, "\t\tlane1_sw: %d", lane_para->lane1_sw);
	count += sprintf(buf + count, "\t\tlane2_sw: %d", lane_para->lane2_sw);
	count += sprintf(buf + count, "\t\tlane3_sw: %d\n", lane_para->lane3_sw);
	count += sprintf(buf + count, "lane0_pre: %d", lane_para->lane0_pre);
	count += sprintf(buf + count, "\t\tlane1_pre: %d", lane_para->lane1_pre);
	count += sprintf(buf + count, "\t\tlane2_pre: %d", lane_para->lane2_pre);
	count += sprintf(buf + count, "\t\tlane3_pre: %d\n", lane_para->lane3_pre);

	return count;
}

static ssize_t lane_config_def_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	struct edp_tx_core *edp_core;
	struct edp_lane_para *lane_para;
	struct platform_device *pdev = dev_get_drvdata(dev);
	u32 sel = pdev->id;
	u32 count = 0;

	edp_core = &g_edp_info[sel].edp_core;
	lane_para = &edp_core->def_lane_para;

	count += sprintf(buf + count, "bit_rate: %lld", lane_para->bit_rate);
	count += sprintf(buf + count, "\t\tbit_rate: %d", lane_para->lane_cnt);
	count += sprintf(buf + count, "\t\tfps: %d", lane_para->fps);
	count += sprintf(buf + count, "\t\tcolordepth: %d\n", lane_para->colordepth);
	count += sprintf(buf + count, "lane0_sw: %d", lane_para->lane0_sw);
	count += sprintf(buf + count, "\t\tlane1_sw: %d", lane_para->lane1_sw);
	count += sprintf(buf + count, "\t\tlane2_sw: %d", lane_para->lane2_sw);
	count += sprintf(buf + count, "\t\tlane3_sw: %d\n", lane_para->lane3_sw);
	count += sprintf(buf + count, "lane0_pre: %d", lane_para->lane0_pre);
	count += sprintf(buf + count, "\t\tlane1_pre: %d", lane_para->lane1_pre);
	count += sprintf(buf + count, "\t\tlane2_pre: %d", lane_para->lane2_pre);
	count += sprintf(buf + count, "\t\tlane3_pre: %d\n", lane_para->lane3_pre);

	return count;
}

static ssize_t lane_config_recom_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	struct edp_tx_core *edp_core;
	struct edp_lane_para *lane_para;
	struct platform_device *pdev = dev_get_drvdata(dev);
	u32 sel = pdev->id;
	u32 count = 0;

	edp_core = &g_edp_info[sel].edp_core;
	lane_para = &edp_core->recom_lane_para;

	count += sprintf(buf + count, "bit_rate: %lld", lane_para->bit_rate);
	count += sprintf(buf + count, "\t\tbit_rate: %d", lane_para->lane_cnt);
	count += sprintf(buf + count, "\t\tfps: %d", lane_para->fps);
	count += sprintf(buf + count, "\t\tcolordepth: %d\n", lane_para->colordepth);
	count += sprintf(buf + count, "lane0_sw: %d", lane_para->lane0_sw);
	count += sprintf(buf + count, "\t\tlane1_sw: %d", lane_para->lane1_sw);
	count += sprintf(buf + count, "\t\tlane2_sw: %d", lane_para->lane2_sw);
	count += sprintf(buf + count, "\t\tlane3_sw: %d\n", lane_para->lane3_sw);
	count += sprintf(buf + count, "lane0_pre: %d", lane_para->lane0_pre);
	count += sprintf(buf + count, "\t\tlane1_pre: %d", lane_para->lane1_pre);
	count += sprintf(buf + count, "\t\tlane2_pre: %d", lane_para->lane2_pre);
	count += sprintf(buf + count, "\t\tlane3_pre: %d\n", lane_para->lane3_pre);

	return count;
}

static ssize_t timings_now_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	struct edp_tx_core *edp_core;
	struct disp_video_timings *timings;
	struct platform_device *pdev = dev_get_drvdata(dev);
	u32 sel = pdev->id;
	u32 count = 0;

	edp_core = &g_edp_info[sel].edp_core;
	timings = &edp_core->timings;

	if (g_edp_info[sel].use_def_timings)
		count += sprintf(buf + count, "timing_use: default\n");
	else if (g_edp_info[sel].use_user_timings)
		count += sprintf(buf + count, "timing_use: user\n");
	else if (g_edp_info[sel].use_edid_timings)
		count += sprintf(buf + count, "timing_use: edid\n");
	else
		count += sprintf(buf + count, "timing_use: unkonw\n");


	count += sprintf(buf + count, "vic: %d\n", timings->vic);
	count += sprintf(buf + count, "tv_mode: %d\n", timings->tv_mode);
	count += sprintf(buf + count, "pixel_clk: %d\n", timings->pixel_clk);
	count += sprintf(buf + count, "pixel_repeat: %d\n", timings->pixel_repeat);
	count += sprintf(buf + count, "x_res: %d\n", timings->x_res);
	count += sprintf(buf + count, "y_res: %d\n", timings->y_res);
	count += sprintf(buf + count, "hor_total_time: %d\n", timings->hor_total_time);
	count += sprintf(buf + count, "hor_back_porch: %d\n", timings->hor_back_porch);
	count += sprintf(buf + count, "hor_front_porch: %d\n", timings->hor_front_porch);
	count += sprintf(buf + count, "hor_sync_time: %d\n", timings->hor_sync_time);
	count += sprintf(buf + count, "ver_total_time: %d\n", timings->ver_total_time);
	count += sprintf(buf + count, "ver_back_porch: %d\n", timings->ver_back_porch);
	count += sprintf(buf + count, "ver_front_porch: %d\n", timings->ver_front_porch);
	count += sprintf(buf + count, "ver_sync_time: %d\n", timings->ver_sync_time);
	count += sprintf(buf + count, "hor_sync_polarity: %d\n", timings->hor_sync_polarity);
	count += sprintf(buf + count, "ver_sync_polarity: %d\n", timings->ver_sync_polarity);
	count += sprintf(buf + count, "b_interlace: %d\n", timings->b_interlace);
	count += sprintf(buf + count, "trd_mode: %d\n", timings->trd_mode);
	count += sprintf(buf + count, "dclk_rate_set: %ld\n", timings->dclk_rate_set);
	count += sprintf(buf + count, "frame_period: %lld\n", timings->frame_period);
	count += sprintf(buf + count, "start_delay: %d\n", timings->start_delay);

	return count;
}

static ssize_t timings_default_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	struct edp_tx_core *edp_core;
	struct disp_video_timings *timings;
	struct platform_device *pdev = dev_get_drvdata(dev);
	u32 sel = pdev->id;
	u32 count = 0;

	edp_core = &g_edp_info[sel].edp_core;
	timings = &edp_core->def_timings;

	count += sprintf(buf + count, "vic: %d\n", timings->vic);
	count += sprintf(buf + count, "tv_mode: %d\n", timings->tv_mode);
	count += sprintf(buf + count, "pixel_clk: %d\n", timings->pixel_clk);
	count += sprintf(buf + count, "pixel_repeat: %d\n", timings->pixel_repeat);
	count += sprintf(buf + count, "x_res: %d\n", timings->x_res);
	count += sprintf(buf + count, "y_res: %d\n", timings->y_res);
	count += sprintf(buf + count, "hor_total_time: %d\n", timings->hor_total_time);
	count += sprintf(buf + count, "hor_back_porch: %d\n", timings->hor_back_porch);
	count += sprintf(buf + count, "hor_front_porch: %d\n", timings->hor_front_porch);
	count += sprintf(buf + count, "hor_sync_time: %d\n", timings->hor_sync_time);
	count += sprintf(buf + count, "ver_total_time: %d\n", timings->ver_total_time);
	count += sprintf(buf + count, "ver_back_porch: %d\n", timings->ver_back_porch);
	count += sprintf(buf + count, "ver_front_porch: %d\n", timings->ver_front_porch);
	count += sprintf(buf + count, "ver_sync_time: %d\n", timings->ver_sync_time);
	count += sprintf(buf + count, "hor_sync_polarity: %d\n", timings->hor_sync_polarity);
	count += sprintf(buf + count, "ver_sync_polarity: %d\n", timings->ver_sync_polarity);
	count += sprintf(buf + count, "b_interlace: %d\n", timings->b_interlace);
	count += sprintf(buf + count, "trd_mode: %d\n", timings->trd_mode);
	count += sprintf(buf + count, "dclk_rate_set: %ld\n", timings->dclk_rate_set);
	count += sprintf(buf + count, "frame_period: %lld\n", timings->frame_period);
	count += sprintf(buf + count, "start_delay: %d\n", timings->start_delay);

	return count;
}

static ssize_t timings_edid_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	struct edp_tx_core *edp_core;
	struct disp_video_timings *timings;
	struct platform_device *pdev = dev_get_drvdata(dev);
	u32 sel = pdev->id;
	u32 count = 0;

	edp_core = &g_edp_info[sel].edp_core;
	timings = &edp_core->edid_timings;

	count += sprintf(buf + count, "vic: %d\n", timings->vic);
	count += sprintf(buf + count, "tv_mode: %d\n", timings->tv_mode);
	count += sprintf(buf + count, "pixel_clk: %d\n", timings->pixel_clk);
	count += sprintf(buf + count, "pixel_repeat: %d\n", timings->pixel_repeat);
	count += sprintf(buf + count, "x_res: %d\n", timings->x_res);
	count += sprintf(buf + count, "y_res: %d\n", timings->y_res);
	count += sprintf(buf + count, "hor_total_time: %d\n", timings->hor_total_time);
	count += sprintf(buf + count, "hor_back_porch: %d\n", timings->hor_back_porch);
	count += sprintf(buf + count, "hor_front_porch: %d\n", timings->hor_front_porch);
	count += sprintf(buf + count, "hor_sync_time: %d\n", timings->hor_sync_time);
	count += sprintf(buf + count, "ver_total_time: %d\n", timings->ver_total_time);
	count += sprintf(buf + count, "ver_back_porch: %d\n", timings->ver_back_porch);
	count += sprintf(buf + count, "ver_front_porch: %d\n", timings->ver_front_porch);
	count += sprintf(buf + count, "ver_sync_time: %d\n", timings->ver_sync_time);
	count += sprintf(buf + count, "hor_sync_polarity: %d\n", timings->hor_sync_polarity);
	count += sprintf(buf + count, "ver_sync_polarity: %d\n", timings->ver_sync_polarity);
	count += sprintf(buf + count, "b_interlace: %d\n", timings->b_interlace);
	count += sprintf(buf + count, "trd_mode: %d\n", timings->trd_mode);
	count += sprintf(buf + count, "dclk_rate_set: %ld\n", timings->dclk_rate_set);
	count += sprintf(buf + count, "frame_period: %lld\n", timings->frame_period);
	count += sprintf(buf + count, "start_delay: %d\n", timings->start_delay);

	return count;
}

static ssize_t timings_user_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	struct edp_tx_core *edp_core;
	struct disp_video_timings *timings;
	struct platform_device *pdev = dev_get_drvdata(dev);
	u32 sel = pdev->id;
	u32 count = 0;
	s32 mode;

	mode = edp_get_mode(sel);
	if (mode >= DISP_TV_MODE_NUM) {
		count += sprintf(buf + count, "user timing is not set now!\n");
		return count;
	}

	edp_core = &g_edp_info[sel].edp_core;
	timings = &edp_core->edid_timings;

	edp_mode_to_timings_transfer(mode, timings);

	count += sprintf(buf + count, "vic: %d\n", timings->vic);
	count += sprintf(buf + count, "tv_mode: %d\n", timings->tv_mode);
	count += sprintf(buf + count, "pixel_clk: %d\n", timings->pixel_clk);
	count += sprintf(buf + count, "pixel_repeat: %d\n", timings->pixel_repeat);
	count += sprintf(buf + count, "x_res: %d\n", timings->x_res);
	count += sprintf(buf + count, "y_res: %d\n", timings->y_res);
	count += sprintf(buf + count, "hor_total_time: %d\n", timings->hor_total_time);
	count += sprintf(buf + count, "hor_back_porch: %d\n", timings->hor_back_porch);
	count += sprintf(buf + count, "hor_front_porch: %d\n", timings->hor_front_porch);
	count += sprintf(buf + count, "hor_sync_time: %d\n", timings->hor_sync_time);
	count += sprintf(buf + count, "ver_total_time: %d\n", timings->ver_total_time);
	count += sprintf(buf + count, "ver_back_porch: %d\n", timings->ver_back_porch);
	count += sprintf(buf + count, "ver_front_porch: %d\n", timings->ver_front_porch);
	count += sprintf(buf + count, "ver_sync_time: %d\n", timings->ver_sync_time);
	count += sprintf(buf + count, "hor_sync_polarity: %d\n", timings->hor_sync_polarity);
	count += sprintf(buf + count, "ver_sync_polarity: %d\n", timings->ver_sync_polarity);
	count += sprintf(buf + count, "b_interlace: %d\n", timings->b_interlace);
	count += sprintf(buf + count, "trd_mode: %d\n", timings->trd_mode);
	count += sprintf(buf + count, "dclk_rate_set: %ld\n", timings->dclk_rate_set);
	count += sprintf(buf + count, "frame_period: %lld\n", timings->frame_period);
	count += sprintf(buf + count, "start_delay: %d\n", timings->start_delay);

	return count;
}


static DEVICE_ATTR(dpcd, 0664, dpcd_show, NULL);
static DEVICE_ATTR(edid, 0664, edid_show, NULL);
static DEVICE_ATTR(hotplug, 0664, hotplug_show, NULL);
static DEVICE_ATTR(sink_info, 0664, sink_info_show, NULL);
static DEVICE_ATTR(source_info, 0664, source_info_show, NULL);
static DEVICE_ATTR(aux_read, 0664, aux_read_show, aux_read_store);
static DEVICE_ATTR(aux_write, 0664, aux_write_show, aux_write_store);
static DEVICE_ATTR(lane_config_now, 0664, lane_config_now_show, NULL);
static DEVICE_ATTR(lane_config_def, 0664, lane_config_def_show, NULL);
static DEVICE_ATTR(lane_config_recom, 0664, lane_config_recom_show, NULL);
static DEVICE_ATTR(timings_now, 0664, timings_now_show, NULL);
static DEVICE_ATTR(timings_default, 0664, timings_default_show, NULL);
static DEVICE_ATTR(timings_edid, 0664, timings_edid_show, NULL);
static DEVICE_ATTR(timings_user, 0664, timings_user_show, NULL);

static struct attribute *edp_attributes[] = {
	&dev_attr_dpcd.attr,
	&dev_attr_edid.attr,
	&dev_attr_hotplug.attr,
	&dev_attr_sink_info.attr,
	&dev_attr_source_info.attr,
	&dev_attr_aux_read.attr,
	&dev_attr_aux_write.attr,
	&dev_attr_lane_config_now.attr,
	&dev_attr_lane_config_def.attr,
	&dev_attr_lane_config_recom.attr,
	&dev_attr_timings_now.attr,
	&dev_attr_timings_default.attr,
	&dev_attr_timings_edid.attr,
	&dev_attr_timings_user.attr,
	NULL
};

static struct attribute_group edp_attribute_group = {
	.name = "attr",
	.attrs = edp_attributes
};




static s32 edp_open(struct inode *inode, struct file *filp)
{
	return -EINVAL;
}

static s32 edp_release(struct inode *inode, struct file *filp)
{
	return -EINVAL;
}

static ssize_t edp_read(struct file *file, char __user *buf,
						size_t count,
						loff_t *ppos)
{
	return -EINVAL;
}

static ssize_t edp_write(struct file *file, const char __user *buf,
						size_t count,
						loff_t *ppos)
{
	return -EINVAL;
}

static s32 edp_mmap(struct file *filp, struct vm_area_struct *vma)
{
	return -EINVAL;
}

static long edp_ioctl(struct file *filp, u32 cmd, unsigned long arg)
{
	return -EINVAL;
}

#ifdef CONFIG_COMPAT
static long edp_compat_ioctl(struct file *filp, u32 cmd,
						unsigned long arg)
{
	return -EINVAL;
}
#endif

static const struct file_operations edp_fops = {
	.owner		= THIS_MODULE,
	.open		= edp_open,
	.release	= edp_release,
	.write		= edp_write,
	.read		= edp_read,
	.unlocked_ioctl	= edp_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl	= edp_compat_ioctl,
#endif
	.mmap		= edp_mmap,
};


s32 snd_edp_get_func(__edp_audio_func *edp_func)
{
	edp_func->edp_audio_enable = edp_core_audio_enable;
	edp_func->edp_audio_disable = edp_core_audio_disable;
	edp_func->edp_audio_set_para = edp_core_audio_set_para;

	return RET_OK;
}
EXPORT_SYMBOL_GPL(snd_edp_get_func);

static void edp_hotplugin_proc(u32 sel)
{
	s32 ret;
	struct edp_tx_core *edp_core;
	struct edid *edid;

	edp_core = &g_edp_info[sel].edp_core;
	edid = edp_core->edid;

	edid = edp_edid_get(sel);
	if (edid == NULL)
		EDP_ERR("fail to read edid\n");

	edp_parse_edid(sel, edid);
	edp_correct_timings(sel);

	memset(dpcd_rx_buf, 0, sizeof(dpcd_rx_buf));
	ret = edp_read_dpcd(sel, dpcd_rx_buf);
	if (ret < 0)
		EDP_WRN("fail to read edp dpcd!\n");

	if (dpcd_rx_buf == NULL)
		EDP_WRN("Sink DPCD can't be read correctly!\n");
	edp_parse_dpcd(sel, dpcd_rx_buf);
	edp_correct_lane_para(sel);
}

static void edp_hotplugout_proc(u32 sel)
{
	struct edp_tx_core *edp_core;
	struct edid *edid;

	edp_core = &g_edp_info[sel].edp_core;
	edid = edp_core->edid;

	edp_edid_put(edid);
	edid_cap_reset(sel);
}


s32 edp_running_thread(void *parg)
{
	s32 hpd_state_now[EDP_NUM_MAX];
	struct edp_info_t *p_edp_info = NULL;
	u32 sel = 0;
	int i;

	if (!parg) {
		EDP_WRN("NUll ndl\n");
		return -1;
	}

	p_edp_info = (struct edp_info_t *)parg;
	sel = p_edp_info->dev->id;

	while (1) {
		if (kthread_should_stop())
			break;

		set_current_state(TASK_INTERRUPTIBLE);
		schedule_timeout(200);

		if (!p_edp_info->suspend) {
			hpd_state_now[sel] = edp_core_get_hpd_status(sel);
			if (hpd_state_now[sel] != hpd_state[sel]) {
				/*manual debounce*/
				for (i = 0; i < 2; i++) {
					msleep(50);
					hpd_state_now[sel] = edp_core_get_hpd_status(sel);
					if (hpd_state_now[sel] == hpd_state[sel])
						break;
				}

				if (hpd_state_now[sel] != hpd_state[sel]) {
					if (!hpd_state_now[sel])
						edp_hotplugout_proc(sel);
					else
						edp_hotplugin_proc(sel);

					EDP_DBG("hpd[%d] = %d\n", sel, hpd_state_now[sel]);
					edp_report_hpd_work(sel, hpd_state_now[sel]);
					hpd_state[sel] = hpd_state_now[sel];
				}
			}
		}
	}
	return 0;
}

s32 edp_kthread_start(u32 sel)
{
	s32 err = 0;

	if (!edp_task[sel]) {
		edp_task[sel] = kthread_create(edp_running_thread,
					      &(g_edp_info[sel]),
					      "edp detect");
		if (IS_ERR(edp_task[sel])) {
			err = PTR_ERR(edp_task[sel]);
			edp_task[sel] = NULL;
			return err;
		}
		EDP_DBG("edp_task is ok!\n");
		wake_up_process(edp_task[sel]);
	}
	return 0;
}

s32 edp_kthread_stop(u32 sel)
{
	if (edp_task[sel]) {
		kthread_stop(edp_task[sel]);
		edp_task[sel] = NULL;
	}
	return 0;
}


/**
 * @name       edp_disable
 * @brief      disable edp module
 * @param[IN]  sel:index of edp
 * @param[OUT] none
 * @return     0 if success
 */
s32 edp_disable(u32 sel)
{
	s32 ret = 0;

	if (g_edp_info[sel].enable) {
		edp_core_disable(sel, &g_edp_info[sel].edp_core);
		mutex_lock(&g_edp_info[sel].mlock);
		g_edp_info[sel].enable = 0;
		mutex_unlock(&g_edp_info[sel].mlock);
	}
	return ret;
}

/**
 * @name       edp_enable
 * @brief      edp enable
 * @param[IN]  sel index of edp
 * @return     0 if success
 */
s32 edp_enable(u32 sel)
{
	s32 ret = 0;

	if (hpd_state[sel]) {
		EDP_WRN("sink device unconnect!\n");
		goto OUT;
	}

	if (!sink_is_edp(sel)) {
		EDP_WRN("Sink device is not an edp device, please check it out!\n");
		goto OUT;
	}


	if (g_edp_info[sel].training_done)
		edp_recommaend_lane_config(sel);

	if (!g_edp_info[sel].enable) {
		edp_core_set_video_mode(sel, &g_edp_info[sel].edp_core);
		ret = edp_core_enable(sel, &g_edp_info[sel].edp_core);
		if (ret < 0)
			goto OUT;

		mutex_lock(&g_edp_info[sel].mlock);
		g_edp_info[sel].enable = 1;
		mutex_unlock(&g_edp_info[sel].mlock);

		if (!g_edp_info[sel].training_done) {
			/* get recommend lane para after tarining done*/
			ret = edp_core_get_training_info(sel, &g_edp_info[sel].edp_core);
			if (ret) {
				EDP_ERR("get training info fail\n");
				goto OUT;
			}

			mutex_lock(&g_edp_info[sel].mlock);
			g_edp_info[sel].training_done = true;
			mutex_unlock(&g_edp_info[sel].mlock);
		}
	} else
		EDP_WRN("edp already enabled!\n");

	return RET_OK;

OUT:
	EDP_ERR("edp fail to enable!\n");
	return ret;
}

s32 edp_resume(u32 sel)
{
	s32 ret = 0;

	if (g_edp_info[sel].suspend) {
		if (!IS_ERR_OR_NULL(g_edp_info[sel].regulator))
			ret = regulator_enable(g_edp_info[sel].regulator);
		edp_clk_enable(sel, true);
		mutex_lock(&g_edp_info[sel].mlock);
		g_edp_info[sel].suspend = false;
		mutex_unlock(&g_edp_info[sel].mlock);

		/*phy need re-init?*/
		edp_hardware_reset(sel);
		ret = edp_core_phy_init(sel, &g_edp_info[sel].edp_core);
		if (ret < 0)
			EDP_WRN("edp phy init fail!\n");

		if (!g_edp_info[sel].enable)
			edp_enable(sel);

		//if (g_edp_info[sel].training_done)
		//	edp_recommaend_lane_config(sel);
		//edp_core_set_video_mode(sel, &g_edp_info[sel].edp_core);
		//ret = edp_core_enable(sel, &g_edp_info[sel].edp_core);

		edp_kthread_start(sel);
	}
	return ret;
}

s32 edp_suspend(u32 sel)
{
	s32 ret = -1;

	if (!g_edp_info[sel].suspend) {
		mutex_lock(&g_edp_info[sel].mlock);
		g_edp_info[sel].suspend = true;
		mutex_unlock(&g_edp_info[sel].mlock);
		edp_kthread_stop(sel);
		ret = edp_clk_enable(sel, false);

		if (!IS_ERR_OR_NULL(g_edp_info[sel].regulator))
			ret = regulator_disable(g_edp_info[sel].regulator);

		mutex_lock(&g_edp_info[sel].mlock);
		g_edp_info[sel].training_done = false;
		g_edp_info[sel].enable = 0;
		mutex_unlock(&g_edp_info[sel].mlock);
	}
	return ret;
}

static s32 edp_probe(struct platform_device *pdev)
{
	struct disp_tv_func edp_func;
	s32 ret = -1;
	char edp_dev_name[10];

	EDP_DBG("edp_probe: %d\n", g_edp_num);

	if (!g_edp_num)
		memset(&g_edp_info, 0,
		       sizeof(struct edp_info_t) * EDP_NUM_MAX);

	if (g_edp_num > EDP_NUM_MAX - 1) {
		EDP_DEV_ERR(&pdev->dev,
			"g_edp_num(%d) is greater then EDP_NUM_MAX-1(%d)\n",
			g_edp_num, EDP_NUM_MAX - 1);
		goto OUT;
	}

	ret = edp_resource_init(pdev);
	if (ret < 0)
		goto OUT;

	ret = edp_init(pdev);
	if (ret) {
		EDP_DEV_ERR(&pdev->dev, "edp_init for edp%d fail!\n", pdev->id);
		goto OUT;
	}

	ret = edp_kthread_start(pdev->id);
	if (ret) {
		EDP_DEV_ERR(&pdev->dev, "edp_kthread_start fail!\n");
		goto OUT;
	}

	if (!g_edp_num) {
		memset(&edp_func, 0, sizeof(struct disp_tv_func));
		edp_func.tv_enable = edp_enable;
		edp_func.tv_disable = edp_disable;
		edp_func.tv_resume = edp_resume;
		edp_func.tv_suspend = edp_suspend;
		edp_func.tv_set_mode = edp_set_mode;
		edp_func.tv_get_mode = edp_get_mode;
		edp_func.tv_mode_support = edp_mode_support;
		edp_func.tv_get_video_timing_info = edp_get_video_timing_info;
		edp_func.tv_irq_enable = edp_irq_enable;
		edp_func.tv_irq_query = edp_irq_query;
		/*fixme: not sure*/
		/*edp_func.tv_irq_handler = edp_irq_handler;*/
		edp_func.tv_get_cur_line = edp_get_cur_line;
		edp_func.tv_get_startdelay = edp_get_start_delay;
		edp_func.tv_show_builtin_patten = edp_show_builtin_patten;
		ret = disp_set_edp_func(&edp_func);
		if (ret) {
			EDP_DEV_ERR(&pdev->dev, "disp_set_edp_func edp%d fail!\n",
				pdev->id);
			goto ERR_KTHREAD;
		}
	} else
		ret = 0;

	snprintf(edp_dev_name, sizeof(edp_dev_name), "edp%d", pdev->id);

	/*Create and add a character device*/
	alloc_chrdev_region(&devid[pdev->id], 0, 1, edp_dev_name);/*corely for device number*/

	edp_cdev[pdev->id] = cdev_alloc();

	cdev_init(edp_cdev[pdev->id], &edp_fops);

	edp_cdev[pdev->id]->owner = THIS_MODULE;
	ret = cdev_add(edp_cdev[pdev->id], devid[pdev->id], 1);/*/proc/device/edp*/
	if (ret) {
		EDP_ERR("edp%d cdev_add fail!\n", pdev->id);
		ret = RET_FAIL;
		goto ERR_KTHREAD;
	}

	/*Create a path: sys/class/edp*/
	edp_class[pdev->id] = class_create(THIS_MODULE, edp_dev_name);
	if (IS_ERR(edp_class[pdev->id])) {
		EDP_ERR("edp%d class_create fail\n", pdev->id);
		ret = RET_FAIL;
		goto ERR_CDEV;
	}

	/*Create a path "sys/class/edp/edp"*/
	edp_dev[pdev->id] = device_create(edp_class[pdev->id], NULL, devid[pdev->id], NULL, edp_dev_name);
	if (IS_ERR(edp_dev[pdev->id])) {
		EDP_ERR("edp%d device_create fail\n", pdev->id);
		ret = RET_FAIL;
		goto ERR_CLASS;
	}

	/*Create a path: sys/class/edp/edp/attr*/
	ret = sysfs_create_group(&edp_dev[pdev->id]->kobj, &edp_attribute_group);
	if (ret) {
		EDP_ERR("edp sysfs_create_group failed!\n");
		ret = RET_FAIL;
		goto ERR_DEV;
	}

	dev_set_drvdata(edp_dev[pdev->id], pdev);

	++g_edp_num;

	return RET_OK;

ERR_DEV:
	device_destroy(edp_class[pdev->id], devid[pdev->id]);
ERR_CLASS:
	class_destroy(edp_class[pdev->id]);
ERR_CDEV:
	cdev_del(edp_cdev[pdev->id]);
ERR_KTHREAD:
	edp_kthread_stop(pdev->id);
OUT:
	return ret;
}

s32 edp_remove(struct platform_device *pdev)
{
	s32 ret = 0;
	u32 i = 0;

	for (i = 0; i < g_edp_num; ++i) {
		edp_kthread_stop(i);
		edp_disable(i);
		edp_clk_enable(i, false);

		if (!IS_ERR_OR_NULL(g_edp_info[pdev->id].regulator)) {
			regulator_disable(g_edp_info[i].regulator);
			regulator_put(g_edp_info[i].regulator);
		}
	}

	sysfs_remove_group(&edp_dev[pdev->id]->kobj, &edp_attribute_group);
	device_destroy(edp_class[pdev->id], devid[pdev->id]);

	class_destroy(edp_class[pdev->id]);
	cdev_del(edp_cdev[pdev->id]);

	return ret;
}

static const struct of_device_id sunxi_edp_match[] = {
	{
		.compatible = "allwinner,sunxi-edp0",
	},
	{
		.compatible = "allwinner,sunxi-edp1",
	},
	{},
};

static struct platform_driver edp_driver = {
	.probe = edp_probe,
	.remove = edp_remove,
	.driver = {
		.name = "edp",
		.owner = THIS_MODULE,
		.of_match_table = sunxi_edp_match,
	},
};

s32 __init edp_module_init(void)
{
	s32 ret = 0;


	ret = platform_driver_register(&edp_driver);

	return ret;
}

static void __exit edp_module_exit(void)
{
	platform_driver_unregister(&edp_driver);
}


late_initcall(edp_module_init);
module_exit(edp_module_exit);

MODULE_AUTHOR("huangyongxing@allwinnertech.com");
MODULE_DESCRIPTION("edp driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:edp");
/*End of File*/
