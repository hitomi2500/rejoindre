#include <yaul.h>
#include <stdlib.h>
#include "video.h"
#include "video_vdp1.h"
#include "video_vdp2.h"
#include "font_renderer.h"
#include "font_pacifico_16.h"
#include "battle.h"

#define GRID_SIZE_X 10
#define GRID_SIZE_Y 12

#define SOUND_EFFECT_GRAB 0
#define SOUND_EFFECT_RELEASE 1
#define SOUND_EFFECT_GRAB_FAIL 2
#define SOUND_EFFECT_LINK 3
#define SOUND_EFFECT_FUSE 4
#define SOUND_EFFECT_FUSE_MULTIPLE 5

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

uint8_t Curves_X[16][16];//[GRID_SIZE_Y - 1][GRID_SIZE_X - 1];
uint8_t Curves_Y[16][16];//[GRID_SIZE_Y - 1][GRID_SIZE_X - 1];

extern vdp1_cmdt_list_t *_cmdt_list;

int battle_cursor_x;
int battle_cursor_y;
int battle_cursor_tile_offset_x;
int battle_cursor_tile_offset_y;

extern uint8_t asset_cursor1_tga[];
extern uint8_t asset_cursor2_tga[];
extern uint8_t e_fuse_adp[];
extern uint8_t e_fuse_adp_end[];
extern uint8_t e_fuse_m_adp[];
extern uint8_t e_fuse_m_adp_end[];
extern uint8_t e_grab_adp[];
extern uint8_t e_grab_adp_end[];
extern uint8_t e_grab_f_adp[];
extern uint8_t e_grab_f_adp_end[];
extern uint8_t e_link_adp[];
extern uint8_t e_link_adp_end[];
extern uint8_t e_release_adp[];
extern uint8_t e_release_adp_end[];

int battle_cursor_history[16] = {0};

vdp1_vram_partitions_t battle_vdp1_vram_partitions;

int pieces_x[120];
int pieces_y[120];

uint8_t pieces_link_array[120];
int pieces_link_groups;

int selected_piece = -1;

uint8_t * tga_copy;

extern int global_frame_count;

//every part of battle treats the titles in an unsorted linear manner, with the exception of VDP1 setup
//pieces textures are stored in VDP1 vram constantly, but the commandlist is updated in accordance to current sort order

uint8_t pieces_game_to_vdp1[120]; //fetch converts indexes from game index to vdp1 command index
uint8_t pieces_vdp1_to_game[120]; //fetch converts indexes from vdp1 command index to vdp1 index

void generate_piece_mask(uint8_t * drawarea, int piece_x, int piece_y)
{
            //tile size is 26 x 20, tile grid is 10x12, resolution is 260x240
            //cleaning draw area
            memset(drawarea,0,40*30);
            //drawing base quad at coords 7,5
            for (int yy=5;yy<24;yy++)
                for (int xx=7;xx<33;xx++)
                    drawarea[yy*40+xx] = 1;//pixel color

            //top curve, excluding top tiles
            if (piece_y==0) {
                for (int xx=7;xx<33;xx++)
                    drawarea[5*40+xx] = 2;//border color
            } else {
                if (Curves_X[piece_y][piece_x] % 2 == 0) {
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

            //bottom curve, excluding bottom tiles
            if (piece_y >= (GRID_SIZE_Y-1)) {
                for (int xx=7;xx<33;xx++)
                    drawarea[24*40+xx] = 2;//border color
            } else {
                if (Curves_X[piece_y+1][piece_x] % 2 == 0) {
                    for (int yy=21;yy<30;yy++)
                        for (int xx=7;xx<33;xx++)
                            switch (Horizontal_Curve_26[(yy-22)*26+(xx-7)]) { //21
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
                            switch (Horizontal_Curve_26[(28-yy)*26+(xx-7)]) { //27
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

            //left curve, excluding left tiles
            if (piece_x==0) {
                for (int yy=5;yy<25;yy++)
                    drawarea[yy*40+7] = 2;//border color
            } else {
                if (Curves_Y[piece_y][piece_x] % 2 == 0) {
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

            //right curve, excluding right tiles
            if (piece_x>=(GRID_SIZE_X-1)) {
                for (int yy=5;yy<25;yy++)
                    drawarea[yy*40+33] = 2;//border color
            } else {
                if (Curves_Y[piece_y][piece_x+1] % 2 == 0) {
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
}

void mcu_renderer_draw_text(mr_t mr, char * string, const uint8_t * font, mr_rectangle_t rect, mr_color_t color,
		mr_color_t background_color)
{
    mr_set_font(&mr, font);
    mr_set_fill_color(&mr, background_color);
    mr_set_stroke_color(&mr, color);
    mr_point_t offset = (mr_point_t){ (rect.width - mr_get_utf8_text_width(&mr, (uint8_t *)string)) / 2,
                           (rect.height - mr_get_line_height(&mr)) / 2};
    mr_draw_utf8_text(&mr,(uint8_t *)string,&rect,&offset);
}

int link_neigbour(root,neigbour,link_x,link_y) {
                //rendering debug info
            char draw_string[32];
            mr_t mr;
            mr.display_width = 32;
            mr.display_height = 20;
            mr_point_t offset;
            mr.draw_rectangle_callback = mr_draw_rectangle_framebuffer_color;
            mr.draw_string_callback = mr_draw_string_framebuffer_color;
            mr.display = 0;
            mr.buffer = (uint8_t *)LWRAM(0);
    mr_rectangle_t rectangle;

    //if already linked, do not link again
    if ((pieces_link_array[root]) && 
        (pieces_link_array[root]==pieces_link_array[neigbour]))
            return 0;
    int abs_x = pieces_x[neigbour] - pieces_x[root];
    abs_x -= link_x;
    abs_x = (abs_x > 0) ? abs_x : -abs_x;
    int abs_y = pieces_y[neigbour] - pieces_y[root];
    abs_y -= link_y;
    abs_y = (abs_y > 0) ? abs_y : -abs_y;

                    sprintf(draw_string,"%d",abs_x);
                rectangle = (mr_rectangle_t){0,0,32,20};
                mcu_renderer_draw_text(mr, draw_string, font_pacifico_16,rectangle,mr_get_color(0x00007F),mr_get_color(0x000000));
                for (int i = 0; i<32*20; i++)
                    *(uint8_t*)(battle_vdp1_vram_partitions.texture_base+32*100+i) = 16+*(uint8_t*)LWRAM(i*2+1);

                sprintf(draw_string,"%d",abs_y);
                rectangle = (mr_rectangle_t){0,0,32,20};
                mcu_renderer_draw_text(mr, draw_string, font_pacifico_16,rectangle,mr_get_color(0x00007F),mr_get_color(0x000000));
                for (int i = 0; i<32*20; i++)
                    *(uint8_t*)(battle_vdp1_vram_partitions.texture_base+32*120+i) = 16+*(uint8_t*)LWRAM(i*2+1);

    //using mahnattan distance
    if ( (abs_x < 5) && (abs_y < 5) ) {
        //it's a link, shifting neighbour to fit!
        adcpm_play_sample(2,SOUND_EFFECT_LINK);
        int neighbour_snap_offset_x = pieces_x[neigbour];
        int neighbour_snap_offset_y = pieces_y[neigbour];
        pieces_x[neigbour] = pieces_x[root]+link_x;
        pieces_y[neigbour] = pieces_y[root]+link_y;
        neighbour_snap_offset_x -= pieces_x[neigbour];
        neighbour_snap_offset_y -= pieces_y[neigbour];
        //updating vdp1 coords
        _cmdt_list->cmdts[pieces_game_to_vdp1[neigbour]+10].cmd_xa = pieces_x[neigbour];
        _cmdt_list->cmdts[pieces_game_to_vdp1[neigbour]+10].cmd_ya = pieces_y[neigbour];
        if ((pieces_link_array[root]) && (0==pieces_link_array[neigbour])) {
            //adding neigbour to group
            pieces_link_array[neigbour] = pieces_link_array[root];
        } else if (0 == (pieces_link_array[root]) && (pieces_link_array[neigbour])) {
            //root being added to neighbour's group
            pieces_link_array[root] = pieces_link_array[neigbour];
            //updating entire group coords
            for (int i=0;i<120;i++) {
                if (pieces_link_array[i] == pieces_link_array[neigbour])
                    if  ( (i != root) && (i != neigbour) ) {
                        //it's a link, shifting neighbour to fit!
                        pieces_x[i] -= neighbour_snap_offset_x;
                        pieces_y[i] -= neighbour_snap_offset_y;
                        //updating vdp1 coords
                        _cmdt_list->cmdts[pieces_game_to_vdp1[i]+10].cmd_xa = pieces_x[i];
                        _cmdt_list->cmdts[pieces_game_to_vdp1[i]+10].cmd_ya = pieces_y[i];
                    }
            }
        } else if ((pieces_link_array[root]) && (pieces_link_array[neigbour])) {
            //merging groups! fun times!
            int merger = pieces_link_array[root];
            int merged = pieces_link_array[neigbour];
            for (int i=0;i<120;i++)
                if (pieces_link_array[i] == merged) pieces_link_array[i] = merger;
            //updating entire group coords
            for (int i=0;i<120;i++) {
                if (pieces_link_array[i] == pieces_link_array[neigbour])
                    if  ( (i != root) && (i != neigbour) ) {
                        //it's a link, shifting neighbour to fit!
                        pieces_x[i] -= neighbour_snap_offset_x;
                        pieces_y[i] -= neighbour_snap_offset_y;
                        //updating vdp1 coords
                        _cmdt_list->cmdts[pieces_game_to_vdp1[i]+10].cmd_xa = pieces_x[i];
                        _cmdt_list->cmdts[pieces_game_to_vdp1[i]+10].cmd_ya = pieces_y[i];
                    }
                }
                    
        } else {
            //new group
            pieces_link_groups++;
            pieces_link_array[root] = pieces_link_groups;
            pieces_link_array[neigbour] = pieces_link_groups;
        }
        return 1;
    }
    return 0;
}

void fuse_piece(int selected_piece)
{
    uint8_t drawarea[40*30];
    int piece_x = (selected_piece%10);
    int piece_y = (selected_piece/10);
    //match, move tile away
                pieces_x[selected_piece] = -200;
                pieces_y[selected_piece] = -200;
                //update piece coord
                _cmdt_list->cmdts[pieces_game_to_vdp1[selected_piece]+10].cmd_xa = pieces_x[selected_piece];
                _cmdt_list->cmdts[pieces_game_to_vdp1[selected_piece]+10].cmd_ya = pieces_y[selected_piece];
                //update VDP2 layer, generating mask first
                generate_piece_mask(drawarea, piece_x, piece_y);

                //now restoring VDP2 pixels according to this mask
                int fuse_x = piece_x*50-14;
                int fuse_y = piece_y*40-10;
                for (int yy=0; yy<30; yy++)
                    for (int xx=0; xx<40; xx++)
                        if (drawarea[yy*40+xx]==1) {
                            int index = (fuse_y+yy*2)*512+fuse_x+xx*2;
                            *((uint8_t *)VDP2_VRAM_ADDR(0, index)) = tga_copy[18+256*3+index];
                            *((uint8_t *)VDP2_VRAM_ADDR(0, index+1)) = tga_copy[18+256*3+index+1];
                            *((uint8_t *)VDP2_VRAM_ADDR(0, index+512)) = tga_copy[18+256*3+index+512];
                            *((uint8_t *)VDP2_VRAM_ADDR(0, index+513)) = tga_copy[18+256*3+index+513];
                            *((uint8_t *)VDP2_VRAM_ADDR(2, index)) = tga_copy[18+256*3+index];
                            *((uint8_t *)VDP2_VRAM_ADDR(2, index+1)) = tga_copy[18+256*3+index+1];
                            *((uint8_t *)VDP2_VRAM_ADDR(2, index+512)) = tga_copy[18+256*3+index+512];
                            *((uint8_t *)VDP2_VRAM_ADDR(2, index+513)) = tga_copy[18+256*3+index+513];
                        } else if (drawarea[yy*40+xx]==2) {
                            int index = (fuse_y+yy*2)*512+fuse_x+xx*2;
                            *((uint8_t *)VDP2_VRAM_ADDR(0, index)) = 0;
                            *((uint8_t *)VDP2_VRAM_ADDR(0, index+1)) = tga_copy[18+256*3+index+1];
                            *((uint8_t *)VDP2_VRAM_ADDR(0, index+512)) = tga_copy[18+256*3+index+512];
                            *((uint8_t *)VDP2_VRAM_ADDR(0, index+513)) = 0;
                            *((uint8_t *)VDP2_VRAM_ADDR(2, index)) = 0;
                            *((uint8_t *)VDP2_VRAM_ADDR(2, index+1)) = tga_copy[18+256*3+index+1];
                            *((uint8_t *)VDP2_VRAM_ADDR(2, index+512)) = tga_copy[18+256*3+index+512];
                            *((uint8_t *)VDP2_VRAM_ADDR(2, index+513)) = 0;
                        } 
}

void battle_init(uint8_t * tga, uint8_t * tga_half)
{
    uint8_t drawarea[40*30];
    tga_copy = tga;
    srand(100500);//vdp2_tvmd_vcount_get());//(((uint32_t)vdp2_tvmd_hcount_get())<<16) | (vdp2_tvmd_vcount_get()));
    vdp1_vram_partitions_get(&battle_vdp1_vram_partitions);

    //loading sound effects into sound ram, reserving space for 64 sounds 
    int sound_ram_offset = 0x2000 + 64*sizeof(uint32_t);
    //0 = fuse
    adpcm_add_sample(sound_ram_offset,SOUND_EFFECT_FUSE,e_fuse_adp,e_fuse_adp_end-e_fuse_adp);
    sound_ram_offset+=e_fuse_adp_end-e_fuse_adp;
    //1 - fuse multiple
    adpcm_add_sample(sound_ram_offset,SOUND_EFFECT_FUSE_MULTIPLE,e_fuse_m_adp,e_fuse_m_adp_end-e_fuse_m_adp);
    sound_ram_offset+=e_fuse_m_adp_end-e_fuse_m_adp;
    //2 - grab
    adpcm_add_sample(sound_ram_offset,SOUND_EFFECT_GRAB,e_grab_adp,e_grab_adp_end-e_grab_adp);
    sound_ram_offset+=e_grab_adp_end-e_grab_adp;
    //3 - grab failed
    adpcm_add_sample(sound_ram_offset,SOUND_EFFECT_GRAB_FAIL,e_grab_f_adp,e_grab_f_adp_end-e_grab_f_adp);
    sound_ram_offset+=e_grab_f_adp_end-e_grab_f_adp;
    //4 = link
    adpcm_add_sample(sound_ram_offset,SOUND_EFFECT_LINK,e_link_adp,e_link_adp_end-e_link_adp);
    sound_ram_offset+=e_link_adp_end-e_link_adp;
    //5 - release
    adpcm_add_sample(sound_ram_offset,SOUND_EFFECT_RELEASE,e_release_adp,e_release_adp_end-e_release_adp);
    sound_ram_offset+=e_release_adp_end-e_release_adp;

    //step 1 - DO NOT load tga into VDP2 megaplane, FILL WITH RANDOM SHIT!
    //memcpy ((void *)VDP2_VRAM_ADDR(0, 0x00000), &(tga[18+256*3]),512*256);
	//memcpy ((void *)VDP2_VRAM_ADDR(2, 0x00000), &(tga[18+256*3]),512*256);
	//memcpy ((void *)VDP2_VRAM_ADDR(1, 0x00000), &(tga[18+256*3+0x20000]),512*256);
	//memcpy ((void *)VDP2_VRAM_ADDR(3, 0x00000), &(tga[18+256*3+0x20000]),512*256);
    for (int i=0;i<512*512;i++) {
        *((uint8_t *)VDP2_VRAM_ADDR(0, i)) = rand();
        *((uint8_t *)VDP2_VRAM_ADDR(2, i)) = rand();
    }

    //load only 8 pixels to the right
    for (int y=0;y<512;y++) {
        for (int x=504;x<512;x++) {
            *((uint8_t *)VDP2_VRAM_ADDR(0, y*512+x)) = (tga[18+256*3+y*512+x]);
            *((uint8_t *)VDP2_VRAM_ADDR(2, y*512+x)) = (tga[18+256*3+y*512+x]);
        }
    }


	rgb888_t _color  = {0,0,0,0};
	for (int i = 0; i<256; i++)
    {
        _color.r = tga[18+i*3+2];
        _color.g = tga[18+i*3+1];
        _color.b = tga[18+i*3+0];
        video_vdp2_set_palette_part(0, &_color, i, i);
    }
    //replacing pink 0 color from TGA with black
    _color.r =0; _color.g =0; _color.b =0;
    video_vdp2_set_palette_part(0, &_color, 0, 0);

    //step 2 - generate random curve directions
    for (int y=0;y<GRID_SIZE_Y; y++) {
        for (int x=0;x<GRID_SIZE_X; x++) {
            Curves_X[y][x] = rand() & 0x01;
            Curves_Y[y][x] = rand() & 0x01;
        }
    }

    //step 3 - using downsampled same-palette image to fill VDP1 sprites
    int sprite = 0;
    for (int y=0;y<(GRID_SIZE_Y); y++) {
        for (int x=0;x<(GRID_SIZE_X); x++) {

            //creating mask image first
            generate_piece_mask(drawarea,x,y);

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
            pieces_game_to_vdp1[sprite] = sprite;
            pieces_vdp1_to_game[sprite] = sprite;
            sprite++;
        }
    }

    //step 4 - assign random coords for every piece
    int piece = 0;
    for (int y=0;y<(GRID_SIZE_Y); y++) {
        for (int x=0;x<(GRID_SIZE_X); x++) {
            pieces_x[piece] = ((unsigned short)rand()) % 300 - 10;
            pieces_y[piece] = ((unsigned short)rand()) % 200 + 10;
            //pieces_x[piece] = x*28+60;//((unsigned short)rand()) % 300 - 10;
            //pieces_y[piece] = y*20+2;// ((unsigned short)rand()) % 200 + 10;
            _cmdt_list->cmdts[pieces_game_to_vdp1[piece]+10].cmd_xa = pieces_x[piece];
            _cmdt_list->cmdts[pieces_game_to_vdp1[piece]+10].cmd_ya = pieces_y[piece];
            piece++;
        }
    }

    //cursor stuff 
    battle_cursor_x = 128;
    battle_cursor_y = 120;
    memcpy ((void *)(battle_vdp1_vram_partitions.texture_base+0x2a000), &(asset_cursor1_tga[18+3*3]),32*32);
    _cmdt_list->cmdts[200].cmd_xa = battle_cursor_x;
    _cmdt_list->cmdts[200].cmd_ya = battle_cursor_y;

    //adding cursor colors to palette 3
	for (int i = 0; i<3; i++)
    {
        _color.r = asset_cursor1_tga[18+i*3+2];
        _color.g = asset_cursor1_tga[18+i*3+1];
        _color.b = asset_cursor1_tga[18+i*3+0];
        video_vdp2_set_palette_part(3, &_color, i, i);
    }

    //adding font colors to palette 3 at offset +16
	for (int i = 0; i<16; i++)
    {
        _color.r = i*16;
        _color.g = i*16;
        _color.b = i*16;
        video_vdp2_set_palette_part(3, &_color, 16+i, 16+i);
    }

    //rendering test text on side bodred
    char draw_string[32];
    mr_t mr;
    mr.display_width = 32;
	mr.display_height = 20;
    mr_point_t offset;
    mr.draw_rectangle_callback = mr_draw_rectangle_framebuffer_color;
    mr.draw_string_callback = mr_draw_string_framebuffer_color;
    mr.display = 0;
    mr.buffer = (uint8_t *)LWRAM(0);

    strcpy (draw_string,"Test");
    mr_rectangle_t rectangle = (mr_rectangle_t){0,0,32,20};
   	mcu_renderer_draw_text(mr, draw_string, font_pacifico_16,rectangle,mr_get_color(0x00007F),mr_get_color(0x000000));

    for (int i = 0; i<32*20; i++)
        *(uint8_t*)(battle_vdp1_vram_partitions.texture_base+i) = 16+*(uint8_t*)LWRAM(i*2+1);

    //update vdp1
    vdp1_sync_cmdt_list_put(_cmdt_list, 0);

    //unlink all pieces
    for (int i = 0; i<120; i++)
        pieces_link_array[i] = 0;

    pieces_link_groups = 0;//starting with no link groups
}


void battle_scheduler(smpc_peripheral_digital_t * controller)
{
    uint8_t drawarea[40*30];
                //rendering debug info
            char draw_string[32];
            mr_t mr;
            mr.display_width = 32;
            mr.display_height = 20;
            mr_point_t offset;
            mr.draw_rectangle_callback = mr_draw_rectangle_framebuffer_color;
            mr.draw_string_callback = mr_draw_string_framebuffer_color;
            mr.display = 0;
            mr.buffer = (uint8_t *)LWRAM(0);
    mr_rectangle_t rectangle;


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

    if (-1 == selected_piece) {
        //show cursor
        _cmdt_list->cmdts[200].cmd_xa = battle_cursor_x;
        _cmdt_list->cmdts[200].cmd_ya = battle_cursor_y;
    } else {
        //centering tile on cursor hot point
        pieces_x[selected_piece] = battle_cursor_x + 12 - 20;
        pieces_y[selected_piece] = battle_cursor_y + 2 - 15;

        //update tile coord
        _cmdt_list->cmdts[pieces_game_to_vdp1[selected_piece]+10].cmd_xa = pieces_x[selected_piece];
        _cmdt_list->cmdts[pieces_game_to_vdp1[selected_piece]+10].cmd_ya = pieces_y[selected_piece];
        //if linked, move entire array
        if (pieces_link_array[selected_piece]){
            for (int i=0;i<120;i++)
                if (pieces_link_array[i] == pieces_link_array[selected_piece]) {
                    int diff_x = (i%10 - selected_piece%10)*25;
                    int diff_y = (i/10 - selected_piece/10)*20;
                    //shifting pieces
                    pieces_x[i] = pieces_x[selected_piece] + diff_x;
                    pieces_y[i] = pieces_y[selected_piece] + diff_y;
                    //update tile coord
                    _cmdt_list->cmdts[pieces_game_to_vdp1[i]+10].cmd_xa = pieces_x[i];
                    _cmdt_list->cmdts[pieces_game_to_vdp1[i]+10].cmd_ya = pieces_y[i];
                }
        }
        //hide cursor
        _cmdt_list->cmdts[200].cmd_xa = -100;
        _cmdt_list->cmdts[200].cmd_ya = -100;
    }
    
    //cursor grab button
    if ( (controller->pressed.button.a) || (controller->pressed.button.c) ) {
        if (0 == battle_cursor_history[5]) {
            //pressing grab button
            //searching for nearby tile
            selected_piece = -1;
            //cursor hot point is {12;2}
            int hot_x = battle_cursor_x+12;
            int hot_y = battle_cursor_y+2;
            //looking for last (top) tile that hits
            for (int tile=0;tile<120;tile++)
            {
                if ( (hot_x >= pieces_x[tile]+7) && (hot_x <= pieces_x[tile]+33) && (hot_y >= pieces_y[tile]+5) && (hot_y <= pieces_y[tile]+24) ){
                    if ((selected_piece == -1) || (pieces_game_to_vdp1[tile] > pieces_game_to_vdp1[selected_piece]))
                        selected_piece = tile;
                }
            }
            if (-1 == selected_piece) {
                //no tile found, changing cursor to no go
                adcpm_play_sample(2,SOUND_EFFECT_GRAB_FAIL);
                memcpy ((void *)(battle_vdp1_vram_partitions.texture_base+0x2a000), &(asset_cursor2_tga[18+3*3]),32*32);
            } else {
                // grab sucessful, play effect on channel 2
                adcpm_play_sample(2,SOUND_EFFECT_GRAB);
                //piece found, hiding cursor
                _cmdt_list->cmdts[200].cmd_xa = -100;
                _cmdt_list->cmdts[200].cmd_ya = -100;
                //sorting the list to put the found tile on top
                //shift entire list, putting selected_piece at the end. since only xa,ya and addr differ, we only update them
                short backup_xa = _cmdt_list->cmdts[pieces_game_to_vdp1[selected_piece]+10].cmd_xa;
                short backup_ya = _cmdt_list->cmdts[pieces_game_to_vdp1[selected_piece]+10].cmd_ya;
                short backup_srca = _cmdt_list->cmdts[pieces_game_to_vdp1[selected_piece]+10].cmd_srca;
                int starting_vdp1 = pieces_game_to_vdp1[selected_piece];
                *((uint8_t*)LWRAM(0xFF0)) = selected_piece;
                *((uint8_t*)LWRAM(0xFF1)) = starting_vdp1;
                for (int i = starting_vdp1; i<129; i++){
                    //shift next command to current command to the
                    _cmdt_list->cmdts[i+10].cmd_xa = _cmdt_list->cmdts[i+11].cmd_xa;
                    _cmdt_list->cmdts[i+10].cmd_ya = _cmdt_list->cmdts[i+11].cmd_ya;
                    _cmdt_list->cmdts[i+10].cmd_srca = _cmdt_list->cmdts[i+11].cmd_srca;
                }
                //update pieces_game_to_vdp1
                for (int i = 119; i>=starting_vdp1; i--){
                    pieces_game_to_vdp1[pieces_vdp1_to_game[i]] = pieces_game_to_vdp1[pieces_vdp1_to_game[i-1]];
                }

                //update pieces_vdp1_to_game
                for (int i = starting_vdp1; i<119; i++){
                    pieces_vdp1_to_game[i] = pieces_vdp1_to_game[i+1];
                }
                
                //updating the moved entry
                _cmdt_list->cmdts[129].cmd_xa = backup_xa;
                _cmdt_list->cmdts[129].cmd_ya = backup_ya;
                _cmdt_list->cmdts[129].cmd_srca = backup_srca;
                pieces_game_to_vdp1[selected_piece] = 119;
                pieces_vdp1_to_game[119] = selected_piece;
            }
        }
        battle_cursor_history[5]++;
    } else {
        if (battle_cursor_history[5]) {
            //releasing grab button
            if (selected_piece != -1) //only if piece was grabbed
            {
                //checking if sprite is placed correctly
                int piece_x = (selected_piece%10);
                int piece_y = (selected_piece/10);

                int expected_x = piece_x*25+24;
                int expected_y = piece_y*20-5;
                
                sprintf(draw_string,"%d",expected_x);
                mr_rectangle_t rectangle = (mr_rectangle_t){0,0,32,20};
                mcu_renderer_draw_text(mr, draw_string, font_pacifico_16,rectangle,mr_get_color(0x00007F),mr_get_color(0x000000));
                for (int i = 0; i<32*20; i++)
                    *(uint8_t*)(battle_vdp1_vram_partitions.texture_base+32*20+i) = 16+*(uint8_t*)LWRAM(i*2+1);

                sprintf(draw_string,"%d",expected_y);
                rectangle = (mr_rectangle_t){0,0,32,20};
                mcu_renderer_draw_text(mr, draw_string, font_pacifico_16,rectangle,mr_get_color(0x00007F),mr_get_color(0x000000));
                for (int i = 0; i<32*20; i++)
                    *(uint8_t*)(battle_vdp1_vram_partitions.texture_base+32*40+i) = 16+*(uint8_t*)LWRAM(i*2+1);

                sprintf(draw_string,"%d",pieces_x[selected_piece]);
                rectangle = (mr_rectangle_t){0,0,32,20};
                mcu_renderer_draw_text(mr, draw_string, font_pacifico_16,rectangle,mr_get_color(0x00007F),mr_get_color(0x000000));
                for (int i = 0; i<32*20; i++)
                    *(uint8_t*)(battle_vdp1_vram_partitions.texture_base+32*60+i) = 16+*(uint8_t*)LWRAM(i*2+1);

                sprintf(draw_string,"%d",pieces_y[selected_piece]);
                rectangle = (mr_rectangle_t){0,0,32,20};
                mcu_renderer_draw_text(mr, draw_string, font_pacifico_16,rectangle,mr_get_color(0x00007F),mr_get_color(0x000000));
                for (int i = 0; i<32*20; i++)
                    *(uint8_t*)(battle_vdp1_vram_partitions.texture_base+32*80+i) = 16+*(uint8_t*)LWRAM(i*2+1);


                int normal_release = 1;
                
                //checking for fuse, using mahnattan distance
                int abs_x = (expected_x > pieces_x[selected_piece]) ? (expected_x - pieces_x[selected_piece]) : (pieces_x[selected_piece] - expected_x);
                int abs_y = (expected_y > pieces_y[selected_piece]) ? (expected_y - pieces_y[selected_piece]) : (pieces_y[selected_piece] - expected_y);
                if ( (abs_x < 5) && (abs_y < 5) ){
                    normal_release = 0;
                    //if linked, fuse entire list
                    if (pieces_link_array[selected_piece]){
                        adcpm_play_sample(2,SOUND_EFFECT_FUSE_MULTIPLE);
                        for (int i=0;i<120;i++)
                            if (pieces_link_array[i] == pieces_link_array[selected_piece]) {
                                fuse_piece(i);
                            }
                    } else {
                        //fuse single tile
                        adcpm_play_sample(2,SOUND_EFFECT_FUSE);
                        fuse_piece(selected_piece);
                    }
                }
                else
                {
                    //fuse failed, checking for link, only neigbours, starting with top neighbour
                    if (selected_piece>10) 
                        if (link_neigbour(selected_piece,selected_piece-10,0,-20))
                            normal_release = 0;
                    //now checking bottom neighbour
                    if (selected_piece<110) 
                        if (link_neigbour(selected_piece,selected_piece+10,0,20))
                            normal_release = 0;
                    //left neighbour
                    if (selected_piece%10 > 0) 
                        if (link_neigbour(selected_piece,selected_piece-1,-25,0))
                            normal_release = 0;
                    //right neighbour
                    if (selected_piece%10 < 9) 
                        if (link_neigbour(selected_piece,selected_piece+1,25,0))
                            normal_release = 0;
                    //if we're in the link already, checking all our neigbougs too
                    if (pieces_link_array[selected_piece]) {
                        for (int i=0;i<120;i++){
                            if ( (selected_piece != i) && (pieces_link_array[selected_piece] ==  pieces_link_array[i]) ) {
                                if (i>10) 
                                    if (link_neigbour(i,i-10,0,-20))
                                        normal_release = 0;
                                //now checking bottom neighbour
                                if (i<110) 
                                    if (link_neigbour(i,i+10,0,20))
                                        normal_release = 0;
                                //left neighbour
                                if (i%10 > 0) 
                                    if (link_neigbour(i,i-1,-25,0))
                                        normal_release = 0;
                                //right neighbour
                                if (i%10 < 9) 
                                    if (link_neigbour(i,i+1,25,0))
                                        normal_release = 0;
                                    }
                        }
                    }
                } 
                //normal release
                if (normal_release)
                    adcpm_play_sample(2,SOUND_EFFECT_RELEASE);
            } //only if piece was grabbed
            //releasing grab button
            selected_piece = -1;
            //restore cursor sprite in case it was different
            memcpy ((void *)(battle_vdp1_vram_partitions.texture_base+0x2a000), &(asset_cursor1_tga[18+3*3]),32*32);
        }
        battle_cursor_history[5] = 0;
    }

    //draw timer
    int secs = (global_frame_count*1001)/60000;
    if (VDP2_TVMD_TV_STANDARD_PAL == vdp2_tvmd_tv_standard_get())
        secs = global_frame_count/50;
    
    sprintf(draw_string,"%d:%02d",secs/60,secs%60);
    rectangle = (mr_rectangle_t){0,0,32,20};
    mcu_renderer_draw_text(mr, draw_string, font_pacifico_16,rectangle,mr_get_color(0x00007F),mr_get_color(0x000000));
    for (int i = 0; i<32*20; i++)
        *(uint8_t*)(battle_vdp1_vram_partitions.texture_base+0x2000+i) = 16+*(uint8_t*)LWRAM(i*2+1);

    //TODO: sounds for link, fuse, grab, release, can't grab

    //TODO :end battle message

    //TODO : 480i support

    memcpy(LWRAM(0x1000),pieces_game_to_vdp1,sizeof(pieces_game_to_vdp1));
    memcpy(LWRAM(0x1100),pieces_vdp1_to_game,sizeof(pieces_vdp1_to_game));
    memcpy(LWRAM(0x1200),pieces_link_array,sizeof(pieces_link_array));
    *((uint8_t*)LWRAM(0x11F0)) = pieces_link_groups;

    vdp1_sync_cmdt_list_put(_cmdt_list, 0);
}