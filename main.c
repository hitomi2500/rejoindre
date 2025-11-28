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

extern uint8_t asset_bitmap_tga[];

int
main(void)
{
 
        tga_t tga;
        int ret __unused;
        ret = tga_read(&tga, asset_bitmap_tga);
        assert(ret == TGA_FILE_OK);

		memcpy ((void *)VDP2_VRAM_ADDR(0, 0x00000), &(asset_bitmap_tga[18+256*3]),512*256);
		memcpy ((void *)VDP2_VRAM_ADDR(2, 0x00000), &(asset_bitmap_tga[18+256*3]),512*256);
		memcpy ((void *)VDP2_VRAM_ADDR(1, 0x00000), &(asset_bitmap_tga[18+256*3+0x20000]),512*256);
		memcpy ((void *)VDP2_VRAM_ADDR(3, 0x00000), &(asset_bitmap_tga[18+256*3+0x20000]),512*256);

		rgb888_t _color  = {0,0,0,0};
		for (int i = 0; i<256; i++)
        {
            _color.r = asset_bitmap_tga[18+i*3+2];
            _color.g = asset_bitmap_tga[18+i*3+1];
            _color.b = asset_bitmap_tga[18+i*3+0];
            video_vdp2_set_palette_part(0, &_color, i, i);
            video_vdp2_set_palette_part(1, &_color, i, i);
            video_vdp2_set_palette_part(2, &_color, i, i);
            video_vdp2_set_palette_part(3, &_color, i, i);
            video_vdp2_set_palette_part(4, &_color, i, i);
            video_vdp2_set_palette_part(5, &_color, i, i);
            video_vdp2_set_palette_part(6, &_color, i, i);
        }

        	int sel = 0;
	bool redrawMenu = true, redrawBG = true, key_pressed = false;
	int menu_size=0;
	char string_buf[128];

	video_screen_mode_t screenMode =
	{
		.scanmode = VIDEO_SCANMODE_480P,
		.x_res = VIDEO_X_RESOLUTION_320,
		.y_res = VDP2_TVMD_VERT_240,
		.x_res_doubled = true,
		.colorsystem = VDP2_TVMD_TV_STANDARD_NTSC,
	};
	
		//show yaul logo in 240p
	video_init(screenMode,BITMAP_MODE_256_COLORS);
	video_vdp2_set_cycle_patterns_nbg_bmp(screenMode);

        vdp2_sync();
        vdp2_sync_wait();
		
        while (true) {
        }

        return 0;
}
