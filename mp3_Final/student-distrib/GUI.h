#ifndef GUI_H
#define GUI_H

#include "lib.h"
#include "VGA_image.h"
/**
 * Input: name of the .tga format file provided by user, the upper-left coordinate of display 
 * ! 1. Don't trust user, check file format and whether that file exists
 * ! 2. does not draw button in shell windows
 * ! 3. cut off the images that falls out of the screen
 */
extern void GUI_drawButton(int8_t *fname, uint32_t pos_x, uint32_t pos_y);

/* user gives the name of object to draw (i.e.) */
extern void GUI_drawStatusBar();

/* GUI init */
void GUI_init();


#endif
