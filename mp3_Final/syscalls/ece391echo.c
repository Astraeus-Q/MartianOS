#include <stdint.h>

#include "ece391support.h"
#include "ece391syscall.h"

#define MAX_TEXT_LEN 1024*63

int main ()
{
    int32_t fd, cnt;
    int32_t arg1_length;
    int32_t len = 0;
    int32_t i, j, q;
    uint8_t buf[1024]; 
    uint8_t text[MAX_TEXT_LEN];
    uint8_t arg1[100];   // text to echo on screen or ">" or ">>"
    // uint8_t arg2;
    uint8_t arg3[50];
    

    arg1[0] = arg3[0] = '\0';

    if (0 != ece391_getargs (buf, 1024)) {
        ece391_fdputs (1, (uint8_t*)"could not read arguments\n");
	    return 3;
    }

    /* get the first argument */
    for (i = 0; i < 100 && buf[i] != ' ' && buf[i] != '\n'; i++)
        arg1[i] = buf[i];
    

    arg1[i] = '\0';
    arg1_length = i;
    /* if the second argument is not present, print whatever to the screen */
    if (i == 100) {
        ece391_fdputs(1, (uint8_t *)arg1);
        return 0;
    }

    /* filter out the spaces between the first and second argument */
    while (buf[++i] == ' ');
    if (buf[i] != '>' || buf[i+1] != '>'){
        ece391_fdputs (1, (uint8_t*)"Error! The format of echo should be : echo $text >> $filename.\n");
        return 3;
    }
    else{
        i += 2;
    }

    /* filter out the spaces between the second and third argument */
    while (buf[++i] == ' ');

    /* get the second argument */
    for (j = 0; i < 50 && buf[i] != ' ' && buf[i] != '\n'; i++, j++)
        arg3[j] = buf[i];
    
    arg3[j] = '\0';


    for (q = 0; q < MAX_TEXT_LEN; q++){
        text[q] = 0;
    }


    if (-1 == (fd = ece391_open(arg3))) {
        ece391_fdputs(1, (uint8_t *)"Error! cannot open file!\n");
        return 3;
    }

    while (0 != (cnt = ece391_read(fd, text, 1024))){
        if (-1 == cnt) {
        ece391_fdputs (1, (uint8_t*)"file read failed\n");
        return 3;
        }
        len += cnt;
    }  

    // Append
    for (q = 0; q < arg1_length+1; q++){
        text[len+q] = arg1[q];
    }



    if (-1 == ece391_write(fd, text, len+arg1_length+1)) {
        ece391_fdputs(1, (uint8_t *)"Error! File write fail!\n");
        return 3;
    }

    return 0;
}

