#ifndef VGA_FONT_H
#define VGA_FONT_H

#include "types.h"
#include "VBE.h"
#include "VGA.h"
#include "VGA_image.h"

extern unsigned char font_data[256][16];

#define TEXT_OFF_X 2
#define TEXT_OFF_Y (WINDOW_TITLE_HEIGHT + 2)

/* display character `ch` on position (cur_x, cur_y), NO UPDATE CURSOR */
/* cur_x and cur_y is the relative x, y offset w.r.t the upperleft position of window */
// extern void VGA_windowsputc(uint8_t ch, uint32_t cur_x, uint32_t cur_y, windows_t *cur_window);

/* display a string starting  */
extern void VGA_putSentence(int8_t *str, uint32_t start_x, uint32_t start_y, uint8_t on_screen);

extern void VGA_putc(uint8_t ch, uint32_t scr_x, uint32_t scr_y, uint8_t on_screen);

extern void VGA_windowputc(int32_t win_idx, uint8_t ch, int32_t off_x, int32_t off_y);

extern void VGA_windowputsentence(int32_t win_idx, int8_t *str, int32_t start_x, int32_t start_y);

extern void VGA_windowscrolluponeline(int window_idx);

extern void VGA_kbdputc(uint8_t ch, uint32_t scr_x, uint32_t scr_y, uint8_t on_screen);

#endif
