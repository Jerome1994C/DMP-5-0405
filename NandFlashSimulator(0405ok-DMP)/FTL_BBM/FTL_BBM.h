/*********************************************************
*     File Name : FTL_BBM.h
*     YIN  @ 2016.12.27
*     Description  : Initial create
*     Revise-01: 2017.3.2
*     Revise-02: 2017.4.1
**********************************************************/
#ifndef _FTL_BBM_H_
#define _FTL_BBM_H_
/*********************************************************
*  Include
**********************************************************/
#include <stdint.h>
#include "Hardware.h"

#define NAND_BLOCKS_BBM        (NAND_BLOCK_COUNT/2)

#define GOOD_BLK   0
#define BAD_BLK    1

#define BUSY_BLK   0/*坏块或已被占用的块*/
#define FREE_BLK   1

#define BLKINFO_BAD_OFFSET         (BYTE_PER_FILEROWPAGE + 0)
#define BLKINFO_USED_OFFSET        (BYTE_PER_FILEROWPAGE + 1)

#define NAND_USED_BLOCK_FLAG    0xF0
#define NAND_BAD_BLOCK_FLAG     0x00

/*Funcation*/
uint8_t NAND_BuildBBT(void);
uint8_t NAND_IsBadBlock(uint32_t m32BlkIdx);
uint32_t NAND_IsFreeBlock(uint32_t m32BlkIdx);
uint8_t NAND_MarkUsedBlock(uint32_t m32BlkIdx);

#endif
