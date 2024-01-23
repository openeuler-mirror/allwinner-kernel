 /*
 * Copyright (c) 2007-2022 Allwinnertech Co., Ltd.
 * Author: huangyongxing <huangyongxing@allwinnertech.com>
 *
 * api for inno edp tx based on edp_1.3 hardware operation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include "inno_edp13_lowlevel.h"
#include "edp_lowlevel.h"
#include "../edp_core/edp_core.h"
#include "../edp_configs.h"
#include <linux/pinctrl/consumer.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <video/sunxi_edp.h>

/*link symbol per TU*/
#define LS_PER_TU 64

static void __iomem *edp_base[2];
static char edid_tx_buf[16];
static u32 g_bpp;
static u32 g_lane_cnt;
static u32 training_interval_EQ;
static u32 training_interval_CR;
static char g_tx_buf[16];
static char g_rx_buf[16];

struct training_para_recommend recom_training_para;

static struct recommand_corepll recom_corepll[] = {
	/* 1.62G*/
	{
		.prediv = 0x2,
		.fbdiv_h4 = 0x0,
		.fbdiv_l8 = 0x87,
		.postdiv = 0x1,
		.frac_pd = 0x3,
		.frac_h8 = 0x0,
		.frac_m8 = 0x0,
		.frac_l8 = 0x0,
	},
	/*2.7G*/
	{
		.prediv = 0x2,
		.fbdiv_h4 = 0x0,
		.fbdiv_l8 = 0xe1,
		.postdiv = 0x1,
		.frac_pd = 0x3,
		.frac_h8 = 0x0,
		.frac_m8 = 0x0,
		.frac_l8 = 0x0,
	},
	{},
};

static struct recommand_pixpll recom_pixpll[] = {
	{
		.pixel_clk = 216000000,
		.prediv = 0x1,
		.fbdiv_h4 = 0x0,
		.fbdiv_l8 = 0xc6,
		.plldiv_a = 0x1,
		.plldiv_b = 0x0,
		.plldiv_c = 0xb,
		.frac_pd = 0x3,
		.frac_h8 = 0x0,
		.frac_m8 = 0x0,
		.frac_l8 = 0x0,

	},
	{
		.pixel_clk = 54000000,
		.prediv = 0x1,
		.fbdiv_h4 = 0x0,
		.fbdiv_l8 = 0x63,
		.plldiv_a = 0x1,
		.plldiv_b = 0x1,
		.plldiv_c = 0xb,
		.frac_pd = 0x3,
		.frac_h8 = 0x0,
		.frac_m8 = 0x0,
		.frac_l8 = 0x0,

	},
	{
		.pixel_clk = 74250000,
		.prediv = 0x1,
		.fbdiv_h4 = 0x0,
		.fbdiv_l8 = 0x63,
		.plldiv_a = 0x1,
		.plldiv_b = 0x1,
		.plldiv_c = 0x8,
		.frac_pd = 0x3,
		.frac_h8 = 0x0,
		.frac_m8 = 0x0,
		.frac_l8 = 0x0,

	},
	{
		.pixel_clk = 148500000,
		.prediv = 0x1,
		.fbdiv_h4 = 0x0,
		.fbdiv_l8 = 0x63,
		.plldiv_a = 0x1,
		.plldiv_b = 0x1,
		.plldiv_c = 0x4,
		.frac_pd = 0x3,
		.frac_h8 = 0x0,
		.frac_m8 = 0x0,
		.frac_l8 = 0x0,

	},
	{
		.pixel_clk = 297000000,
		.prediv = 0x1,
		.fbdiv_h4 = 0x0,
		.fbdiv_l8 = 0x63,
		.plldiv_a = 0x1,
		.plldiv_b = 0x1,
		.plldiv_c = 0x2,
		.frac_pd = 0x3,
		.frac_h8 = 0x0,
		.frac_m8 = 0x0,
		.frac_l8 = 0x0,

	},
	{},
};

void edp_corepll_config(u32 sel, s32 bit_rate)
{
	u32 reg_val;
	u32 index;

	if (bit_rate == BIT_RATE_1G62)
		index = 0;
	else
		index = 1;

	/*turnoff corepll*/
	reg_val = readl(edp_base[sel] + REG_EDP_ANA_PLL_FBDIV);
	reg_val = SET_BITS(0, 1, reg_val, 0);
	writel(reg_val, edp_base[sel] + REG_EDP_ANA_PLL_FBDIV);

	/*config corepll prediv*/
	reg_val = readl(edp_base[sel] + REG_EDP_ANA_PLL_FBDIV);
	reg_val = SET_BITS(8, 4, reg_val, recom_corepll[index].prediv);
	writel(reg_val, edp_base[sel] + REG_EDP_ANA_PLL_FBDIV);

	/*config corepll fbdiv*/
	reg_val = readl(edp_base[sel] + REG_EDP_ANA_PLL_FBDIV);
	reg_val = SET_BITS(16, 4, reg_val, recom_corepll[index].fbdiv_h4);
	reg_val = SET_BITS(24, 8, reg_val, recom_corepll[index].fbdiv_l8);
	writel(reg_val, edp_base[sel] + REG_EDP_ANA_PLL_FBDIV);

	/*config corepll postdiv*/
	reg_val = readl(edp_base[sel] + REG_EDP_ANA_PLL_POSDIV);
	reg_val = SET_BITS(2, 2, reg_val, recom_corepll[index].postdiv);
	writel(reg_val, edp_base[sel] + REG_EDP_ANA_PLL_POSDIV);

	/*config corepll frac_pd*/
	reg_val = readl(edp_base[sel] + REG_EDP_ANA_PLL_FBDIV);
	reg_val = SET_BITS(4, 2, reg_val, recom_corepll[index].frac_pd);
	writel(reg_val, edp_base[sel] + REG_EDP_ANA_PLL_FBDIV);

	/*config corepll frac*/
	reg_val = readl(edp_base[sel] + REG_EDP_ANA_PLL_FRAC);
	reg_val = SET_BITS(0, 8, reg_val, recom_corepll[index].frac_h8);
	reg_val = SET_BITS(8, 8, reg_val, recom_corepll[index].frac_m8);
	reg_val = SET_BITS(16, 8, reg_val, recom_corepll[index].frac_l8);
	writel(reg_val, edp_base[sel] + REG_EDP_ANA_PLL_FRAC);

	/*turnon corepll*/
	reg_val = readl(edp_base[sel] + REG_EDP_ANA_PLL_FBDIV);
	reg_val = SET_BITS(0, 1, reg_val, 1);
	writel(reg_val, edp_base[sel] + REG_EDP_ANA_PLL_FBDIV);
}

void edp_pixpll_cfg(u32 sel, u32 pixel_clk)
{
	u32 reg_val;
	s32 i;

	for (i = 0; i < ARRAY_SIZE(recom_pixpll); i++) {
		if (recom_pixpll[i].pixel_clk == pixel_clk)
			break;
	}

	/*turnoff pixpll*/
	reg_val = readl(edp_base[sel] + REG_EDP_ANA_PIXPLL_FBDIV);
	reg_val = SET_BITS(0, 1, reg_val, 0);
	writel(reg_val, edp_base[sel] + REG_EDP_ANA_PIXPLL_FBDIV);

	/*config pixpll prediv*/
	reg_val = readl(edp_base[sel] + REG_EDP_ANA_PIXPLL_FBDIV);
	reg_val = SET_BITS(8, 6, reg_val, recom_pixpll[i].prediv);
	writel(reg_val, edp_base[sel] + REG_EDP_ANA_PIXPLL_FBDIV);

	/*config pixpll fbdiv*/
	reg_val = readl(edp_base[sel] + REG_EDP_ANA_PIXPLL_FBDIV);
	reg_val = SET_BITS(16, 4, reg_val, recom_pixpll[i].fbdiv_h4);
	reg_val = SET_BITS(24, 8, reg_val, recom_pixpll[i].fbdiv_l8);
	writel(reg_val, edp_base[sel] + REG_EDP_ANA_PIXPLL_FBDIV);

	/*config pixpll divabc*/
	reg_val = readl(edp_base[sel] + REG_EDP_ANA_PIXPLL_DIV);
	reg_val = SET_BITS(0, 5, reg_val, recom_pixpll[i].plldiv_a);
	reg_val = SET_BITS(8, 2, reg_val, recom_pixpll[i].plldiv_b);
	reg_val = SET_BITS(16, 5, reg_val, recom_pixpll[i].plldiv_c);
	writel(reg_val, edp_base[sel] + REG_EDP_ANA_PIXPLL_DIV);

	/*config pixpll frac_pd*/
	reg_val = readl(edp_base[sel] + REG_EDP_ANA_PIXPLL_FBDIV);
	reg_val = SET_BITS(4, 2, reg_val, recom_pixpll[i].frac_pd);
	writel(reg_val, edp_base[sel] + REG_EDP_ANA_PIXPLL_FBDIV);

	/*config pixpll frac*/
	reg_val = readl(edp_base[sel] + REG_EDP_ANA_PIXPLL_FRAC);
	reg_val = SET_BITS(0, 8, reg_val, recom_pixpll[i].frac_h8);
	reg_val = SET_BITS(8, 8, reg_val, recom_pixpll[i].frac_m8);
	reg_val = SET_BITS(16, 8, reg_val, recom_pixpll[i].frac_l8);
	writel(reg_val, edp_base[sel] + REG_EDP_ANA_PIXPLL_FRAC);

	/*turnon pixpll*/
	reg_val = readl(edp_base[sel] + REG_EDP_ANA_PIXPLL_FBDIV);
	reg_val = SET_BITS(0, 1, reg_val, 1);
	writel(reg_val, edp_base[sel] + REG_EDP_ANA_PIXPLL_FBDIV);
}

void edp_set_misc(u32 sel, u32 misc0_val, u32 misc1_val)
{
	u32 reg_val;

	/*misc0 setting*/
	reg_val = readl(edp_base[sel] + REG_EDP_MSA_MISC0);
	reg_val = SET_BITS(24, 8, reg_val, misc0_val);
	writel(reg_val, edp_base[sel] + REG_EDP_MSA_MISC0);

	/*misc1 setting*/
	reg_val = readl(edp_base[sel] + REG_EDP_MSA_MISC1);
	reg_val = SET_BITS(24, 8, reg_val, misc1_val);
	writel(reg_val, edp_base[sel] + REG_EDP_MSA_MISC1);
}

void edp_lane_config(u32 sel, u32 lane_cnt, u32 bit_rate)
{
	u32 reg_val;

	if ((lane_cnt < 0) || (lane_cnt > 4)) {
		EDP_WRN("unsupport lane number!\n");
	}

	g_lane_cnt = lane_cnt;

	/*config lane number*/
	reg_val = readl(edp_base[sel] + REG_EDP_CAPACITY);
	switch (lane_cnt) {
	case 0:
	case 3:
		EDP_WRN("edp lane number can not be configed to 0/3!\n");
	case 1:
		reg_val = SET_BITS(6, 2, reg_val, 0x0);
		reg_val = SET_BITS(8, 4, reg_val, 0x01);
		break;
	case 2:
		reg_val = SET_BITS(6, 2, reg_val, 0x1);
		reg_val = SET_BITS(8, 4, reg_val, 0x3);
		break;
	case 4:
		reg_val = SET_BITS(6, 2, reg_val, 0x2);
		reg_val = SET_BITS(8, 4, reg_val, 0xf);
		break;
	}
	writel(reg_val, edp_base[sel] + REG_EDP_CAPACITY);

	/*config lane bit rate*/
	reg_val = readl(edp_base[sel] + REG_EDP_CAPACITY);
	switch (bit_rate) {
	case BIT_RATE_1G62:
		reg_val = SET_BITS(26, 3, reg_val, 0x0);
		reg_val = SET_BITS(4, 2, reg_val, 0x0);
		break;
	case BIT_RATE_2G7:
	default:
		reg_val = SET_BITS(26, 3, reg_val, 0x0);
		reg_val = SET_BITS(4, 2, reg_val, 0x1);
		break;
	}
	writel(reg_val, edp_base[sel] + REG_EDP_CAPACITY);
}

void edp_assr_enable(u32 sel)
{
	u32 reg_val;

	reg_val = readl(edp_base[sel] + REG_EDP_VIDEO_STREAM_EN);
	reg_val = SET_BITS(24, 1, reg_val, 1);
	writel(reg_val, edp_base[sel] + REG_EDP_VIDEO_STREAM_EN);
}

void edp_assr_disable(u32 sel)
{
	u32 reg_val;

	reg_val = readl(edp_base[sel] + REG_EDP_VIDEO_STREAM_EN);
	reg_val = SET_BITS(24, 1, reg_val, 0);
	writel(reg_val, edp_base[sel] + REG_EDP_VIDEO_STREAM_EN);
}

void edp_video_stream_enable(u32 sel)
{
	u32 reg_val;

	reg_val = readl(edp_base[sel] + REG_EDP_VIDEO_STREAM_EN);
	reg_val = SET_BITS(5, 1, reg_val, 1);
	writel(reg_val, edp_base[sel] + REG_EDP_VIDEO_STREAM_EN);
}

void edp_video_stream_disable(u32 sel)
{
	u32 reg_val;

	reg_val = readl(edp_base[sel] + REG_EDP_VIDEO_STREAM_EN);
	reg_val = SET_BITS(5, 1, reg_val, 0);
	writel(reg_val, edp_base[sel] + REG_EDP_VIDEO_STREAM_EN);
}

void edp_training_pattern_config(u32 sel, s32 pattern)
{
	u32 reg_val;

	reg_val = readl(edp_base[sel] + REG_EDP_CAPACITY);
	reg_val = SET_BITS(0, 4, reg_val, pattern);
	writel(reg_val, edp_base[sel] + REG_EDP_CAPACITY);
}

void edp_link_lane_para_setting(u32 sel, u8 sw0, u8 pre0, u8 sw1, u8 pre1, u8 sw2, u8 pre2,
		 u8 sw3, u8 pre3)
{
	u32 reg_val;

	/*sw0*/
	reg_val = readl(edp_base[sel] + REG_EDP_TX_MAINSEL);
	if (sw0 == 0)
		reg_val = SET_BITS(0, 4, reg_val, 0);
	else if (sw0 == 1)
		reg_val = SET_BITS(0, 4, reg_val, 2);
	else if (sw0 == 2)
		reg_val = SET_BITS(0, 4, reg_val, 4);
	else if (sw0 == 3)
		reg_val = SET_BITS(0, 4, reg_val, 6);
	else
		reg_val = SET_BITS(0, 4, reg_val, 0);
	writel(reg_val, edp_base[sel] + REG_EDP_TX_MAINSEL);

	/*sw1*/
	reg_val = readl(edp_base[sel] + REG_EDP_TX_MAINSEL);
	if (sw1 == 0)
		reg_val = SET_BITS(4, 4, reg_val, 0);
	else if (sw1 == 1)
		reg_val = SET_BITS(4, 4, reg_val, 2);
	else if (sw1 == 2)
		reg_val = SET_BITS(4, 4, reg_val, 4);
	else if (sw1 == 3)
		reg_val = SET_BITS(4, 4, reg_val, 6);
	else
		reg_val = SET_BITS(4, 4, reg_val, 0);

	writel(reg_val, edp_base[sel] + REG_EDP_TX_MAINSEL);

	/*sw2*/
	reg_val = readl(edp_base[sel] + REG_EDP_TX32_ISEL_DRV);
	if (sw2 == 0)
		reg_val = SET_BITS(24, 4, reg_val, 0);
	else if (sw2 == 1)
		reg_val = SET_BITS(24, 4, reg_val, 2);
	else if (sw2 == 2)
		reg_val = SET_BITS(24, 4, reg_val, 4);
	else if (sw2 == 3)
		reg_val = SET_BITS(24, 4, reg_val, 6);
	else
		reg_val = SET_BITS(24, 4, reg_val, 0);
	writel(reg_val, edp_base[sel] + REG_EDP_TX32_ISEL_DRV);

	/*sw3*/
	reg_val = readl(edp_base[sel] + REG_EDP_TX32_ISEL_DRV);
	if (sw3 == 0)
		reg_val = SET_BITS(28, 4, reg_val, 0);
	else if (sw3 == 1)
		reg_val = SET_BITS(28, 4, reg_val, 2);
	else if (sw3 == 2)
		reg_val = SET_BITS(28, 4, reg_val, 4);
	else if (sw3 == 3)
		reg_val = SET_BITS(28, 4, reg_val, 6);
	else
		reg_val = SET_BITS(28, 4, reg_val, 0);
	writel(reg_val, edp_base[sel] + REG_EDP_TX32_ISEL_DRV);

	/*pre0*/
	reg_val = readl(edp_base[sel] + REG_EDP_TX_POSTSEL);
	if (pre0 == 0)
		reg_val = SET_BITS(24, 4, reg_val, 0);
	else if (pre0 == 1)
		reg_val = SET_BITS(24, 4, reg_val, 1);
	else if (pre0 == 2)
		reg_val = SET_BITS(24, 4, reg_val, 2);
	else if (pre0 == 3)
		reg_val = SET_BITS(24, 4, reg_val, 3);
	else
		reg_val = SET_BITS(24, 4, reg_val, 0);
	writel(reg_val, edp_base[sel] + REG_EDP_TX_POSTSEL);

	/*pre1*/
	reg_val = readl(edp_base[sel] + REG_EDP_TX_POSTSEL);
	if (pre1 == 0)
		reg_val = SET_BITS(28, 4, reg_val, 0);
	else if (pre0 == 1)
		reg_val = SET_BITS(28, 4, reg_val, 1);
	else if (pre0 == 2)
		reg_val = SET_BITS(28, 4, reg_val, 2);
	else if (pre0 == 3)
		reg_val = SET_BITS(28, 4, reg_val, 3);
	else
		reg_val = SET_BITS(28, 4, reg_val, 0);
	writel(reg_val, edp_base[sel] + REG_EDP_TX_POSTSEL);

	/*pre2*/
	reg_val = readl(edp_base[sel] + REG_EDP_TX_POSTSEL);
	if (pre2 == 0)
		reg_val = SET_BITS(16, 4, reg_val, 0);
	else if (pre2 == 1)
		reg_val = SET_BITS(16, 4, reg_val, 1);
	else if (pre2 == 2)
		reg_val = SET_BITS(16, 4, reg_val, 2);
	else if (pre2 == 3)
		reg_val = SET_BITS(16, 4, reg_val, 3);
	else
		reg_val = SET_BITS(16, 4, reg_val, 0);
	writel(reg_val, edp_base[sel] + REG_EDP_TX_POSTSEL);

	/*pre3*/
	reg_val = readl(edp_base[sel] + REG_EDP_TX_POSTSEL);
	if (pre3 == 0)
		reg_val = SET_BITS(20, 4, reg_val, 0);
	else if (pre3 == 1)
		reg_val = SET_BITS(20, 4, reg_val, 1);
	else if (pre3 == 2)
		reg_val = SET_BITS(20, 4, reg_val, 2);
	else if (pre3 == 3)
		reg_val = SET_BITS(20, 4, reg_val, 3);
	else
		reg_val = SET_BITS(20, 4, reg_val, 0);
	writel(reg_val, edp_base[sel] + REG_EDP_TX_POSTSEL);

	/*set main isel*/
	reg_val = readl(edp_base[sel] + REG_EDP_TX_MAINSEL);
	reg_val = SET_BITS(16, 5, reg_val, 0x1f);
	reg_val = SET_BITS(24, 5, reg_val, 0x1f);
	writel(reg_val, edp_base[sel] + REG_EDP_TX_MAINSEL);

	reg_val = readl(edp_base[sel] + REG_EDP_TX_POSTSEL);
	reg_val = SET_BITS(0, 5, reg_val, 0x1f);
	reg_val = SET_BITS(8, 5, reg_val, 0x1f);
	writel(reg_val, edp_base[sel] + REG_EDP_TX_POSTSEL);

	/*set pre isel*/
	writel(0x0, edp_base[sel] + REG_EDP_TX_PRESEL);
}

void edp_audio_stream_vblank_setting(u32 sel, bool enable)
{
	u32 reg_val;

	reg_val = readl(edp_base[sel] + REG_EDP_AUDIO_VBLANK_EN);
	reg_val = SET_BITS(1, 1, reg_val, enable);
	writel(reg_val, edp_base[sel] + REG_EDP_AUDIO_VBLANK_EN);
}

void edp_audio_timestamp_vblank_setting(u32 sel, bool enable)
{
	u32 reg_val;

	reg_val = readl(edp_base[sel] + REG_EDP_AUDIO_VBLANK_EN);
	reg_val = SET_BITS(0, 1, reg_val, enable);
	writel(reg_val, edp_base[sel] + REG_EDP_AUDIO_VBLANK_EN);
}

void edp_audio_stream_hblank_setting(u32 sel, bool enable)
{
	u32 reg_val;

	reg_val = readl(edp_base[sel] + REG_EDP_AUDIO_HBLANK_EN);
	reg_val = SET_BITS(1, 1, reg_val, enable);
	writel(reg_val, edp_base[sel] + REG_EDP_AUDIO_HBLANK_EN);
}

void edp_audio_timestamp_hblank_setting(u32 sel, bool enable)
{
	u32 reg_val;

	reg_val = readl(edp_base[sel] + REG_EDP_AUDIO_HBLANK_EN);
	reg_val = SET_BITS(0, 1, reg_val, enable);
	writel(reg_val, edp_base[sel] + REG_EDP_AUDIO_HBLANK_EN);
}

void edp_audio_interface_config(u32 sel, u32 interface)
{
	u32 reg_val;

	reg_val = readl(edp_base[sel] + REG_EDP_AUDIO);
	reg_val = SET_BITS(0, 1, reg_val, interface);
	writel(reg_val, edp_base[sel] + REG_EDP_AUDIO);
}

void edp_audio_channel_config(u32 sel, u32 chn_num)
{
	u32 reg_val;

	reg_val = readl(edp_base[sel] + REG_EDP_AUDIO);

	switch (chn_num) {
	case 2:
		reg_val = SET_BITS(12, 3, reg_val, 0x1);
		reg_val = SET_BITS(1, 2, reg_val, 0x3);
		break;
	case 8:
		reg_val = SET_BITS(12, 3, reg_val, 0x2);
		reg_val = SET_BITS(1, 4, reg_val, 0xf);
		break;
	case 1:
	default:
		reg_val = SET_BITS(12, 3, reg_val, 0x0);
		reg_val = SET_BITS(1, 1, reg_val, 0x1);
		break;
	}
	writel(reg_val, edp_base[sel] + REG_EDP_AUDIO);
}

void edp_audio_mute_config(u32 sel, bool mute)
{

	u32 reg_val;

	reg_val = readl(edp_base[sel] + REG_EDP_AUDIO);
	reg_val = SET_BITS(15, 1, reg_val, mute);
	writel(reg_val, edp_base[sel] + REG_EDP_AUDIO);
}

void edp_audio_data_width_config(u32 sel, u32 data_width)
{

	u32 reg_val;

	reg_val = readl(edp_base[sel] + REG_EDP_AUDIO);
	switch (data_width) {
	case 20:
		reg_val = SET_BITS(5, 5, reg_val, 0x18);
		break;
	case 24:
		reg_val = SET_BITS(5, 5, reg_val, 0x14);
		break;
	case 16:
	default:
		reg_val = SET_BITS(5, 5, reg_val, 0x10);
		break;
	}
	writel(reg_val, edp_base[sel] + REG_EDP_AUDIO);
}

void edp_audio_soft_reset(u32 sel)
{
	u32 reg_val;

	reg_val = readl(edp_base[sel] + REG_EDP_RESET);
	reg_val = SET_BITS(3, 1, reg_val, 1);
	writel(reg_val, edp_base[sel] + REG_EDP_RESET);
	udelay(5);
	reg_val = SET_BITS(3, 1, reg_val, 0);
	writel(reg_val, edp_base[sel] + REG_EDP_RESET);
}

void edp_set_input_video_mapping(u32 sel, enum edp_video_mapping_e mapping)
{
	u32 reg_val;
	u32 mapping_val;
	u32 misc0_val = 0;
	u32 misc1_val = 0;

	switch (mapping) {
	case RGB_6BIT:
		mapping_val = 0;
		misc0_val = (0 << 5);
		g_bpp = 18;
		break;
	case RGB_8BIT:
		mapping_val = 1;
		misc0_val = (1 << 5);
		g_bpp = 24;
		break;
	case RGB_10BIT:
		mapping_val = 2;
		misc0_val = (2 << 5);
		g_bpp = 30;
		break;
	case RGB_12BIT:
		mapping_val = 3;
		misc0_val = (3 << 5);
		g_bpp = 36;
		break;
	case RGB_16BIT:
		mapping_val = 4;
		misc0_val = (4 << 5);
		g_bpp = 48;
		break;
	case YCBCR444_8BIT:
		mapping_val = 5;
		misc0_val = (1 << 5) | (1 << 2);
		g_bpp = 24;
		break;
	case YCBCR444_10BIT:
		mapping_val = 6;
		misc0_val = (2 << 5) | (1 << 2);
		g_bpp = 30;
		break;
	case YCBCR444_12BIT:
		mapping_val = 7;
		misc0_val = (3 << 5) | (1 << 2);
		g_bpp = 36;
		break;
	case YCBCR444_16BIT:
		mapping_val = 8;
		misc0_val = (4 << 5) | (1 << 2);
		g_bpp = 48;
		break;
	case YCBCR422_8BIT:
		mapping_val = 9;
		misc0_val = (1 << 5) | (1 << 1);
		g_bpp = 16;
		break;
	case YCBCR422_10BIT:
		mapping_val = 10;
		misc0_val = (2 << 5) | (1 << 1);
		g_bpp = 20;
		break;
	case YCBCR422_12BIT:
		mapping_val = 11;
		misc0_val = (3 << 5) | (1 << 1);
		g_bpp = 24;
		break;
	case YCBCR422_16BIT:
		mapping_val = 12;
		misc0_val = (4 << 5) | (1 << 1);
		g_bpp = 32;
		break;
	}

	reg_val = readl(edp_base[sel] + REG_EDP_VIDEO_STREAM_EN);
	reg_val = SET_BITS(16, 5, reg_val, mapping_val);
	writel(reg_val, edp_base[sel] + REG_EDP_VIDEO_STREAM_EN);

	edp_set_misc(sel, misc0_val, misc1_val);
}

void edp_hpd_enable(u32 sel)
{
	writel(0x8, edp_base[sel] + REG_EDP_HPD_SCALE);
	/* only hpd enable need, irq is not necesssary*/
	writel(0x1, edp_base[sel] + REG_EDP_HPD_EN);
	//edp_hpd_irq_enable(sel);
}

s32 edp_bist_test(u32 sel, struct edp_tx_core *edp_core)
{
	u32 reg_val;
	s32 ret;
	struct pinctrl_state *state;

	/*set bist_test_sel to 1*/
	reg_val = readl(edp_base[sel] + REG_EDP_BIST_CFG);
	reg_val = SET_BITS(0, 1, reg_val, 1);
	writel(reg_val, edp_base[sel] + REG_EDP_BIST_CFG);

	/*assert reset pin*/
	if (!IS_ERR(edp_core->rst_pin)) {
		state = pinctrl_lookup_state(edp_core->rst_pin, "rst_assert");
		if (IS_ERR(state)) {
			EDP_DBG("pinctrl_lookup_state for edp reset assert fail\n");
			return RET_FAIL;
		}

		ret = pinctrl_select_state(edp_core->rst_pin, state);
		if (ret < 0) {
			EDP_DBG("pinctrl_select_state for edp reset assert fail\n");
			return RET_FAIL;
		}
	}

	/*set bist_test_en to 1*/
	reg_val = readl(edp_base[sel] + REG_EDP_BIST_CFG);
	reg_val = SET_BITS(1, 1, reg_val, 1);
	writel(reg_val, edp_base[sel] + REG_EDP_BIST_CFG);

	udelay(5);

	/*wait for bist_test_done to 1*/
	while (1) {
		reg_val = readl(edp_base[sel] + REG_EDP_BIST_CFG);
		reg_val = GET_BITS(4, 1, reg_val);
		if (reg_val == 1)
			break;
	}

	/*checke bist_test_done*/
	reg_val = readl(edp_base[sel] + REG_EDP_BIST_CFG);
	reg_val = GET_BITS(5, 1, reg_val);
	if (reg_val == 1)
		ret = RET_OK;
	else
		ret = RET_FAIL;

	/*deassert reset pin and disable bist_test_sel to exit bist*/
	if (!IS_ERR(edp_core->rst_pin)) {
		state = pinctrl_lookup_state(edp_core->rst_pin, "rst_deassert");
		if (IS_ERR(state)) {
			EDP_DBG("pinctrl_lookup_state for edp reset assert fail\n");
			return RET_FAIL;
		}

		ret = pinctrl_select_state(edp_core->rst_pin, state);
		if (ret < 0) {
			EDP_DBG("pinctrl_select_state for edp reset assert fail\n");
			return RET_FAIL;
		}
	}
	writel(0x0, edp_base[sel] + REG_EDP_BIST_CFG);

	return RET_OK;
}

void edp_phy_soft_reset(u32 sel)
{
	u32 reg_val;

	reg_val = readl(edp_base[sel] + REG_EDP_RESET);
	reg_val = SET_BITS(1, 1, reg_val, 0);
	writel(reg_val, edp_base[sel] + REG_EDP_RESET);
	udelay(5);
	reg_val = SET_BITS(1, 1, reg_val, 1);
	writel(reg_val, edp_base[sel] + REG_EDP_RESET);
}

s32 edp_aux_read(u32 sel, s32 addr, s32 len, char *buf)
{
	u32 reg_val[4];
	u32 regval = 0;
	s32 i;

	if ((len > 16) || (len <= 0)) {
		EDP_DBG("aux read out of len:%d\n", len);
		return -1;
	}

	memset(buf, 0, 16);

	/* aux read request*/
	regval |= len;
	regval = SET_BITS(8, 20, regval, addr);
	regval = SET_BITS(28, 4, regval, NATIVE_READ);
	printk("!!!reg_val = 0x%x\n", regval);
	writel(regval, edp_base[sel] + REG_EDP_PHY_AUX);
	udelay(100);
	writel(0, edp_base[sel] + REG_EDP_AUX_START);
	udelay(1);
	writel(1, edp_base[sel] + REG_EDP_AUX_START);
	udelay(1);
	writel(0, edp_base[sel] + REG_EDP_AUX_START);

	/* wait for AUX_ACK*/
	while ((readl(edp_base[sel] + REG_EDP_AUX_TIMEOUT) >> 16) != 0) {
	} // can it be async?

	regval = readl(edp_base[sel] + REG_EDP_AUX_TIMEOUT);
	regval &= 0xf0;
	if ((regval >> 4) == AUX_REPLY_NACK)
		return -1;

	/* aux read reply*/
	for (i = 0; i < 4; i++) {
		reg_val[i] = readl(edp_base[sel] + REG_EDP_AUX_DATA0 + i * 0x4);
		printk("!!!! aux_read reg_val[%d] = 0x%x", i, reg_val[i]);
	}

	for (i = 0; i < len; i++) {
		buf[i] = GET_BITS((i % 4) * 8, 8, reg_val[i / 4]);
	}

	return 0;
}

s32 edp_aux_write(u32 sel, s32 addr, s32 len, char *buf)
{
	u32 reg_val[4];
	u32 regval = 0;
	u32 timeout = 0;
	s32 i;

	if ((len > 16) || (len <= 0)) {
		EDP_WRN("aux read out of len:%d\n", len);
		return -1;
	}

	/* aux write request*/
	regval |= len;
	regval = SET_BITS(8, 20, regval, addr);
	regval = SET_BITS(28, 4, regval, NATIVE_WRITE);
	EDP_DBG("%s, reg_val = 0x%x\n", __func__, regval);
	writel(regval, edp_base[sel] + REG_EDP_PHY_AUX);

	for (i = 0; i < len; i++) {
		reg_val[i / 4] = SET_BITS((i % 4) * 8, 8, reg_val[i / 4], buf[i]);
	}

	for (i = 0; i < 4; i++) {
		writel(reg_val[i], edp_base[sel] + REG_EDP_AUX_DATA0 + i * 0x4);
		EDP_DBG("%s, edp_aux_write reg_val[%d] = 0x%x", __func__, i, reg_val[i]);
	}

	udelay(100);
	writel(0, edp_base[sel] + REG_EDP_AUX_START);
	udelay(1);
	writel(1, edp_base[sel] + REG_EDP_AUX_START);
	udelay(1);
	writel(0, edp_base[sel] + REG_EDP_AUX_START);

	/* wait for AUX_ACK*/
	while ((readl(edp_base[sel] + REG_EDP_AUX_TIMEOUT) >> 16) != 0) {
		if (timeout >= 2000) {
			EDP_WRN("edp wait AUX_ACK timeout\n");
			return RET_FAIL;
		}
		timeout++;
	} // can it be async?

	regval = readl(edp_base[sel] + REG_EDP_AUX_TIMEOUT);
	regval &= 0xf0;
	if ((regval >> 4) == AUX_REPLY_NACK)
		return -1;

	return 0;
}

s32 edp_hal_aux_read(u32 sel, s32 addr, s32 len, char *buf)
{
	return edp_aux_read(sel, addr, len, buf);
}

s32 edp_hal_aux_write(u32 sel, s32 addr, s32 len, char *buf)
{
	return edp_aux_write(sel, addr, len, buf);
}

s32 edp_hal_aux_i2c_read(u32 sel, s32 addr, s32 len, char *buf)
{
	u32 reg_val[4];
	u32 regval = 0;
	s32 i;

	if ((len > 16) || (len <= 0)) {
		EDP_WRN("aux read out of len:%d\n", len);
		return -1;
	}

	memset(buf, 0, 16);

	/* aux read request*/
	regval |= len;
	regval = SET_BITS(8, 20, regval, addr);
	regval = SET_BITS(28, 4, regval, AUX_I2C_READ);
	printk("!!!reg_val = 0x%x\n", regval);
	writel(regval, edp_base[sel] + REG_EDP_PHY_AUX);
	udelay(100);
	writel(0, edp_base[sel] + REG_EDP_AUX_START);
	udelay(1);
	writel(1, edp_base[sel] + REG_EDP_AUX_START);
	udelay(1);
	writel(0, edp_base[sel] + REG_EDP_AUX_START);

	/* wait for AUX_ACK*/
	while ((readl(edp_base[sel] + REG_EDP_AUX_TIMEOUT) >> 16) != 0) {
	} // can it be async?

	regval = readl(edp_base[sel] + REG_EDP_AUX_TIMEOUT);
	regval &= 0xf0;
	if ((regval >> 4) == AUX_REPLY_NACK)
		return -1;

	/* aux read reply*/
	for (i = 0; i < 4; i++) {
		reg_val[i] = readl(edp_base[sel] + REG_EDP_AUX_DATA0 + i * 0x4);
		printk("!!!! aux_i2c_read reg_val[%d] = 0x%x", i, reg_val[i]);
	}

	for (i = 0; i < len; i++) {
		buf[i] = GET_BITS(i % 4, 8, reg_val[i / 4]);
	}

	return 0;
}

s32 edp_hal_aux_i2c_write(u32 sel, s32 addr, s32 len, char *buf)
{
	u32 reg_val[4];
	u32 regval = 0;
	s32 i;

	if ((len > 16) || (len <= 0)) {
		EDP_WRN("aux read out of len:%d\n", len);
		return -1;
	}

	/* aux write request*/
	regval |= len;
	regval = SET_BITS(8, 20, regval, addr);
	regval = SET_BITS(28, 4, regval, NATIVE_WRITE);
	printk("!!!reg_val = 0x%x\n", regval);
	writel(regval, edp_base[sel] + REG_EDP_PHY_AUX);

	for (i = 0; i < len; i++) {
		reg_val[i / 4] = SET_BITS(i % 4, 8, reg_val[i / 4], buf[i]);
	}

	for (i = 0; i < 4; i++) {
		writel(reg_val[i], edp_base[sel] + REG_EDP_AUX_DATA0 + i * 0x4);
		printk("!!!! aux_i2c_write reg_val[%d] = 0x%x", i, reg_val[i]);
	}

	udelay(100);
	writel(0, edp_base[sel] + REG_EDP_AUX_START);
	udelay(1);
	writel(1, edp_base[sel] + REG_EDP_AUX_START);
	udelay(1);
	writel(0, edp_base[sel] + REG_EDP_AUX_START);

	/* wait for AUX_ACK*/
	while ((readl(edp_base[sel] + REG_EDP_AUX_TIMEOUT) >> 16) != 0) {
	} // can it be async?

	regval = readl(edp_base[sel] + REG_EDP_AUX_TIMEOUT);
	regval &= 0xf0;
	if ((regval >> 4) == AUX_REPLY_NACK)
		return -1;

	return 0;
}


s32 edp_link_training1(u32 sel, struct edp_lane_para *para, bool bypass_1st_train)
{
	s32 ret = RET_OK;
	u32 to_cnt;
	u32 lane0_sw, lane0_pre;
	u32 lane1_sw, lane1_pre;
	u32 lane2_sw, lane2_pre;
	u32 lane3_sw, lane3_pre;
	u32 lane0_is_pre_max, lane1_is_pre_max,
	    lane2_is_pre_max, lane3_is_pre_max;
	u32 lane0_is_swing_max, lane1_is_swing_max,
	    lane2_is_swing_max, lane3_is_swing_max;

	u32 lane0_sw_old, lane0_pre_old;
	u32 lane1_sw_old, lane1_pre_old;
	u32 lane2_sw_old, lane2_pre_old;
	u32 lane3_sw_old, lane3_pre_old;

	lane0_sw = 0;
	lane0_pre = 0;
	lane1_sw = 0;
	lane1_pre = 0;
	lane2_sw = 0;
	lane2_pre = 0;
	lane3_sw = 0;
	lane3_pre = 0;

	lane0_sw_old = 0;
	lane0_pre_old = 0;
	lane1_sw_old = 0;
	lane1_pre_old = 0;
	lane2_sw_old = 0;
	lane2_pre_old = 0;
	lane3_sw_old = 0;
	lane3_pre_old = 0;

	lane0_is_pre_max = 0;
	lane1_is_pre_max = 0;
	lane2_is_pre_max = 0;
	lane3_is_pre_max = 0;
	lane0_is_swing_max = 0;
	lane1_is_swing_max = 0;
	lane2_is_swing_max = 0;
	lane3_is_swing_max = 0;

	if (para != NULL) {
		if (para->lane0_sw)
			lane0_sw = para->lane0_sw;
		if (para->lane1_sw)
			lane1_sw = para->lane1_sw;
		if (para->lane2_sw)
			lane2_sw = para->lane2_sw;
		if (para->lane3_sw)
			lane3_sw = para->lane3_sw;

		if (para->lane0_pre)
			lane0_pre = para->lane0_pre;
		if (para->lane1_pre)
			lane1_pre = para->lane1_pre;
		if (para->lane2_pre)
			lane2_pre = para->lane2_pre;
		if (para->lane3_pre)
			lane3_pre = para->lane3_pre;

		if (lane0_sw == VOL_SWING_LEVEL_NUM - 1)
			lane0_is_swing_max = 1;
		else
			lane0_is_swing_max = 0;

		if (lane1_sw == VOL_SWING_LEVEL_NUM - 1)
			lane1_is_swing_max = 1;
		else
			lane1_is_swing_max = 0;

		if (lane2_sw == VOL_SWING_LEVEL_NUM - 1)
			lane2_is_swing_max = 1;
		else
			lane2_is_swing_max = 0;

		if (lane3_sw == VOL_SWING_LEVEL_NUM - 1)
			lane3_is_swing_max = 1;
		else
			lane3_is_swing_max = 0;

		if (lane0_pre == PRE_EMPHASIS_LEVEL_NUM - 1)
			lane0_is_pre_max = 1;
		else
			lane0_is_pre_max = 0;

		if (lane1_pre == PRE_EMPHASIS_LEVEL_NUM - 1)
			lane1_is_pre_max = 1;
		else
			lane1_is_pre_max = 0;

		if (lane2_pre == PRE_EMPHASIS_LEVEL_NUM - 1)
			lane2_is_pre_max = 1;
		else
			lane2_is_pre_max = 0;

		if (lane3_pre == PRE_EMPHASIS_LEVEL_NUM - 1)
			lane3_is_pre_max = 1;
		else
			lane3_is_pre_max = 0;
	}

	memset(g_rx_buf, 0, sizeof(g_rx_buf));
	memset(g_tx_buf, 0, sizeof(g_tx_buf));

	if (!bypass_1st_train) {
		edp_training_pattern_config(sel, 1);

		edp_link_lane_para_setting(sel, lane0_sw, lane0_pre, lane1_sw, lane1_pre,
			    lane2_sw, lane2_pre, lane3_sw, lane3_pre);

		mdelay(5);

		g_tx_buf[0] = 0x21; // set pattern 1 with scramble disable
		g_tx_buf[1] =
		    ((lane0_is_pre_max & 0x1) << 5) | ((lane0_pre & 0x3) << 3) |
		    ((lane0_is_swing_max & 0x1) << 2) | (lane0_sw & 0x3);
		g_tx_buf[2] =
		    ((lane1_is_pre_max & 0x1) << 5) | ((lane1_pre & 0x3) << 3) |
		    ((lane1_is_swing_max & 0x1) << 2) | (lane1_sw & 0x3);
		g_tx_buf[3] =
		    ((lane2_is_pre_max & 0x1) << 5) | ((lane2_pre & 0x3) << 3) |
		    ((lane2_is_swing_max & 0x1) << 2) | (lane2_sw & 0x3);
		g_tx_buf[4] =
		    ((lane3_is_pre_max & 0x1) << 5) | ((lane3_pre & 0x3) << 3) |
		    ((lane3_is_swing_max & 0x1) << 2) | (lane3_sw & 0x3);

		ret = edp_aux_write(sel, 0x00102, g_lane_cnt + 1, g_tx_buf);

		if (ret != RET_OK)
			return RET_FAIL;
	}

	to_cnt = TRAIN_CNT;

	while (1) {
		udelay(training_interval_CR); // wait for the training
						     // finish

		if (g_lane_cnt < 4)
			ret = edp_aux_read(sel, 0x0202, 1, g_rx_buf);
		else
			ret = edp_aux_read(sel, 0x0202, 2, g_rx_buf);
		if (ret != RET_OK)
			return RET_FAIL;

		if (g_lane_cnt < 4) {
			EDP_DBG("CR training reg[202h]:0x%x\n", g_rx_buf[0]);
		} else {
			EDP_DBG("CR training reg[202h]:0x%x\n", g_rx_buf[0]);
			EDP_DBG("CR training reg[203h]:0x%x\n", g_rx_buf[1]);
		}

		if (g_lane_cnt == 1) {
			if ((g_rx_buf[0] & 0x01) == 0x01) {
				recom_training_para.lane0_sw = lane0_sw;
				recom_training_para.lane0_pre = lane0_pre;
				return RET_OK;
			}
		} else if (g_lane_cnt == 2) {
			if ((g_rx_buf[0] & 0x11) == 0x11) {
				recom_training_para.lane0_sw = lane0_sw;
				recom_training_para.lane0_pre = lane0_pre;
				recom_training_para.lane1_sw = lane1_sw;
				recom_training_para.lane1_pre = lane1_pre;
				return RET_OK;
			}
		} else if (g_lane_cnt == 4) {
			if (((g_rx_buf[0] & 0x11) == 0x11) &&
			    ((g_rx_buf[1] & 0x11) == 0x11)) {
				recom_training_para.lane0_sw = lane0_sw;
				recom_training_para.lane0_pre = lane0_pre;
				recom_training_para.lane1_sw = lane1_sw;
				recom_training_para.lane1_pre = lane1_pre;
				recom_training_para.lane2_sw = lane2_sw;
				recom_training_para.lane2_pre = lane2_pre;
				recom_training_para.lane3_sw = lane3_sw;
				recom_training_para.lane3_pre = lane3_pre;
				return RET_OK;
			}
		}

		if (g_lane_cnt < 4)
			ret = edp_aux_read(sel, 0x0206, 1, g_rx_buf);
		else
			ret = edp_aux_read(sel, 0x0206, 2, g_rx_buf);
		if (ret !=  RET_OK)
			return RET_FAIL;
		if (g_lane_cnt < 4)
			EDP_DBG("CR training reg[206h]:%xh\n", g_rx_buf[0]);
		else {
			EDP_DBG("CR training reg[206h]:%xh\n", g_rx_buf[0]);
			EDP_DBG("CR training reg[207h]:%xh\n", g_rx_buf[1]);
		}

		if (g_lane_cnt == 1) {
			lane0_sw = g_rx_buf[0] & 0x3;
			lane0_pre = (g_rx_buf[0] >> 2) & 0x3;

			if ((lane0_sw != lane0_sw_old)) {
				lane0_sw_old = lane0_sw;
				to_cnt = TRAIN_CNT;
			}

			if ((lane0_pre != lane0_pre_old)) {
				lane0_pre_old = lane0_pre;
				to_cnt = TRAIN_CNT;
			}

			if (lane0_sw == VOL_SWING_LEVEL_NUM - 1)
				lane0_is_swing_max = 1;
			else
				lane0_is_swing_max = 0;

			if (lane0_pre == PRE_EMPHASIS_LEVEL_NUM - 1)
				lane0_is_pre_max = 1;
			else
				lane0_is_pre_max = 0;
		} else if (g_lane_cnt == 2) {
			lane0_sw = g_rx_buf[0] & 0x3;
			lane0_pre = (g_rx_buf[0] >> 2) & 0x3;
			lane1_sw = (g_rx_buf[0] >> 4) & 0x3;
			lane1_pre = (g_rx_buf[0] >> 6) & 0x3;

			if ((lane0_sw != lane0_sw_old) ||
			    (lane1_sw != lane1_sw_old)) {

				lane0_sw_old = lane0_sw;
				lane1_sw_old = lane1_sw;
				to_cnt = TRAIN_CNT;
			}

			if ((lane0_pre != lane0_pre_old) ||
			    (lane1_pre != lane1_pre_old)) {
				lane0_pre_old = lane0_pre;
				lane1_pre_old = lane1_pre;
				to_cnt = TRAIN_CNT;
			}

			if (lane0_sw == VOL_SWING_LEVEL_NUM - 1)
				lane0_is_swing_max = 1;
			else
				lane0_is_swing_max = 0;

			if (lane1_sw == VOL_SWING_LEVEL_NUM - 1)
				lane1_is_swing_max = 1;
			else
				lane1_is_swing_max = 0;

			if (lane0_pre == PRE_EMPHASIS_LEVEL_NUM - 1)
				lane0_is_pre_max = 1;
			else
				lane0_is_pre_max = 0;

			if (lane1_pre == PRE_EMPHASIS_LEVEL_NUM - 1)
				lane1_is_pre_max = 1;
			else
				lane1_is_pre_max = 0;
		} else if (g_lane_cnt == 4) {
			lane0_sw = g_rx_buf[0] & 0x3;
			lane0_pre = (g_rx_buf[0] >> 2) & 0x3;
			lane1_sw = (g_rx_buf[0] >> 4) & 0x3;
			lane1_pre = (g_rx_buf[0] >> 6) & 0x3;

			lane2_sw = g_rx_buf[1] & 0x3;
			lane2_pre = (g_rx_buf[1] >> 2) & 0x3;
			lane3_sw = (g_rx_buf[1] >> 4) & 0x3;
			lane3_pre = (g_rx_buf[1] >> 6) & 0x3;

			if ((lane0_sw != lane0_sw_old) ||
			    (lane1_sw != lane1_sw_old) ||
			    (lane2_sw != lane2_sw_old) ||
			    (lane3_sw != lane3_sw_old)) {
				lane0_sw_old = lane0_sw;
				lane1_sw_old = lane1_sw;
				lane2_sw_old = lane2_sw;
				lane3_sw_old = lane3_sw;
				to_cnt = TRAIN_CNT;
			}

			if ((lane0_pre != lane0_pre_old) ||
			    (lane1_pre != lane1_pre_old) ||
			    (lane2_pre != lane2_pre_old) ||
			    (lane3_pre != lane3_pre_old)) {
				lane0_pre_old = lane0_pre;
				lane1_pre_old = lane1_pre;
				lane2_pre_old = lane2_pre;
				lane3_pre_old = lane3_pre;
				to_cnt = TRAIN_CNT;
			}

			if (lane0_sw == VOL_SWING_LEVEL_NUM - 1)
				lane0_is_swing_max = 1;
			else
				lane0_is_swing_max = 0;

			if (lane1_sw == VOL_SWING_LEVEL_NUM - 1)
				lane1_is_swing_max = 1;
			else
				lane1_is_swing_max = 0;

			if (lane2_sw == VOL_SWING_LEVEL_NUM - 1)
				lane2_is_swing_max = 1;
			else
				lane2_is_swing_max = 0;

			if (lane3_sw == VOL_SWING_LEVEL_NUM - 1)
				lane3_is_swing_max = 1;
			else
				lane3_is_swing_max = 0;

			if (lane0_pre == PRE_EMPHASIS_LEVEL_NUM - 1)
				lane0_is_pre_max = 1;
			else
				lane0_is_pre_max = 0;

			if (lane1_pre == PRE_EMPHASIS_LEVEL_NUM - 1)
				lane1_is_pre_max = 1;
			else
				lane1_is_pre_max = 0;

			if (lane2_pre == PRE_EMPHASIS_LEVEL_NUM - 1)
				lane2_is_pre_max = 1;
			else
				lane2_is_pre_max = 0;

			if (lane3_pre == PRE_EMPHASIS_LEVEL_NUM - 1)
				lane3_is_pre_max = 1;
			else
				lane3_is_pre_max = 0;
		}

		to_cnt--;

		if (to_cnt == 0) {
			return RET_FAIL;
		}

		g_tx_buf[0] = 0x21; // set pattern 1 with scramble disable
		// set pattern 1 with max swing and emphasis 0x13
		g_tx_buf[1] =
		    ((lane0_is_pre_max & 0x1) << 5) | ((lane0_pre & 0x3) << 3) |
		    ((lane0_is_swing_max & 0x1) << 2) | (lane0_sw & 0x3);
		g_tx_buf[2] =
		    ((lane1_is_pre_max & 0x1) << 5) | ((lane1_pre & 0x3) << 3) |
		    ((lane1_is_swing_max & 0x1) << 2) | (lane1_sw & 0x3);
		g_tx_buf[3] =
		    ((lane2_is_pre_max & 0x1) << 5) | ((lane2_pre & 0x3) << 3) |
		    ((lane2_is_swing_max & 0x1) << 2) | (lane2_sw & 0x3);
		g_tx_buf[4] =
		    ((lane3_is_pre_max & 0x1) << 5) | ((lane3_pre & 0x3) << 3) |
		    ((lane3_is_swing_max & 0x1) << 2) | (lane3_sw & 0x3);

		/*set training pattern*/
		edp_training_pattern_config(sel, 1);

		edp_link_lane_para_setting(sel, lane0_sw, lane0_pre, lane1_sw, lane1_pre,
			    lane2_sw, lane2_pre, lane3_sw, lane3_pre);
		mdelay(5);

		ret = edp_aux_write(sel, 0x00102, g_lane_cnt + 1, g_tx_buf);

		if (ret != RET_OK)
			return RET_FAIL;
	}
}

s32 edp_link_training2(u32 sel, struct edp_lane_para *para, bool bypass_1st_train)
{
	u32 to_cnt;
	s32 ret;
	s32 eq_end = 0;
	u32 lane_cnt = para->lane_cnt;

	u32 lane0_sw, lane0_pre;
	u32 lane1_sw, lane1_pre;
	u32 lane2_sw, lane2_pre;
	u32 lane3_sw, lane3_pre;
	u32 lane0_is_pre_max, lane1_is_pre_max, lane2_is_pre_max,
	    lane3_is_pre_max;
	u32 lane0_is_swing_max, lane1_is_swing_max, lane2_is_swing_max,
	    lane3_is_swing_max;

	u32 lane0_sw_old, lane0_pre_old;
	u32 lane1_sw_old, lane1_pre_old;
	u32 lane2_sw_old, lane2_pre_old;
	u32 lane3_sw_old, lane3_pre_old;

	lane0_sw = 0;
	lane0_pre = 0;
	lane1_sw = 0;
	lane1_pre = 0;
	lane2_sw = 0;
	lane2_pre = 0;
	lane3_sw = 0;
	lane3_pre = 0;

	lane0_sw_old = 0;
	lane0_pre_old = 0;
	lane1_sw_old = 0;
	lane1_pre_old = 0;
	lane2_sw_old = 0;
	lane2_pre_old = 0;
	lane3_sw_old = 0;
	lane3_pre_old = 0;

	lane0_is_pre_max = 0;
	lane1_is_pre_max = 0;
	lane2_is_pre_max = 0;
	lane3_is_pre_max = 0;
	lane0_is_swing_max = 0;
	lane1_is_swing_max = 0;
	lane2_is_swing_max = 0;
	lane3_is_swing_max = 0;

	if (para != NULL) {
		if (para->lane0_sw)
			lane0_sw = para->lane0_sw;
		if (para->lane1_sw)
			lane1_sw = para->lane1_sw;
		if (para->lane2_sw)
			lane2_sw = para->lane2_sw;
		if (para->lane3_sw)
			lane3_sw = para->lane3_sw;

		if (para->lane0_pre)
			lane0_pre = para->lane0_pre;
		if (para->lane1_pre)
			lane1_pre = para->lane1_pre;
		if (para->lane2_pre)
			lane2_pre = para->lane2_pre;
		if (para->lane3_pre)
			lane3_pre = para->lane3_pre;

		if (lane0_sw == VOL_SWING_LEVEL_NUM - 1)
			lane0_is_swing_max = 1;
		else
			lane0_is_swing_max = 0;

		if (lane1_sw == VOL_SWING_LEVEL_NUM - 1)
			lane1_is_swing_max = 1;
		else
			lane1_is_swing_max = 0;

		if (lane2_sw == VOL_SWING_LEVEL_NUM - 1)
			lane2_is_swing_max = 1;
		else
			lane2_is_swing_max = 0;

		if (lane3_sw == VOL_SWING_LEVEL_NUM - 1)
			lane3_is_swing_max = 1;
		else
			lane3_is_swing_max = 0;

		if (lane0_pre == PRE_EMPHASIS_LEVEL_NUM - 1)
			lane0_is_pre_max = 1;
		else
			lane0_is_pre_max = 0;

		if (lane1_pre == PRE_EMPHASIS_LEVEL_NUM - 1)
			lane1_is_pre_max = 1;
		else
			lane1_is_pre_max = 0;

		if (lane2_pre == PRE_EMPHASIS_LEVEL_NUM - 1)
			lane2_is_pre_max = 1;
		else
			lane2_is_pre_max = 0;

		if (lane3_pre == PRE_EMPHASIS_LEVEL_NUM - 1)
			lane3_is_pre_max = 1;
		else
			lane3_is_pre_max = 0;
	}

	memset(g_rx_buf, 0, sizeof(g_rx_buf));
	memset(g_tx_buf, 0, sizeof(g_tx_buf));

	if (!bypass_1st_train) {
		to_cnt = TRAIN_CNT;
		edp_training_pattern_config(sel, 2);

		edp_link_lane_para_setting(sel, lane0_sw, lane0_pre, lane1_sw, lane1_pre,
			    lane2_sw, lane2_pre, lane3_sw, lane3_pre);

		mdelay(5);

		g_tx_buf[0] = 0x22; // set pattern 1 with scramble disable
		g_tx_buf[1] =
		    ((lane0_is_pre_max & 0x1) << 5) | ((lane0_pre & 0x3) << 3) |
		    ((lane0_is_swing_max & 0x1) << 2) | (lane0_sw & 0x3);
		g_tx_buf[2] =
		    ((lane1_is_pre_max & 0x1) << 5) | ((lane1_pre & 0x3) << 3) |
		    ((lane1_is_swing_max & 0x1) << 2) | (lane1_sw & 0x3);
		g_tx_buf[3] =
		    ((lane2_is_pre_max & 0x1) << 5) | ((lane2_pre & 0x3) << 3) |
		    ((lane2_is_swing_max & 0x1) << 2) | (lane2_sw & 0x3);
		g_tx_buf[4] =
		    ((lane3_is_pre_max & 0x1) << 5) | ((lane3_pre & 0x3) << 3) |
		    ((lane3_is_swing_max & 0x1) << 2) | (lane3_sw & 0x3);

		ret = edp_aux_write(sel, 0x00102, g_lane_cnt + 1, g_tx_buf);

		if (ret != RET_OK)
			return RET_FAIL;
	} else
		to_cnt = TRAIN_CNT + 1;


	while (1) {
		if (training_interval_EQ > 1) {
			/*wait for the training finish*/
			udelay(training_interval_EQ);
		} else {
			udelay(400);
		}
		ret = edp_aux_read(sel, 0x0202, 3, g_rx_buf);
		if (ret == -1)
			return RET_FAIL;

		if (lane_cnt < 4) {
			EDP_DBG("EQ training reg[202h]:0x%x\n", g_rx_buf[0]);
			EDP_DBG("EQ training reg[204h]:0x%x\n", g_rx_buf[2]);
		} else {
			EDP_DBG("EQ training reg[202h]:0x%x\n", g_rx_buf[0]);
			EDP_DBG("EQ training reg[203h]:0x%x\n", g_rx_buf[1]);
			EDP_DBG("EQ training reg[204h]:0x%x\n", g_rx_buf[2]);
		}

		if (lane_cnt == 1) {
			if ((g_rx_buf[0] & 0x01) != 0x01) {
				EDP_DBG("CR is not done before EQ training\n");
				return RET_FAIL;
			}
		} else if (lane_cnt == 2) {
			if (((g_rx_buf[0] & 0x11) != 0x11)) {
				EDP_DBG("CR is not done before EQ training\n");
				return RET_FAIL;
			}
		} else if (lane_cnt == 4) {
			if (((g_rx_buf[0] & 0x11) != 0x11) &&
			    ((g_rx_buf[1] & 0x11) != 0x11)) {
				EDP_DBG("CR is not done before EQ training\n");
				return RET_FAIL;
			}
		}

		if (g_rx_buf[2] & 0x01) {
			if (lane_cnt == 1) {
				if ((g_rx_buf[0] & 0x07) == 0x07) {
					eq_end = 1;
				}
			} else if (lane_cnt == 2) {
				if (((g_rx_buf[0] & 0x77) == 0x77)) {
					eq_end = 1;
				}
			} else if (lane_cnt == 4) {
				if (((g_rx_buf[0] & 0x77) == 0x77) &&
				    ((g_rx_buf[1] & 0x77) == 0x77)) {
					eq_end = 1;
				}
			}

			if (eq_end == 1) {
				g_tx_buf[0] = 0x00; // 102 --- indicate the end
						     // of training
				g_tx_buf[1] = 0x00; /* 103 */
				g_tx_buf[2] = 0x00; /* 104 */
				g_tx_buf[3] = 0x00; /* 105 */
				g_tx_buf[4] = 0x00; /* 106 */
				g_tx_buf[5] = 0x00; /* 107 */
				g_tx_buf[6] = 0x01; /* 108 */
				g_tx_buf[7] = 0x00; /* 109 */
				g_tx_buf[8] = 0x00; /* 10a */
				g_tx_buf[9] = 0x00;	 /* 10b */
				g_tx_buf[10] = 0x00;	/* 10c */
				g_tx_buf[11] = 0x00;	/* 10d */
				g_tx_buf[12] = 0x00;	/* 10e */
				ret = edp_aux_write(sel, 0x0102, 13, g_tx_buf);
				if (ret != RET_OK)
					return RET_FAIL;

				return RET_OK;
			}
		}

		if (lane_cnt < 4)
			ret = edp_aux_read(sel, 0x0206, 1, g_rx_buf);
		else
			ret = edp_aux_read(sel, 0x0206, 2, g_rx_buf);
		if (ret == -1)
			return RET_FAIL;
		if (lane_cnt < 4)
			EDP_DBG("EQ training reg[206h]:%xh\n", g_rx_buf[0]);
		else {
			EDP_DBG("EQ training reg[206h]:%xh\n", g_rx_buf[0]);
			EDP_DBG("EQ training reg[207h]:%xh\n", g_rx_buf[1]);
		}

		if (lane_cnt == 1) {
			lane0_sw = g_rx_buf[0] & 0x3;
			lane0_pre = (g_rx_buf[0] >> 2) & 0x3;

			if ((lane0_sw != lane0_sw_old)) {
				lane0_sw_old = lane0_sw;
				to_cnt = TRAIN_CNT;
			}

			if ((lane0_pre != lane0_pre_old)) {
				lane0_pre_old = lane0_pre;
				to_cnt = TRAIN_CNT;
			}

			if (lane0_sw == VOL_SWING_LEVEL_NUM - 1)
				lane0_is_swing_max = 1;
			else
				lane0_is_swing_max = 0;

			if (lane0_pre == PRE_EMPHASIS_LEVEL_NUM - 1)
				lane0_is_pre_max = 1;
			else
				lane0_is_pre_max = 0;
		} else if (lane_cnt == 2) {
			lane0_sw = g_rx_buf[0] & 0x3;
			lane0_pre = (g_rx_buf[0] >> 2) & 0x3;
			lane1_sw = (g_rx_buf[0] >> 4) & 0x3;
			lane1_pre = (g_rx_buf[0] >> 6) & 0x3;

			if ((lane0_sw != lane0_sw_old) ||
			    (lane1_sw != lane1_sw_old)) {

				lane0_sw_old = lane0_sw;
				lane1_sw_old = lane1_sw;
				to_cnt = TRAIN_CNT;
			}

			if ((lane0_pre != lane0_pre_old) ||
			    (lane1_pre != lane1_pre_old)) {
				lane0_pre_old = lane0_pre;
				lane1_pre_old = lane1_pre;
				to_cnt = TRAIN_CNT;
			}

			if (lane0_sw == VOL_SWING_LEVEL_NUM - 1)
				lane0_is_swing_max = 1;
			else
				lane0_is_swing_max = 0;

			if (lane1_sw == VOL_SWING_LEVEL_NUM - 1)
				lane1_is_swing_max = 1;
			else
				lane1_is_swing_max = 0;

			if (lane0_pre == PRE_EMPHASIS_LEVEL_NUM - 1)
				lane0_is_pre_max = 1;
			else
				lane0_is_pre_max = 0;

			if (lane1_pre == PRE_EMPHASIS_LEVEL_NUM - 1)
				lane1_is_pre_max = 1;
			else
				lane1_is_pre_max = 0;
		} else if (lane_cnt == 4) {
			lane0_sw = g_rx_buf[0] & 0x3;
			lane0_pre = (g_rx_buf[0] >> 2) & 0x3;
			lane1_sw = (g_rx_buf[0] >> 4) & 0x3;
			lane1_pre = (g_rx_buf[0] >> 6) & 0x3;

			lane2_sw = g_rx_buf[1] & 0x3;
			lane2_pre = (g_rx_buf[1] >> 2) & 0x3;
			lane3_sw = (g_rx_buf[1] >> 4) & 0x3;
			lane3_pre = (g_rx_buf[1] >> 6) & 0x3;

			if ((lane0_sw != lane0_sw_old) ||
			    (lane1_sw != lane1_sw_old) ||
			    (lane2_sw != lane2_sw_old) ||
			    (lane3_sw != lane3_sw_old)) {
				lane0_sw_old = lane0_sw;
				lane1_sw_old = lane1_sw;
				lane2_sw_old = lane2_sw;
				lane3_sw_old = lane3_sw;
				to_cnt = TRAIN_CNT;
			}

			if ((lane0_pre != lane0_pre_old) ||
			    (lane1_pre != lane1_pre_old) ||
			    (lane2_pre != lane2_pre_old) ||
			    (lane3_pre != lane3_pre_old)) {
				lane0_pre_old = lane0_pre;
				lane1_pre_old = lane1_pre;
				lane2_pre_old = lane2_pre;
				lane3_pre_old = lane3_pre;
				to_cnt = TRAIN_CNT;
			}

			if (lane0_sw == VOL_SWING_LEVEL_NUM - 1)
				lane0_is_swing_max = 1;
			else
				lane0_is_swing_max = 0;

			if (lane1_sw == VOL_SWING_LEVEL_NUM - 1)
				lane1_is_swing_max = 1;
			else
				lane1_is_swing_max = 0;

			if (lane2_sw == VOL_SWING_LEVEL_NUM - 1)
				lane2_is_swing_max = 1;
			else
				lane2_is_swing_max = 0;

			if (lane3_sw == VOL_SWING_LEVEL_NUM - 1)
				lane3_is_swing_max = 1;
			else
				lane3_is_swing_max = 0;

			if (lane0_pre == PRE_EMPHASIS_LEVEL_NUM - 1)
				lane0_is_pre_max = 1;
			else
				lane0_is_pre_max = 0;

			if (lane1_pre == PRE_EMPHASIS_LEVEL_NUM - 1)
				lane1_is_pre_max = 1;
			else
				lane1_is_pre_max = 0;

			if (lane2_pre == PRE_EMPHASIS_LEVEL_NUM - 1)
				lane2_is_pre_max = 1;
			else
				lane2_is_pre_max = 0;

			if (lane3_pre == PRE_EMPHASIS_LEVEL_NUM - 1)
				lane3_is_pre_max = 1;
			else
				lane3_is_pre_max = 0;
		}

		to_cnt--;

		if (to_cnt == 0) {
			return RET_FAIL;
		}

		g_tx_buf[0] = 0x22; /* set pattern 2 with scramble disable */
		/* set pattern 1 with max swing and emphasis 0x13 */
		g_tx_buf[1] =
		    ((lane0_is_pre_max & 0x1) << 5) | ((lane0_pre & 0x3) << 3) |
		    ((lane0_is_swing_max & 0x1) << 2) | (lane0_sw & 0x3);
		g_tx_buf[2] =
		    ((lane1_is_pre_max & 0x1) << 5) | ((lane1_pre & 0x3) << 3) |
		    ((lane1_is_swing_max & 0x1) << 2) | (lane1_sw & 0x3);
		g_tx_buf[3] =
		    ((lane2_is_pre_max & 0x1) << 5) | ((lane2_pre & 0x3) << 3) |
		    ((lane2_is_swing_max & 0x1) << 2) | (lane2_sw & 0x3);
		g_tx_buf[4] =
		    ((lane3_is_pre_max & 0x1) << 5) | ((lane3_pre & 0x3) << 3) |
		    ((lane3_is_swing_max & 0x1) << 2) | (lane3_sw & 0x3);

		edp_training_pattern_config(sel, 2);

		edp_link_lane_para_setting(sel, lane0_sw, lane0_pre, lane1_sw, lane1_pre,
			    lane2_sw, lane2_pre, lane3_sw, lane3_pre);

		mdelay(20);

		ret = edp_aux_write(sel, 0x0102, g_lane_cnt + 1, g_tx_buf);
		if (ret != RET_OK)
			return RET_FAIL;
	}
}


void edp_transmit_unit_config(u32 sel, u32 lane_cnt, u32 bit_rate, u32 pixel_clk)
{
	u32 reg_val;
	u32 pack_data_rate;
	u32 valid_symbol;
	u32 hblank;
	u32 bandwidth;
	u32 pre_muti = 1000;

	reg_val = readl(edp_base[sel] + REG_EDP_HACTIVE_BLANK);
	hblank = GET_BITS(2, 14, reg_val);

	/*
	 * avg valid syobol per TU: pack_data_rate / bandwidth * LS_PER_TU
	 * pack_data_rate = bpp * pix_clk / 8bit / lane_cnt (1 symbol is 8 bit)
	 */
	pixel_clk = pixel_clk / 1000000;
	bandwidth = bit_rate / 10000000;

	pack_data_rate = g_bpp * pixel_clk / (8 * lane_cnt);
	valid_symbol = pre_muti * LS_PER_TU * pack_data_rate / bandwidth;

	reg_val = readl(edp_base[sel] + REG_EDP_FRAME_UNIT);
	reg_val = SET_BITS(0, 7, reg_val, valid_symbol / pre_muti);
	reg_val = SET_BITS(16, 4, reg_val, (valid_symbol % pre_muti) / 100);

	if ((valid_symbol / 1000) < 6)
		reg_val = SET_BITS(7, 7, reg_val, 32);
	else {
		if (hblank < 80)
			reg_val = SET_BITS(7, 7, reg_val, 12);
		else
			reg_val = SET_BITS(7, 7, reg_val, 16);
	}
	writel(reg_val, edp_base[sel] + REG_EDP_FRAME_UNIT);
}


void edp_set_link_clk_cyc(u32 sel, u32 bit_rate, u32 pixel_clk)
{
	u32 reg_val;
	u32 hblank;
	u32 symbol_clk;
	u32 link_cyc;

	/*hblank_link_cyc = hblank * (symbol_clk / 4) / pixclk*/
	reg_val = readl(edp_base[sel] + REG_EDP_HACTIVE_BLANK);
	hblank = GET_BITS(2, 14, reg_val);

	symbol_clk = bit_rate / 10000000;


	link_cyc = 1000 * hblank * (symbol_clk / 4) / (pixel_clk / 1000);
	printk("!!!! link_cyc:%d\n", link_cyc);

	reg_val = readl(edp_base[sel] + REG_EDP_HBLANK_LINK_CYC);
	reg_val = SET_BITS(0, 16, reg_val, link_cyc);
	writel(reg_val, edp_base[sel] + REG_EDP_HBLANK_LINK_CYC);
}

/*edp_hal_xxx means xxx function is from lowlevel*/
s32 edp_hal_phy_init(u32 sel, struct edp_tx_core *edp_core)
{
	s32 ret = 0;

	ret = edp_bist_test(sel, edp_core);
	if (ret < 0)
		return ret;
	edp_phy_soft_reset(sel);
	edp_hpd_enable(sel);

	return ret;
}

void edp_hal_set_reg_base(u32 sel, uintptr_t base)
{
	edp_base[sel] = (void __iomem *)(base);
}

s32 edp_hal_enable(u32 sel, struct edp_tx_core *edp_core)
{
	u32 bit_rate;
	u32 lane_cnt;
	u32 support_hbr2;
	s32 ret;

	bit_rate = edp_core->lane_para.bit_rate;
	lane_cnt = edp_core->lane_para.lane_cnt;
	support_hbr2 = edp_core->lane_para.support_hbr2;

	edp_hal_lane_config(sel, edp_core);
	ret = edp_hal_sink_init(sel, bit_rate, lane_cnt);
	if (ret < 0) {
		EDP_ERR("edp sink init fail!\n");
		return ret;
	}
	edp_assr_enable(sel);

	edp_hal_link_training1(sel, edp_core, false);
	edp_hal_link_training2(sel, edp_core, false);
	if (support_hbr2)
		edp_hal_link_training3(sel, edp_core, false);
	ret = edp_hal_link_start(sel);

	return ret;
}

s32 edp_hal_disable(u32 sel, struct edp_tx_core *edp_core)
{
	edp_video_stream_disable(sel);

	return 0;
}


void edp_hal_lane_config(u32 sel, struct edp_tx_core *edp_core)
{
	u32 lane0_sw, lane0_pre;
	u32 lane1_sw, lane1_pre;
	u32 lane2_sw, lane2_pre;
	u32 lane3_sw, lane3_pre;
	u32 bit_rate, lane_cnt;

	lane0_sw = edp_core->lane_para.lane0_sw;
	lane1_sw = edp_core->lane_para.lane1_sw;
	lane2_sw = edp_core->lane_para.lane2_sw;
	lane3_sw = edp_core->lane_para.lane3_sw;

	lane0_pre = edp_core->lane_para.lane0_pre;
	lane1_pre = edp_core->lane_para.lane1_pre;
	lane2_pre = edp_core->lane_para.lane2_pre;
	lane3_pre = edp_core->lane_para.lane3_pre;

	lane_cnt = edp_core->lane_para.lane_cnt;
	bit_rate = edp_core->lane_para.bit_rate;

	edp_lane_config(sel, lane_cnt, bit_rate);
	edp_link_lane_para_setting(sel, lane0_sw, lane0_pre, lane1_sw, lane1_pre,
			    lane2_sw, lane2_pre, lane3_sw, lane3_pre);

	edp_corepll_config(sel, bit_rate);
}

s32 edp_hal_link_training1(u32 sel, struct edp_tx_core *edp_core, bool bypass_1st_train)
{
	return edp_link_training1(sel, &edp_core->lane_para, bypass_1st_train);
}

s32 edp_hal_link_training2(u32 sel, struct edp_tx_core *edp_core, bool bypass_1st_train)
{
	return edp_link_training2(sel, &edp_core->lane_para, bypass_1st_train);
}

s32  edp_hal_link_training3(u32 sel, struct edp_tx_core *edp_core, bool bypass_1st_train)
{
	return 0;
}

s32 edp_hal_get_training_info(u32 sel, struct edp_lane_para *para)
{
	para->lane0_sw = recom_training_para.lane0_sw;
	para->lane0_pre = recom_training_para.lane0_pre;
	para->lane1_sw = recom_training_para.lane1_sw;
	para->lane1_pre = recom_training_para.lane1_pre;
	para->lane2_sw = recom_training_para.lane2_sw;
	para->lane2_pre = recom_training_para.lane2_pre;
	para->lane3_sw = recom_training_para.lane3_sw;
	para->lane3_pre = recom_training_para.lane3_pre;

	return 0;
}

s32 edp_hal_read_edid(u32 sel, struct edid *edid)
{
	s32 i;
	s32 ret;

	for (i = 0; i < (EDID_LENGTH / 16); i++) {
		memset(edid_tx_buf, 0, 16);
		ret = edp_hal_aux_i2c_write(sel, EDID_ADDR, 0, edid_tx_buf);
		if (ret < 0)
			return ret;

		ret = edp_hal_aux_i2c_read(sel, EDID_ADDR, 16, (char *)(edid) + (i * 16));
		if (ret < 0)
			return ret;

	}
	for (i = 0; i < 128; i++)
		printk("!!!!edid[%d] = 0x%x\n", i, *(char *)(edid) + i);

	return 0;
}

s32 edp_hal_read_edid_ext(u32 sel, u32 block_cnt, struct edid *edid)
{
	s32 i, j, ret;
	struct edid *edid_ext;

	for (i = 1; i < block_cnt; i++) {
		edid_ext = edid + i;
		ret = edp_hal_read_edid(sel, edid_ext);
		if (ret < 0)
			return ret;

		for (j = 0; j < 128; j++)
			printk("!!!!edid_ext[%d] = 0x%x\n", j, *(char *)(edid_ext) + j);
	}

	return 0;
}

void edp_hal_set_video_timing(u32 sel, struct disp_video_timings *timings)
{
	u32 reg_val;

	/*hsync/vsync polarity setting*/
	reg_val = readl(edp_base[sel] + REG_EDP_SYNC_POLARITY);
	reg_val = SET_BITS(1, 1, reg_val, timings->hor_sync_polarity);
	reg_val = SET_BITS(0, 1, reg_val, timings->ver_sync_polarity);
	writel(reg_val, edp_base[sel] + REG_EDP_SYNC_POLARITY);

	/*h/vactive h/vblank setting*/
	reg_val = readl(edp_base[sel] + REG_EDP_HACTIVE_BLANK);
	reg_val = SET_BITS(16, 16, reg_val, timings->x_res);
	reg_val = SET_BITS(2, 14, reg_val, (timings->hor_total_time - timings->x_res));
	writel(reg_val, edp_base[sel] + REG_EDP_HACTIVE_BLANK);

	reg_val = readl(edp_base[sel] + REG_EDP_VACTIVE_BLANK);
	reg_val = SET_BITS(0, 16, reg_val, timings->y_res);
	reg_val = SET_BITS(16, 16, reg_val, (timings->ver_total_time - timings->y_res));
	writel(reg_val, edp_base[sel] + REG_EDP_VACTIVE_BLANK);

	/*h/vstart setting*/
	reg_val = readl(edp_base[sel] + REG_EDP_SYNC_START);
	reg_val = SET_BITS(0, 16, reg_val, (timings->hor_sync_time + timings->hor_back_porch));
	reg_val = SET_BITS(16, 16, reg_val, (timings->ver_sync_time + timings->ver_back_porch));
	writel(reg_val, edp_base[sel] + REG_EDP_SYNC_START);

	/*hs/vswidth  h/v_front_porch setting*/
	reg_val = readl(edp_base[sel] + REG_EDP_HSW_FRONT_PORCH);
	reg_val = SET_BITS(16, 16, reg_val, timings->hor_sync_time);
	reg_val = SET_BITS(0, 16, reg_val, (timings->hor_total_time - timings->x_res - \
			   timings->hor_sync_time - timings->hor_back_porch));
	writel(reg_val, edp_base[sel] + REG_EDP_HSW_FRONT_PORCH);

	reg_val = readl(edp_base[sel] + REG_EDP_VSW_FRONT_PORCH);
	reg_val = SET_BITS(16, 16, reg_val, timings->ver_sync_time);
	reg_val = SET_BITS(0, 16, reg_val, (timings->ver_total_time - timings->y_res -
			   timings->ver_sync_time - timings->ver_back_porch));
	writel(reg_val, edp_base[sel] + REG_EDP_VSW_FRONT_PORCH);
}


void edp_hal_set_video_mode(u32 sel, struct edp_tx_core *edp_core)
{
	struct disp_video_timings *tmgs;
	u32 pixel_clk;
	u32 colordepth;
	u32 bit_rate;
	u32 video_map;
	u32 lane_cnt;

	tmgs = &edp_core->timings;
	pixel_clk = tmgs->pixel_clk;
	colordepth = edp_core->lane_para.colordepth;
	bit_rate = edp_core->lane_para.bit_rate;
	lane_cnt = edp_core->lane_para.lane_cnt;

	switch (colordepth) {
	case 6:
		video_map = RGB_6BIT;
		break;
	case 8:
		video_map = RGB_8BIT;
		break;
	case 10:
		video_map = RGB_10BIT;
		break;
	case 12:
		video_map = RGB_12BIT;
		break;
	case 16:
		video_map = RGB_16BIT;
		break;
	}

	edp_hal_set_video_timing(sel, tmgs);
	edp_pixpll_cfg(sel, pixel_clk);
	edp_set_input_video_mapping(sel, (enum edp_video_mapping_e) video_map);
	edp_set_link_clk_cyc(sel, bit_rate, pixel_clk);
	edp_transmit_unit_config(sel, lane_cnt, bit_rate, pixel_clk);
}

s32 edp_hal_audio_enable(u32 sel)
{
	edp_audio_timestamp_hblank_setting(sel, true);
	edp_audio_timestamp_vblank_setting(sel, true);
	edp_audio_stream_hblank_setting(sel, true);
	edp_audio_stream_vblank_setting(sel, true);
	edp_audio_soft_reset(sel);

	return RET_OK;
}

s32 edp_hal_audio_disable(u32 sel)
{
	edp_audio_timestamp_hblank_setting(sel, false);
	edp_audio_timestamp_vblank_setting(sel, false);
	edp_audio_stream_hblank_setting(sel, false);
	edp_audio_stream_vblank_setting(sel, false);

	return RET_OK;
}


s32 edp_hal_audio_set_para(u32 sel, edp_audio_t *para)
{
	edp_audio_interface_config(sel, para->interface);
	edp_audio_channel_config(sel, para->chn_cnt);
	edp_audio_mute_config(sel, para->mute);
	edp_audio_data_width_config(sel, para->data_width);

	return RET_OK;
}

s32 edp_hal_ssc_enable(u32 sel, bool enable)
{
	u32 reg_val;

	reg_val = readl(edp_base[sel] + REG_EDP_ANA_PLL_FBDIV);
	if (enable)
		reg_val = SET_BITS(21, 1, reg_val, 0);
	else
		reg_val = SET_BITS(21, 1, reg_val, 1);
	writel(reg_val, edp_base[sel] + REG_EDP_ANA_PLL_FBDIV);

	return RET_OK;
}

bool edp_hal_ssc_is_enabled(u32 sel)
{
	u32 reg_val;

	reg_val = readl(edp_base[sel] + REG_EDP_ANA_PLL_FBDIV);
	reg_val = GET_BITS(21, 1, reg_val);

	if (!reg_val)
		return true;

	return false;
}

s32 edp_hal_ssc_get_mode(u32 sel)
{
	u32 reg_val;

	reg_val = readl(edp_base[sel] + REG_EDP_ANA_PLL_FBDIV);
	reg_val = GET_BITS(20, 1, reg_val);

	return reg_val;
}

s32 edp_hal_ssc_set_mode(u32 sel, u32 mode)
{
	u32 reg_val;

	reg_val = readl(edp_base[sel] + REG_EDP_ANA_PLL_FBDIV);

	switch (mode) {
	case SSC_CENTER_MODE:
		reg_val = SET_BITS(20, 1, reg_val, 0);
		break;
	case SSC_DOWNSPR_MODE:
	default:
		reg_val = SET_BITS(20, 1, reg_val, 1);
		break;
	}

	writel(reg_val, edp_base[sel] + REG_EDP_ANA_PLL_FBDIV);

	return RET_OK;
}


s32 edp_hal_psr_enable(u32 sel, bool enable)
{
	EDP_ERR("psr isn't support\n");
	return RET_FAIL;
}

bool edp_hal_psr_is_enabled(u32 sel)
{
	EDP_ERR("psr isn't support\n");
	return false;
}

s32 edp_hal_get_color_fmt(u32 sel)
{
	u32 reg_val;

	reg_val = readl(edp_base[sel] + REG_EDP_VIDEO_STREAM_EN);

	reg_val = GET_BITS(16, 5, reg_val);

	switch (reg_val) {
	case 0:
		return RGB_6BIT;
	case 1:
		return RGB_8BIT;
	case 2:
		return RGB_10BIT;
	case 3:
		return RGB_12BIT;
	case 4:
		return RGB_16BIT;
	case 5:
		return YCBCR444_8BIT;
	case 6:
		return YCBCR444_10BIT;
	case 7:
		return YCBCR444_12BIT;
	case 8:
		return YCBCR444_16BIT;
	case 9:
		return YCBCR422_8BIT;
	case 10:
		return YCBCR422_10BIT;
	case 11:
		return YCBCR422_12BIT;
	case 12:
		return YCBCR422_16BIT;
	}

	return RET_FAIL;
}


s32 edp_hal_get_pixclk(u32 sel)
{
	u32 reg_val;
	u32 fb_div;
	u32 pre_div;
	u32 pll_divb;
	u32 pll_divc;
	u32 pixclk;

	reg_val = readl(edp_base[sel] + REG_EDP_ANA_PIXPLL_FBDIV);
	fb_div = GET_BITS(24, 8, reg_val);
	pre_div = GET_BITS(8, 6, reg_val);

	reg_val = readl(edp_base[sel] + REG_EDP_ANA_PIXPLL_DIV);
	pll_divc = GET_BITS(16, 5, reg_val);
	reg_val = GET_BITS(8, 2, reg_val);
	if (reg_val == 1)
		pll_divb = 2;
	else if (reg_val == 2)
		pll_divb = 3;
	else if (reg_val == 3)
		pll_divb = 5;
	else
		pll_divb = 1;

	pixclk = (24 * fb_div) / pre_div;
	pixclk = pixclk / (2 * pll_divb * pll_divc);

	return pixclk * 1000000;
}

s32 edp_hal_get_train_pattern(u32 sel)
{
	u32 reg_val;

	reg_val = readl(edp_base[sel] + REG_EDP_CAPACITY);

	reg_val = GET_BITS(0, 1, reg_val);

	return reg_val;
}

s32 edp_hal_get_lane_para(u32 sel, struct edp_lane_para *tmp_lane_para)
{
	u32 reg_val;
	u32 regval;

	/* bit rate */
	reg_val = readl(edp_base[sel] + REG_EDP_CAPACITY);
	regval = GET_BITS(26, 3, reg_val);
	if (regval == 1)
		tmp_lane_para->bit_rate = 2160000000;
	else if (regval == 2)
		tmp_lane_para->bit_rate = 2430000000;
	else {
		regval = GET_BITS(4, 2, reg_val);
		if (regval == 0)
			tmp_lane_para->bit_rate = 1620000000;
		else if (regval == 1)
			tmp_lane_para->bit_rate = 2700000000;
		else
			tmp_lane_para->bit_rate = 0;
	}

	/* lane count */
	regval = GET_BITS(6, 2, reg_val);
	if (regval == 0)
		tmp_lane_para->lane_cnt = 1;
	else if (regval == 1)
		tmp_lane_para->lane_cnt = 2;
	else if (regval == 2)
		tmp_lane_para->lane_cnt = 4;
	else
		tmp_lane_para->lane_cnt = 0;

	/*sw*/
	reg_val = readl(edp_base[sel] + REG_EDP_TX_MAINSEL);
	regval = GET_BITS(0, 4, reg_val);
	if (regval == 0)
		tmp_lane_para->lane0_sw = 0;
	if (regval == 2)
		tmp_lane_para->lane0_sw = 1;
	if (regval == 4)
		tmp_lane_para->lane0_sw = 2;
	if (regval == 6)
		tmp_lane_para->lane0_sw = 3;

	regval = GET_BITS(4, 4, reg_val);
	if (regval == 0)
		tmp_lane_para->lane1_sw = 0;
	if (regval == 2)
		tmp_lane_para->lane1_sw = 1;
	if (regval == 4)
		tmp_lane_para->lane1_sw = 2;
	if (regval == 6)
		tmp_lane_para->lane1_sw = 3;

	reg_val = readl(edp_base[sel] + REG_EDP_TX32_ISEL_DRV);
	regval = GET_BITS(24, 4, reg_val);
	if (regval == 0)
		tmp_lane_para->lane2_sw = 0;
	if (regval == 2)
		tmp_lane_para->lane2_sw = 1;
	if (regval == 4)
		tmp_lane_para->lane2_sw = 2;
	if (regval == 6)
		tmp_lane_para->lane2_sw = 3;

	regval = GET_BITS(28, 4, reg_val);
	if (regval == 0)
		tmp_lane_para->lane3_sw = 0;
	if (regval == 2)
		tmp_lane_para->lane3_sw = 1;
	if (regval == 4)
		tmp_lane_para->lane3_sw = 2;
	if (regval == 6)
		tmp_lane_para->lane3_sw = 3;


	/*pre*/
	reg_val = readl(edp_base[sel] + REG_EDP_TX_POSTSEL);
	regval = GET_BITS(24, 4, reg_val);
	if (regval == 0)
		tmp_lane_para->lane0_pre = 0;
	if (regval == 1)
		tmp_lane_para->lane0_pre = 1;
	if (regval == 2)
		tmp_lane_para->lane0_pre = 2;
	if (regval == 3)
		tmp_lane_para->lane0_pre = 3;

	regval = GET_BITS(28, 4, reg_val);
	if (regval == 0)
		tmp_lane_para->lane1_pre = 0;
	if (regval == 1)
		tmp_lane_para->lane1_pre = 1;
	if (regval == 2)
		tmp_lane_para->lane1_pre = 2;
	if (regval == 3)
		tmp_lane_para->lane1_pre = 3;

	regval = GET_BITS(16, 4, reg_val);
	if (regval == 0)
		tmp_lane_para->lane2_pre = 0;
	if (regval == 1)
		tmp_lane_para->lane2_pre = 1;
	if (regval == 2)
		tmp_lane_para->lane2_pre = 2;
	if (regval == 3)
		tmp_lane_para->lane2_pre = 3;

	regval = GET_BITS(20, 4, reg_val);
	if (regval == 0)
		tmp_lane_para->lane3_pre = 0;
	if (regval == 1)
		tmp_lane_para->lane3_pre = 1;
	if (regval == 2)
		tmp_lane_para->lane3_pre = 2;
	if (regval == 3)
		tmp_lane_para->lane3_pre = 3;

	return RET_OK;
}

s32 edp_hal_get_tu_size(u32 sel)
{
	u32 reg_val;
	u32 regval;
	u32 tu_size;

	reg_val = readl(edp_base[sel] + REG_EDP_FRAME_UNIT);
	regval = GET_BITS(0, 7, reg_val);
	tu_size = regval * 10;

	regval = GET_BITS(16, 4, reg_val);
	tu_size += regval;

	return tu_size;
}

s32 edp_hal_get_symbol_rate(u32 sel)
{
	return 64;
}

bool edp_hal_audio_is_enabled(u32 sel)
{
	u32 reg_val;
	u32 regval0;
	u32 regval1;
	u32 regval2;
	u32 regval3;

	reg_val = readl(edp_base[sel] + REG_EDP_AUDIO_HBLANK_EN);
	regval0 = GET_BITS(0, 1, reg_val);
	regval1 = GET_BITS(1, 1, reg_val);

	reg_val = readl(edp_base[sel] + REG_EDP_AUDIO_VBLANK_EN);
	regval2 = GET_BITS(0, 1, reg_val);
	regval3 = GET_BITS(1, 1, reg_val);

	if (regval0 && regval1 && regval2 && regval3)
		return true;

	return false;
}

s32 edp_hal_get_audio_if(u32 sel)
{
	u32 reg_val;

	reg_val = readl(edp_base[sel] + REG_EDP_AUDIO);
	reg_val = GET_BITS(0, 1, reg_val);

	return reg_val;
}

s32 edp_hal_audio_is_mute(u32 sel)
{
	u32 reg_val;

	reg_val = readl(edp_base[sel] + REG_EDP_AUDIO);
	reg_val = GET_BITS(15, 1, reg_val);

	return reg_val;
}

s32 edp_hal_get_audio_chn_cnt(u32 sel)
{
	u32 reg_val;

	reg_val = readl(edp_base[sel] + REG_EDP_AUDIO);
	reg_val = GET_BITS(12, 3, reg_val);

	if (reg_val == 0)
		return 1;
	else if (reg_val == 1)
		return 2;
	else
		return 8;
}

s32 edp_hal_get_audio_date_width(u32 sel)
{
	u32 reg_val;

	reg_val = readl(edp_base[sel] + REG_EDP_AUDIO);
	reg_val = GET_BITS(5, 5, reg_val);

	if (reg_val == 0x10)
		return 16;
	else if (reg_val == 0x14)
		return 20;
	else
		return 24;
}

s32 edp_hal_low_power_mode_enable(u32 sel, bool en)
{
	if (en)
		g_tx_buf[0] = 0x02;
	else
		g_tx_buf[0] = 0x01;
	return edp_aux_write(sel, 0x00600, 1,
		     g_tx_buf);
}

s32 edp_hal_read_dpcd(u32 sel, char *dpcd_rx_buf)
{
	s32 i, blk, ret = 0;

	/*link configuration*/
	blk = 8;
	/* read 16 Byte dp sink capability */
	for (i = 0; i < 256 / blk; i++) {
		ret = edp_aux_read(sel, 0x0000 + i * blk, blk, dpcd_rx_buf + i * blk);
		if (ret < 0)
			return ret;
	}

	switch (dpcd_rx_buf[0x0e]) {
	case 0x00:
		/*Link Status/Adjust Request read interval during CR*/
		/*phase --- 100us*/
		training_interval_CR = 100;
		/*Link Status/Adjust Request read interval during EQ*/
		/*phase --- 400us*/
		training_interval_EQ = 400;

		break;
	case 0x01:
		training_interval_CR = 4000;
		training_interval_EQ = 4000;
		break;
	case 0x02:
		training_interval_CR = 8000;
		training_interval_EQ = 8000;
		break;
	case 0x03:
		training_interval_CR = 12000;
		training_interval_EQ = 12000;
		break;
	case 0x04:
		training_interval_CR = 16000;
		training_interval_EQ = 16000;
		break;
	default:
		training_interval_CR = 100;
		training_interval_EQ = 400;
	}

	return ret;
}

s32 edp_hal_sink_init(u32 sel, u32 bit_rate, u32 lane_cnt)
{
	s32 ret = 0;

	/*link configuration*/
	/* set bandwidth set lane_cnt */
	g_tx_buf[0] = bit_rate / 10000000 / 27;
	g_tx_buf[1] = lane_cnt;
	ret = edp_aux_write(sel, 0x00100, 2, g_tx_buf);
	if (ret == -1)
		return ret;

	/*set sink to D0 mode(Normal Operation Mode)*/
	/*--- DP_PWR keeps the default 3.3V*/
	ret = edp_hal_low_power_mode_enable(sel, false);

	return ret;
}






void edp_hpd_irq_enable(u32 sel)
{
	writel(0x1, edp_base[sel] + REG_EDP_HPD_INT);
	writel(0x1, edp_base[sel] + REG_EDP_HPD_EN);
}

void edp_hpd_irq_disable(u32 sel)
{
	writel(0x0, edp_base[sel] + REG_EDP_HPD_INT);
	writel(0x0, edp_base[sel] + REG_EDP_HPD_EN);
}

void edp_hpd_disable(u32 sel)
{
	writel(0x0, edp_base[sel] + REG_EDP_HPD_SCALE);
	edp_hpd_irq_disable(sel);
}

s32 edp_hal_irq_enable(u32 sel, u32 irq_id)
{
	/*fixme: irq is not need?*/
	//edp_hpd_irq_enable(sel);
	return 0;
}

s32 edp_hal_irq_disable(u32 sel, u32 irq_id)
{
	/*fixme: irq is not need?*/
	//edp_hpd_irq_disable(sel);
	return 0;
}

s32 edp_hal_irq_query(u32 sel)
{
	return 0;
}

s32 edp_hal_irq_clear(u32 sel)
{
	return 0;
}

s32 edp_hal_get_cur_line(u32 sel)
{
	return 0;
}

s32 edp_hal_get_start_dly(u32 sel)
{
	return 0;
}

void edp_hal_show_builtin_patten(u32 sel, u32 pattern)
{
}


bool edp_hal_get_hpd_status(u32 sel)
{
	u32 hpd_event;
	u32 hpd_plug_event;

	hpd_event = readl(edp_base[sel] + REG_EDP_HPD_EVENT) & 0x1;
	hpd_plug_event = readl(edp_base[sel] + REG_EDP_HPD_PLUG) & 0x1;

	if ((hpd_event == 1) && (hpd_plug_event == 1))
		return true;
	else
		return false;
}

void edp_hal_clean_hpd_interrupt_status(u32 sel)
{
	writel(0x1, edp_base[sel] + REG_EDP_HPD_PLUG);
}


s32 edp_hal_link_start(u32 sel)
{
	s32 ret;

	edp_video_stream_enable(sel);

	g_tx_buf[0] = 0x00; // 102 --- indicate the end
			     // of training
	g_tx_buf[1] = 0x00; /* 103 */
	g_tx_buf[2] = 0x00; /* 104 */
	g_tx_buf[3] = 0x00; /* 105 */
	g_tx_buf[4] = 0x00; /* 106 */
	g_tx_buf[5] = 0x00; /* 107 */
	/*fixme: spec wants 0x10*/
	//g_tx_buf[5] = 0x10; /* 107 */
	g_tx_buf[6] = 0x01; /* 108 */
	g_tx_buf[7] = 0x00; /* 109 */
	g_tx_buf[8] = 0x00; /* 10a */
	g_tx_buf[9] = 0x00;	 /* 10b */
	g_tx_buf[10] = 0x00;	/* 10c */
	g_tx_buf[11] = 0x00;	/* 10d */
	g_tx_buf[12] = 0x00;	/* 10e */
	ret = edp_aux_write(sel, 0x0102, 13, g_tx_buf);

	return ret;
}



