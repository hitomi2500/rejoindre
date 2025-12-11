ifeq ($(strip $(YAUL_INSTALL_ROOT)),)
  $(error Undefined YAUL_INSTALL_ROOT (install root directory))
endif

include $(YAUL_INSTALL_ROOT)/share/build.pre.mk

# Required for library usage
include $(YAUL_INSTALL_ROOT)/share/build.tga.mk

# Each asset follows the format: <path>;<symbol>. Duplicates are removed
BUILTIN_ASSETS+= \
	assets/adp68k.bin;asset_sound_driver \
	assets/BITMAP.TGA;asset_bitmap_tga \
	assets/BITMAP2.TGA;asset_bitmap2_tga \
	assets/e_fuse.adp;e_fuse_adp \
	assets/e_fuse_m.adp;e_fuse_m_adp \
	assets/e_grab.adp;e_grab_adp \
	assets/e_grab_f.adp;e_grab_f_adp \
	assets/e_link.adp;e_link_adp \
	assets/e_release.adp;e_release_adp \
	assets/cursor1.tga;asset_cursor1_tga \
	assets/cursor2.tga;asset_cursor2_tga

SH_PROGRAM:= rejoindre
SH_SRCS:= \
	main.c \
	adpcm.c \
	input.c \
	battle.c \
	font_renderer.c \
	video_vdp1.c \
	video_vdp2.c \
	video.c

SH_CFLAGS+= -Os -I. $(TGA_CFLAGS)
SH_LDFLAGS+= $(TGA_LDFLAGS)

IP_VERSION:= V1.000
IP_RELEASE_DATE:= 20160101
IP_AREAS:= JTUBKAEL
IP_PERIPHERALS:= JAMKST
IP_TITLE:= Re:Joindre
IP_MASTER_STACK_ADDR:= 0x06004000
IP_SLAVE_STACK_ADDR:= 0x06001E00
IP_1ST_READ_ADDR:= 0x06004000
IP_1ST_READ_SIZE:= 0

include $(YAUL_INSTALL_ROOT)/share/build.post.iso-cue.mk
