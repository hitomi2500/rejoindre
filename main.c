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

extern uint8_t asset_bitmap_tga[];

int
main(void)
{
	//init video
	video_screen_mode_t screenMode =
	{
		.scanmode = VIDEO_SCANMODE_480P,
		.x_res = VIDEO_X_RESOLUTION_320,
		.y_res = VDP2_TVMD_VERT_240,
		.x_res_doubled = true,
		.colorsystem = VDP2_TVMD_TV_STANDARD_NTSC,
	};
	
	video_init(screenMode,BITMAP_MODE_256_COLORS);
	video_vdp2_set_cycle_patterns_nbg_bmp(screenMode);
 
    /*tga_t tga;
    int ret __unused;
    ret = tga_read(&tga, asset_bitmap_tga);
    assert(ret == TGA_FILE_OK);*/

	battle_init(asset_bitmap_tga);

    vdp2_sync();
    vdp2_sync_wait();

	while (true) {
    }

    return 0;
}
