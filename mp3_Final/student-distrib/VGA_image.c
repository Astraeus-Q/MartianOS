#include "VGA_image.h"
#include "lib.h"
#include "VBE.h"
#include "fs.h"
#include "paging.h"
#include "VGA_font.h"
#include "VGA_image.h"
#include "terminal.h"

static void print_struct(tga_header_t tga_header);
static int _get_frame_start(int scr_x, int scr_y, int load_width, int load_height);
static void _update_frame_pos(int *frame_pos, int load_width);

// pixel_t *frame_copy = (pixel_t *) VBE_NEXT;
icon_info_t icon_info;
pixel_t *frame_buffer = (pixel_t *)VBA_MEMORY_BASE;
pixel_t *icon_buf = (pixel_t *)ICON_START;
pixel_t mouse_bg[9][16];
volatile int screen_need_update = 0;
windows_t windows[3];

void print_struct(tga_header_t tga_header) {
    printf("magic number is %d\n", tga_header.magic1);    // 0
    printf("colormap is %d\n", tga_header.colormap);      // 0
    printf("encoding is %d\n", tga_header.encoding);      // 2
    printf("cmaporig is %d\n", tga_header.cmaporig);      // 0
    printf("cmaplen is %d\n", tga_header.cmaplen);        // 0
    printf("cmapent is %d\n", tga_header.cmapent);        // 0
    printf("x is %d\n", tga_header.x);                    // 0
    printf("y is %d\n", tga_header.y);                    // 0
    printf("h is %d\n", tga_header.h);                    // height
    printf("w is %d\n", tga_header.w);                    // width
    printf("bpp is %d\n", tga_header.bpp);                // 32 or 24
    printf("pixeltype is %d\n", tga_header.pixeltype);    // pixel ordering
}

int VGA_loadIcon(char *fname) {
    int8_t buf[18];
    tga_header_t tga_header;
    int res;
    unsigned char left_right, top_bottom;

    dentry_t dentry;
    if (-1 == read_dentry_by_name(fname, &dentry)) {
        ERROR_MSG;
        return -1;
    }

    if (-1 == (res = read_data(dentry.nr_inode, 0, buf, 18))) {
        ERROR_MSG;
        return -1;
    }

    memcpy(&tga_header, buf, 18);

    if (tga_header.encoding != 2) 
        return -1;

    if (tga_header.colormap != 0) 
        return -1;

    if (tga_header.w < 1 || tga_header.h < 1) 
        return -1;
    
    if ((tga_header.bpp != 24) && (tga_header.bpp != 32))
        return -1;

    left_right = !CHECK_BIT(tga_header.pixeltype, 4);
    top_bottom = CHECK_BIT(tga_header.pixeltype, 5);

    
    if (tga_header.w * tga_header.h * tga_header.bpp / 8 != (res = read_data(dentry.nr_inode, 18, (int8_t *)icon_buf, tga_header.w * tga_header.h * tga_header.bpp / 8))) {
        ERROR_MSG;
        return -1;
    }

    // fill in inco_info struct
    icon_info.bpp = tga_header.bpp;
    icon_info.height = tga_header.h;
    icon_info.width = tga_header.w;
    icon_info.top_to_bottom = top_bottom;
    icon_info.left_to_right = left_right;

    return 0;
}

void VGA_drawIcon(int scr_x, int scr_y) {

    // int frame_i, frame_j;
    int icon_i, icon_j;
    int load_width, load_height;
    int frame_pos;
    pixel_t pixel;
    pixel_24_t pixel_24;

    load_width = icon_info.width;
    load_height = icon_info.height;
    if (scr_x + load_width >= X_RES)
        load_width = X_RES - scr_x;
    if (scr_y + load_height >= Y_RES)
        load_height = Y_RES - scr_y;
    

    frame_pos = _get_frame_start(scr_x, scr_y, load_width, load_height);
    for (icon_j = 0; icon_j < load_height; icon_j++) {
        for (icon_i = 0; icon_i < load_width; icon_i++) {
            if (icon_info.bpp == 32) {
                pixel = icon_buf[icon_j * icon_info.width + icon_i];
                frame_buffer[frame_pos].r = pixel.r;
                frame_buffer[frame_pos].g = pixel.g;
                frame_buffer[frame_pos].b = pixel.b;
            }
            else {
                pixel_24 = ((pixel_24_t *)icon_buf)[icon_j * icon_info.width + icon_i];
                frame_buffer[frame_pos].r = pixel_24.r;
                frame_buffer[frame_pos].g = pixel_24.g;
                frame_buffer[frame_pos].b = pixel_24.b;
            }

            icon_info.left_to_right ? (frame_pos++) : (frame_pos--);
        }
        _update_frame_pos(&frame_pos, load_width);
    }
    
    screen_need_update = 1;
}   

int _get_frame_start(int scr_x, int scr_y, int load_width, int load_height) {

    if (icon_info.left_to_right & 0x1) {

        if (icon_info.top_to_bottom & 0x1)
            return scr_y * X_RES + scr_x;
        
        return (scr_y + load_height - 1) * X_RES + scr_x;
    } 

    if (icon_info.top_to_bottom & 0x1)
        return scr_y * X_RES + (scr_x + load_width - 1);

    return (scr_y + load_height - 1) * X_RES + (scr_x + load_width - 1);
}

void _update_frame_pos(int *frame_pos, int load_width) {
    if (icon_info.left_to_right & 0x1) {

        if (icon_info.top_to_bottom & 0x1) {
            *frame_pos = *frame_pos + X_RES - load_width;
            return;
        }
        
        *frame_pos = *frame_pos - X_RES - load_width;
        return;
    } 

    if (icon_info.top_to_bottom & 0x1) {
        *frame_pos = *frame_pos + X_RES + load_width;
        return;
    }

    *frame_pos = *frame_pos - X_RES + load_width;
    return;
}

void VGA_drawdesktop() {
    // int i, j;
    // uint32_t color;
    // for (i = 0; i < Y_RES; i++) {
	// 	for (j = 0; j < X_RES; j++) {
    //         color = i * i + i * j + j * j + 0x500000;
	// 		frame_buffer[i * X_RES + j].r = (color >> 16) & 0xFF;
    //         frame_buffer[i * X_RES + j].g = (color >> 8) & 0xFF;
    //         frame_buffer[i * X_RES + j].b = color & 0xFF;
	// 	}
	// }
    // screen_need_update = 1;
    if (-1 == VGA_loadIcon("desktop.tga")) {
        printf("Error in file: %s, function: %s, line: %d\n", __FILE__, __FUNCTION__, __LINE__);
        for (;;);
    }

    VGA_drawIcon(0, 0);

    return;
}

void VGA_drawRect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint8_t r, uint8_t g, uint8_t b, uint8_t on_screen) {
    int i, j;

    for (i = 0; i < h; i++) {
        for (j = 0; j < w; j++) {
            frame_buffer[(y + i) * X_RES + (x + j)].r = r;
            frame_buffer[(y + i) * X_RES + (x + j)].g = g;
            frame_buffer[(y + i) * X_RES + (x + j)].b = b;
        }
    }
}

void VGA_drawStatusbar() {
    VGA_drawRect(0, Y_RES - STATUS_BAR_HEIGHT, X_RES, STATUS_BAR_HEIGHT, 0xFF, 0, 0, 0);
    VGA_putSentence("Welcome!!!", 400, Y_RES - STATUS_BAR_HEIGHT + 2, 0);
}

void VGA_initWindows() {

    // 1
    windows[0].window_x = WINDOW1_X;
    windows[0].window_y = WINDOW1_Y;
    windows[0].layer = TOP;

    // 2
    windows[1].window_x = WINDOW2_X;
    windows[1].window_y = WINDOW2_X;
    windows[1].layer = MIDDLE;

    // 3
    windows[2].window_x = WINDOW3_X;
    windows[2].window_y = WINDOW3_Y;
    windows[2].layer = BOTTOM;

    // set buffer boundary (used to determine whether the character should also be printed on the screen (always print to the windows buffer))
    set_window_buf_boundary(&(windows[0].window_buf_x_start), &(windows[0].window_buf_x_end), &(windows[0].window_buf_y_start), &(windows[0].window_buf_y_end), 0, TOP);
    set_window_buf_boundary(&(windows[0].window_buf2_x_start), &(windows[0].window_buf2_x_end), &(windows[0].window_buf2_y_start), &(windows[0].window_buf2_y_end), 0, TOP);

    set_window_buf_boundary(&(windows[1].window_buf_x_start), &(windows[1].window_buf_x_end), &(windows[1].window_buf_y_start), &(windows[1].window_buf_y_end), 1, MIDDLE);
    set_window_buf_boundary(&(windows[1].window_buf2_x_start), &(windows[1].window_buf2_x_end), &(windows[1].window_buf2_y_start), &(windows[1].window_buf2_y_end), 1, TOP);

    set_window_buf_boundary(&(windows[2].window_buf_x_start), &(windows[2].window_buf_x_end), &(windows[2].window_buf_y_start), &(windows[2].window_buf_y_end), 2, BOTTOM);
    set_window_buf_boundary(&(windows[2].window_buf2_x_start), &(windows[2].window_buf2_x_end), &(windows[2].window_buf2_y_start), &(windows[2].window_buf2_y_end), 2, MIDDLE);
}

// !not considering the screen boundary
void set_window_buf_boundary(int16_t *x_start, int16_t *x_end, int16_t *y_start, int16_t *y_end, int window_idx, int type) {
    int temp_idx;
    switch (type) {
        // screen on the top does not have buffer (i.e. empty)
        // should print everything on the screen

        case TOP:
            *x_start = -1;
            *x_end = -1;
            *y_start = -1;
            *y_end = -1;
            break;
        case MIDDLE:
            temp_idx = get_top_window_id();

            // no overlaping at all
            if (windows[temp_idx].window_x + WINDOW_WIDTH <= windows[window_idx].window_x || 
                windows[temp_idx].window_x >= windows[window_idx].window_x + WINDOW_WIDTH ||
                windows[temp_idx].window_y + WINDOW_HEIGHT <= windows[window_idx].window_y || 
                windows[temp_idx].window_y >= windows[window_idx].window_y + WINDOW_HEIGHT) {

                *x_start = -1;
                *x_end = -1;
                *y_start = -1;
                *y_end = -1;
                return;
            }

            if ((*x_start = windows[temp_idx].window_x - windows[window_idx].window_x) < 0)
                *x_start = 0;

            if ((*y_start = windows[temp_idx].window_y - windows[window_idx].window_y) < 0)
                *y_start = 0;

            if ((*x_end = windows[temp_idx].window_x - windows[window_idx].window_x + WINDOW_WIDTH) > WINDOW_WIDTH)
                *x_end = WINDOW_WIDTH;
            
            if ((*y_end = windows[temp_idx].window_y - windows[window_idx].window_y + WINDOW_HEIGHT) > WINDOW_HEIGHT)
                *y_end = WINDOW_HEIGHT;

            break;
        case BOTTOM:
            temp_idx = get_middle_window_id();

            // no overlaping at all
            if (windows[temp_idx].window_x + WINDOW_WIDTH <= windows[window_idx].window_x || 
                windows[temp_idx].window_x >= windows[window_idx].window_x + WINDOW_WIDTH ||
                windows[temp_idx].window_y + WINDOW_HEIGHT <= windows[window_idx].window_y || 
                windows[temp_idx].window_y >= windows[window_idx].window_y + WINDOW_HEIGHT) {

                *x_start = -1;
                *x_end = -1;
                *y_start = -1;
                *y_end = -1;
                return;
            }

            if ((*x_start = windows[temp_idx].window_x - windows[window_idx].window_x) < 0)
                *x_start = 0;

            if ((*y_start = windows[temp_idx].window_y - windows[window_idx].window_y) < 0)
                *y_start = 0;

            if ((*x_end = windows[temp_idx].window_x - windows[window_idx].window_x + WINDOW_WIDTH) > WINDOW_WIDTH)
                *x_end = WINDOW_WIDTH;
            
            if ((*y_end = windows[temp_idx].window_y - windows[window_idx].window_y + WINDOW_HEIGHT) > WINDOW_HEIGHT)
                *y_end = WINDOW_HEIGHT;
            break;
        default:
            break;
    }
}

void VGA_drawWindows(int i, uint8_t on_screen) {

    VGA_drawRect(windows[i].window_x, windows[i].window_y, WINDOW_WIDTH, WINDOW_HEIGHT, 0x0, 0x0, 0x0, on_screen);
    VGA_drawRect(windows[i].window_x, windows[i].window_y, WINDOW_WIDTH, WINDOW_TITLE_HEIGHT, 0xFF, 0xFF, 0xFF, on_screen);
        
    switch (i) {
        case 0:
            VGA_putSentence("Terminal 1", windows[i].window_x + 80, windows[i].window_y + 2, on_screen);
            break;
        case 1:
            VGA_putSentence("Terminal 2", windows[i].window_x + 80, windows[i].window_y + 2, on_screen);
            break;
        case 2:
            VGA_putSentence("Terminal 3", windows[i].window_x + 80, windows[i].window_y + 2, on_screen);
            break;

        default:
            break;    
    }
}

void VGA_initterms() {
    VGA_initWindows();
}

void VGA_drawMouse(int scr_x, int scr_y) {
    unsigned char mouse_bitmap[][17] = {
        "0010000000000000",
        "0010100000000000",
        "0010001000000000",
        "0010000010000000",
        "0010000000100000",
        "0010000000001000",
        "0010100111111100",
        "0011010010000000",
        "0010001111000000"
    };

    int mouse_bound[9][2] = {
        {2, 3},
        {2, 5},
        {2, 7},
        {2, 9},
        {2, 11},
        {2, 13},
        {2, 14},
        {2, 9},
        {2, 10}
    };

    VGA_saveMouseBG(scr_x, scr_y);

    int i, j;
    for (i = 0; i < 9; i++) {
        for (j = mouse_bound[i][0]; j < mouse_bound[i][1]; j++) {
            if (mouse_bitmap[i][j] == '1') {
                frame_buffer[(scr_y + i) * X_RES + (scr_x + j)].b = 0;
                frame_buffer[(scr_y + i) * X_RES + (scr_x + j)].r = 0;
                frame_buffer[(scr_y + i) * X_RES + (scr_x + j)].g = 0;
            } else {
                frame_buffer[(scr_y + i) * X_RES + (scr_x + j)].b = 0xFF;
                frame_buffer[(scr_y + i) * X_RES + (scr_x + j)].r = 0xFF;
                frame_buffer[(scr_y + i) * X_RES + (scr_x + j)].g = 0xFF;
            }
        }
    }

    screen_need_update = 1;
}

void VGA_saveMouseBG(int scr_x, int src_y) {
    int i, j;
    for (i = 0; i < 9; i++) {
        for (j = 0; j < 16; j++) {
            if (scr_x + j >= X_RES || src_y + i >= Y_RES) {
                ERROR_MSG;
                for(;;);
            }
            mouse_bg[i][j] = frame_buffer[(src_y + i) * X_RES + (scr_x + j)];
        }
    }
}

void VGA_putmouseBG(int scr_x, int src_y) {
    int i, j;
    for (i = 0; i < 9; i++) {
        for (j = 0; j < 16; j++) {
            if (scr_x + j >= X_RES || src_y + i >= Y_RES) {
                ERROR_MSG;
                for(;;);
            }
            frame_buffer[(src_y + i) * X_RES + (scr_x + j)] = mouse_bg[i][j];
        }
    }
}


// return 0 if the character should be printed on buffer
// 1 if on screen
// char_off_x and char_off_y are relative coordinate
int print_screen_or_buf(int window_id, int char_off_x, int char_off_y) {

    switch (windows[window_id].layer)
    {
    case TOP:
        
        return check_pixel_in_screen(char_off_x + windows[window_id].window_x + 7, char_off_y + windows[window_id].window_y + 15);

    case MIDDLE:
        
        return check_pixel_in_middle_window(window_id, char_off_x, char_off_y) && check_pixel_in_middle_window(window_id, char_off_x + 7, char_off_y + 15);

    case BOTTOM:
        
        return check_pixel_in_bottom_window(window_id, char_off_x, char_off_y) && check_pixel_in_bottom_window(window_id, char_off_x + 7, char_off_y + 15);

    default:
        ERROR_MSG;
        for (;;);
        break;
    }
}

int get_top_window_id() {
    int i;
    for (i = 0; i < 3; i++) {
        if (windows[i].layer == TOP)
            return i;
    }
    ERROR_MSG;
    for (;;);
    return -1;
}

int get_middle_window_id() {
    int i;
    for (i = 0; i < 3; i++) {
        if (windows[i].layer == MIDDLE)
            return i;
    }
    ERROR_MSG;
    for (;;);
    return -1;
}

int get_bottom_window_id() {
    int i;
    for (i = 0; i < 3; i++) {
        if (windows[i].layer == BOTTOM)
            return i;
    }

    ERROR_MSG;
    for (;;);
    return -1;
}

// switch VBE_base paging to the terminal `tid`'s buffer
void switchpagingtobuffer(int tid) {
    int target_place = VBE_START >> 22;
    page_dir_base[target_place].kernel_page_desc.pg_addr = term_arr[tid].vid_mem_buffer >> 22;
    page_dir_base[FISH_VIRTUAL >> 22].kernel_page_desc.present = term_arr[tid].vidmap_flag;
    page_dir_base[FISH_VIRTUAL >> 22].kernel_page_desc.pg_addr = term_arr[tid].vid_mem_buffer >> 22;
    flush_tlb();
}

void switchpagingtovideomem(int tid) {
    int target_place = VBE_START >> 22;
    page_dir_base[target_place].kernel_page_desc.pg_addr = target_place;
    page_dir_base[FISH_VIRTUAL >> 22].kernel_page_desc.present = term_arr[tid].vidmap_flag;
    page_dir_base[FISH_VIRTUAL >> 22].kernel_page_desc.pg_addr = target_place;
    flush_tlb();
}

// return 1 if the pixel is on the screen
// return 0 if the pixel is in the buffer
// pixel_x and pixel_y are relative coordinate
int check_pixel_in_middle_window(int window_idx, int pixel_x, int pixel_y) {
    // sanity check
    if (windows[window_idx].layer != MIDDLE) {
        ERROR_MSG;
        for (;;);
    }

    if (pixel_x >= windows[window_idx].window_buf_x_start && pixel_x < windows[window_idx].window_buf_x_end &&
        pixel_y >= windows[window_idx].window_buf_y_start && pixel_y < windows[window_idx].window_buf_y_end)

        return 0;
    
    return 1;
}

// return 1 if the pixel is on the screen
// return 0 if the pixel is in the buffer
// pixel_x and pixel_y are relative coordinate
int check_pixel_in_bottom_window(int window_idx, int pixel_x, int pixel_y) {
    // sanity check
    if (windows[window_idx].layer != BOTTOM) {
        ERROR_MSG;
        for (;;);
    }

    if (pixel_x >= windows[window_idx].window_buf_x_start && pixel_x < windows[window_idx].window_buf_x_end &&
        pixel_y >= windows[window_idx].window_buf_y_start && pixel_y < windows[window_idx].window_buf_y_end)

        return 0;

    if (pixel_x >= windows[window_idx].window_buf2_x_start && pixel_x < windows[window_idx].window_buf2_x_end &&
        pixel_y >= windows[window_idx].window_buf2_y_start && pixel_y < windows[window_idx].window_buf2_y_end)

        return 0;
    
    return 1;
}

// absolute coordinate
// return 1 if on screen
int check_pixel_in_screen(int pixel_x, int pixel_y) {
    if (pixel_x >= 0 && pixel_x < X_RES && pixel_y >= 0 && pixel_y < Y_RES)
        return 1;

    return 0;
}
