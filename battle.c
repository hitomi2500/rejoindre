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

int battle_cursor_x;
int battle_cursor_y;
int battle_cursor_tile_offset_x;
int battle_cursor_tile_offset_y;

extern uint8_t asset_cursor1_tga[];
extern uint8_t asset_cursor2_tga[];

int battle_cursor_history[16] = {0};

vdp1_vram_partitions_t battle_vdp1_vram_partitions;

int tiles_x[120];
int tiles_y[120];

int selected_tile = -1;

void battle_init(uint8_t * tga, uint8_t * tga_half)
{
    srand(100500);//vdp2_tvmd_vcount_get());//(((uint32_t)vdp2_tvmd_hcount_get())<<16) | (vdp2_tvmd_vcount_get()));
    vdp1_vram_partitions_get(&battle_vdp1_vram_partitions);

    //step 1 - DO NOT load tga into VDP2 megaplane, FILL WITH RANDOM SHIT!
    //memcpy ((void *)VDP2_VRAM_ADDR(0, 0x00000), &(tga[18+256*3]),512*256);
	//memcpy ((void *)VDP2_VRAM_ADDR(2, 0x00000), &(tga[18+256*3]),512*256);
	//memcpy ((void *)VDP2_VRAM_ADDR(1, 0x00000), &(tga[18+256*3+0x20000]),512*256);
	//memcpy ((void *)VDP2_VRAM_ADDR(3, 0x00000), &(tga[18+256*3+0x20000]),512*256);
    for (int i=0;i<512*512;i++) {
        *((uint8_t *)VDP2_VRAM_ADDR(0, i)) = rand();
        *((uint8_t *)VDP2_VRAM_ADDR(2, i)) = rand();
    }

	rgb888_t _color  = {0,0,0,0};
	for (int i = 0; i<256; i++)
    {
        /*_color.r = tga[18+i*3+2]/2;
        _color.g = tga[18+i*3+1]/2;
        _color.b = tga[18+i*3+0]/2;
        video_vdp2_set_palette_part(1, &_color, i, i);*/
        _color.r = tga[18+i*3+2];
        _color.g = tga[18+i*3+1];
        _color.b = tga[18+i*3+0];
        video_vdp2_set_palette_part(0, &_color, i, i);
    }

    //step 2 - generate random curve directions
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
                uint8_t * sprite_dst = (uint8_t *) (battle_vdp1_vram_partitions.texture_base + 0x4000 + sprite*0x500 + yy*40);
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

    //step 4 - assign random coords for every tile
    int tile = 0;
    for (int y=0;y<(GRID_SIZE_Y); y++) {
        for (int x=0;x<(GRID_SIZE_X); x++) {
            tiles_x[tile] = ((unsigned short)rand()) % 300 - 10;
            tiles_y[tile] = ((unsigned short)rand()) % 200 + 10;
            _cmdt_list->cmdts[10+tile].cmd_xa = tiles_x[tile];
            _cmdt_list->cmdts[10+tile].cmd_ya = tiles_y[tile];
            tile++;
        }
    }

    //cursor stuff 
    battle_cursor_x = 128;
    battle_cursor_y = 120;
    memcpy ((void *)(battle_vdp1_vram_partitions.texture_base+0x1000), &(asset_cursor1_tga[18+3*3]),32*32);
    _cmdt_list->cmdts[200].cmd_xa = battle_cursor_x;
    _cmdt_list->cmdts[200].cmd_ya = battle_cursor_y;

    //cursor palette
	for (int i = 0; i<3; i++)
    {
        _color.r = asset_cursor1_tga[18+i*3+2];
        _color.g = asset_cursor1_tga[18+i*3+1];
        _color.b = asset_cursor1_tga[18+i*3+0];
        video_vdp2_set_palette_part(3, &_color, i, i);
    }

    //update vdp1
    vdp1_sync_cmdt_list_put(_cmdt_list, 0);
}


void battle_scheduler(smpc_peripheral_digital_t * controller)
{
        //cursor or tile movement
        if(controller->pressed.button.up) {
            battle_cursor_y -= 2;
            battle_cursor_y -= (battle_cursor_history[0] > 5) ? 2 : 0;
            if (battle_cursor_y < 2) battle_cursor_y  = 2;
            battle_cursor_history[0]++;
        } else
            battle_cursor_history[0] = 0;
        if(controller->pressed.button.down) {
            battle_cursor_y += 2;
            battle_cursor_y += (battle_cursor_history[1] > 5) ? 2 : 0;
            if (battle_cursor_y > 234) battle_cursor_y  = 234;
            battle_cursor_history[1]++;
        } else
            battle_cursor_history[1] = 0;
        if(controller->pressed.button.left) {
            battle_cursor_x -= 2;
            battle_cursor_x -= (battle_cursor_history[2] > 5) ? 2 : 0;
            if (battle_cursor_x < -8) battle_cursor_x  = -8;
            battle_cursor_history[2]++;
        } else
            battle_cursor_history[2] = 0;
        if(controller->pressed.button.right) {
            battle_cursor_x += 2;
            battle_cursor_x += (battle_cursor_history[3] > 5) ? 2 : 0;
            if (battle_cursor_x > 304) battle_cursor_x  = 304;
            battle_cursor_history[3]++;
        } else
            battle_cursor_history[3] = 0;

    if (-1 == selected_tile) {
        //show cursor
        _cmdt_list->cmdts[200].cmd_xa = battle_cursor_x;
        _cmdt_list->cmdts[200].cmd_ya = battle_cursor_y;
    } else {
        //centering tile on cursor hot point
        tiles_x[selected_tile] = battle_cursor_x + 12 - 20;
        tiles_y[selected_tile] = battle_cursor_y + 2 - 15;
        //update tile coord
        _cmdt_list->cmdts[10+selected_tile].cmd_xa = tiles_x[selected_tile];
        _cmdt_list->cmdts[10+selected_tile].cmd_ya = tiles_y[selected_tile];
        //hide cursor
        _cmdt_list->cmdts[200].cmd_xa = -100;
        _cmdt_list->cmdts[200].cmd_ya = -100;
    }
    
    //cursor grab button
    if ( (controller->pressed.button.a) || (controller->pressed.button.c) ) {
        if (0 == battle_cursor_history[5]) {
            //pressing grab button
            //searching for nearby tile
            selected_tile = -1;
            //cursor hot point is {12;2}
            int hot_x = battle_cursor_x+12;
            int hot_y = battle_cursor_y+2;
            for (int tile=0;tile<120;tile++)
            {
                //NOT using mahnattan distance
                //tile active border is {7,5} - {33,24}
                /*int abs_x = ((battle_cursor_x+12) > tiles_x[tile]) ? ((battle_cursor_x+12) - tiles_x[tile]) : (tiles_x[tile] - (battle_cursor_x+12));
                int abs_y = ((battle_cursor_y+2) > tiles_y[tile]) ? ((battle_cursor_y+2) - tiles_y[tile]) : (tiles_y[tile] - (battle_cursor_y+2));
                if ( (abs_x < 5) && (abs_y < 5) ) */
                if ( (hot_x >= tiles_x[tile]+7) && (hot_x <= tiles_x[tile]+33) && (hot_y >= tiles_y[tile]+5) && (hot_y <= tiles_y[tile]+24) ){
                    selected_tile = tile;
                }
            }
            if (-1 == selected_tile) {
                //no tile found, changing cursor to no go
                memcpy ((void *)(battle_vdp1_vram_partitions.texture_base+0x1000), &(asset_cursor2_tga[18+3*3]),32*32);
            } else {
                //tile found, hiding cursor
                _cmdt_list->cmdts[200].cmd_xa = -100;
                _cmdt_list->cmdts[200].cmd_ya = -100;
            }
        }
        battle_cursor_history[5]++;
    } else {
        if (battle_cursor_history[5]) {
            //checking if sprite is placed correctly
            int expected_x = (selected_tile%10)*26-20;
            int expected_y = (selected_tile/10)*20-15;
            //using mahnattan distance
            int abs_x = (expected_x > tiles_x[selected_tile]) ? (expected_x - tiles_x[selected_tile]) : (tiles_x[selected_tile] - expected_x);
            int abs_y = (expected_y > tiles_y[selected_tile]) ? (expected_y - tiles_y[selected_tile]) : (tiles_y[selected_tile] - expected_y);
            if ( (abs_x < 4) && (abs_y < 4) ){
                //match, move tile away
                tiles_x[selected_tile] = -200;
                tiles_y[selected_tile] = -200;
                //TODO: update VDP2 layer
            }
            //releasing grab button
            selected_tile = -1;
            //restore cursor sprite in case it was different
            memcpy ((void *)(battle_vdp1_vram_partitions.texture_base+0x1000), &(asset_cursor1_tga[18+3*3]),32*32);
        }
        battle_cursor_history[5] = 0;
    }

    //using random shifts for every tile
    /*int x,y;
    uint16_t buf;
    for (int tile=0;tile<(GRID_SIZE_Y*GRID_SIZE_X); tile++)
    {
        buf = _cmdt_list->cmdts[10+tile].cmd_xa;
        buf += ((rand()&0x7)-4) + shift_x;
        if (buf < 0) buf = 0;
        if (buf > 320) buf = 320;
        _cmdt_list->cmdts[10+tile].cmd_xa = buf;

        buf = _cmdt_list->cmdts[10+tile].cmd_ya;
        buf += ((rand()&0x7)-4) + shift_y;
        if (buf < 0) buf = 0;
        if (buf > 240) buf = 240;
        _cmdt_list->cmdts[10+tile].cmd_ya = buf;
    }*/
    vdp1_sync_cmdt_list_put(_cmdt_list, 0);
}