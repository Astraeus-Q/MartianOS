#ifndef VBE_H
#define VBE_H

#include "types.h"

#define BGA_VERSION 0xB0C0
#define BGA_INDEX_PORT 0x01CE
#define BGA_DATA_PORT 0x01CF
#define VBA_MEMORY_BASE 0xFD000000  /* `info pic` in test machine*/

/* possible index value written into BGA_WRITE_PORT */
#define BGA_VERSION_INDEX       0
#define BGA_X_RESOLUTION_INDEX  1
#define BGA_Y_RESOLUTION_INDEX  2
#define BGA_BIT_PER_PIXEL_INDEX 3
#define BGA_ENABLE_INDEX        4   // write 0 to this index before changing x_res, y_res and bpp, write 1 to this index to visualize the change

/* defined parameters */
#define X_RES 1024
#define Y_RES 768   /* 768KB video memory used, smaller than 4MB */
#define BPP    32   

/* define masks */
#define VBE_LINEAR_BUFFER_MASK 0x40
#define VBE_NO_CLEAR_MEM       0x80

/* check if BGA is available */
extern int32_t BGA_isAvail();

/* Set VBE version */
extern int32_t BGA_setVersion();

/* set x_res = 1024, y_res = 768 and bpp = 8 */
extern void BGA_setParam();

#endif
