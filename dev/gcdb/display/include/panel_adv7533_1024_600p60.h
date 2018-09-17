/* Copyright (c) 2017, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *   * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *   * Neither the name of The Linux Foundation nor the names of its
 *      contributors may be used to endorse or promote products derived
 *      from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#ifndef _PANEL_ADV7533_1024_600p60_H_
#define _PANEL_ADV7533_1024_600p60_H_
/*---------------------------------------------------------------------------*/
/* HEADER files                                                              */
/*---------------------------------------------------------------------------*/
#include "panel.h"

/*---------------------------------------------------------------------------*/
/* Panel configuration                                                       */
/*---------------------------------------------------------------------------*/
#if 0
static struct panel_config adv7533_1024_600p_video_panel_data = {
	"qcom,mdss_dsi_adv7533_1024_600p", "dsi:0:", "qcom,mdss-dsi-panel",
	10, 0, "DISPLAY_1", 0, 0, 60, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "NONE"
};
#endif

#if 1
// This structure is for dual DSI setup, 1024x600p video panel @ DSI0
static struct panel_config adv7533_1024_600p_video_DSI0_panel_data = {
	"qcom,mdss_dsi_adv7533_1024_600p", "dsi:0:", "qcom,mdss-dsi-panel",
	10, 0, "DISPLAY_1", 0, 0, 60, 0, 0, 0, 0, 0, 0, 0, 0, DUAL_DSI_FLAG, 0, 0, "NONE"
};
#endif

#if 0
// This structure is for dual DSI setup, 1024x600p video panel @ DSI1
static struct panel_config adv7533_1024_600p_video_DSI1_panel_data = {
	"qcom,mdss_dsi_adv7533_1024_600p", "dsi:1:", "qcom,mdss-dsi-panel",
	10, 0, "DISPLAY_2", 0, 0, 60, 0, 0, 0, 0, 0, 0, 0, 0, DUAL_DSI_FLAG, 0, 0, "NONE"
};
#endif

/*---------------------------------------------------------------------------*/
/* Panel resolution                                                          */
/*---------------------------------------------------------------------------*/
static struct panel_resolution adv7533_1024_600p_video_panel_res = {
	1024, 600, 110, 220, 40, 0, 5, 20, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/*---------------------------------------------------------------------------*/
/* Panel color information                                                   */
/*---------------------------------------------------------------------------*/
static struct color_info adv7533_1024_600p_video_color = {
	24, 0, 0xff, 0, 0, 0
};

static struct mipi_dsi_i2c_cmd adv7533_1024_600p_common_cfg[] = {
	{ADV7533_MAIN, 0xd6, 0x48, 5},		/* HPD overriden */
	{ADV7533_MAIN, 0x41, 0x10, 5},		/* HDMI normal */
	{ADV7533_CEC_DSI, 0x03, 0x89, 0},	/* HDMI enabled */
	{ADV7533_MAIN, 0x16, 0x20, 0},
	{ADV7533_MAIN, 0x9A, 0xE0, 0},
	{ADV7533_MAIN, 0xBA, 0x70, 0},
	{ADV7533_MAIN, 0xDE, 0x82, 0},
	{ADV7533_MAIN, 0xE4, 0xC0, 0},
	{ADV7533_MAIN, 0xE5, 0x80, 0},
	{ADV7533_CEC_DSI, 0x15, 0xD0, 0},
	{ADV7533_CEC_DSI, 0x17, 0xD0, 0},
	{ADV7533_CEC_DSI, 0x24, 0x20, 0},
	{ADV7533_CEC_DSI, 0x57, 0x11, 0},
	/* hdmi or dvi mode: hdmi */
	{ADV7533_MAIN, 0xAF, 0x06, 0},
	{ADV7533_MAIN, 0x40, 0x80, 0},
	{ADV7533_MAIN, 0x4C, 0x04, 0},
	{ADV7533_MAIN, 0x49, 0x02, 0},
	{ADV7533_MAIN, 0x0D, 1 << 6, 0},
	{ADV7533_CEC_DSI, 0x1C, 0x30, 0},
};

#define ADV7533_1024_600P_CONFIG_COMMANDS 19

static struct mipi_dsi_i2c_cmd adv7533_1024_600p_tg_i2c_command[] = {
	/*3 Lanes*/
	{ADV7533_CEC_DSI, 0x1C, 0x30, 0},
	/* hsync and vsync active low */
	{ADV7533_MAIN, 0x17, 0x02, 0},
	/* Control for Pixel Clock Divider */
	{ADV7533_CEC_DSI, 0x16, 0x00, 0},
	/* h_width 0x572 1394*/
	{ADV7533_CEC_DSI, 0x28, 0x57, 0},
	{ADV7533_CEC_DSI, 0x29, 0x20, 0},
	/* hsync_width 0x28 40*/
	{ADV7533_CEC_DSI, 0x2A, 0x02, 0},
	{ADV7533_CEC_DSI, 0x2B, 0x80, 0},
	/* hfp 0x6E 110 */
	{ADV7533_CEC_DSI, 0x2C, 0x06, 0},
	{ADV7533_CEC_DSI, 0x2D, 0xE0, 0},
	/* hbp 0xDC 220 */
	{ADV7533_CEC_DSI, 0x2E, 0x0D, 0},
	{ADV7533_CEC_DSI, 0x2F, 0xC0, 0},
	/* v_total 0x276 630*/
	{ADV7533_CEC_DSI, 0x30, 0x27, 0},
	{ADV7533_CEC_DSI, 0x31, 0x60, 0},
	/* vsync_width 0x05 5*/
	{ADV7533_CEC_DSI, 0x32, 0x00, 0},
	{ADV7533_CEC_DSI, 0x33, 0x50, 0},
	/* vfp 0x05 5  */
	{ADV7533_CEC_DSI, 0x34, 0x00, 0},
	{ADV7533_CEC_DSI, 0x35, 0x50, 0},
	/* vbp 0x14 20 */
	{ADV7533_CEC_DSI, 0x36, 0x01, 0},
	{ADV7533_CEC_DSI, 0x37, 0x40, 0},
	/* Test Pattern Disable (0x55[7] = 0) */
	{ADV7533_CEC_DSI, 0x55, 0x00, 0},
	/* HDMI disabled */
	{ADV7533_CEC_DSI, 0x03, 0x09, 0},
	/* HDMI enabled */
	{ADV7533_CEC_DSI, 0x03, 0x89, 0},
};

#define ADV7533_1024_600P_TG_COMMANDS 22

static struct command_state adv7533_1024_600p_video_state = {
	0, 1
};

/*---------------------------------------------------------------------------*/
/* Command mode panel information                                            */
/*---------------------------------------------------------------------------*/
static struct commandpanel_info adv7533_1024_600p_video_command_panel = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/*---------------------------------------------------------------------------*/
/* Video mode panel information                                              */
/*---------------------------------------------------------------------------*/
static struct videopanel_info adv7533_1024_600p_video_video_panel = {
	1, 0, 0, 0, 1, 1, 0, 0, 0x9
};

/*---------------------------------------------------------------------------*/
/* Lane configuration                                                        */
/*---------------------------------------------------------------------------*/
static struct lane_configuration adv7533_1024_600p_video_lane_config = {
	3, 0, 1, 1, 1, 0, 1
};

/*---------------------------------------------------------------------------*/
/* Panel timing                                                              */
/*---------------------------------------------------------------------------*/
static const uint32_t adv7533_1024_600p_video_timings[] = {
	0xab, 0x1a, 0x10, 0x00, 0x3e, 0x43, 0x16, 0x1e, 0x15, 0x03, 0x04, 0x00
};

static struct panel_timing adv7533_1024_600p_video_timing_info = {
	0x0, 0x04, 0x03, 0x20
};

static const uint32_t adv7533_1024_600p_thulium_video_timings[] = {
		0x1c, 0x19, 0x02, 0x03, 0x01, 0x03, 0x04, 0xa0,
		0x1c, 0x19, 0x02, 0x03, 0x01, 0x03, 0x04, 0xa0,
		0x1c, 0x19, 0x02, 0x03, 0x01, 0x03, 0x04, 0xa0,
		0x1c, 0x19, 0x02, 0x03, 0x01, 0x03, 0x04, 0xa0,
		0x1c, 0x08, 0x02, 0x03, 0x01, 0x03, 0x04, 0xa0,
};

#endif /*_PANEL_ADV7533_1024_600p60_H_*/

