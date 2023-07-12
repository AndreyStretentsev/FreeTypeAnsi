#include <string.h>
#include <stdbool.h>
#include <font_render.h>
#include <ansiGraphic2.1.h>
#include <freetype/ftlogging.h>

#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

typedef struct {
	const unsigned char *data; 
	size_t data_size; 
	uint8_t face_index;  
} face_rec_t, *face_t;

struct cache_parameter {
	unsigned int max_faces;
	unsigned int max_sizes;
	unsigned long max_bytes;
};

struct render_flags {
	bool enable_optimized_drawing;
	bool support_vertical;
};

struct saved_state_vars {
	struct cursor drawn_bg_point;
	uint32_t prev_max_font_height;
	unsigned int prev_font_size;
};

static FT_Library g_library;
static FTC_Manager g_ftc_manager = NULL;
static FTC_CMapCache g_ftc_cmap_cache = NULL;
static FTC_ImageCache g_ftc_image_cache = NULL;
static FT_Face g_face = NULL;
static face_rec_t g_face_id = {
	.data = NULL,
	.data_size = 0,
	.face_index = 0
};
static struct cache_parameter g_cache = {
	.max_faces = 1,
	.max_bytes = 2048,
	.max_sizes = 2
};
static bool g_need_init = true;

struct text_settings g_text = {
	.line_space_ratio = 1.0f,
	.size = 44,
	.fg_color = 0x00FFFFFF,
	.bg_color = 0x0,
	.align = ALIGN_LEFT,
	.bg_fill = RENDER_BG_NONE
};
static struct render_flags g_flags = {
	.enable_optimized_drawing = true
};
struct saved_state_vars g_saved_state = {
	.drawn_bg_point.x = 0,
	.drawn_bg_point.y = 0,
	.prev_font_size = 0,
	.prev_max_font_height = 0
};

static uint32_t decode_utf8(const char *str, int *index, int remaining) {
	uint32_t c = str[(*index)++];

	if ((c & 0x80) == 0x00)
		return c;

	if (((c & 0xE0) == 0xC0) && (remaining > 1))
		return ((c & 0x1F) << 6) | (str[(*index)++] & 0x3F);

	if (((c & 0xF0) == 0xE0) && (remaining > 2)) {
		c = ((c & 0x0F) << 12) | ((str[(*index)++] & 0x3F) << 6);
		return c | ((str[(*index)++] & 0x3F));
	}

	return c;
}

static int utf8_strlen(const char *str, int len) {
	int wlen, ind;
	wlen = 0;
	ind = 0;
	while(ind < len) {
		wchar_t c = str[ind++];
		if ((c & 0x80) == 0) {
			wlen++;
			continue;
		}

		if (((c & 0xE0) == 0xC0) && (ind > 1)) {
			wlen++;
			ind++;
			continue;
		}

		if (((c & 0xF0) == 0xE0) && (ind > 2)) {
			wlen++;
			ind += 2;
			continue;
		}
	}
	return wlen;
}

static void str_to_unicode(const char *in, uint32_t *out, int in_len) {
	int n, out_ind;
	n = 0;
	out_ind = 0;
	while(n < in_len) {
		out[out_ind++] = decode_utf8(in, &n, in_len - n);
	}
}

static uint32_t alpha_blend(uint8_t a, uint32_t fg_color, uint32_t bg_color) {
	uint32_t r, g, b;
	uint16_t fg_r, fg_g, fg_b;
	uint16_t bg_r, bg_g, bg_b;
	fg_r = FB_GET_RED_ARGB8888(fg_color);
	fg_g = FB_GET_GREEN_ARGB8888(fg_color);
	fg_b = FB_GET_BLUE_ARGB8888(fg_color);
	bg_r = FB_GET_RED_ARGB8888(bg_color);
	bg_g = FB_GET_GREEN_ARGB8888(bg_color);
	bg_b = FB_GET_BLUE_ARGB8888(bg_color);
	r = (((fg_r * a) + (bg_r * (255 - a))) >> 8);
	g = (((fg_g * a) + (bg_g * (255 - a))) >> 8);
	b = (((fg_b * a) + (bg_b * (255 - a))) >> 8);
	return FB_MAKE_COLOR(r, g, b);
}

static void draw_pixel_console(uint32_t x, uint32_t y, uint32_t color) {
    ansigraphic_ivector2_t xy;
    xy.x = x;
    xy.y = y;
	ansigraphic_pixelSetColor_RGB(g_text.display, &xy, &color, &color);
}

static void draw_hline_console(uint32_t x, uint32_t y, uint32_t w, uint32_t color) {
	ansigraphic_ivector2_t xy;
    xy.x = x;
    xy.y = y;
    for (xy.x; xy.x < xy.x + w; xy.x++) {
        ansigraphic_pixelSetColor_RGB(g_text.display, &xy, &color, &color);
    }
}

static void draw_pixel(uint32_t x, uint32_t y, uint32_t color) {

}

static void draw_hline(uint32_t x, uint32_t y, uint32_t w, uint32_t color) {

}

static void draw_glyph_opt(FT_BitmapGlyph glyph, uint32_t x, uint32_t y, 
						   uint32_t fg, uint32_t bg) 
{
	uint32_t fxs = x; 
	uint32_t fl = 0;
	uint32_t bxs = x; 
	uint32_t bl = 0; 
	uint8_t alpha;
	int32_t _y;
	int32_t _x;
	uint32_t color;

	printf("draw opt\n");
    printf("height = %d\n", glyph->bitmap.rows);
    printf("width = %d\n", glyph->bitmap.width);

	for (_y = 0; _y < glyph->bitmap.rows; ++_y) {
        printf("y = %d\n", _y);
		for (_x = 0; _x < glyph->bitmap.width; ++_x) {
            printf("x = %d\n", _x);
			alpha = glyph->bitmap.buffer[_y * glyph->bitmap.pitch + _x];
			if (alpha) {
				if (bl && (FB_GET_ALPHA_ARGB8888(bg) != 0xFF)) {
					draw_hline(bxs, _y + y - glyph->top, bl, bg);
					bl = 0;
				}
				if (alpha != 0xFF) {
					if (fl) {
						if (fl == 1) {
							draw_pixel(fxs, _y + y - glyph->top, fg);
						} else {
							draw_hline(fxs, _y + y - glyph->top, fl, fg);
						}
						fl = 0;
					}
					if (FB_GET_ALPHA_ARGB8888(bg) == 0xFF) {
						color = 0;
						draw_pixel(_x + x + glyph->left, 
							   	   _y + y - glyph->top, 
							   	   alpha_blend(alpha, fg, color));
					}
					draw_pixel(_x + x + glyph->left, 
							   _y + y - glyph->top, 
							   alpha_blend(alpha, fg, bg));
				} else {
					if (fl == 0) {
						fxs = _x + x + glyph->left;
					}
					fl++;
				}
				continue;
			} 
			if (fl) {
				draw_hline(fxs, _y + y - glyph->top, fl, fg);
				fl = 0;
			}
			if (g_text.bg_fill == RENDER_BG_MINIMUM) {
				if (g_saved_state.drawn_bg_point.x <= (x + _x)) {
					if (bl == 0) {
						bxs = _x + x + glyph->left;
					}
					bl++;
				}
			}
		}

		if (fl) {
			draw_hline(fxs, _y + y - glyph->top, fl, fg);
			fl = 0;
		} else if (bl) {
			draw_hline(bxs, _y + y - glyph->top, bl, bg);
			bl = 0;
		}
	}
}

static void draw_glyph(FT_BitmapGlyph glyph, uint32_t x, uint32_t y, 
					   uint32_t fg, uint32_t bg) {
	int32_t _y;
	int32_t _x;
	uint8_t alpha;
	if (g_flags.enable_optimized_drawing) {
		draw_glyph_opt(glyph, x, y, fg, bg);
		return;
	} 
	for (_y = 0; _y < glyph->bitmap.rows; ++_y) {
		for (_x = 0; _x < glyph->bitmap.width; ++_x) {
			alpha = glyph->bitmap.buffer[_y * glyph->bitmap.pitch + _x];
			if (alpha) {
				draw_pixel(_x + x + glyph->left, _y + y - glyph->top, 
						   alpha_blend(alpha, fg, bg));
			} else if (g_text.bg_fill == RENDER_BG_MINIMUM) {
				if (g_saved_state.drawn_bg_point.x <= (x + _x)) {
					draw_pixel(_x + x + glyph->left, _y + y - glyph->top, bg);
				}
			}
		}
	}
}

static uint32_t get_font_max_height() {
	FT_Error error;
	FT_Face face;
	FTC_ScalerRec scaler;
	FT_Size asize;
	asize = NULL;

	if (g_saved_state.prev_font_size == g_text.size) {
		return g_saved_state.prev_max_font_height;
	}

	scaler.face_id = &g_face_id;
	scaler.width = 0;
	scaler.height = g_text.size;
	scaler.pixel = true;
	scaler.x_res = 0;
	scaler.y_res = 0;

	error = FTC_Manager_LookupSize(g_ftc_manager, &scaler, &asize);
	if (error) {
		return 0;
	}
	face = asize->face;

	int bbox_ymax = FT_MulFix(face->bbox.yMax, face->size->metrics.y_scale) >> 6;
	int bbox_ymin = FT_MulFix(face->bbox.yMin, face->size->metrics.y_scale) >> 6;

	g_saved_state.prev_max_font_height = (bbox_ymax - bbox_ymin);
	g_saved_state.prev_font_size = g_text.size;
	return g_saved_state.prev_max_font_height;
}

static FT_Error ftc_face_requester(FTC_FaceID face_id, FT_Library library, 
								   FT_Pointer request_data, FT_Face *aface) 
{
	FT_Error error = FT_Err_Ok;
	face_t face = (face_t)face_id;

	error = FT_New_Memory_Face(library, face->data, face->data_size, 0, aface);
	if (error) {
		printf("Font load Failed: 0x%02X\n", error);
	} else {
		printf("Font load Success!\n");
	}
	return error;
}

static FT_Error load_font() {
	FT_Face face;
	FT_Error error = FT_Err_Ok;

	if (g_need_init) {
		error = FT_Init_FreeType(&g_library);
		if (error) {
			printf("FT_Init_FreeType error: 0x%02X\n", error);
		}
		g_need_init = false;
	}

	error = FTC_Manager_New(g_library, 
							g_cache.max_faces, 
							g_cache.max_sizes, 
							g_cache.max_bytes, 
							ftc_face_requester,
							NULL,
							&g_ftc_manager);
	if (error) {
		printf("FTC_Manager_New error: 0x%02X\n", error);
		return error;
	}

	error = FTC_Manager_LookupFace(g_ftc_manager, &g_face_id, &face);
	if (error) {
		printf("FTC_Manager_LookupFace error: 0x%02X\n", error);
		return error;
	}

	error = FTC_CMapCache_New(g_ftc_manager, &g_ftc_cmap_cache);
	if (error) {
		printf("FTC_CMapCache_New error: 0x%02X\n", error);
		return error;
	}

	error = FTC_ImageCache_New(g_ftc_manager, &g_ftc_image_cache);
	if (error) {
		printf("FTC_ImageCache_New error: 0x%02X\n", error);
		return error;
	}

	if (FT_HAS_VERTICAL(face) == 0) {
		g_flags.support_vertical = false;
	} else {
		g_flags.support_vertical = true;
	}

	return error;
}

FT_Error font_render_load_font(const unsigned char *data, size_t size,
						   uint8_t target_face_index) 
{
	g_face_id.data = data;
	g_face_id.data_size = size;
	g_face_id.face_index = target_face_index;
	return load_font();
}

void font_render_unload_font() {
	if (!g_need_init) {
		FTC_Manager_RemoveFaceID(g_ftc_manager, &g_face_id);
		FTC_Manager_Reset(g_ftc_manager);
		FTC_Manager_Done(g_ftc_manager);
		FT_Done_FreeType(g_library);
	}
	g_need_init = true;
}

void font_render_text_line(const char *text)
{
	struct cursor init_pos;
	struct cursor cur_pos;
	FT_Face aface;
	FT_Pos ascender;
	FT_BBox abbox;
	FT_BBox bbox;
	FT_Glyph aglyph;
	FT_UInt glyph_index;
	FT_BBox glyph_bbox;
	FT_Int cmap_index;
	FT_Size asize;
	FT_Vector offset;
	FT_Vector bearing_left;
	FT_Vector pos;
	FT_Error error;
	FTC_ScalerRec scaler;
	FTC_ImageTypeRec image_type;
	FT_BitmapGlyph bit;
	uint32_t *unicode;
	uint32_t *to_render;
	bool ctrl_char_detected;
	bool is_line_started;
	int wc_cnt;
	int rc_cnt;
	int len;
	int wlen;
	int ind;
	FTC_Node node;

	memcpy(&init_pos, &g_text.curs, sizeof(init_pos));

	image_type.face_id = &g_face_id;
	image_type.width = g_text.size;
	image_type.height = g_text.size;

	abbox.xMax = abbox.yMax = LONG_MAX;
	abbox.xMin = abbox.yMin = LONG_MIN;

	len = strlen(text);
	wlen = utf8_strlen(text, len);
	unicode = malloc(sizeof(*unicode) * wlen);
	to_render = malloc(sizeof(*to_render) * wlen);
	str_to_unicode(text, unicode, len);

	asize = NULL;
	scaler.face_id = &g_face_id;
	scaler.width = 0;
	scaler.height = g_text.size;
	scaler.pixel = true;
	scaler.x_res = 0;
	scaler.y_res = 0;

	error = FTC_Manager_LookupSize(g_ftc_manager, &scaler, &asize);
	if (error) {
		printf("FTC_Manager_LookupSize returned 0x%02X\n", error);
		goto exit;
	}
	cmap_index = FT_Get_Charmap_Index(asize->face->charmap);
	ascender = asize->face->size->metrics.ascender;
	wc_cnt = 0;
	while (wc_cnt < wlen) {
		offset.x = 0;
		offset.y = 0;
		bearing_left.x = 0;
		bearing_left.y = 0;
		bbox.xMin = bbox.yMin = LONG_MAX;
		bbox.xMax = bbox.yMax = LONG_MIN;
		rc_cnt = 0;

		ctrl_char_detected = false;
		memcpy(&cur_pos, &g_text.curs, sizeof(cur_pos));
		image_type.flags = FT_LOAD_DEFAULT;
		is_line_started = true;

		while (wc_cnt < wlen && !ctrl_char_detected) {
			printf("get glyph %08X\n", unicode[wc_cnt]);
			switch (unicode[wc_cnt]) {
			case '\r':
			case '\n':
				ctrl_char_detected = true;
				break;
			default:
				glyph_index = FTC_CMapCache_Lookup(g_ftc_cmap_cache,
				                                   &g_face_id,
				                                   cmap_index,
				                                   unicode[wc_cnt]);
				printf("index %d\n", glyph_index);
				error = FTC_ImageCache_Lookup(g_ftc_image_cache, 
											  &image_type, 
											  glyph_index, &aglyph, NULL);
				if (error) {
					printf("FTC_SBitCache_Lookup error: 0x%02X\n", error);
					goto exit;
				}

                FT_Glyph_Get_CBox(aglyph, FT_GLYPH_BBOX_PIXELS, &glyph_bbox);

				if (is_line_started) {
					FTC_Manager_LookupFace(g_ftc_manager, &g_face_id, &aface);
					bearing_left.x = (aface->glyph->metrics.horiBearingX >> 6);
					is_line_started = false;
				}

				glyph_bbox.xMin += g_text.curs.x;
				glyph_bbox.xMax += g_text.curs.x;
				glyph_bbox.yMin += g_text.curs.y;
				glyph_bbox.yMax += g_text.curs.y;

				bbox.xMin = MIN(bbox.xMin, glyph_bbox.xMin);
				bbox.yMin = MIN(bbox.yMin, glyph_bbox.yMin);
				bbox.xMax = MAX(bbox.xMax, glyph_bbox.xMax);
				bbox.yMax = MAX(bbox.yMax, glyph_bbox.yMax);

				g_text.curs.x += (aglyph->advance.x >> 16);
				to_render[rc_cnt++] = unicode[wc_cnt];
			}
			wc_cnt++;
		}

		if (bbox.xMin > bbox.xMax) {
			bbox.xMin = bbox.yMin = 0;
			bbox.xMax = bbox.yMax = 0;
		} else {
			bbox.yMax = g_text.curs.y - (bbox.yMax - g_text.curs.y) 
						+ ((ascender) >> 6);
			bbox.yMin = g_text.curs.y + (g_text.curs.y - bbox.yMin) 
						+ ((ascender) >> 6);
			if (bbox.yMax < bbox.yMin) {
				bbox.yMax ^= bbox.yMin;
				bbox.yMin ^= bbox.yMax;
				bbox.yMax ^= bbox.yMin;
			}
			offset.x = bbox.xMin - cur_pos.x;
		}

		switch (g_text.align) {
		case ALIGN_LEFT:
		case ALIGN_TOP_LEFT:
			offset.x -= bearing_left.x;
			break;
		case ALIGN_MIDDLE_LEFT:
			offset.x -= bearing_left.x;
			offset.y += ((bbox.yMax - bbox.yMin) / 2);
			break;
		case ALIGN_BOTTOM_LEFT:
			offset.x -= bearing_left.x;
			offset.y += (bbox.yMax - bbox.yMin);
			break;
		case ALIGN_CENTER:
		case ALIGN_TOP_CENTER:
			offset.x += ((bbox.xMax - bbox.xMin) / 2);
			offset.x -= bearing_left.x;
			cur_pos.x -= (bearing_left.x / 2);
			break;
		case ALIGN_MIDDLE_CENTER:
			offset.x += ((bbox.xMax - bbox.xMin) / 2);
			offset.x -= bearing_left.x;
			cur_pos.x -= (bearing_left.x / 2);

			offset.y += ((bbox.yMax - bbox.yMin) / 2);
			break;
		case ALIGN_BOTTOM_CENTER:
			offset.x += ((bbox.xMax - bbox.xMin) / 2);
			offset.x -= bearing_left.x;
			cur_pos.x -= (bearing_left.x / 2);

			offset.y += (bbox.yMax - bbox.yMin);
			break;
		case ALIGN_RIGHT:
		case ALIGN_TOP_RIGHT:
			offset.x += (bbox.xMax - bbox.xMin);
			offset.x -= bearing_left.x;
			cur_pos.x -= bearing_left.x;
			break;
		case ALIGN_MIDDLE_RIGHT:
			offset.x += (bbox.xMax - bbox.xMin);
			offset.x -= bearing_left.x;
			cur_pos.x -= bearing_left.x;

			offset.y += ((bbox.yMax - bbox.yMin) / 2);
			break;
		case ALIGN_BOTTOM_RIGHT:
			offset.x += (bbox.xMax - bbox.xMin);
			offset.x -= bearing_left.x;
			cur_pos.x -= bearing_left.x;

			offset.y += (bbox.yMax - bbox.yMin);
			break;
		default:
			break;
		}

		bbox.xMin -= offset.x;
		bbox.xMax -= offset.x;
		bbox.yMin -= offset.y;
		bbox.yMax -= offset.y;

		if (g_text.bg_fill == RENDER_BG_BLOCK) {
			if (g_flags.enable_optimized_drawing) {
				for (int _y = bbox.yMin; _y <= bbox.yMax; _y++) {
					draw_hline(bbox.xMin, _y, bbox.xMax - bbox.xMin, 
							   g_text.bg_color);
				}
			} else {
				for (int _y = bbox.yMin; _y <= bbox.yMax; _y++) {
					for (int _x = bbox.xMin; _x <= bbox.xMax; _x++) {
						draw_pixel(_x, _y, g_text.bg_color);
					}
				}
			}
		}

		g_text.curs.x = cur_pos.x;
		g_text.curs.y = cur_pos.y;
		image_type.flags = FT_LOAD_RENDER;
		g_saved_state.drawn_bg_point.x = g_text.curs.x - offset.x;
        printf("rc_cnt = %d\n", rc_cnt);
		for (ind = 0; ind < rc_cnt; ind++) {
			printf("rend %08X\n", to_render[ind]);
			glyph_index = FTC_CMapCache_Lookup(g_ftc_cmap_cache,
											   &g_face_id,
											   cmap_index,
											   to_render[ind++]);
			printf("index %d\n", glyph_index);
			
			error = FTC_ImageCache_Lookup(g_ftc_image_cache, 
										  &image_type, 
										  glyph_index, &aglyph, NULL);
                                          
			if (error) {
				printf("FTC_ImageCache_Lookup error: 0x%02X\n", error);
				goto exit;
			}
			pos.x = g_text.curs.x - offset.x; 
			pos.y = g_text.curs.y - offset.y;
			pos.y += ((ascender) >> 6);

			bit = (FT_BitmapGlyph)aglyph;

			draw_glyph(bit, pos.x, pos.y, g_text.fg_color, g_text.bg_color);

			if (g_saved_state.drawn_bg_point.x < 
				(pos.x + (uint32_t)bit->bitmap.width)) {
				g_saved_state.drawn_bg_point.x = 
					pos.x + (uint32_t)bit->bitmap.width;
			}
			g_text.curs.x += (aglyph->advance.x >> 16);
		}

		if (ctrl_char_detected) {
			switch (unicode[wc_cnt]) {
			case '\r':
				g_text.curs.x = init_pos.x;
				break;
			case '\n':
				g_text.curs.x = init_pos.x;
				g_text.curs.y += 
					(int32_t)(get_font_max_height() * g_text.line_space_ratio);
				break;
			default:
				break;
			}
		}
		abbox.xMin = MIN(bbox.xMin, abbox.xMin);
		abbox.yMin = MIN(bbox.yMin, abbox.yMin);
		abbox.xMax = MAX(bbox.xMax, abbox.xMax);
		abbox.yMax = MAX(bbox.yMax, abbox.yMax);
	} 

exit:
	free(unicode);
	free(to_render);
}

struct text_settings *font_render_get_text_settings() {
	return &g_text;
}

void font_render_set_text_settings(struct text_settings *draw) {
	memcpy(&g_text, draw, sizeof(g_text));
}

