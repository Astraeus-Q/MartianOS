#include "VBE.h"
#include "lib.h"

void BGA_setParam() {
    // disable VBE extension
    outw(BGA_ENABLE_INDEX, BGA_INDEX_PORT);
    outw(0x0, BGA_DATA_PORT);

    // set x_res
    outw(BGA_X_RESOLUTION_INDEX, BGA_INDEX_PORT);
    outw(X_RES, BGA_DATA_PORT);

    // set y_res
    outw(BGA_Y_RESOLUTION_INDEX, BGA_INDEX_PORT);
    outw(Y_RES, BGA_DATA_PORT);

    // set bpp
    outw(BGA_BIT_PER_PIXEL_INDEX, BGA_INDEX_PORT);
    outw(BPP, BGA_DATA_PORT);

    // enable VBE extension
    outw(BGA_ENABLE_INDEX, BGA_INDEX_PORT);
    outw(0x1 | VBE_LINEAR_BUFFER_MASK | VBE_NO_CLEAR_MEM, BGA_DATA_PORT);
}



