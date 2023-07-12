#include <stdio.h>
#include <string.h>
#include <ansiGraphic2.1.h>
#include <font_render.h>
#include <ttf.h>

#define DISPLAY_WIDTH 200
#define DISPLAY_HEIGHT 60

static const char g_tets_str[] = "From shit & sticks";

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
	settings->align = ALIGN_MIDDLE_CENTER;
	settings->bg_fill = RENDER_BG_NONE;
	settings->bg_color = FB_MAKE_ARGB8888(0xFF, 0, 0, 0);
	settings->fg_color = FB_MAKE_COLOR(0xFF, 0xFF, 0xFF);
	settings->size = 18;
	settings->curs.x = DISPLAY_WIDTH / 2;
	settings->curs.y = DISPLAY_HEIGHT / 2;
    settings->display = console_create_display(DISPLAY_WIDTH, DISPLAY_HEIGHT);

	font_render_text_line(g_tets_str);
    printf("exit ok\n");
    ansigraphic_imagePrint_RGB(settings->display);
    console_delete_display(settings->display);
    return 0;
}