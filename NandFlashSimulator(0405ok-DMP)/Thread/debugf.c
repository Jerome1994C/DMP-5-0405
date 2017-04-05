/***************************************************************************
    begin                : Tue Oct 14 2003
    copyright            : (C) 2003 - 2009 by Alper Akcan
    email                : alper.akcan@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License as        *
 *   published by the Free Software Foundation; either version 2.1 of the  *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 ***************************************************************************/
#include "thread.h"
#include <stdio.h>
#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#if defined(CONFIG_PLATFORM_PSPDEV)
	#include <pspdebug.h>
	/* for psplink
	#define printf	pspDebugScreenPrintf
	*/
#endif

void s_debug_debugf (unsigned short flags, char *file, int line, char *func, char *fmt, ...)
{
	int n;
	int s;
	char *p;
	va_list args;

#if !defined(CONFIG_DEBUG)
	if ((flags & DFAT) == 0) {
		return;
	}
#endif
/*
	fprintf(stdout, "[0x%08X] ", s_thread_self());

	if (flags & DFAT) { fprintf(stdout, "FATAL : ");   }
	if (flags & DSYS) { fprintf(stdout, "SYSERR : ");  }
	if (flags & DSER) { fprintf(stdout, "SERVER :: "); }
	if (flags & DCLI) { fprintf(stdout, "CLIENT :: "); }
*/
	s = 100;
	if ((p = s_malloc(sizeof(char) * s)) == NULL) {
		goto err;
	}

	while (1) {
		va_start(args, fmt);
		n = vsnprintf(p, s, fmt, args);
		va_end(args);
		if (n > -1 && n < s) {
			break;
		}
		if (n > -1) {
			s = n + 1;
		} else {
			s *= 2;
		}
		if ((p = s_realloc(p, s))  == NULL) {
			goto err;
		}
	}

	//fprintf(stdout, p);
	s_free(p);

	if (flags & DSYS) {
		fprintf(stdout, " : %s", strerror(errno));
	}
	//fprintf(stdout, " [%s (%s:%d)]\n", func, file, line);
	if (flags & DFAT) {
		goto err;
	}
	return;
err: exit(1);
}



void DumpBuf(void * start, uint32_t size)
{
    int i;
    unsigned char *addr = start;
	char c;

    for(i = 0; i < size; i++)
    {

        if( (i & 0xF) == 0 )
        {
			printf("\n%08X: ", (uint32_t)addr);
        }
		c = ( (i & 0xF) == 7) ? '-': ' ';

		printf("%02x%c", *addr, c);
        addr++;
    }
    printf("\n");
}
