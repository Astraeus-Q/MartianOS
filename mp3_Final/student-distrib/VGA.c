#include "VGA.h"
#include "VGA_image.h"
#include "lib.h"
#include "VBE.h"

/* init the VGA stuff */
void VGA_init() {
    BGA_setParam();
}

/* VGA clean screen */
void VGA_clear() {
    int x, y;
    // TODO: save the screen state ?
    VGA_saveScreen();

    uint32_t *screen_start = (uint32_t *)VBA_MEMORY_BASE;
    for (x = 0; x < X_RES; x++) {
        for (y = 0; y < Y_RES; y++) {
            screen_start[y * X_RES + x] = 0x0;   // black
        }
    }
}

// work on this later (not show_screen ...)
void VGA_updateScreen() {

}

// work on this later
void VGA_saveScreen() {

}

// void VGA_showscreen() {
//     if (screen_need_update) {
//         memcpy(frame_buffer, frame_copy, PIXEL_BUF_MAXSIZE * 4);
//         screen_need_update = 0;
//     }
// }

