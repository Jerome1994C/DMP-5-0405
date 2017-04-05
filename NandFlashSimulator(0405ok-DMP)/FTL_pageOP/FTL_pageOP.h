/*********************************************************
*     File Name : FTL_pageOP.h
*     YIN  @ 2017.03.31
*     Description  : Initial create(SOFTWARE LAYER OPERATE PAGE)
*     Revise-01: 2017.4.1
**********************************************************/
#ifndef _FTL_PAGEOP_H_
#define _FTL_PAGEOP_H_
/*********************************************************
*  Include
**********************************************************/
#include "Driver.h"
#include "FTL_Struct.h"

/*Function*/

//读取一页数据
uint8_t pageOP_Read(uint32_t PageAddrBase, uint8_t *pBuffer);

//写入一页数据
uint8_t pageOP_Write(uint32_t PageAddrBase, uint8_t *pBuffer);

//读取spare区
uint8_t pageOP_ReadSpare(uint32_t PageAddrBase, SPARE_TAG_TYPE *SpareTag);

//写入spare区
uint8_t pageOP_WriteSpare(uint32_t PageAddrBase, SPARE_TAG_TYPE *SpareTag);

uint8_t pageOP_ReadData(uint32_t PageAddrBase, uint8_t *pBuffer);

uint8_t pageOP_WriteData(uint32_t PageAddrBase, uint8_t *pBuffer);

uint8_t pageOP_EraseBlk(uint16_t Blk_Num);

#endif
