#ifndef MOUSE_H
#define MOUSE_H

#include    "types.h"

// constants
#define MOUSE_IRQ 12
#define SCREEN_HEIGHT 25
#define SCREEN_WIDTH 80
#define MOUSE_SMOOTH 5

// commands
#define ENABLE_AUX_DEV 0xA8
#define GET_COMPAQ_STATUS 0x20
#define SET_COMPAQ_STATUS 0x60
#define PRECEDING_SEND 0xD4

#define GET_ID 0xF2
#define SET_SAMPLE_RATE 0xF3
#define ENABLE_STREAMING 0xF4
#define DISABLE_STREAMING 0xF5
#define SET_DEFAULTS 0xF6
#define RESET 0xFF

// mouse ports
#define MOUSE_DATA_PORT 0x60
#define MOUSE_STATUS_PORT 0x64

// masks
#define AVAIL_MASK 0x01
#define IS_MOUSE_MASK 0x20
#define UNCLEAR_MASK 0x02
#define OR_NEG_MASK 0xFFFFFF00

extern volatile int mouse_x;
extern volatile int mouse_y;

typedef union mouse_packet_info {
    uint8_t val;
    struct {
        uint8_t btn_left    : 1;
        uint8_t btn_right   : 1;
        uint8_t btn_middle  : 1;
        uint8_t always1     : 1;
        uint8_t x_sign      : 1;
        uint8_t y_sign      : 1;
        uint8_t x_overflow  : 1;
        uint8_t y_overflow  : 1;
    } __attribute__ ((packed));
} mouse_packet_info_t;

void mouse_init(void);
void mouse_int_handler(void);
uint8_t receive_byte_0x60(void);
void send_byte_0x64(uint8_t data);
void send_byte_0x60(uint8_t data);
void out_waiting();
void in_waiting();

int check_mouse_in_which_terminal();

//! ==========================================================

#define MOUSE_DATA_PORT 0x60
#define MOUSE_CHECK_PORT 0x64
#define MOUSE_ACKNOWLEDGE 0xFA
#define RESET 0xFF
#define Y_OVERFLOW 0x080
#define X_OVERFLOW 0x40
#define Y_SIGN 0x20
#define X_SIGN 0x10
#define VERIFY_ONE 0x08
#define MID_BUTTON 0x04
#define RIGHT_BUTTON 0x02
#define LEFT_BUTTON 0x01
#define RESET 0xFF
#define PRE_COMMAND 0xD4

//! ==========================================================

#endif 



