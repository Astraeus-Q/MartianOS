// /* Reference: OSDev https://wiki.osdev.org/Mouse */

#include "mouse.h"
#include "keyboard.h"
#include "i8259.h"
#include "lib.h"
#include "terminal.h"
#include "VGA_image.h"

int changes = 0;
volatile int32_t mouse_x = 0;
volatile int32_t mouse_y = 0;
uint8_t mouse_left = 0;
uint8_t mouse_middle = 0;
uint8_t mouse_right = 0;


void mouse_init(void)
{
    send_byte_0x64(ENABLE_AUX_DEV);
    send_byte_0x64(GET_COMPAQ_STATUS);

    uint8_t status = receive_byte_0x60();
    status |= 2;
    status &= 0xDF;

    send_byte_0x64(SET_COMPAQ_STATUS);

    out_waiting();
    outb(status, MOUSE_DATA_PORT);

    send_byte_0x60(SET_DEFAULTS);
    send_byte_0x60(ENABLE_STREAMING);
    send_byte_0x60(SET_SAMPLE_RATE);

    out_waiting();
    outb(200, MOUSE_DATA_PORT);

    enable_irq(MOUSE_IRQ);
    enable_irq(SLAVE_IRQ_LINE);
}

void mouse_int_handler(void)
{
    cli();

    if (0 == (inb(MOUSE_CHECK_PORT) & 0x1)) {
        send_eoi(MOUSE_IRQ);
        sti();
        return;
    }
    if (0 == (inb(MOUSE_CHECK_PORT) & 0x20)) {
        send_eoi(MOUSE_IRQ);
        sti();
        return;
    }

    mouse_packet_info_t packet;
    packet.val = receive_byte_0x60();
    if (packet.val == MOUSE_ACKNOWLEDGE) {
        send_eoi(MOUSE_IRQ);
        sti();
        return;
    }
    // if (packet.x_overflow || packet.y_overflow || !packet.always1)  {
    //     send_eoi(MOUSE_IRQ);
    //     printf("Overflow!\n");
    //     return;
    // }

    int32_t x_move = receive_byte_0x60();
    int32_t y_move = receive_byte_0x60();

    // x_move -= (packet.val << 4) & 0x100;
    // y_move -= (packet.val << 3) & 0x100;

    if (packet.x_sign)
        x_move |= OR_NEG_MASK;
    if (packet.y_sign)
        y_move |= OR_NEG_MASK;

    // x_move /= MOUSE_SMOOTH;
    // y_move /= MOUSE_SMOOTH;

    // int i;
    // for (i = 0; i < 10; i++);

    int32_t new_x = mouse_x + x_move;
    int32_t new_y = mouse_y - y_move;

    if (new_x < 0)
        new_x = 0;
    if (new_y < 0)
        new_y = 0;
    if (new_x >= X_RES - 16)    // the width of mouse is 16
        new_x = X_RES - 16;
    if (new_y >= Y_RES - 9)   // the height of mouse is 9
        new_y = Y_RES - 9;

    if (new_x != mouse_x || new_y != mouse_y) 
    {
        // change_vidmem_mapping(viewing_term_ptr->tid);
        switchpagingtovideomem(0);
        VGA_putmouseBG(mouse_x, mouse_y);
        // change_vidmem_mapping(running_term_ptr->tid);
        mouse_x = new_x;
        mouse_y = new_y;
        changes++;
        // printf("new (x,y) = (%d, %d)\n", mouse_x, mouse_y);
        // change_vidmem_mapping(viewing_term_ptr->tid);
        VGA_drawMouse(new_x, new_y);  // save (mouse_x, mouse_y), then draw
        // change_vidmem_mapping(running_term_ptr->tid);
    }

    // mouse click
    if (packet.btn_left)
    {
        int which_terminal = check_mouse_in_which_terminal();
        if (which_terminal != -1)
        {
            // change_vidmem_mapping(viewing_term_ptr->tid);
            switchpagingtovideomem(0);
            VGA_putmouseBG(mouse_x, mouse_y);
            // change_vidmem_mapping(running_term_ptr->tid);

            switch_terminal(which_terminal);
            
            // change_vidmem_mapping(viewing_term_ptr->tid);
            switchpagingtovideomem(0);
            VGA_drawMouse(mouse_x, mouse_y);
            // change_vidmem_mapping(running_term_ptr->tid);
        }
    }

    send_eoi(MOUSE_IRQ);
    sti();
}

void send_byte_0x64(uint8_t data)
{
    out_waiting();
    outb(data, MOUSE_STATUS_PORT);
}

void send_byte_0x60(uint8_t data)
{
    out_waiting();
    outb(PRECEDING_SEND, MOUSE_STATUS_PORT);
    out_waiting();
    outb(data, MOUSE_DATA_PORT);
}

uint8_t receive_byte_0x60(void)
{
    in_waiting();
    return inb(MOUSE_DATA_PORT);
}

void out_waiting()
{
    int i = 1000;
    while (i-- && inb(MOUSE_STATUS_PORT) & UNCLEAR_MASK)
        ;
}

void in_waiting()
{
    int i = 1000;
    while (i-- && inb(MOUSE_STATUS_PORT) & AVAIL_MASK)
        ;
}

int check_mouse_in_which_terminal() {
    if (mouse_x >= TERM1_ICON_X && mouse_x < TERM1_ICON_X + ICON_WIDTH && mouse_y >= TERM1_ICON_Y && mouse_y < TERM1_ICON_Y + ICON_HEIGHT) {
        return 0;
    } else if (mouse_x >= TERM2_ICON_X && mouse_x < TERM2_ICON_X + ICON_WIDTH && mouse_y >= TERM2_ICON_Y && mouse_y < TERM2_ICON_Y + ICON_HEIGHT) {
        return 1;
    } else if (mouse_x >= TERM3_ICON_X && mouse_x < TERM3_ICON_X + ICON_WIDTH && mouse_y >= TERM3_ICON_Y && mouse_y < TERM3_ICON_Y + ICON_HEIGHT) {
        return 2;
    }

    return -1;
}
