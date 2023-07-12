#include <stdio.h>
#include <string.h>
#include <ansiGraphic2.1.h>
#include <font_render.h>
#include <ttf.h>

#define DISPLAY_WIDTH 1280
#define DISPLAY_HEIGHT 480

static const char g_tets_str[] = "0123456789---123---111-222-333";

static void *console_create_display(int width, int height) {
    ansigraphic_image_RGB_t *screen = 
	ansigraphic_newImage_RGB(width * 2, height);
	return screen;
}

static void console_delete_display(void *screen) {
    ansigraphic_deleteImage_RGB(screen);
}

int main() {

    font_render_load_font(test_ttf, sizeof(test_ttf), 0);
    struct text_settings *settings;
	printf("draw frame\n");
	settings = font_render_get_text_settings();
	settings->align = ALIGN_MIDDLE_LEFT;
	settings->bg_fill = RENDER_BG_NONE;
	settings->bg_color = FB_MAKE_ARGB8888(0xFF, 0, 0, 0);
	settings->fg_color = FB_MAKE_COLOR(0xFF, 0xFF, 0xFF);
	settings->size = 44;
	settings->curs.x = 0;
	settings->curs.y = 0;
    // settings->display = console_create_display(DISPLAY_WIDTH, DISPLAY_HEIGHT);

	font_render_text_line(g_tets_str);
    printf("exit ok\n");
    // ansigraphic_imagePrint_RGB(settings->display);
    // console_delete_display(settings->display);
    return 0;
}