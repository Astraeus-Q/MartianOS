#ifndef VGA_H
#define VGA_H

#include "lib.h"

/* window buffer */
// extern uint32_t windows_buf[WINDOW_NUM][WINDOW_DIM_X * WINDOW_DIM_Y];

/* macro used to write an array of one-byte values to a port, copied from MP2 */
#define REP_OUTSB(port, source, count)                              \
do {                                                                \
    asm volatile ("                                               \n\
        1: movb 0(%1), %%al                                       \n\
        outb %%al, (%w2)                                          \n\
        incl %1                                                   \n\
        decl %0                                                   \n\
        jne 1b                                                    \n\
        "                                                           \
        : /* no outputs */                                          \
        : "c"((count)), "S"((source)), "d"((port))                  \
        : "eax", "memory", "cc"                                     \
    );                                                              \
} while (0)


extern void VGA_init();

extern void VGA_clear();

extern void VGA_updateScreen();

extern void VGA_saveScreen();

extern void VGA_showscreen();

#endif

