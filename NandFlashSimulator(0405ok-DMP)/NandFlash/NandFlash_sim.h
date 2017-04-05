/*********************************************************
*     File Name : NandFlash_sim.h
*     YIN  @ 2016.11.30
*     Description  : Initial create
**********************************************************/
#ifndef _NANDFLASH_SIM_H_
#define _NANDFLASH_SIM_H_
/*********************************************************
*  Include
**********************************************************/
#include <stdint.h>
#include "MyFile.h"
#include "Hardware.h"

enum {DATA_TYPE_META, DATA_TYPE_USER};

typedef struct _NAND_FILE
{
    uint32_t       mChipAddr;
    FILE_HANDLE_S* pFileHandle;
}NAND_FILE_S;

typedef struct _NANDFLASH_CHIP
{
    uint32_t  mChipAddr;
    uint32_t  mChipNum;
    uint8_t   mID[MAX_NAND_ID_LENGTH];
	uint8_t   mBadBlkImgDirty;
	uint8_t   mWrPageImgDirty;
	uint8_t   mBlkTypeImg[MAX_BLOCKS_PER_CHIP/8];
	uint8_t   mBadBlkImg[MAX_BLOCKS_PER_CHIP/8];
	uint8_t   mWrPageImg[MAX_BLOCKS_PER_CHIP][MAX_PAGES_PER_BLOCK/8];
	uint32_t  mPageRdCnt[MAX_BLOCKS_PER_CHIP][MAX_PAGES_PER_BLOCK];
	uint32_t  mBlkErsCnt[MAX_BLOCKS_PER_CHIP];
}NANDFLASH_CHIP_S;

extern uint8_t   NandSetParam();
extern void      NandSimInit();

//extern uint32_t  NandIsBadBlock(uint32_t aChipAddr, uint32_t aRowAddr);
//extern uint32_t  NandMarkBadBlock(uint32_t aChipAddr, uint32_t aRowAddr);
extern uint32_t  NandCheckPageBlank(uint32_t aChipAddr, uint32_t aRowAddr, uint8_t aIsWr);

extern uint32_t  NandLoadChip(uint32_t aChipAddr, NANDFLASH_CHIP_S* pNandFlashChip);
extern uint32_t  NandSaveChip(uint32_t aChipAddr, NANDFLASH_CHIP_S* pNandFlashChip);

extern uint32_t  NandReadID(uint32_t aChipAddr, uint8_t* pID);
extern uint32_t  NandSimRead(uint32_t aChipAddr, uint32_t aPageAddr, uint8_t* pDataBuffer, uint8_t TypeOp);
extern uint32_t  NandSimWrite(uint32_t aChipAddr, uint32_t aPageAddr, uint8_t* pDataBuffer, uint8_t TypeOp);
extern uint32_t  NandSimErase(uint32_t aChipAddr, uint32_t aPageAddr);
#endif
