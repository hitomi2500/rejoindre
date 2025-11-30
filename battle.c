#include <yaul.h>
#include <stdlib.h>
#include "video.h"
#include "video_vdp1.h"
#include "video_vdp2.h"

#define GRID_SIZE_X 10
#define GRID_SIZE_Y 12

//storing curves as bitmaps
const int Horizontal_Curve_26[260] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,2,2,2,2,2,2,0,0,0,0,0,0,2,2,2,2,2,2,0,0,0,0,
    2,2,2,2,1,1,1,1,1,1,2,0,0,0,0,2,1,1,1,1,1,1,2,2,2,2,
    1,1,1,1,1,1,1,1,1,1,2,0,0,0,0,2,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,2,0,0,0,0,0,0,2,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,2,0,0,0,0,0,0,2,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,2,0,0,0,0,2,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
};

static unsigned char Vertical_Curve_20[200] = {
    0,0,2,1,1,1,1,1,1,1,
    0,0,2,1,1,1,1,1,1,1,
    0,0,2,1,1,1,1,1,1,1,
    0,2,1,1,1,1,1,1,1,1,
    0,2,1,1,1,1,1,1,1,1,
    0,2,1,1,1,1,1,1,1,1,
    0,2,1,1,2,2,2,1,1,1,
    0,0,2,2,0,0,0,2,1,1,
    0,0,0,0,0,0,0,0,2,1,
    0,0,0,0,0,0,0,0,2,1,
    0,0,0,0,0,0,0,0,2,1,
    0,0,2,2,0,0,0,2,1,1,
    0,2,1,1,2,2,2,1,1,1,
    0,2,1,1,1,1,1,1,1,1,
    0,2,1,1,1,1,1,1,1,1,
    0,2,1,1,1,1,1,1,1,1,
    0,2,1,1,1,1,1,1,1,1,
    0,0,2,1,1,1,1,1,1,1,
    0,0,2,1,1,1,1,1,1,1,
    0,0,2,1,1,1,1,1,1,1
};

uint8_t drawarea[40*30];
int Curves_X[GRID_SIZE_Y - 1][GRID_SIZE_X - 1];
int Curves_Y[GRID_SIZE_Y - 1][GRID_SIZE_X - 1];

extern vdp1_cmdt_list_t *_cmdt_list;

void battle_init(uint8_t * tga, uint8_t * tga_half)
{
    vdp1_vram_partitions_t vdp1_vram_partitions;
    vdp1_vram_partitions_get(&vdp1_vram_partitions);

    //step 1 - load tga into VDP2 megaplane
    memcpy ((void *)VDP2_VRAM_ADDR(0, 0x00000), &(tga[18+256*3]),512*256);
	memcpy ((void *)VDP2_VRAM_ADDR(2, 0x00000), &(tga[18+256*3]),512*256);
	memcpy ((void *)VDP2_VRAM_ADDR(1, 0x00000), &(tga[18+256*3+0x20000]),512*256);
	memcpy ((void *)VDP2_VRAM_ADDR(3, 0x00000), &(tga[18+256*3+0x20000]),512*256);

	rgb888_t _color  = {0,0,0,0};
	for (int i = 0; i<256; i++)
    {
        _color.r = tga[18+i*3+2]/2;
        _color.g = tga[18+i*3+1]/2;
        _color.b = tga[18+i*3+0]/2;
        video_vdp2_set_palette_part(1, &_color, i, i);
        _color.r = tga[18+i*3+2];
        _color.g = tga[18+i*3+1];
        _color.b = tga[18+i*3+0];
        video_vdp2_set_palette_part(0, &_color, i, i);
    }

    //step 2 - generate random curve directions

    srand(100500);//vdp2_tvmd_vcount_get());//(((uint32_t)vdp2_tvmd_hcount_get())<<16) | (vdp2_tvmd_vcount_get()));

    for (int y=0;y<(GRID_SIZE_Y-1); y++) {
        for (int x=0;x<(GRID_SIZE_X-1); x++) {
            Curves_X[y][x] = rand();
            Curves_Y[y][x] = rand();
        }
    }

    //step 3 - using downsampled same-palette image to fill VDP1 sprites
    int sprite = 0;
    for (int y=0;y<(GRID_SIZE_Y); y++) {
        for (int x=0;x<(GRID_SIZE_X); x++) {

            //creating mask image first
            //tile size is 26 x 20, tile grid is 10x12, resolution is 260x240
            //cleaning draw area
            memset(drawarea,0,40*30);
            //drawing base quad at coords 7,5
            for (int yy=5;yy<24;yy++)
                for (int xx=7;xx<33;xx++)
                    drawarea[yy*40+xx] = 1;//pixel color

            //top curve, excluding top tiles
            if (y==0) {
                for (int xx=7;xx<33;xx++)
                    drawarea[5*40+xx] = 2;//border color
            } else {
                if (Curves_X[y][x] % 2 == 0) {
                    for (int yy=2;yy<12;yy++)
                        for (int xx=7;xx<33;xx++)
                            switch (Horizontal_Curve_26[(yy-2)*26+(xx-7)]) {
                            case 0:
                                drawarea[yy*40+xx] = 0;//transparent
                                break;
                            case 1:
                                drawarea[yy*40+xx] = 1;//pixel color
                                break;
                            case 2:
                                drawarea[yy*40+xx] = 2;//border color
                                break;
                            }
                } else {
                    for (int yy=0;yy<9;yy++)
                        for (int xx=7;xx<33;xx++)
                            switch (Horizontal_Curve_26[(8-yy)*26+(xx-7)]) {
                            case 0:
                                drawarea[yy*40+xx] = 1;//pixel color
                                break;
                            case 1:
                                drawarea[yy*40+xx] = 0;//transparent
                                break;
                            case 2:
                                drawarea[yy*40+xx] = 2;//border color
                                break;
                            }
                }
            }

            //bottom gradient, excluding bottom tiles
            if (y == (GRID_SIZE_Y-1)) {
                for (int xx=7;xx<33;xx++)
                    drawarea[24*40+xx] = 2;//border color
            } else {
                if (Curves_X[y+1][x] % 2 == 0) {
                    for (int yy=21;yy<30;yy++)
                        for (int xx=7;xx<33;xx++)
                            switch (Horizontal_Curve_26[(yy-21)*26+(xx-7)]) {
                            case 0:
                                drawarea[yy*40+xx] = 1;//pixel color
                                break;
                            case 1:
                                drawarea[yy*40+xx] = 0;//transparent
                                break;
                            case 2:
                                drawarea[yy*40+xx] = 2;//border color
                                break;
                            }
                } else {
                    for (int yy=18;yy<28;yy++)
                        for (int xx=7;xx<33;xx++)
                            switch (Horizontal_Curve_26[(27-yy)*26+(xx-7)]) {
                            case 0:
                                drawarea[yy*40+xx] = 0;//transparent
                                break;
                            case 1:
                                drawarea[yy*40+xx] = 1;//pixel color
                                break;
                            case 2:
                                drawarea[yy*40+xx] = 2;//border color
                                break;
                            }
                }
            }

            //left gradient, excluding left tiles
            if (x==0) {
                for (int yy=5;yy<25;yy++)
                    drawarea[yy*40+7] = 2;//border color
            } else {
                if (Curves_Y[y][x] % 2 == 0) {
                    for (int yy=6;yy<24;yy++)
                        for (int xx=5;xx<9;xx++)
                            switch (Vertical_Curve_20[(yy-5)*10+(xx-5)]) {
                            case 0:
                                drawarea[yy*40+xx] = 0;//transparent
                                break;
                            case 1:
                                drawarea[yy*40+xx] = 1;//pixel color
                                break;
                            case 2:
                                drawarea[yy*40+xx] = 2;//border color
                                break;
                            }
                    for (int yy=11;yy<18;yy++)
                        for (int xx=9;xx<15;xx++)
                            switch (Vertical_Curve_20[(yy-5)*10+(xx-5)]) {
                            case 0:
                                drawarea[yy*40+xx] = 0;//transparent
                                break;
                            case 1:
                                drawarea[yy*40+xx] = 1;//pixel color
                                break;
                            case 2:
                                drawarea[yy*40+xx] = 2;//border color
                                break;
                            }
                } else {
                    for (int yy=6;yy<24;yy++)
                        for (int xx=0;xx<10;xx++)
                            switch (Vertical_Curve_20[(yy-5)*10+(9-xx)]) {
                            case 0:
                                drawarea[yy*40+xx] = 1;//pixel color
                                break;
                            case 1:
                                drawarea[yy*40+xx] = 0;//transparent
                                break;
                            case 2:
                                drawarea[yy*40+xx] = 2;//border color
                                break;
                            }
                }
            }

            //right gradient, excluding right tiles
            if (x==(GRID_SIZE_X-1)) {
                for (int yy=5;yy<25;yy++)
                    drawarea[yy*40+33] = 2;//border color
            } else {
                if (Curves_Y[y][x+1] % 2 == 0) {
                    for (int yy=6;yy<24;yy++)
                        for (int xx=31;xx<40;xx++)
                            switch (Vertical_Curve_20[(yy-4)*10+(xx-40)]) {
                            case 0:
                                drawarea[yy*40+xx] = 1;//pixel color
                                break;
                            case 1:
                                drawarea[yy*40+xx] = 0;//transparent
                                break;
                            case 2:
                                drawarea[yy*40+xx] = 2;//border color
                                break;
                            }
                } else {
                    for (int yy=11;yy<20;yy++)
                        for (int xx=25;xx<35;xx++)
                            switch (Vertical_Curve_20[(yy-4)*10+(24-xx)]) {
                            case 0:
                                drawarea[yy*40+xx] = 0;//transparent
                                break;
                            case 1:
                                drawarea[yy*40+xx] = 1;//pixel color
                                break;
                            case 2:
                                drawarea[yy*40+xx] = 2;//border color
                                break;
                            }
                    for (int yy=6;yy<24;yy++)
                        for (int xx=29;xx<35;xx++)
                            switch (Vertical_Curve_20[(yy-4)*10+(24-xx)]) {
                            case 0:
                                drawarea[yy*40+xx] = 0;//transparent
                                break;
                            case 1:
                                drawarea[yy*40+xx] = 1;//pixel color
                                break;
                            case 2:
                                drawarea[yy*40+xx] = 2;//border color
                                break;
                            }
                }
            }

            //now applying mask image for VDP1 sprites
            for (int yy=0;yy<30;yy++) {
                uint8_t * sprite_dst = (uint8_t *) (vdp1_vram_partitions.texture_base + 0x4000 + sprite*0x500 + yy*40);
                uint8_t * sprite_src = (uint8_t *) &tga_half[18+256*3+(y*20+yy-5)*256+x*25-7];
                int y_index = yy*40;
                for (int xx=0;xx<40;xx++)
                    switch(drawarea[y_index+xx]) {
                        case 0:
                            sprite_dst[xx] = 0;//transparent
                            break;
                        case 1:
                            sprite_dst[xx] = sprite_src[xx];
                            break;
                        case 2:
                            sprite_dst[xx] = 1;//border color
                            break;
                    }
                }

            sprite++;
        }
    }
}


void battle_scheduler()
{
    //using random shifts for every tile
    int x,y;
    uint16_t buf;
    for (int tile=0;tile<(GRID_SIZE_Y*GRID_SIZE_X); tile++)
    {
        buf = _cmdt_list->cmdts[10+tile].cmd_xa;
        buf += ((rand()&0x7)-4);
        if (buf < 0) buf = 0;
        if (buf > 320) buf = 320;
        _cmdt_list->cmdts[10+tile].cmd_xa = buf;

        buf = _cmdt_list->cmdts[10+tile].cmd_ya;
        buf += ((rand()&0x7)-4);
        if (buf < 0) buf = 0;
        if (buf > 240) buf = 240;
        _cmdt_list->cmdts[10+tile].cmd_ya = buf;
    }
    vdp1_sync_cmdt_list_put(_cmdt_list, 0);
}