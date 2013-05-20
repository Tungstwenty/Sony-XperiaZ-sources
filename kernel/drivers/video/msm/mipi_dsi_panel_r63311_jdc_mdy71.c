/* drivers/video/msm/mipi_dsi_panel_r63311_jdc_mdy71.c
 *
 * Copyright (C) 2012-2013 Sony Mobile Communications AB.
 *
 * Author: Yutaka Seijyou <Yutaka.X.Seijyou@sonymobile.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2; as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */


#include "msm_fb.h"
#include "mipi_dsi.h"
#include "mipi_dsi_panel_driver.h"

/* Initial Sequence */
static char soft_reset[] = {
	0x01
};
static char mcap[] = {
	0xB0, 0x00
};
static char set_address_mode[] = {
	0x36, 0xC0
};

/* Display ON Sequence */
static char exit_sleep[] = {
	0x11
};
static char display_on[] = {
	0x29
};

/* Display OFF Sequence */
static char display_off[] = {
	0x28
};
static char enter_sleep[] = {
	0x10
};

/* Reading DDB Sequence */
static char read_ddb_start[] = {
	0xA1, 0x00
};

/* Reading Color */
static char read_color[] = {
	0xEF, 0x00
};

static struct dsi_cmd_desc display_init_cmd_seq[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, 5,
		sizeof(soft_reset), soft_reset},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,
		sizeof(mcap), mcap},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,
		sizeof(set_address_mode), set_address_mode},
};

static struct dsi_cmd_desc display_on_cmd_seq[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, 120,
		sizeof(exit_sleep), exit_sleep},
	{DTYPE_DCS_WRITE, 1, 0, 0, 0,
		sizeof(display_on), display_on},
};

static struct dsi_cmd_desc display_off_cmd_seq[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, 20,
		sizeof(display_off), display_off},
	{DTYPE_DCS_WRITE, 1, 0, 0, 80,
		sizeof(enter_sleep), enter_sleep},
};

static struct dsi_cmd_desc read_ddb_cmd_seq[] = {
	{DTYPE_DCS_READ, 1, 0, 1, 5, sizeof(read_ddb_start), read_ddb_start},
};

static struct dsi_cmd_desc read_color_init_cmd[] = {
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(mcap), mcap},
};

static struct dsi_cmd_desc read_color_cmd[] = {
	{DTYPE_GEN_READ1, 1, 0, 1, 5, sizeof(read_color), read_color},
};

static const struct panel_cmd display_init_cmds[] = {
	{CMD_DSI, {.dsi_payload = {display_init_cmd_seq,
				ARRAY_SIZE(display_init_cmd_seq)} } },
	{CMD_END, {} },
};

static const struct panel_cmd display_on_cmds[] = {
	{CMD_DSI, {.dsi_payload = {display_on_cmd_seq,
				ARRAY_SIZE(display_on_cmd_seq)} } },
	{CMD_END, {} },
};

static const struct panel_cmd display_off_cmds[] = {
	{CMD_DSI, {.dsi_payload = {display_off_cmd_seq,
				ARRAY_SIZE(display_off_cmd_seq)} } },
	{CMD_END, {} },
};

static const struct panel_cmd read_ddb_cmds[] = {
	{CMD_DSI, {.dsi_payload = {read_ddb_cmd_seq,
				ARRAY_SIZE(read_ddb_cmd_seq)} } },
	{CMD_END, {} },
};

static const struct panel_cmd read_color_cmds[] = {
	{CMD_DSI, {.dsi_payload = {read_color_init_cmd,
				ARRAY_SIZE(read_color_init_cmd)} } },
	{CMD_DSI, {.dsi_payload = {read_color_cmd,
				ARRAY_SIZE(read_color_cmd)} } },
	{CMD_END, {} },
};

static const struct mipi_dsi_phy_ctrl dsi_video_mode_phy_db[] = {
	/* 1080*1920, RGB888, 4 Lane 60 fps video mode */
	{
		/* regulator */
		{0x03, 0x0a, 0x04, 0x00, 0x20},
		/* timing */
		{0xe6, 0x38, 0x27, 0x00, 0x69, 0x72, 0x2b, 0x3c,
		 0x41, 0x03, 0x04, 0xa0},
		/* phy ctrl */
		{0x5f, 0x00, 0x00, 0x10},
		/* strength */
		{0xff, 0x00, 0x06, 0x00},
		/* pll control */
		{0x00, 0xaf, 0x31, 0xd9, 0x00, 0x50, 0x48, 0x63,
		 0x41, 0x0f, 0x03,
		 0x00, 0x14, 0x03, 0x00, 0x02, 0x00, 0x20, 0x00, 0x01 },
	},
};

static const struct mdp_pcc_cfg_rgb color_correction_data[] = {
	{0x08000, 0x06D00, 0x08000},	/* CLR01_GRN */
	{0x07A00, 0x07100, 0x08000},	/* CLR02_GRN */
	{0x07400, 0x07180, 0x08000},	/* CLR03_LG */
	{0x07100, 0x07200, 0x08000},	/* CLR04_LG */
	{0x06C80, 0x07180, 0x08000},	/* CLR05_YEL */
	{0x06680, 0x07300, 0x08000},	/* CLR06_YEL */
	{0x08000, 0x06B80, 0x07080},	/* CLR07_GRN */
	{0x08000, 0x07400, 0x08000},	/* CLR08_GRN */
	{0x07A00, 0x07700, 0x08000},	/* CLR09_LG */
	{0x07600, 0x07780, 0x08000},	/* CLR10_LG */
	{0x06F80, 0x07980, 0x08000},	/* CLR11_YEL */
	{0x07080, 0x07E80, 0x08000},	/* CLR12_YEL */
	{0x08000, 0x06B80, 0x06D00},	/* CLR13_LB */
	{0x08000, 0x07380, 0x07480},	/* CLR14_LB */
	{0x08000, 0x08000, 0x08000},	/* CLR15_WHT */
	{0x07500, 0x08000, 0x07E80},	/* CLR16_ORG */
	{0x07100, 0x08000, 0x07C80},	/* CLR17_ORG */
	{0x08000, 0x06900, 0x06500},	/* CLR18_BLE */
	{0x08000, 0x07300, 0x06F00},	/* CLR19_BLE */
	{0x07F00, 0x08000, 0x07900},	/* CLR20_PUR */
	{0x07E80, 0x08000, 0x07680},	/* CLR21_PUR */
	{0x07380, 0x08000, 0x07600},	/* CLR22_RED */
	{0x07080, 0x08000, 0x07900},	/* CLR23_RED */
	{0x07A00, 0x08000, 0x07580},	/* CLR24_RED */
	{0x06F00, 0x08000, 0x07200},	/* CLR25_RED */
};

static const struct mdp_pcc_cfg_rgb  color_sub_area[] = {
	{0x05FA9, 0x07760, 0x08000}, /* AREA1 */
	{0x067BF, 0x07988, 0x08000},
	{0x067BF, 0x07988, 0x08000},
	{0x06CCC, 0x07AE1, 0x08000},
	{0x06CCC, 0x07AE1, 0x08000},
	{0x06CCC, 0x07AE1, 0x08000},
	{0x079EF, 0x07E62, 0x08000},
	{0x079EF, 0x07E62, 0x08000},
	{0x079EF, 0x07E62, 0x08000},
	{0x079EF, 0x07E62, 0x08000},

	{0x08000, 0x08000, 0x08000},
	{0x08000, 0x08000, 0x08000},
	{0x08000, 0x08000, 0x08000},
	{0x08000, 0x08000, 0x08000},
	{0x08000, 0x08000, 0x08000},
	{0x08000, 0x08000, 0x08000},
	{0x08000, 0x08000, 0x08000},
	{0x08000, 0x08000, 0x08000},
	{0x08000, 0x08000, 0x08000},
	{0x08000, 0x08000, 0x08000},

	{0x08000, 0x08000, 0x08000},
	{0x08000, 0x08000, 0x08000},
	{0x08000, 0x08000, 0x08000},
	{0x08000, 0x08000, 0x08000},
	{0x08000, 0x08000, 0x08000},
	{0x08000, 0x08000, 0x08000},
	{0x08000, 0x08000, 0x08000},
	{0x08000, 0x08000, 0x08000},
	{0x08000, 0x08000, 0x08000},
	{0x08000, 0x08000, 0x08000},

	{0x08000, 0x08000, 0x08000},
	{0x08000, 0x08000, 0x08000},
	{0x08000, 0x08000, 0x08000},
	{0x08000, 0x08000, 0x08000},
	{0x08000, 0x08000, 0x08000},
	{0x08000, 0x08000, 0x08000},
	{0x08000, 0x08000, 0x08000},
	{0x08000, 0x08000, 0x08000},
	{0x08000, 0x08000, 0x08000},
	{0x08000, 0x08000, 0x08000},

	{0x08000, 0x08000, 0x07AE1},
	{0x08000, 0x08000, 0x07AE1},
	{0x08000, 0x08000, 0x07AE1},
	{0x08000, 0x08000, 0x07AE1},
	{0x08000, 0x08000, 0x076A7},
	{0x08000, 0x08000, 0x076A7},
	{0x08000, 0x08000, 0x076A7},
	{0x08000, 0x08000, 0x07333},
	{0x08000, 0x08000, 0x07333},
	{0x08000, 0x08000, 0x070A3},
};

static struct msm_panel_info pinfo;

static struct msm_panel_info *get_panel_info(void)
{
	pinfo.xres = 1080;
	pinfo.yres = 1920;
	pinfo.type = MIPI_VIDEO_PANEL;
	pinfo.pdest = DISPLAY_1;
	pinfo.wait_cycle = 0;
	pinfo.bpp = 24;
	pinfo.lcdc.h_back_porch = 75;
	pinfo.lcdc.h_front_porch = 129;
	pinfo.lcdc.h_pulse_width = 5;
	pinfo.lcdc.v_back_porch = 4;
	pinfo.lcdc.v_front_porch = 8;
	pinfo.lcdc.v_pulse_width = 4;
	pinfo.lcdc.border_clr = 0;	/* blk */
	pinfo.lcdc.underflow_clr = 0xff;	/* blue */
	pinfo.lcdc.hsync_skew = 0;
	pinfo.bl_max = 15;
	pinfo.bl_min = 1;
	pinfo.fb_num = 2;
	pinfo.clk_rate = 898000000;

	pinfo.mipi.mode = DSI_VIDEO_MODE;
	pinfo.mipi.pulse_mode_hsa_he = TRUE;
	pinfo.mipi.hfp_power_stop = FALSE;
	pinfo.mipi.hbp_power_stop = FALSE;
	pinfo.mipi.hsa_power_stop = FALSE;
	pinfo.mipi.eof_bllp_power_stop = TRUE;
	pinfo.mipi.bllp_power_stop = TRUE;
	pinfo.mipi.traffic_mode = DSI_NON_BURST_SYNCH_EVENT;
	pinfo.mipi.dst_format = DSI_VIDEO_DST_FORMAT_RGB888;
	pinfo.mipi.vc = 0;
	pinfo.mipi.dlane_swap = 0x00;
	pinfo.mipi.rgb_swap = DSI_RGB_SWAP_RGB;
	pinfo.mipi.r_sel = 0;
	pinfo.mipi.g_sel = 0;
	pinfo.mipi.b_sel = 0;
	pinfo.mipi.data_lane0 = TRUE;
	pinfo.mipi.data_lane1 = TRUE;
	pinfo.mipi.data_lane2 = TRUE;
	pinfo.mipi.data_lane3 = TRUE;
	pinfo.mipi.tx_eot_append = TRUE;
	pinfo.mipi.t_clk_post = 0x02;
	pinfo.mipi.t_clk_pre = 0x2d;
	pinfo.mipi.esc_byte_ratio = 9;
	pinfo.mipi.stream = 0; /* dma_p */
	pinfo.mipi.mdp_trigger = DSI_CMD_TRIGGER_SW;
	pinfo.mipi.dma_trigger = DSI_CMD_TRIGGER_SW;
	pinfo.mipi.frame_rate  = 60;
	pinfo.mipi.dsi_phy_db =
		(struct mipi_dsi_phy_ctrl *)dsi_video_mode_phy_db;

	return &pinfo;
}

static struct dsi_controller dsi_video_controller_panel = {
	.get_panel_info = get_panel_info,
	.display_init = display_init_cmds,
	.display_on = display_on_cmds,
	.display_off = display_off_cmds,
	.read_id = read_ddb_cmds,
	.read_color = read_color_cmds,
};

static char ddb_val_1a[] = {
	0x12, 0x69, 0x22, 0x28, 0x1a, 0x01, 0x00
};

static char ddb_val[] = {
	0x12, 0x69, 0x22, 0x28, 0xff, 0x01, 0x00
};

const struct panel jdc_mdy71_panel_id_1a = {
	.name = "mipi_video_jdc_Full_HD_mdy71_id_1a",
	.pctrl = &dsi_video_controller_panel,
	.id = ddb_val_1a,
	.id_num = ARRAY_SIZE(ddb_val_1a),
	.width = 62,
	.height = 110,
	.send_video_data_before_display_on = true,
	.panel_id = "mdy71",
	.panel_rev = "1a",
};

const struct panel jdc_mdy71_panel_id = {
	.name = "mipi_video_jdc_Full_HD_mdy71",
	.pctrl = &dsi_video_controller_panel,
	.id = ddb_val,
	.id_num = ARRAY_SIZE(ddb_val),
	.width = 62,
	.height = 110,
	.send_video_data_before_display_on = true,
	.panel_id = "mdy71",
	.panel_rev = "generic",
	.color_correction_tbl = color_correction_data,
	.color_subdivision_tbl = color_sub_area,
};