#ifndef FONT_RENDER_H_
#define FONT_RENDER_H_

#include <stdint.h>
#include <ft2build.h>
#include FT_CACHE_H
#include FT_FREETYPE_H

#define CACHE_SIZE_NO_LIMIT 0
#define CACHE_SIZE_MINIMUM 1

#define FB_MAKE_ARGB8888(a, r, g, b)                               \
	(((uint32_t)((a)&0xFF) << 24) | ((uint32_t)((r)&0xFF) << 16) | \
	 ((uint32_t)((g)&0xFF) << 8) | ((uint32_t)(b)&0xFF))
#define FB_GET_ALPHA_ARGB8888(c) (((c) >> 24) & 0xFF)
#define FB_GET_RED_ARGB8888(c) (((c) >> 16) & 0xFF)
#define FB_GET_GREEN_ARGB8888(c) (((c) >> 8) & 0xFF)
#define FB_GET_BLUE_ARGB8888(c) ((c)&0xFF)
#define FB_SET_ALPHA_ARGB8888(c, a) \
	(((c) & ~((uint32_t)0xFF << 24)) | ((uint32_t)((a)&0xFF) << 24))
#define FB_MAKE_RGB565(r, g, b)              \
	(((uint16_t)(((r) >> 3) & 0x1F) << 11) | \
	 ((uint16_t)(((g) >> 2) & 0x3F) << 5) | ((uint16_t)((b) >> 3) & 0x1F))
#define FB_MAKE_COLOR(r, g, b) FB_MAKE_ARGB8888(0xFF, r, g, b)

enum render_align {
    ALIGN_LEFT,         
	ALIGN_CENTER,       
	ALIGN_RIGHT,        
	ALIGN_TOP_LEFT,     
	ALIGN_TOP_CENTER,   
	ALIGN_TOP_RIGHT,    
	ALIGN_MIDDLE_LEFT,  
	ALIGN_MIDDLE_CENTER,
	ALIGN_MIDDLE_RIGHT, 
	ALIGN_BOTTOM_LEFT,  
	ALIGN_BOTTOM_CENTER,
	ALIGN_BOTTOM_RIGHT, 
};

enum render_bg_fill {
   RENDER_BG_NONE,
   RENDER_BG_MINIMUM,
   RENDER_BG_BLOCK
};

struct cursor {
	int x;
	int y;
};

struct text_settings {
	float line_space_ratio;
    void *display;
	FT_UInt32 fg_color;
	FT_UInt32 bg_color;
	unsigned int size;
	struct cursor curs;
	enum render_align align;
	enum render_bg_fill bg_fill;
};

FT_Error font_render_load_font(const unsigned char *data, size_t size,
						   FT_Byte target_face_index);
void font_render_unload_font();
void font_render_text_line(const char *text);
struct text_settings *font_render_get_text_settings();
void font_render_set_text_settings(struct text_settings *draw);

#endif // FONT_RENDER_H_