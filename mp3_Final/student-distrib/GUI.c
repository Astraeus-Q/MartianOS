#include "GUI.h"
#include "types.h"
#include "VGA.h"
#include "VGA_font.h"
#include "paging.h"
#include "VGA_image.h"
#include "terminal.h"


void GUI_init() {
    int i;
    VGA_init();
    VGA_drawdesktop();
    VGA_drawStatusbar();

    // draw terminal icon
    if (-1 == VGA_loadIcon("terminal-icon.tga")) {
        printf("Error in file: %s, function: %s, line: %d\n", __FILE__, __FUNCTION__, __LINE__);
        for (;;);
    }
    VGA_drawIcon(100, 600);  // 72 X 72
    VGA_drawIcon(300, 600);  // 72 x 72
    VGA_drawIcon(500, 600);  // 72 x 72
  
    VGA_initterms();
    // ! we don't care which tid cz no shell is running lol
    switchpagingtovideomem(0);
    VGA_drawWindows(0, 1);
    switchpagingtobuffer(0);
    VGA_drawWindows(0, 0);

    switchpagingtovideomem(0);
    VGA_drawWindows(1, 1);
    switchpagingtobuffer(1);
    VGA_drawWindows(1, 0);

    switchpagingtovideomem(0);
    VGA_drawWindows(2, 1);
    switchpagingtobuffer(2);
    VGA_drawWindows(2, 0);

    switchpagingtovideomem(0);
    int j;
    for (i = 0; i < 9; i++) {
        for (j = 0; j < 16; j++) {
            mouse_bg[i][j] = frame_buffer[i * X_RES + j];
        }
    }

    VGA_drawMouse(0, 0);
    switch_terminal(0);
    screen_need_update = 1;
}
