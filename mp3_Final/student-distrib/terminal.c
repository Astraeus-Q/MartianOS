#include "terminal.h"
#include "lib.h"
#include "syscall_handler.h"
#include "paging.h"
#include "i8259.h"
#include "keyboard.h"
#include "x86_desc.h"
#include "VGA_font.h"
#include "VGA_image.h"

// terminal_t terminal_array[3];
// volatile int current_term_id = 0;
// int8_t pcb_record_for_each_terminal[3][5];
terminal_t term_arr[3];

terminal_t *running_term_ptr = &(term_arr[0]);
terminal_t *viewing_term_ptr = &(term_arr[0]);

// extern int32_t ece391_execute(uint8_t *command);

/*
 * terminal_init
 * Description: Initialize the terminal.
 *  Inputs: None
 *  Outputs: None
 * Side Effects: None.
 */
void terminal_init(void)
{
    // int i;
    // // int j;
    // for (i = 0; i < 6; i++)
    //     pcb_bitmap[i] = NONE;
    // // set_paging_multi_terminal();
    // for (i = 0; i < 3; i++) {
    //     terminal_array[i].valid = 1;  //check whether there is base shell running in the terminal
    //     // for(j = 0; j < 5; j++){
    //     //     pcb_record_for_each_terminal[i][j] = -1;
    //     // }
    //     // terminal_array[i].num_char_in_buf = 0;
    //     // terminal_array[i].num_char_on_screen = 0;
    //     // memset(terminal_array[i].kbd_buf, 0, 128);
    // }

    // this call will create PCB0 for base shell, write screen content on VIDEO_MEMORY
    // write global variables (i.e. cursor info, keyboard buffer, all flags, etc)

    int i;
    for (i = 0; i < 3; i++)
    {
        term_arr[i].tid = i;
        term_arr[i].pid = -1;
        term_arr[i].buf_pos = 0;
        term_arr[i].kbd_buf_ready = 0;
        term_arr[i].vidmap_flag = 0;
    }

    term_arr[0].cursor_x = WINDOW1_X + TEXT_OFF_X;
    term_arr[0].cursor_y = WINDOW1_Y + TEXT_OFF_Y;
    term_arr[1].cursor_x = WINDOW2_X + TEXT_OFF_X;
    term_arr[1].cursor_y = WINDOW2_Y + TEXT_OFF_Y;
    term_arr[2].cursor_x = WINDOW3_X + TEXT_OFF_X;
    term_arr[2].cursor_y = WINDOW3_Y + TEXT_OFF_Y;

    flush_tlb();

    kbd_buf = term_arr[0].kbd_buf;
    buf_pos = 0;
    screen_x_ptr = &term_arr[0].cursor_x;
    screen_y_ptr = &term_arr[0].cursor_y;

    term_arr[0].vid_mem_buffer = TERM_1_4M_VIDEO;
    term_arr[1].vid_mem_buffer = TERM_2_4M_VIDEO;
    term_arr[2].vid_mem_buffer = TERM_3_4M_VIDEO;

    term_arr[0].next_term_to_run = &(term_arr[1]);
    term_arr[1].next_term_to_run = &(term_arr[2]);
    term_arr[2].next_term_to_run = &(term_arr[0]);
}

/*
 * terminal_open
 * Description: Open the terminal.
 *  Inputs:
 *      - filename: file name
 *  Outputs: 0
 * Side Effects: None.
 */
int terminal_open(const uint8_t *filename)
{
    // do nothing
    return 0;
}

/*
 * terminal_close
 * Description: Close the terminal.
 *  Inputs:
 *      - filename: file name
 *  Outputs: 0
 * Side Effects: None.
 */
int terminal_close(const uint8_t *filename)
{
    // do nothing
    return 0;
}

/*
 * terminal_read
 * Description: Reads FROM the keyboard buffer into buf, return number of bytes read.
 *  Inputs:
 *      - fd: not used yet
 *      - buf: buffer to read char into
 *      - nbytes: size to read
 *  Outputs: # of byte read
 * Side Effects: change "kbd_buf_ready".
 */
int terminal_read(int32_t fd, void *buf, int32_t nbytes)
{
    // sanity check
    if (buf == NULL || nbytes <= 0)
    {
        // printf("terminal read fails!\n");
        return -1;
    }
    // please go to "keyboard.c"
    return read_from_kbd_buf_to_buf(fd, buf, nbytes);
}

/*
 * terminal_write
 * Description: Writes TO the screen from buf, return number of bytes written or -1.
 *  Inputs:
 *      - fd: not used yet
 *      - buf: buffer to write char into
 *      - nbytes: size to write
 *  Outputs: # of byte written
 * Side Effects: None.
 */
int terminal_write(int32_t fd, void *buf, int32_t nbytes)
{
    // sanity check
    if (buf == NULL || nbytes <= 0)
    {
        // printf("terminal write fail!\n");
        return -1;
    }

    int i;
    for (i = 0; i < nbytes; i++)
        nb_putc(((char *)buf)[i]);

    // return the number of bytes written
    return i;
}
/*
 * switch_terminal
 * Description: switch the terminal
 *  Inputs:
 *      -target_tid
 *  Outputs: none
 * Side Effects: None.
 */
void switch_terminal(int target_tid)
{
    if (target_tid == viewing_term_ptr->tid) {
        switchpagingtovideomem(target_tid);
        int i, j;
        for (j = windows[target_tid].window_y; j < windows[target_tid].window_y + WINDOW_HEIGHT; j++) {
            for (i = windows[target_tid].window_x; i < windows[target_tid].window_x + WINDOW_WIDTH; i++) {
                frame_buffer[j * X_RES + i] = ((pixel_t *)(viewing_term_ptr->vid_mem_buffer))[j * X_RES + i];
            }
        }
        return;
    }

    terminal_t *target_term_ptr = &(term_arr[target_tid]);
    int one, two, three;
    one = target_tid;
    two = viewing_term_ptr->tid;
    three = 3 - target_tid - viewing_term_ptr->tid;

    // save things
    viewing_term_ptr->buf_pos = buf_pos;

    // restore things
    kbd_buf = target_term_ptr->kbd_buf;
    buf_pos = target_term_ptr->buf_pos;

    update_screen_xy_ptr(&(target_term_ptr->cursor_x), &(target_term_ptr->cursor_y));

    // actually, we don't care which tid
    switchpagingtovideomem(target_tid);
    int i, j;
    for (j = windows[target_tid].window_y; j < windows[target_tid].window_y + WINDOW_HEIGHT; j++) {
        for (i = windows[target_tid].window_x; i < windows[target_tid].window_x + WINDOW_WIDTH; i++) {
            frame_buffer[j * X_RES + i] = ((pixel_t *)(target_term_ptr->vid_mem_buffer))[j * X_RES + i];
        }
    }

    // update the window layer
    windows[one].layer = TOP;
    windows[two].layer = MIDDLE;
    windows[three].layer = BOTTOM;
    set_window_buf_boundary(&(windows[one].window_buf_x_start), &(windows[one].window_buf_x_end), &(windows[one].window_buf_y_start), &(windows[one].window_buf_y_end), one, TOP);
    set_window_buf_boundary(&(windows[one].window_buf2_x_start), &(windows[one].window_buf2_x_end), &(windows[one].window_buf2_y_start), &(windows[one].window_buf2_y_end), one, TOP);
 
    set_window_buf_boundary(&(windows[two].window_buf_x_start), &(windows[two].window_buf_x_end), &(windows[two].window_buf_y_start), &(windows[two].window_buf_y_end), two, MIDDLE);
    set_window_buf_boundary(&(windows[two].window_buf2_x_start), &(windows[two].window_buf2_x_end), &(windows[two].window_buf2_y_start), &(windows[two].window_buf2_y_end), two, TOP);
    
    set_window_buf_boundary(&(windows[three].window_buf_x_start), &(windows[three].window_buf_x_end), &(windows[three].window_buf_y_start), &(windows[three].window_buf_y_end), three, BOTTOM);
    set_window_buf_boundary(&(windows[three].window_buf2_x_start), &(windows[three].window_buf2_x_end), &(windows[three].window_buf2_y_start), &(windows[three].window_buf2_y_end), three, MIDDLE);

    viewing_term_ptr = &(term_arr[target_tid]);
    
}

/*
 * change_vidmem_mapping
 * Description: change the video memory mapping
 *  Inputs:
 *      - tid: terminal id
 *  Outputs: None
 * Side Effects: None.
 */
void change_vidmem_mapping(int tid)
{
    int32_t temp_flag;

    screen_x_ptr = &term_arr[tid].cursor_x;
    screen_y_ptr = &term_arr[tid].cursor_y;
    temp_flag = (tid != viewing_term_ptr->tid);
    switch_fish_paging(tid, temp_flag);
    flush_tlb();
}

/*
 * switch_fish_paging
 * Description: switch the paging of the video memory
 *  Inputs:
 *      -tid
 *      -temp_flag
 *  Outputs: none
 * Side Effects: None.
 */
void switch_fish_paging(int32_t tid, int32_t temp_flag)
{
    int32_t target_place;

    // target_place = VIDEO_START >> 12;
    target_place = VBE_START >> 22;

    if (temp_flag)
    {
        // usr_page_table_base[target_place].pg_addr = term_arr[tid].vid_mem_buffer >> 12;
        // usrmap_page_table_base[target_place].present = term_arr[tid].vidmap_flag;
        // usrmap_page_table_base[target_place].pg_addr = term_arr[tid].vid_mem_buffer >> 12;
        page_dir_base[target_place].kernel_page_desc.pg_addr = term_arr[tid].vid_mem_buffer >> 22;
        page_dir_base[FISH_VIRTUAL >> 22].kernel_page_desc.present = term_arr[tid].vidmap_flag;
        page_dir_base[FISH_VIRTUAL >> 22].kernel_page_desc.pg_addr = term_arr[tid].vid_mem_buffer >> 22;
    }
    else
    {
        // usr_page_table_base[target_place].pg_addr = target_place;
        // usrmap_page_table_base[target_place].present = term_arr[tid].vidmap_flag;
        // usrmap_page_table_base[target_place].pg_addr = target_place;
        page_dir_base[target_place].kernel_page_desc.pg_addr = target_place;
        page_dir_base[FISH_VIRTUAL >> 22].kernel_page_desc.present = term_arr[tid].vidmap_flag;
        page_dir_base[FISH_VIRTUAL >> 22].kernel_page_desc.pg_addr = target_place;
    }
}
