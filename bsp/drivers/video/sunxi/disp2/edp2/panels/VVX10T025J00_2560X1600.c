/* VVX10T025J00_2560X1600.c
 *
 * Copyright (c) 2022 Allwinnertech Co., Ltd.
 * Author: huangyongxing <huangyongxing@allwinnertech.com>
 *
 * VVX10T025J00 edp panel driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
[edp0]
 * edp_used            = 1
 *
 * edp_driver_name     = "VVX10T025J00_2560X1600"
 *
 * edp_if              = 4
 * edp_x               = 480
 * edp_y               = 640
 * edp_width           = 36
 * edp_height          = 65
 * edp_dclk_freq       = 25
 *
 * edp_pwm_used        = 1
 * edp_pwm_ch          = 8
 * edp_pwm_freq        = 50000
 * edp_pwm_pol         = 1
 * edp_pwm_max_limit   = 255
 *
 * edp_hbp             = 70
 * edp_ht              = 615
 * edp_hspw            = 8
 * edp_vbp             = 30
 * edp_vt              = 690
 * edp_vspw            = 10
 *
 * edp_dsi_if          = 0
 * edp_dsi_lane        = 2
 * edp_dsi_format      = 0
 * edp_dsi_te          = 0
 * edp_dsi_eotp        = 0
 *
 * edp_frm             = 0
 * edp_io_phase        = 0x0000
 * edp_hv_clk_phase    = 0
 * edp_hv_sync_polarity= 0
 * edp_gamma_en        = 0
 * edp_bright_curve_en = 0
 * edp_cmap_en         = 0
 *
 * ;edp_bl_en           = port:PD09<1><0><default><1>
 * edp_panel_power      = "vcc-edp"
 * vcc-edp-supply       = &reg_cldo1;
 * edp_bl_power         = "edp-bl"
 * edp-bl-supply        = &reg_dcdc1;
 *
 * ;reset
 * edp_gpio_0          = port:PD09<1><0><default><1>
*/
#include "VVX10T025J00_2560X1600.h"

static void edp_power_on(u32 sel);
static void edp_power_off(u32 sel);
static void edp_bl_open(u32 sel);
static void edp_bl_close(u32 sel);

static void edp_panel_init(u32 sel);
static void edp_panel_exit(u32 sel);

#define panel_reset(sel, val) sunxi_edp_gpio_set_value(sel, 0, val)

static s32 edp_open_flow(u32 sel)
{
	EDP_OPEN_FUNC(sel, edp_power_on, 0);
	EDP_OPEN_FUNC(sel, edp_panel_init, 10);
	EDP_OPEN_FUNC(sel, edp_bl_open, 0);
	return 0;
}

static s32 edp_close_flow(u32 sel)
{
	EDP_CLOSE_FUNC(sel, edp_bl_close, 0);
	EDP_CLOSE_FUNC(sel, edp_panel_exit, 10);
	//EDP_CLOSE_FUNC(sel, sunxi_edp_tcon_disable, 0);
	EDP_CLOSE_FUNC(sel, edp_power_off, 0);

	return 0;
}

static void edp_power_on(u32 sel)
{
	sunxi_edp_power_enable(sel, 0);
	sunxi_edp_delay_ms(10);

	/* when hpd line pull up power up is need */
	//sunxi_edp_delay_ms(50);
	//sunxi_edp_power_enable(sel, 2);
#if 0
	/* reset edp by gpio */
	panel_reset(sel, 1);
	sunxi_edp_delay_ms(1);
	panel_reset(sel, 0);
	sunxi_edp_delay_ms(1);
	panel_reset(sel, 1);
	sunxi_edp_delay_ms(10);
#endif
}

static void edp_power_off(u32 sel)
{
	panel_reset(sel, 0);
	sunxi_edp_delay_ms(1);
	sunxi_edp_power_disable(sel, 0);
}

static void edp_bl_open(u32 sel)
{
	sunxi_edp_pwm_enable(sel);
	sunxi_edp_backlight_enable(sel);
}

static void edp_bl_close(u32 sel)
{
	sunxi_edp_backlight_disable(sel);
	sunxi_edp_pwm_disable(sel);
}

static void edp_panel_init(u32 sel)
{
	sunxi_edp_delay_ms(10);
}

static void edp_panel_exit(u32 sel)
{
	sunxi_edp_delay_ms(10);
}

struct __edp_panel VVX10T025J00_2560X1600_panel = {
	/* panel driver name, must mach the name of
	 * edp_drv_name in sys_config.fex
	 */
	.name = "VVX10T025J00_2560X1600",
	.func = {
			.cfg_open_flow = edp_open_flow,
			.cfg_close_flow = edp_close_flow,
	},
};
