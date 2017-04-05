/*********************************************************
*     File Name : bitmap.c
*     YIN  @ 2016.12.12
*     Description  : Initial create
**********************************************************/
#include "bitmap.h"

/**
 * BitmapTestBit() - Test bit status(whether 1).
 * startaddr: start address of bitmap buffer
 * offset:    bit offset of bitmap buffer
*/
unsigned int BitmapTestBit(void *startaddr, unsigned int offset)
{
    unsigned int t;

    t = ((unsigned char *)startaddr)[offset/8];
    t &= ( 1 << (offset%8) );
    return t;
}

/**
 * BitMapClearBit() - Clear bit(to 0)
 * startaddr: start address of bitmap buffer
 * offset:    bit offset of bitmap buffer
 */
void BitmapClrBit(void *startaddr, unsigned int offset)
{
    unsigned int t;

    t = ((unsigned char *)startaddr)[offset/8];
    t &= ~( 1 << (offset%8) );
    ((unsigned char *)startaddr)[offset/8] = t;
}
/**
 * BitMapSetBit() - Set bit(to 1)
 * startaddr: start address of bitmap buffer
 * offset:    bit offset of bitmap buffer
 */
void BitmapSetBit(uint8_t *startaddr, unsigned int offset)
{
    unsigned int t;

    t = ((uint8_t *)startaddr)[offset/8];
    t |= ( 1 << (offset%8) );
    ((unsigned char *)startaddr)[offset/8] = t;
}
