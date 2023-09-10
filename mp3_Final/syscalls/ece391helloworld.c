#include <stdint.h>

#include "ece391support.h"
#include "ece391syscall.h"

int main(){
    char buf[33] = "Hello World!\n";
    ece391_write (1, buf, 33);
    return 0;
}