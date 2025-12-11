#include <yaul.h>
#include <video.h>

extern uint8_t asset_modesel_tga[];

void videomode_select(smpc_peripheral_digital_t *controller) {
    //starting with classic 240p mode
	video_screen_mode_t screenMode =
	{
		.scanmode = VIDEO_SCANMODE_240P,
		.x_res = VIDEO_X_RESOLUTION_320,
		.y_res = VDP2_TVMD_VERT_240,
		.x_res_doubled = false,
		.colorsystem = VDP2_TVMD_TV_STANDARD_NTSC,
	};
	video_init(screenMode,BITMAP_MODE_256_COLORS);
	video_vdp2_set_cycle_patterns_nbg_bmp(screenMode);

    for (int y=0;y<240;y++) {
        for (int x=0;x<320;x++) {
            *((uint8_t *)VDP2_VRAM_ADDR(0, y*512+x)) = asset_modesel_tga[18+2*3-15+y*320+x];
        }
    }

    rgb888_t _color  = {0,0,0,0};
	for (int i = 0; i<256; i++)
    {
        _color.r = asset_modesel_tga[18+i*3+2];
        _color.g = asset_modesel_tga[18+i*3+1];
        _color.b = asset_modesel_tga[18+i*3+0];
        video_vdp2_set_palette_part(0, &_color, i, i);
    }


    while(1)
    {
        smpc_peripheral_process();
		get_digital_keypress_anywhere(controller);
        if(controller->pressed.button.a) {
            //reinit to 480i
            screenMode.scanmode = VIDEO_SCANMODE_480I;
            screenMode.x_res = VIDEO_X_RESOLUTION_320;
            screenMode.y_res = VDP2_TVMD_VERT_240;
            screenMode.x_res_doubled = true;
            screenMode.colorsystem = VDP2_TVMD_TV_STANDARD_NTSC;
            
            video_init(screenMode,BITMAP_MODE_256_COLORS);
            video_vdp2_set_cycle_patterns_nbg_bmp(screenMode);

            return;
        } 
        if(controller->pressed.button.c) {
            //reinit to 480i
            screenMode.scanmode = VIDEO_SCANMODE_480P;
            screenMode.x_res = VIDEO_X_RESOLUTION_320;
            screenMode.y_res = VDP2_TVMD_VERT_240;
            screenMode.x_res_doubled = true;
            screenMode.colorsystem = VDP2_TVMD_TV_STANDARD_NTSC;
            
            video_init(screenMode,BITMAP_MODE_256_COLORS);
            video_vdp2_set_cycle_patterns_nbg_bmp(screenMode);

            return;
        } 

    }

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

}