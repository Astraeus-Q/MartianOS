#include <stdint.h>

#include "ece391support.h"
#include "ece391syscall.h"

int main ()
{
    int32_t fd, cnt;
    uint8_t buf[1024]; 

    if (0 != ece391_getargs (buf, 1024)) {
        ece391_fdputs (1, (uint8_t*)"Error! could not read arguments\n");
	    return 3;
    }

	if (0 == ece391_deletefile (buf)){
        ece391_fdputs (1, (uint8_t*)"Error! could not delete file\n");
	    return 3;
    }

    return 0;
}

