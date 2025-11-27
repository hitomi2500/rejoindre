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

        //tga_image_decode(&tga, (void *)VDP2_VRAM_ADDR(1, 0x00000));
		//tga_image_decode(&tga, (void *)VDP2_VRAM_ADDR(3, 0x00000));
		memcpy ((void *)VDP2_VRAM_ADDR(0, 0x00000), &(asset_bitmap_tga[18+256*3]),512*256);
		memcpy ((void *)VDP2_VRAM_ADDR(2, 0x00000), &(asset_bitmap_tga[18+256*3]),512*256);
		memcpy ((void *)VDP2_VRAM_ADDR(1, 0x00000), &(asset_bitmap_tga[18+256*3+0x20000]),512*256);
		memcpy ((void *)VDP2_VRAM_ADDR(3, 0x00000), &(asset_bitmap_tga[18+256*3+0x20000]),512*256);
		/*for (int i=0;i<512*256;i++)
			*((uint8_t *)VDP2_VRAM_ADDR(0, i)) = 0;
		/*for (int i=0;i<512*480;i++)
			*((uint8_t *)VDP2_VRAM_ADDR(1, i)) = 0;*/
		/*for (int i=0;i<512*256;i++)
			*((uint8_t *)VDP2_VRAM_ADDR(1, i)) = 0;
		for (int i=0;i<512*256;i++)
			*((uint8_t *)VDP2_VRAM_ADDR(3, i)) = 0;*/
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
	//video_vdp2_set_cycle_patterns_cpu();
	//background_set_from_assets(asset_bootlogo_bg,(int)(asset_bootlogo_bg_end-asset_bootlogo_bg),VIDEO_VDP2_NBG0_PNDR_START,VIDEO_VDP2_NBG0_CHPNDR_START);
	video_vdp2_set_cycle_patterns_nbg_bmp(screenMode);

        vdp2_sync();
        vdp2_sync_wait();
		
		/*uint16_t * p_WPSX = (uint16_t *)0x25f800c0;
		p_WPSX[0] = 0;//sx
		p_WPSX[1] = 0;//sy
		p_WPSX[2] = 100;//ex
		p_WPSX[3] = 100;//ey
		uint16_t * p_WCTL = (uint16_t *)0x25f800d0;
		p_WCTL[0] = 0x82;*/
			
		//using direct regs access because we need mid-frame hacking
		/*uint16_t * p_MPOFN = (uint16_t *)0x25f8003c;
		uint16_t * p_VCNT =  (uint16_t *)0x25f8000a;
		uint16_t * p_CYC =  (uint16_t *)0x25f80010;
		p_CYC[1] = 0xffff;
		p_CYC[3] = 0xffff;
		p_CYC[5] = 0xffff;
		p_CYC[7] = 0xffff;*/
		/*for (int i=0;i<8;i++) 
			pCYC[i] = 0xffff;*/
        while (true) {
			/*if (*p_VCNT > 250)
			{
				*p_MPOFN = 0x0012;//switching NBG0 to bank 1->2 and NBG1 to bank 2->3
				p_CYC[0] = 0x4444;
				p_CYC[1] = 0xffff;
				p_CYC[2] = 0x4444;
				p_CYC[3] = 0xffff;
			}
			else
			{
				*p_MPOFN = 0x0001;//switching NBG0 to bank 0 and NBG1 to bank 1
				p_CYC[0] = 0xffff;
				p_CYC[1] = 0x4444;
				p_CYC[2] = 0xffff;
				p_CYC[3] = 0x4444;
			}*/

        }

        return 0;
}
