#ifndef VGA_IMAGE_H
#define VGA_IMAGE_H

/* tga format header, from OSdev*/
#define X_RES 1024
#define Y_RES 768
#define PIXEL_BUF_MAXSIZE (X_RES * Y_RES)
#include "types.h"
#define STATUS_BAR_HEIGHT 20

#define WINDOW1_X  100
#define WINDOW1_Y  100
#define WINDOW2_X  200
#define WINDOW2_Y  200
#define WINDOW3_X  300
#define WINDOW3_Y  300

#define WINDOW_WIDTH 600
#define WINDOW_HEIGHT 400
#define WINDOW_TITLE_HEIGHT 20   // Terminal 1, Terminal 2, Terminal 3

#define TERM1_ICON_X 100
#define TERM2_ICON_X 300
#define TERM3_ICON_X 500
#define TERM1_ICON_Y 600
#define TERM2_ICON_Y 600
#define TERM3_ICON_Y 600
#define ICON_WIDTH 72
#define ICON_HEIGHT 72

#define TOP 0
#define MIDDLE 1
#define BOTTOM 2

extern volatile int screen_need_update;

typedef struct {
    uint8_t magic1;             // must be zero
    uint8_t colormap;           // must be zero
    uint8_t encoding;           // must be 2
    uint16_t cmaporig, cmaplen; // must be zero
    uint8_t cmapent;            // must be zero
    uint16_t x;                 // must be zero
    uint16_t y;                 // must be zero
    uint16_t w;                 // image's width
    uint16_t h;                 // image's height
    uint8_t bpp;                // 24
    uint8_t pixeltype;          // 0
} __attribute__((packed)) tga_header_t;

/* pixel (BPP = 32) */
typedef struct { 
    unsigned char b, g, r;
    unsigned char reserved;
} pixel_t;

/* pixel (BPP = 24) */
typedef struct { 
    unsigned char b, g, r;
} pixel_24_t;

// extern pixel_t *frame_copy;

typedef struct {
    short width, height;
    unsigned char left_to_right, top_to_bottom, bpp;
} icon_info_t;

typedef struct {
    int16_t window_x;
    int16_t window_y;
    int16_t window_buf_x_start;    // indexing mode: for i in [buf_x_start, buf_x_end)
    int16_t window_buf_x_end;
    int16_t window_buf_y_start;
    int16_t window_buf_y_end;

    // !only valid when layer = bottom
    int16_t window_buf2_x_start;    // indexing mode: for i in [buf_x_start, buf_x_end)
    int16_t window_buf2_x_end;
    int16_t window_buf2_y_start;
    int16_t window_buf2_y_end;

    uint8_t layer;
} windows_t;

extern windows_t windows[3];

int VGA_loadIcon(char *fname); // load into framebuffer according to 

void VGA_drawIcon(int scr_x, int scr_y);

extern pixel_t *frame_buffer;

extern void VGA_drawdesktop();

extern void VGA_drawRect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint8_t r, uint8_t g, uint8_t b, uint8_t on_screen);

extern void VGA_drawStatusbar();

extern void VGA_initWindows();

// draw three windows
extern void VGA_drawWindows(int i, uint8_t on_screen);

extern void VGA_initterms();

// show mouse fingers
extern void VGA_drawMouse(int scr_x, int scr_y);

// work on this after running 3 shells
extern void VGA_restoreWindow();

// work on this after running 3 shells
extern void VGA_saveScreen();

extern icon_info_t icon_info;

extern pixel_t *icon_buf;

extern pixel_t mouse_bg[9][16];

extern void VGA_saveMouseBG(int scr_x, int src_y);

extern void VGA_putmouseBG(int scr_x, int src_y);

extern int print_screen_or_buf(int window_id, int char_off_x, int char_off_y);

extern void switchpagingtobuffer(int tid);

extern void switchpagingtovideomem(int tid);

extern int check_pixel_in_middle_window(int window_idx, int pixel_x, int pixel_y);

extern int check_pixel_in_bottom_window(int window_idx, int pixel_x, int pixel_y);

extern void set_window_buf_boundary(int16_t *x_start, int16_t *x_end, int16_t *y_start, int16_t *y_end, int window_idx, int type);

extern int get_top_window_id();

extern int get_middle_window_id();

extern int get_bottom_window_id();

extern int check_pixel_in_screen(int pixel_x, int pixel_y);
#endif
