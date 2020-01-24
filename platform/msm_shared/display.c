/* Copyright (c) 2012-2016, 2018-2019, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Foundation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
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

#include <debug.h>
#include <err.h>
#include <msm_panel.h>
#include <mdp5.h>
#include <mipi_dsi.h>
#include <boot_stats.h>
#include <platform.h>
#include <malloc.h>
#include <qpic.h>
#include <target.h>
#ifdef DISPLAY_TYPE_MDSS
#include <target/display.h>
#endif
#if ENABLE_QSEED_SCALAR
#include <target/scalar.h>
#endif
#include "mdp5_rm.h"
#include "target/target_utils.h"

static struct msm_fb_panel_data *panel;
static struct msm_fb_panel_data panel_array[MAX_NUM_DISPLAY];

static uint32_t num_panel = 0;

static uint32_t orientation_mask = 0;

extern int lvds_on(struct msm_fb_panel_data *pdata);

static int msm_fb_alloc(struct fbcon_config *fb)
{
	/* Static splash buffer */
	int num_buffers = 1;

	if (fb == NULL)
		return ERROR;

	if (target_animated_splash_screen()) {
		/* Static splash + animated splash buffers */
		dprintf(SPEW, "Allocate extra buffers for animates splash\n");
		num_buffers =  SPLASH_BUFFER_SIZE / (fb->width * fb->height * (fb->bpp / 8));
	}

	if (fb->base == NULL)
		fb->base = memalign(4096, fb->width
							* fb->height
							* (fb->bpp / 8)
							* num_buffers);

	if (fb->base == NULL) {
		dprintf(CRITICAL, "Error in Allocating %d buffer\n", num_buffers);
		return ERROR;
	}
	else {
		dprintf(SPEW, "Allocated %d buffers\n", num_buffers);
		return NO_ERROR;
	}
}

void msm_display_set_orientation(uint32_t rot_mask)
{
	orientation_mask = rot_mask;
}
static void _msm_display_attach_external_layer(
	struct msm_panel_info *pinfo, struct fbcon_config *fb,
	uint32_t fb_cnt, uint32_t total_fb_cnt, uint32_t index_mask)
{
	int ret = 0;
	char *pipe_name = NULL;
	uint32_t i = 0, index = 0, j =0;
	struct LayerInfo layer;
	struct Scale left_scale_setting;
        struct Scale right_scale_setting;

        //setup Layer structure for scaling
        memset((void*)&left_scale_setting, 0, sizeof (struct Scale));
        memset((void*)&right_scale_setting, 0, sizeof (struct Scale));

	layer.left_pipe.scale_data = &left_scale_setting;
	layer.right_pipe.scale_data = &right_scale_setting;

	layer.left_pipe.scale_data->enable_pxl_ext = 0;

	index = index_mask;

	for (i = 0; i < fb_cnt; i++) {
		for (j = 0; j < total_fb_cnt; j++) {
			if ((1 << j) & index)
				break;
		}
		index &= ~(1 << j);

		pipe_name = target_utils_translate_layer_to_fb(&fb[i], j, &layer);
		dprintf(SPEW, "pipe_name(%d)=%s\n", i, pipe_name);

		if (!pipe_name)
			continue;

		ret = mdp_config_external_pipe(pinfo, &fb[i], pipe_name, &layer);
		if (ret) {
			dprintf(CRITICAL, "config early app fb(%s) failed", pipe_name);
			continue;
		}
	}
}

static int msm_display_attach_external_layer(struct msm_panel_info *pinfo)
{
	struct fbcon_config *early_app_fb = NULL;
	uint32_t fb_cnt_on_single_display = 0, index_mask = 0;
	uint32_t total_early_app_fb_cnt = 0;
	int ret = NO_ERROR;

	target_utils_get_early_app_layer_cnt(pinfo->dest,
			&fb_cnt_on_single_display, &total_early_app_fb_cnt, &index_mask);

	dprintf(SPEW, "single_cnt=%d, total=%d, mask=0x%x\n",
		fb_cnt_on_single_display, total_early_app_fb_cnt, index_mask);
	if ((fb_cnt_on_single_display > 0) &&
		(fb_cnt_on_single_display < MAX_STAGE_FB)) {
		early_app_fb = (struct fbcon_config *)malloc(
			fb_cnt_on_single_display * sizeof(struct fbcon_config));

		if (!early_app_fb) {
			ret = ERR_NOT_VALID;
			dprintf(CRITICAL, "allocate early app fb failed\n");
		} else
			_msm_display_attach_external_layer(pinfo,
				early_app_fb,
				fb_cnt_on_single_display,
				total_early_app_fb_cnt,
				index_mask);
	}

	if (early_app_fb) {
		free(early_app_fb);
		early_app_fb = NULL;
	}

	return ret;
}

int msm_display_config()
{
	int ret = NO_ERROR;
#ifdef DISPLAY_TYPE_MDSS
	int mdp_rev;
#endif
	struct msm_panel_info *pinfo;
	static bool reseted;

	if (!panel)
		return ERR_INVALID_ARGS;

	pinfo = &(panel->panel_info);

	pinfo->orientation = orientation_mask;

	/* Set MDP revision */
	mdp_set_revision(panel->mdp_rev);

	mdp_rm_reset_resource_manager(reseted);
	reseted = true;

	switch (pinfo->type) {
#ifdef DISPLAY_TYPE_MDSS
	case LVDS_PANEL:
		dprintf(SPEW, "Config LVDS_PANEL.\n");
		ret = mdp_lcdc_config(pinfo, panel->fb);
		if (ret)
			goto msm_display_config_out;
		break;
	case MIPI_VIDEO_PANEL:
		dprintf(SPEW, "Config MIPI_VIDEO_PANEL.\n");

		mdp_rev = mdp_get_revision();
		if (pinfo->dest == DISPLAY_1) {
			if (mdp_rev == MDP_REV_50 || mdp_rev == MDP_REV_304 ||
							mdp_rev == MDP_REV_305)
				ret = mdss_dsi_config(panel);
			else
				ret = mipi_config(panel);
		}
		if (ret)
			goto msm_display_config_out;

		if (pinfo->early_config)
			ret = pinfo->early_config((void *)pinfo);

		ret = mdp_dsi_video_config(pinfo, panel->fb,
			pinfo->splitter_is_enabled ? MAX_SPLIT_DISPLAY : SPLIT_DISPLAY_1);
		if (ret)
			goto msm_display_config_out;
		break;
	case MIPI_CMD_PANEL:
		dprintf(SPEW, "Config MIPI_CMD_PANEL.\n");
		mdp_rev = mdp_get_revision();
		if (mdp_rev == MDP_REV_50 || mdp_rev == MDP_REV_304 ||
						mdp_rev == MDP_REV_305)
			ret = mdss_dsi_config(panel);
		else
			ret = mipi_config(panel);
		if (ret)
			goto msm_display_config_out;

		ret = mdp_dsi_cmd_config(pinfo, panel->fb);
		if (ret)
			goto msm_display_config_out;
		break;
	case LCDC_PANEL:
		dprintf(SPEW, "Config LCDC PANEL.\n");
		ret = mdp_lcdc_config(pinfo, panel->fb);
		if (ret)
			goto msm_display_config_out;
		break;
	case SPI_PANEL:
		dprintf(SPEW, "Config SPI PANEL.\n");
		ret = mdss_spi_init();
		if (ret)
			goto msm_display_config_out;
		ret = mdss_spi_panel_init(pinfo);
		if (ret)
			goto msm_display_config_out;
		break;
	case HDMI_PANEL:
		dprintf(SPEW, "Config HDMI PANEL.\n");
		ret = mdss_hdmi_config(pinfo, panel->fb,
			pinfo->splitter_is_enabled ? MAX_SPLIT_DISPLAY : SPLIT_DISPLAY_1);
		if (ret)
			goto msm_display_config_out;
		break;
	case EDP_PANEL:
		dprintf(SPEW, "Config EDP PANEL.\n");
		ret = mdp_edp_config(pinfo, panel->fb);
		if (ret)
			goto msm_display_config_out;
		break;
#endif
#ifdef DISPLAY_TYPE_QPIC
	case QPIC_PANEL:
		dprintf(SPEW, "Config QPIC_PANEL.\n");
		qpic_init(pinfo, (int) panel->fb[SPLIT_DISPLAY_0].base);
		break;
#endif
	default:
		return ERR_INVALID_ARGS;
	};

	if (pinfo->config)
		ret = pinfo->config((void *)pinfo);

#ifdef DISPLAY_TYPE_MDSS
msm_display_config_out:
#endif
	return ret;
}

int msm_display_on()
{
	int ret = NO_ERROR;
#ifdef DISPLAY_TYPE_MDSS
	int mdp_rev;
#endif
	struct msm_panel_info *pinfo;

	if (!panel)
		return ERR_INVALID_ARGS;

	bs_set_timestamp(BS_SPLASH_SCREEN_DISPLAY);

	pinfo = &(panel->panel_info);

	if (pinfo->pre_on) {
		ret = pinfo->pre_on();
		if (ret)
			goto msm_display_on_out;
	}

	switch (pinfo->type) {
#ifdef DISPLAY_TYPE_MDSS
	case LVDS_PANEL:
		dprintf(SPEW, "Turn on LVDS PANEL.\n");
		ret = mdp_lcdc_on(panel);
		if (ret)
			goto msm_display_on_out;
		ret = lvds_on(panel);
		if (ret)
			goto msm_display_on_out;
		break;
	case MIPI_VIDEO_PANEL:
		dprintf(SPEW, "Turn on MIPI_VIDEO_PANEL.\n");
		ret = mdp_dsi_video_on(pinfo);
		if (ret)
			goto msm_display_on_out;

		ret = mdss_dsi_post_on(panel);
		if (ret)
			goto msm_display_on_out;

		ret = mipi_dsi_on(pinfo);
		if (ret)
			goto msm_display_on_out;
		break;
	case MIPI_CMD_PANEL:
		dprintf(SPEW, "Turn on MIPI_CMD_PANEL.\n");
		ret = mdp_dma_on(pinfo);
		if (ret)
			goto msm_display_on_out;
		mdp_rev = mdp_get_revision();
		if (mdp_rev != MDP_REV_50 && mdp_rev != MDP_REV_304 &&
						mdp_rev != MDP_REV_305) {
			ret = mipi_cmd_trigger();
			if (ret)
				goto msm_display_on_out;
		}

		ret = mdss_dsi_post_on(panel);
		if (ret)
			goto msm_display_on_out;

		break;
	case LCDC_PANEL:
		dprintf(SPEW, "Turn on LCDC PANEL.\n");
		ret = mdp_lcdc_on(panel);
		if (ret)
			goto msm_display_on_out;
		break;
	case HDMI_PANEL:
		dprintf(SPEW, "Turn on HDMI PANEL.\n");
		ret = mdss_hdmi_init();
		if (ret)
			goto msm_display_on_out;

		ret = mdss_hdmi_on(pinfo);
		if (ret)
			goto msm_display_on_out;
		break;
	case EDP_PANEL:
		dprintf(SPEW, "Turn on EDP PANEL.\n");
		ret = mdp_edp_on(pinfo);
		if (ret)
			goto msm_display_on_out;
		break;
	case SPI_PANEL:
		dprintf(SPEW, "Turn on SPI_PANEL.\n");
		ret = mdss_spi_on(pinfo, panel->fb);
		if (ret)
			goto msm_display_on_out;
		ret = mdss_spi_cmd_post_on(pinfo);
		if (ret)
			goto msm_display_on_out;
		break;
#endif
#ifdef DISPLAY_TYPE_QPIC
	case QPIC_PANEL:
		dprintf(SPEW, "Turn on QPIC_PANEL.\n");
		ret = qpic_on();
		if (ret) {
			dprintf(CRITICAL, "QPIC panel on failed\n");
			goto msm_display_on_out;
		}
		qpic_update();
		break;
#endif
	default:
		return ERR_INVALID_ARGS;
	};

	if (pinfo->on)
		ret = pinfo->on();

msm_display_on_out:
	return ret;
}

struct fbcon_config* msm_display_get_fb(uint32_t disp_id, uint32_t fb_index)
{
	if ((panel_array == NULL) || (fb_index >= MAX_SPLIT_DISPLAY))
		return NULL;
	else
		return &panel_array[disp_id].fb[fb_index];
}

int msm_display_update(struct fbcon_config *fb, uint32_t fb_cnt,
	uint32_t pipe_id, uint32_t pipe_type, uint32_t *width, uint32_t *height,
	uint32_t disp_id, bool disp_has_rvc_context, bool firstframe)
{
	struct msm_panel_info *pinfo;
	struct msm_fb_panel_data *panel_local;
	int ret = 0, i = 0, max_fb_cnt = 0;
	uint32_t left_mask, right_mask;
	static bool reseted;

	if (!fb) {
		dprintf(CRITICAL, "Error! Inalid args\n");
		return ERR_INVALID_ARGS;
	}
	panel_local = &(panel_array[disp_id]);
	pinfo = &(panel_local->panel_info);

	pinfo->pipe_type = pipe_type;
	pinfo->pipe_id = pipe_id;

	max_fb_cnt = pinfo->splitter_is_enabled ? MAX_SPLIT_DISPLAY : SPLIT_DISPLAY_1;

	for (i = SPLIT_DISPLAY_0; i < max_fb_cnt; i++) {
		pinfo->zorder[i] = fb[i].z_order;
		pinfo->border_top[i] = fb[i].height/2 - height[i]/2;
		pinfo->border_bottom[i] = pinfo->border_top[i];
		pinfo->border_left[i] = fb[i].width/2 - width[i]/2;
		pinfo->border_right[i] = pinfo->border_left[i];
		panel_local->fb[i] = fb[i];
	}

	mdp_rm_reset_resource_manager(reseted);
	reseted = true;

	if (firstframe) {
		/*
		 * When failing to attach external layer, no impact for
		 * normal splash/animation setup, and continue current process.
		 */
		ret = msm_display_attach_external_layer(pinfo);
		if (ret)
			dprintf(CRITICAL, "Attach external layer failed\n");
	}

	switch (pinfo->type) {
		case MIPI_VIDEO_PANEL:
			if (firstframe) {
				dprintf(SPEW, "set DSI config and update once\n");
				ret = mdp_dsi_video_config(pinfo, fb, fb_cnt);
				if (ret) {
					dprintf(CRITICAL, "ERROR in dsi display config\n");
					goto msm_display_update_out;
				}

				ret = mdp_dsi_video_update(pinfo);
				if (ret) {
					dprintf(CRITICAL, "ERROR in dsi display update\n");
					goto msm_display_update_out;
				}
			} else {
				ret = mdp_config_pipe(pinfo, fb, fb_cnt);
				if (ret) {
					dprintf(CRITICAL, "call DSI mdp_config_pipe failed\n");
					goto msm_display_update_out;
				}

				ret = mdp_trigger_flush(pinfo, fb, fb_cnt,
						&left_mask, &right_mask);
				if (ret) {
					dprintf(CRITICAL, "call DSI mdp_trigger_flush failed\n");
					goto msm_display_update_out;
				}
			}
			break;
		case HDMI_PANEL:
			if (firstframe) {
				dprintf(SPEW, "set HDMI config and update once\n");
				ret = mdss_hdmi_config(pinfo, fb, fb_cnt);
				if (ret) {
					dprintf(CRITICAL, "ERROR in hdmi display config\n");
					goto msm_display_update_out;
				}

				ret = mdss_hdmi_update(pinfo);
				if (ret) {
					dprintf(CRITICAL, "ERROR in hdmi display update\n");
					goto msm_display_update_out;
				}
			} else {
				ret = mdp_config_pipe(pinfo, fb, fb_cnt);
				if (ret) {
					dprintf(CRITICAL, "call HDMI mdp_config_pipe failed\n");
					goto msm_display_update_out;
				}

				ret = mdp_trigger_flush(pinfo, fb, fb_cnt,
						&left_mask, &right_mask);
				if (ret) {
					dprintf(CRITICAL, "call HDMI mdp_trigger_flush failed\n");
					goto msm_display_update_out;
				}
			}
			break;
		default:
			dprintf(SPEW, "Update not supported right now\n");
			break;
	}

msm_display_update_out:
	return ret;
}

int msm_display_update_pipe(struct fbcon_config *fb, uint32_t pipe_id,
	uint32_t pipe_type, uint32_t *width, uint32_t *height, uint32_t disp_id)
{
	struct msm_panel_info *pinfo;
	struct msm_fb_panel_data *panel_local;
	int ret = 0, i = 0, max_fb_cnt = 0;
	if (!fb) {
		dprintf(CRITICAL, "Error! Inalid args\n");
		return ERR_INVALID_ARGS;
	}

	panel_local = &(panel_array[disp_id]);
	pinfo = &(panel_local->panel_info);

	pinfo->pipe_type = pipe_type;
	pinfo->pipe_id = pipe_id;
	max_fb_cnt = pinfo->splitter_is_enabled ? MAX_SPLIT_DISPLAY : SPLIT_DISPLAY_1;

	for (i = SPLIT_DISPLAY_0; i < max_fb_cnt; i++) {
		pinfo->zorder[i] = fb[i].z_order;
		pinfo->border_top[i] = fb[i].height/2 - height[i]/2;
		pinfo->border_bottom[i] = pinfo->border_top[i];
		pinfo->border_left[i] = fb[i].width/2 - width[i]/2;
		pinfo->border_right[i] = pinfo->border_left[i];
		panel_local->fb[i] = fb[i];
	}

	ret = mdp_update_pipe(pinfo, fb, max_fb_cnt);
	if (ret)
		dprintf(CRITICAL, "Error in mdp_update_pipe for %s%d display\n",
			(pinfo->type == MIPI_VIDEO_PANEL) ? "DSI" : "HDMI", pinfo->dest - DISPLAY_1);
	return ret;
}


int msm_display_hide_pipe(struct fbcon_config *fb, uint32_t fb_cnt,
	uint32_t pipe_id, uint32_t pipe_type, uint32_t disp_id)
{
	struct msm_panel_info *pinfo;
	struct msm_fb_panel_data *panel_local;
	int ret = 0;
	panel_local = &(panel_array[disp_id]);

	pinfo = &(panel_local->panel_info);
	pinfo->pipe_type = pipe_type;
	pinfo->pipe_id = pipe_id;

	ret = mdss_layer_mixer_hide_pipe(pinfo, fb, fb_cnt);
	if (ret)
		dprintf(CRITICAL, "Error in mdss_layer_mixer_hide_pipe\n");

	return ret;
}

int msm_display_init(struct msm_fb_panel_data *pdata)
{
	int ret = NO_ERROR;
	int i = 0, max_fb_cnt = 0;
	struct fbcon_config *fb;

	panel = pdata;
	if (!panel) {
		ret = ERR_INVALID_ARGS;
		goto msm_display_init_out;
	}

	max_fb_cnt = panel->splitter_is_enabled ? MAX_SPLIT_DISPLAY : SPLIT_DISPLAY_1;
	panel->panel_info.splitter_is_enabled = panel->splitter_is_enabled;

	// HDMI needs explicit assign its destination
	if (!panel->panel_info.dest)
		panel->panel_info.dest = num_panel + 1;
	/* Turn on panel */
	if (pdata->power_func)
		ret = pdata->power_func(1, &(panel->panel_info));

	if (ret)
		goto msm_display_init_out;

	if (pdata->dfps_func)
		ret = pdata->dfps_func(&(panel->panel_info));

	/* Enable clock */
	if (pdata->clk_func)
		ret = pdata->clk_func(1, &(panel->panel_info));

	if (ret)
		goto msm_display_init_out;

	/* Read specifications from panel if available.
	 * If further clocks should be enabled, they can be enabled
	 * using pll_clk_func
	 */
	if (pdata->update_panel_info)
		ret = pdata->update_panel_info(panel->splitter_is_enabled);

	if (ret)
		goto msm_display_init_out;

	/* Enabled for auto PLL calculation or to enable
	 * additional clocks
	 */
	if (pdata->pll_clk_func)
		ret = pdata->pll_clk_func(1, &(panel->panel_info));

	if (ret)
		goto msm_display_init_out;

	/* pinfo prepare  */
	if (pdata->panel_info.prepare) {
		/* this is for edp which pinfo derived from edid */
		ret = pdata->panel_info.prepare();
		if (ret)
			goto msm_display_init_out;

		for (i = SPLIT_DISPLAY_0; i < max_fb_cnt; i++) {
			fb = &panel->fb[i];

			fb->width = panel->panel_info.xres / max_fb_cnt;
			fb->height = panel->panel_info.yres;
			fb->stride =  (panel->panel_info.xres * panel->panel_info.bpp / 8) / max_fb_cnt;
			fb->bpp = panel->panel_info.bpp;
		}
	}

	for (i = SPLIT_DISPLAY_0; i < max_fb_cnt; i++) {
		ret = msm_fb_alloc(&(panel->fb[i]));
		if (ret)
			goto msm_display_init_out;

		fbcon_setup(&(panel->fb[i]));

		display_image_on_screen();
	}

	if ((panel->dsi2HDMI_config) && (panel->panel_info.has_bridge_chip))
		ret = panel->dsi2HDMI_config(&(panel->panel_info));
	if (ret)
		goto msm_display_init_out;

	ret = msm_display_config();
	if (ret)
		goto msm_display_init_out;

	ret = msm_display_on();
	if (ret)
		goto msm_display_init_out;

	if (pdata->post_power_func)
		ret = pdata->post_power_func(1);
	if (ret)
		goto msm_display_init_out;

	/* Turn on backlight */
	if (pdata->bl_func)
		ret = pdata->bl_func(1);

	if (ret)
		goto msm_display_init_out;

	// if panel init correctly, save the panel struct in the array
	memcpy((void*) &panel_array[num_panel], (void*) panel, sizeof(struct  msm_fb_panel_data));
	for (i = SPLIT_DISPLAY_0; i < max_fb_cnt; i++)
		dprintf(SPEW, "Default panel %d init FB[%d] width:%d height:%d\n",
			num_panel, i, panel_array[num_panel].fb[i].width,
			panel_array[num_panel].fb[i].height);

	num_panel++;

msm_display_init_out:
	return ret;
}

int msm_display_init_count() {
	return num_panel;
}

int msm_display_off()
{
	int ret = NO_ERROR;
	struct msm_panel_info *pinfo;

	if (!panel)
		return ERR_INVALID_ARGS;

	pinfo = &(panel->panel_info);

	if (pinfo->pre_off) {
		ret = pinfo->pre_off();
		if (ret)
			goto msm_display_off_out;
	}

	switch (pinfo->type) {
#ifdef DISPLAY_TYPE_MDSS
	case LVDS_PANEL:
		dprintf(SPEW, "Turn off LVDS PANEL.\n");
		mdp_lcdc_off();
		break;
	case MIPI_VIDEO_PANEL:
		dprintf(INFO, "Turn off MIPI_VIDEO_PANEL.\n");
		ret = mdp_dsi_video_off(pinfo);
		if (ret)
			goto msm_display_off_out;
		ret = mipi_dsi_off(pinfo);
		if (ret)
			goto msm_display_off_out;
		break;
	case MIPI_CMD_PANEL:
		dprintf(INFO, "Turn off MIPI_CMD_PANEL.\n");
		ret = mdp_dsi_cmd_off();
		if (ret)
			goto msm_display_off_out;
		ret = mipi_dsi_off(pinfo);
		if (ret)
			goto msm_display_off_out;
		break;
	case LCDC_PANEL:
		dprintf(INFO, "Turn off LCDC PANEL.\n");
		mdp_lcdc_off();
		break;
	case EDP_PANEL:
		dprintf(INFO, "Turn off EDP PANEL.\n");
		ret = mdp_edp_off();
		if (ret)
			goto msm_display_off_out;
		break;
	case HDMI_PANEL:
		dprintf(INFO, "Turn off HDMI PANEL.\n");
		ret = mdss_hdmi_off(pinfo);
		break;

#endif
#ifdef DISPLAY_TYPE_QPIC
	case QPIC_PANEL:
		dprintf(INFO, "Turn off QPIC_PANEL.\n");
		qpic_off();
		break;
#endif
	default:
		return ERR_INVALID_ARGS;
	};

	if (target_cont_splash_screen()) {
		dprintf(INFO, "Continuous splash enabled, keeping panel alive.\n");
		return NO_ERROR;
	}

	if (panel->post_power_func)
		ret = panel->post_power_func(0);
	if (ret)
		goto msm_display_off_out;

	/* Turn off backlight */
	if (panel->bl_func)
		ret = panel->bl_func(0);

	if (pinfo->off)
		ret = pinfo->off();

	/* Disable clock */
	if (panel->clk_func)
		ret = panel->clk_func(0, pinfo);

	/* Only for AUTO PLL calculation */
	if (panel->pll_clk_func)
		ret = panel->pll_clk_func(0, pinfo);

	if (ret)
		goto msm_display_off_out;

	/* Disable panel */
	if (panel->power_func)
		ret = panel->power_func(0, pinfo);

msm_display_off_out:
	return ret;
}
