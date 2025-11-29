#include <yaul.h>
#include <stdlib.h>
#include "video.h"
#include "video_vdp1.h"
#include "video_vdp2.h"

void battle_init(uint8_t * tga)
{
    vdp1_cmdt_list_t *_cmdt_list;

    //step 1 - load tga into VDP2 megaplane
    memcpy ((void *)VDP2_VRAM_ADDR(0, 0x00000), &(tga[18+256*3]),512*256);
	memcpy ((void *)VDP2_VRAM_ADDR(2, 0x00000), &(tga[18+256*3]),512*256);
	memcpy ((void *)VDP2_VRAM_ADDR(1, 0x00000), &(tga[18+256*3+0x20000]),512*256);
	memcpy ((void *)VDP2_VRAM_ADDR(3, 0x00000), &(tga[18+256*3+0x20000]),512*256);

	rgb888_t _color  = {0,0,0,0};
	for (int i = 0; i<256; i++)
    {
        _color.r = tga[18+i*3+2];
        _color.g = tga[18+i*3+1];
        _color.b = tga[18+i*3+0];
        video_vdp2_set_palette_part(0, &_color, i, i);
    }
}