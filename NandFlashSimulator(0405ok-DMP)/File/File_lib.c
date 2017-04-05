/*********************************************************
*     File Name : File_lib.c
*     YIN  @ 2016.12.03
*     Description  : Initial create
**********************************************************/
#include <stdio.h>
#include <stdlib.h>

#include "File_lib.h"

int TestDeleteDir(const char *dir)
{
#if 1
    char cmd[1024];

    sprintf(cmd, "%s %s 1>nul 2>nul", FL_RMDIR, dir);

    return system(cmd);
#else
    return _rmdir(dir);
#endif
}
