/*
 * Copyright (c) 2012 Israel Jacquez
 * See LICENSE for details.
 *
 * Israel Jacquez <mrkotfw@gmail.com>
 */

#include <yaul.h>
#include <tga.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <video.h>
#include "battle.h"
#include "videomode_select.h"

extern uint8_t asset_bitmap_tga[];
extern uint8_t asset_bitmap2_tga[];

static void rejoindre_vblank_out_handler(void *work __unused);

int global_frame_count;

smpc_peripheral_digital_t controller;

extern uint8_t asset_sound_driver[];
extern uint8_t asset_sound_driver_end[];

int
main(void)
{
	//init input first, to select videomode
	smpc_peripheral_init();

	vdp_sync_vblank_out_set(rejoindre_vblank_out_handler, NULL);

	//selecting videomode for the entire game
	videomode_select(&controller);
	//init video
	/*video_screen_mode_t screenMode =
	{
		.scanmode = VIDEO_SCANMODE_480P,
		.x_res = VIDEO_X_RESOLUTION_320,
		.y_res = VDP2_TVMD_VERT_240,
		.x_res_doubled = true,
		.colorsystem = VDP2_TVMD_TV_STANDARD_NTSC,
	};
	
	video_init(screenMode,BITMAP_MODE_256_COLORS);
	video_vdp2_set_cycle_patterns_nbg_bmp(screenMode);*/

	adpcm_load_driver(asset_sound_driver, asset_sound_driver_end - asset_sound_driver);

	battle_init(asset_bitmap_tga,asset_bitmap2_tga);

    vdp2_sync();
    vdp2_sync_wait();

	int divider = 0;
	while (true) {
		smpc_peripheral_process();
		get_digital_keypress_anywhere(&controller);

		//divider++;
		//if (divider > 3)
		//{
		//	divider = 0;
			battle_scheduler(&controller);
		//}
		vdp2_sync();
    	vdp2_sync_wait();
    }

    return 0;
}

static void rejoindre_vblank_out_handler(void *work __unused)
{
    global_frame_count++;
    smpc_peripheral_intback_issue();
}