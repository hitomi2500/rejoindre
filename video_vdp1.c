#include <yaul.h>
#include <stdlib.h>
#include "video.h"
#include "video_vdp1.h"

vdp1_cmdt_list_t *_cmdt_list;

void video_vdp1_init(video_screen_mode_t screen_mode)
{
    int *_pointer32;
    int index;
    int base;

    //-------------- setup VDP1 -------------------
    //trimming to support only EDTV or normal 320x240 mode
    //setting up a small VDP1 list with 3 commands: sys clip, local coords, end
    //VDP1 list structure:
    // 0 : SysClip
    // 1 : Local Coord
    // 2-3 : borders (with timer?)
    // 4 : erase quad
    // 5-9 : reserved
    // 10-129 : tiles sprite, max 120, short-circuit unused with jumps
    // 130-199 : special effects for super moves etc, shortcircuited when unnecessary
    // 200 : cursor
    // 201 : character dialogue line, jump to 10
    // 202 : end
    _cmdt_list = vdp1_cmdt_list_alloc(VIDEO_VDP1_ORDER_LIMIT+1);

    static const int16_vec2_t local_coord_ul = INT16_VEC2_INITIALIZER(0,0);

    static const vdp1_cmdt_draw_mode_t sprite_draw_mode = {
        .raw = 0x0000,
        .pre_clipping_disable = true};

    vdp1_cmdt_color_bank_t pieces_color_bank;
    pieces_color_bank.raw = 0x0;
    vdp1_cmdt_color_bank_t message_color_bank;
    message_color_bank.raw = 0x200;
    vdp1_cmdt_color_bank_t system_color_bank;
    system_color_bank.raw = 0x300;

    assert(_cmdt_list != NULL);

    (void)memset(_cmdt_list->cmdts, 0x00, sizeof(vdp1_cmdt_t) * (VIDEO_VDP1_ORDER_LIMIT+1));

    vdp1_vram_partitions_set(256,//VDP1_VRAM_DEFAULT_CMDT_COUNT,
                              0x7C000, //  VDP1_VRAM_DEFAULT_TEXTURE_SIZE,
                               0,//  VDP1_VRAM_DEFAULT_GOURAUD_COUNT,
                               0);//  VDP1_VRAM_DEFAULT_CLUT_COUNT);

    vdp1_vram_partitions_t vdp1_vram_partitions;
    vdp1_vram_partitions_get(&vdp1_vram_partitions);

    _cmdt_list->count = VIDEO_VDP1_ORDER_LIMIT+1;
    
    //command 0 : clipping coordinates
    vdp1_cmdt_system_clip_coord_set(&_cmdt_list->cmdts[VIDEO_VDP1_ORDER_SYSTEM_CLIP_COORDS_INDEX]);
    vdp1_cmdt_t *cmdt_system_clip_coords;
    cmdt_system_clip_coords = &_cmdt_list->cmdts[VIDEO_VDP1_ORDER_SYSTEM_CLIP_COORDS_INDEX];
    cmdt_system_clip_coords->cmd_xc = 319;
    cmdt_system_clip_coords->cmd_yc = 239;
    //command 1 : 1 : Local Coord
    vdp1_cmdt_local_coord_set(&_cmdt_list->cmdts[VIDEO_VDP1_ORDER_LOCAL_COORDS_INDEX]);
    vdp1_cmdt_vtx_local_coord_set(&_cmdt_list->cmdts[VIDEO_VDP1_ORDER_LOCAL_COORDS_INDEX], local_coord_ul);

    //command 2 : left border, normal sprite : 32x240, storing at +0
    index = 2;
    base = vdp1_vram_partitions.texture_base;
    vdp1_cmdt_normal_sprite_set(&_cmdt_list->cmdts[index]);
    vdp1_cmdt_draw_mode_set(&_cmdt_list->cmdts[index], sprite_draw_mode);
    _cmdt_list->cmdts[index].cmd_draw_mode.trans_pixel_disable = 1;
    _cmdt_list->cmdts[index].cmd_draw_mode.end_code_disable = 1;
    vdp1_cmdt_color_mode4_set(&_cmdt_list->cmdts[index],system_color_bank);//8bpp
    _cmdt_list->cmdts[index].cmd_xa= 0;
    _cmdt_list->cmdts[index].cmd_ya= 0;
    vdp1_cmdt_char_base_set(&_cmdt_list->cmdts[index],base);
    _cmdt_list->cmdts[index].cmd_size=((32/8)<<8)|(240);
    //filling with black
    memset(base,23,32*240);

    //command 3 : right border, normal sprite : 32x240, storing at +0x2000
    index = 3;
    base = vdp1_vram_partitions.texture_base+0x2000;
    vdp1_cmdt_normal_sprite_set(&_cmdt_list->cmdts[index]);
    vdp1_cmdt_draw_mode_set(&_cmdt_list->cmdts[index], sprite_draw_mode);
    _cmdt_list->cmdts[index].cmd_draw_mode.trans_pixel_disable = 1;
    _cmdt_list->cmdts[index].cmd_draw_mode.end_code_disable = 1;
    vdp1_cmdt_color_mode4_set(&_cmdt_list->cmdts[index],system_color_bank);//8bpp
    _cmdt_list->cmdts[index].cmd_xa= 288;
    _cmdt_list->cmdts[index].cmd_ya= 0;
    vdp1_cmdt_char_base_set(&_cmdt_list->cmdts[index],base);
    _cmdt_list->cmdts[index].cmd_size=((32/8)<<8)|(240);
    //filling with black
    memset(base,55,32*240);

    //command 4 : erase quad, distorted sprite : 8x1 -> 256x120, storing at +0x3E00
    index = 4;
    base = vdp1_vram_partitions.texture_base+0x3E00;
    vdp1_cmdt_scaled_sprite_set(&_cmdt_list->cmdts[index]);
    vdp1_cmdt_zoom_set(&_cmdt_list->cmdts[index], VDP1_CMDT_ZOOM_POINT_NONE);
    vdp1_cmdt_draw_mode_set(&_cmdt_list->cmdts[index], sprite_draw_mode);
    _cmdt_list->cmdts[index].cmd_draw_mode.trans_pixel_disable = 1;
    _cmdt_list->cmdts[index].cmd_draw_mode.end_code_disable = 1;
    vdp1_cmdt_color_mode4_set(&_cmdt_list->cmdts[index],pieces_color_bank);//8bpp
    _cmdt_list->cmdts[index].cmd_xa= 32;
    _cmdt_list->cmdts[index].cmd_ya= 120;
    _cmdt_list->cmdts[index].cmd_xc= 32+255;
    _cmdt_list->cmdts[index].cmd_yc= 239;
    vdp1_cmdt_char_base_set(&_cmdt_list->cmdts[index],base);
    _cmdt_list->cmdts[index].cmd_size=((8/8)<<8)|(1);
    //filling with transparent
    memset(base,0,8);

    //jummping to 10 ,skipping currenty reserved 5-9
    vdp1_cmdt_link_type_set(&_cmdt_list->cmdts[index],VDP1_CMDT_LINK_TYPE_JUMP_ASSIGN);
    vdp1_cmdt_link_set(&_cmdt_list->cmdts[index],10);

    //commands 10 thru 129: tiles, each tile is 40x30 = 0x4B0 bytes, reserving 0x500, storing at +0x4000
    for (int index = 10; index<130; index++)
    {
        base = vdp1_vram_partitions.texture_base + 0x4000 + (index-10)*0x500;
        vdp1_cmdt_normal_sprite_set(&_cmdt_list->cmdts[index]);
        vdp1_cmdt_draw_mode_set(&_cmdt_list->cmdts[index], sprite_draw_mode);
        _cmdt_list->cmdts[index].cmd_draw_mode.trans_pixel_disable = 0;
        _cmdt_list->cmdts[index].cmd_draw_mode.end_code_disable = 1;
        vdp1_cmdt_color_mode4_set(&_cmdt_list->cmdts[index],pieces_color_bank);//8bpp
        _cmdt_list->cmdts[index].cmd_xa= ((index-10)%10)*25 - 5;
        _cmdt_list->cmdts[index].cmd_ya= ((index-10)/10)*19 - 5;
        vdp1_cmdt_char_base_set(&_cmdt_list->cmdts[index],base);
        _cmdt_list->cmdts[index].cmd_size=((40/8)<<8)|(30);
        //filling with transparent
        memset(base,0,40*30);
    }

    //jumping from 129 to 200 ,skipping special effect sprites for now
    vdp1_cmdt_link_type_set(&_cmdt_list->cmdts[129],VDP1_CMDT_LINK_TYPE_JUMP_ASSIGN);
    vdp1_cmdt_link_set(&_cmdt_list->cmdts[129],200);

    //command 200 : cursor, normal : 32x32, storing at +0x2a000
    index = 200;
    base = vdp1_vram_partitions.texture_base+0x2a000;
    vdp1_cmdt_normal_sprite_set(&_cmdt_list->cmdts[index]);
    vdp1_cmdt_draw_mode_set(&_cmdt_list->cmdts[index], sprite_draw_mode);
    _cmdt_list->cmdts[index].cmd_draw_mode.trans_pixel_disable = 0;
    _cmdt_list->cmdts[index].cmd_draw_mode.end_code_disable = 1;
    vdp1_cmdt_color_mode4_set(&_cmdt_list->cmdts[index],system_color_bank);
    _cmdt_list->cmdts[index].cmd_xa= 100;
    _cmdt_list->cmdts[index].cmd_ya= 100;
    vdp1_cmdt_char_base_set(&_cmdt_list->cmdts[index],base);
    _cmdt_list->cmdts[index].cmd_size=((32/8)<<8)|(32);
    //filling with transparent
    memset(base,0,32*32);
    
    //command 201 : character message : 256x32, storing at +0x2a400
    index = 201;
    base = vdp1_vram_partitions.texture_base+0x2a400;
    vdp1_cmdt_normal_sprite_set(&_cmdt_list->cmdts[index]);
    vdp1_cmdt_draw_mode_set(&_cmdt_list->cmdts[index], sprite_draw_mode);
    _cmdt_list->cmdts[index].cmd_draw_mode.trans_pixel_disable = 0;
    _cmdt_list->cmdts[index].cmd_draw_mode.end_code_disable = 1;
    vdp1_cmdt_color_mode4_set(&_cmdt_list->cmdts[index],message_color_bank);//8bpp
    _cmdt_list->cmdts[index].cmd_xa= (320-256)/2;
    _cmdt_list->cmdts[index].cmd_ya= 180;
    vdp1_cmdt_char_base_set(&_cmdt_list->cmdts[index],base);
    _cmdt_list->cmdts[index].cmd_size=((256/8)<<8)|(32);
    //filling with transparent
    memset(base,0,256*32);

    //end of commands list
    vdp1_cmdt_end_set(&_cmdt_list->cmdts[VIDEO_VDP1_ORDER_LIMIT]);

    vdp1_sync_cmdt_list_put(_cmdt_list, 0);

    static vdp1_env_t vdp1_env = {
                .erase_color = RGB1555(0, 0, 0, 0),
                .erase_points[0] = {
                        .x = 0,
                        .y = 0
                },
                .erase_points[1] = {
                        .x = 703,
                        .y = 511
                },
                .bpp = VDP1_ENV_BPP_16,
                .rotation = VDP1_ENV_ROTATION_0,
                .hdtv = VDP1_ENV_HDTV_OFF,
                .color_mode = VDP1_ENV_COLOR_MODE_RGB_PALETTE,
                .sprite_type = 0x0
    };

    if (VIDEO_SCANMODE_480P == screen_mode.scanmode)
        vdp1_env.hdtv = VDP1_ENV_HDTV_ON;
    else
        vdp1_env.hdtv = VDP1_ENV_HDTV_OFF;

    vdp1_env.erase_points[0].x = 32;
    vdp1_env.erase_points[0].y = 0;
    vdp1_env.erase_points[1].x = 288;//screen_mode.x_res_doubled ? 703 : 351;
    vdp1_env.erase_points[1].y = 240;//( (VIDEO_SCANMODE_480I == screen_mode.scanmode) || (VIDEO_SCANMODE_480P == screen_mode.scanmode) ) ? 511 : 255;

    vdp1_env_set(&vdp1_env);

    vdp1_sync();
    vdp1_sync_wait();
}

void video_vdp1_deinit()
{
    free(_cmdt_list);
}