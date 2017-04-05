/*********************************************************
*     File Name : bitmap.h
*     YIN  @ 2016.12.12
*     Description  : Initial create
**********************************************************/
#ifndef __BITMAP_H
#define __BITMAP_H

#include <stdint.h>

extern uint32_t BitmapTestBit(void *startaddr, uint32_t offset);
extern void     BitmapClrBit(void *startaddr, uint32_t offset);
extern void     BitmapSetBit(unsigned char *startaddr, uint32_t offset);
#endif
