/*********************************************************
*     File Name : NandFlash_sim.c
*     YIN  @ 2016.11.30
*     Description  : Initial create
**********************************************************/

/*********************************************************
*  Include
**********************************************************/
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>

#include "File_lib.h"
#include "Hardware.h"
#include "FILE_Load.h"
#include "NandFlash_sim.h"

uint8_t   gTempPageDataBuffer[2048];
uint8_t   gDataPageBuffer[2048+64];
uint16_t  gNandFlashChipCnt;

NAND_FILE_S*       gNandFiles = NULL;
NANDFLASH_CHIP_S   gNandFlashChips[MAX_NANDFLASH_CHIP_CNT];

uint8_t NandSetParam()
{
    gHW_NandFlashParam.mSectorsPerPage = 4;//SECTOR_CNT_PER_PAGE
    gHW_NandFlashParam.mBytesPerPage = gHW_NandFlashParam.mSectorsPerPage * BYTE_PER_MDSECTOR;//4*512 = 2048
    gHW_NandFlashParam.mBytesPerSpare = 64;
    gHW_NandFlashParam.mPagesPerBlock = 64;//PAGE_CNT_PER_BLK
    gHW_NandFlashParam.mPlanesPerChip = 2;//PLANE_CNT_PER_DIE
    gHW_NandFlashParam.mBlocksPerPlane = 1024;//BLK_CNT_PER_PLANE
    gHW_NandFlashParam.mBlocksPerChip = gHW_NandFlashParam.mPlanesPerChip*gHW_NandFlashParam.mBlocksPerPlane;//BLK_CNT_PER_DIE  2048
    gHW_NandFlashParam.mSizePerPage = gHW_NandFlashParam.mBytesPerPage + gHW_NandFlashParam.mBytesPerSpare; //2048 + 64

    gHW_LayoutParam.mChannelsPerArray = 1;//CHL_CNT
    gHW_LayoutParam.mChipsPerChannel = 1;//CE_CNT_PER_CHL
    gHW_LayoutParam.mDiesPerChip = 1;

    return 0;
}

static void GenerateChipFileName(uint32_t aChipAddr, char *apFileName)
{
    assert(apFileName);

    sprintf(apFileName, "%s/%08x_data.bin", gHW_NandFlashDir, aChipAddr);
}

static uint8_t ChipFileOpen(NAND_FILE_S* pChipFile)
{
    char filename[64];

    GenerateChipFileName(pChipFile->mChipAddr, filename);
    pChipFile->pFileHandle = MyFileCreate(filename, BYTE_PER_FILECHIP);
    return FileOpen(pChipFile->pFileHandle);
}

static uint32_t ConvertPageAddr(uint32_t aPageAddr)
{
    uint32_t pageNo, blkNo, planeNo;

    pageNo = aPageAddr % gHW_NandFlashParam.mPagesPerBlock;
    blkNo  = aPageAddr / gHW_NandFlashParam.mPagesPerBlock;

    planeNo  = blkNo / gHW_NandFlashParam.mBlocksPerPlane;
    blkNo  = blkNo % gHW_NandFlashParam.mBlocksPerPlane;

    blkNo += planeNo * (gHW_NandFlashParam.mBlocksPerChip / gHW_NandFlashParam.mPlanesPerChip);
    pageNo += blkNo * gHW_NandFlashParam.mPagesPerBlock;

    return pageNo;
}

static NANDFLASH_CHIP_S* GetNandFlashChip(uint32_t aChipAddr)
{
    uint16_t i;

    for (i=0; i<MAX_NANDFLASH_CHIP_CNT; i++)
    {
        if (aChipAddr == gNandFlashChips[i].mChipAddr)
        {
            return &gNandFlashChips[i];
        }
    }

    return NULL;
}

static uint32_t LoadFromFile(const char *apFileName, uint8_t *apBuf, uint32_t aLen)
{
#if 1
    FILE *fp;
    uint32_t length;

    assert(apBuf);
    assert(apFileName);

    fp = fopen(apFileName, WrFlag);
    if (fp == NULL)
    {
        return 0;
    }

    length = fwrite(apBuf, 1, aLen, fp);

    fclose(fp);

    return length;
#endif

    return 0;//no file
}

static void GenerateIDFileName(uint32_t aChipAddr, char *apFileName)
{
    assert(apFileName);

    sprintf(apFileName, "%s/%08x-ID.bin", gHW_NandFlashID, aChipAddr);
}

static void GenerateBBIMGFileName(uint32_t aChipAddr, char *apFileName)
{
    assert(apFileName);

    sprintf(apFileName, "%s/%08x-BadBlkImg.bin",gHW_NandFlashBBIMG, aChipAddr);
}

static void GenerateWRIMGFileName(uint32_t aChipAddr, char *apFileName)
{
    assert(apFileName);

    sprintf(apFileName, "%08x-WrPageIm.bin", aChipAddr);
}

static NAND_FILE_S* GetChipFile(uint32_t aChipAddr)
{
    uint32_t iChip;

    for (iChip=0; iChip<gNandFlashChipCnt; iChip++)
    {
        NAND_FILE_S* pChipFile = &gNandFiles[iChip];
        assert(pChipFile);
        if (pChipFile->mChipAddr == aChipAddr)
        {
            return pChipFile;
        }
    }

    return NULL;
}

static uint32_t NandReadDataPage(uint32_t aChipAddr, uint32_t aPageAddr, uint8_t* pDataBuffer, uint32_t aDataLen)
{
    uint32_t pageNo;
    uint32_t blockNo;
    NAND_FILE_S* pChipFile;

    pChipFile = GetChipFile(aChipAddr);
    assert(pChipFile);

    if(pChipFile != NULL)
    {
        pageNo = aPageAddr % gHW_NandFlashParam.mPagesPerBlock;
        blockNo = aPageAddr / gHW_NandFlashParam.mPagesPerBlock;

        FileRead(pChipFile->pFileHandle, blockNo*BYTE_PER_FILEBLOCK + pageNo*BYTE_PER_FILEPAGE, aDataLen, pDataBuffer);
    }

    return 0;
}

static uint32_t NandWriteDataPage(uint32_t aChipAddr, uint32_t aPageAddr, uint8_t* pDataBuffer, uint32_t aDataLen)
{
    uint32_t pageNo;
    uint32_t blockNo;
    NAND_FILE_S* pChipFile;

    pChipFile = GetChipFile(aChipAddr);
    assert(pChipFile);

    if(pChipFile != NULL)
    {
        pageNo = aPageAddr % gHW_NandFlashParam.mPagesPerBlock;
        blockNo = aPageAddr / gHW_NandFlashParam.mPagesPerBlock;

        FileWrite(pChipFile->pFileHandle, blockNo*BYTE_PER_FILEBLOCK + pageNo*BYTE_PER_FILEPAGE, aDataLen, pDataBuffer);
    }

    return 0;
}

static uint32_t NandReadSpare(uint32_t aChipAddr, uint32_t aPageAddr, uint8_t* pSpareBuffer, uint32_t aSpareLen)
{
    uint32_t pageNo;
    uint32_t blockNo;
    NAND_FILE_S* pChipFile;

    pChipFile = GetChipFile(aChipAddr);
    assert(pChipFile);

    if(pChipFile != NULL)
    {
        pageNo = aPageAddr % gHW_NandFlashParam.mPagesPerBlock;
        blockNo = aPageAddr / gHW_NandFlashParam.mPagesPerBlock;

        FileRead(pChipFile->pFileHandle, blockNo*BYTE_PER_FILEBLOCK + pageNo*BYTE_PER_FILEPAGE + BYTE_PER_FILEROWPAGE, aSpareLen, pSpareBuffer);
    }

    return 0;
}

static uint32_t NandWriteSpare(uint32_t aChipAddr, uint32_t aPageAddr, uint8_t* pSpareBuffer, uint32_t aSpareLen)
{
    uint32_t pageNo;
    uint32_t blockNo;
    NAND_FILE_S* pChipFile;

    pChipFile = GetChipFile(aChipAddr);
    assert(pChipFile);

    if(pChipFile != NULL)
    {
        pageNo = aPageAddr % gHW_NandFlashParam.mPagesPerBlock;
        blockNo = aPageAddr / gHW_NandFlashParam.mPagesPerBlock;

        FileWrite(pChipFile->pFileHandle, blockNo*BYTE_PER_FILEBLOCK + pageNo*BYTE_PER_FILEPAGE + BYTE_PER_FILEROWPAGE, aSpareLen, pSpareBuffer);
    }

    return 0;
}

static uint32_t NandEraseFromBlock(uint32_t aChipAddr, uint32_t aPageAddr)
{
    uint32_t blockNo;
    uint8_t* pBlockDataBuffer;
    NAND_FILE_S* pChipFile;

    pChipFile = GetChipFile(aChipAddr);
    assert(pChipFile);

    if (pChipFile != NULL)
    {
        blockNo = aPageAddr / gHW_NandFlashParam.mPagesPerBlock;
        pBlockDataBuffer = (uint8_t*)malloc(BYTE_PER_FILEBLOCK);
        memset(pBlockDataBuffer, 0xFF, BYTE_PER_FILEBLOCK);
        FileWrite(pChipFile->pFileHandle, blockNo*BYTE_PER_FILEBLOCK, BYTE_PER_FILEBLOCK, pBlockDataBuffer);
        free(pBlockDataBuffer);
    }

    return 0;
}

static uint32_t NandReadPage(uint32_t aChipAddr, uint32_t aPageAddr, uint8_t* pDataBuffer, uint8_t TypeOp)
{
    uint32_t ret;
    uint32_t PageNo;

    PageNo = ConvertPageAddr(aPageAddr);

    memset(gDataPageBuffer, 0x00, gHW_NandFlashParam.mSizePerPage);

    if(TypeOp == FULL_PAGE)
    {
        ret = NandReadDataPage(aChipAddr, PageNo, gDataPageBuffer, BYTE_PER_FILEPAGE);

        if(pDataBuffer != NULL)
        {
            memcpy(pDataBuffer, gDataPageBuffer, BYTE_PER_FILEPAGE);
        }
    }

    if(TypeOp == DATA_PAGE)
    {
        ret = NandReadDataPage(aChipAddr, PageNo, gDataPageBuffer, BYTE_PER_FILEROWPAGE);
        if(pDataBuffer != NULL)
        {
            memcpy(pDataBuffer, gDataPageBuffer, BYTE_PER_FILEROWPAGE);
        }
    }

    if(TypeOp == SPARE_PAGE)
    {
        ret = NandReadSpare(aChipAddr, PageNo, gDataPageBuffer, BYTE_PER_FILESPARE);
        if(pDataBuffer != NULL)
        {
            memcpy(pDataBuffer, &gDataPageBuffer[BYTE_PER_FILEROWPAGE], BYTE_PER_FILESPARE);
        }
    }

    return ret;
}

static uint32_t NandWritePage(uint32_t aChipAddr, uint32_t aPageAddr, uint8_t* pDataBuffer, uint8_t TypeOp)
{
    uint32_t ret;
    uint32_t PageNo;

    PageNo = ConvertPageAddr(aPageAddr);
    memset(gDataPageBuffer, 0x00, gHW_NandFlashParam.mSizePerPage);

    if(TypeOp == FULL_PAGE)
    {
        if(pDataBuffer != NULL)
        {
            memcpy(&gDataPageBuffer[0], pDataBuffer, BYTE_PER_FILEPAGE);
        }

        ret = NandWriteDataPage(aChipAddr, PageNo, gDataPageBuffer, BYTE_PER_FILEPAGE);
    }

    if(TypeOp == DATA_PAGE)
    {
        if(pDataBuffer != NULL)
        {
            memcpy(&gDataPageBuffer[0], pDataBuffer, BYTE_PER_FILEROWPAGE);
        }

        ret = NandWriteDataPage(aChipAddr, PageNo, gDataPageBuffer, BYTE_PER_FILEROWPAGE);
    }

    if(TypeOp == SPARE_PAGE)
    {
        if(pDataBuffer != NULL)
        {
            memcpy(&gDataPageBuffer[BYTE_PER_FILEROWPAGE], pDataBuffer, BYTE_PER_FILESPARE);
        }

        ret = NandWriteSpare(aChipAddr, PageNo, gDataPageBuffer, BYTE_PER_FILESPARE);
    }

    return ret;
}

static uint32_t NandEraseBlock(uint32_t aChipAddr, uint32_t aPageAddr)
{
    uint32_t PageNo = ConvertPageAddr(aPageAddr);

    //NandCheckPageBlank(aChipAddr, PageNo, 2);
    NandEraseFromBlock(aChipAddr, PageNo);

    return 0;
}

static int NandLoadID(uint32_t aChipAddr, uint8_t *apBuf, uint32_t aLen)
{
    char filename[64];

    assert(apBuf);
    assert(aLen > 0);

    GenerateIDFileName(aChipAddr, filename);

    printf("filename1:%0x\n", &filename);

    return LoadFromFile(filename, apBuf, aLen);
}

static int NandLoadBBIMG(uint32_t aChipAddr, uint8_t *apBuf, uint32_t aLen)
{
    char filename[64];

    assert(apBuf);
    assert(aLen > 0);

    GenerateBBIMGFileName(aChipAddr, filename);

    return LoadFromFile(filename, apBuf, aLen);
}

static int NandLoadWRIMG(uint32_t aChipAddr, uint8_t *apBuf, uint32_t aLen)
{
    char filename[64];

    assert(apBuf);
    assert(aLen > 0);

    GenerateWRIMGFileName(aChipAddr, filename);

    return LoadFromFile(filename, apBuf, aLen);
}

uint32_t NandLoadChip(uint32_t aChipAddr, NANDFLASH_CHIP_S* pNandFlashChip)
{
    uint8_t  NandID[MAX_NAND_ID_LENGTH];
    uint32_t aChipNum;
    NandID[0] = 0xAB;
    NandID[1] = 0x10;
    NandID[2] = 0x78;
    NandID[3] = 0xAB;
    NandID[4] = 0x10;
    NandID[5] = 0x78;

    assert(pNandFlashChip);

    aChipNum = (aChipAddr >> 24) + (((aChipAddr << 8)>>24) & 0xFF);

    pNandFlashChip->mChipNum  = aChipNum;
    pNandFlashChip->mChipAddr = aChipAddr;
    memset(pNandFlashChip->mBlkTypeImg, 0xFF, sizeof(pNandFlashChip->mBlkTypeImg));
    memset(pNandFlashChip->mBadBlkImg, 0xFF, sizeof(pNandFlashChip->mBadBlkImg));
    memset(pNandFlashChip->mWrPageImg, 0xFF, sizeof(pNandFlashChip->mWrPageImg));
    memset(pNandFlashChip->mPageRdCnt, 0x00, sizeof(pNandFlashChip->mPageRdCnt));
    memset(pNandFlashChip->mBlkErsCnt, 0x00, sizeof(pNandFlashChip->mBlkErsCnt));

    if (NandLoadID(aChipAddr, NandID, sizeof(NandID)) != sizeof(NandID))
    {
        return 1;
    }

    memcpy(pNandFlashChip->mID, NandID, MAX_NAND_ID_LENGTH);

    if (memcmp(NandID, pNandFlashChip->mID, sizeof(NandID)))
    {
        return 1;
    }
    printf("flashid:%x\n", pNandFlashChip->mID);


    NandLoadBBIMG(aChipAddr, pNandFlashChip->mBadBlkImg, sizeof(pNandFlashChip->mBadBlkImg));

    //NandLoadWRIMG(aChipAddr, (uint8_t*)pNandFlashChip->mWrPageImg, sizeof(pNandFlashChip->mWrPageImg));

    return 0;
}

uint32_t NandReadID(uint32_t aChipAddr, uint8_t* pID)
{
    NANDFLASH_CHIP_S* pNandFlashChip;

    pNandFlashChip = GetNandFlashChip(aChipAddr);

    memset(pID, 0xFF, MAX_NAND_ID_LENGTH);
    if (!pNandFlashChip)
    {
        return 1;
    }

    memcpy(pID, pNandFlashChip->mID, MAX_NAND_ID_LENGTH);

    return 0;
}

uint32_t NandSimRead(uint32_t aChipAddr, uint32_t aPageAddr, uint8_t* pDataBuffer, uint8_t TypeOp)
{
    uint32_t ret;
    //uint32_t blockNo;
    //uint32_t pageOffset;
    NANDFLASH_CHIP_S* pNandFlashChip;

    pNandFlashChip = GetNandFlashChip(aChipAddr);

    //blockNo = aPageAddr / gHW_NandFlashParam.mPagesPerBlock;
    //pageOffset = aPageAddr % gHW_NandFlashParam.mPagesPerBlock;

    ret = NandReadPage(aChipAddr, aPageAddr, pDataBuffer, TypeOp);

    //pNandFlashChip->mPageRdCnt[blockNo][pageOffset]++;

    return ret;
}

uint32_t NandSimWrite(uint32_t aChipAddr, uint32_t aPageAddr, uint8_t* pDataBuffer, uint8_t TypeOp)
{
    uint32_t ret;
    //uint32_t blockNo;
    //uint32_t pageOffset;
    NANDFLASH_CHIP_S* pNandFlashChip;

    pNandFlashChip = GetNandFlashChip(aChipAddr);

    //blockNo = aPageAddr / gHW_NandFlashParam.mPagesPerBlock;
    //pageOffset = aPageAddr % gHW_NandFlashParam.mPagesPerBlock;

    ret = NandWritePage(aChipAddr, aPageAddr, pDataBuffer, TypeOp);

    return ret;
}

uint32_t NandSimErase(uint32_t aChipAddr, uint32_t aPageAddr)
{
    uint32_t ret;
    uint32_t blockNo;
    NANDFLASH_CHIP_S* pNandFlashChip;

    pNandFlashChip = GetNandFlashChip(aChipAddr);

    blockNo = aPageAddr / gHW_NandFlashParam.mPagesPerBlock;

    ret = NandEraseBlock(aChipAddr, aPageAddr);

    pNandFlashChip->mBlkErsCnt[blockNo]++;
    memset((uint8_t*)&pNandFlashChip->mPageRdCnt[blockNo], 0x00, MAX_PAGES_PER_BLOCK*sizeof(uint32_t));
    return ret;
}

static void NandFlashSetup()
{
    uint8_t iDie;
    uint8_t iIndex;
    uint16_t iChip;
    uint8_t iChannel;

    memset(gNandFlashChips, 0xFF, sizeof(gNandFlashChips));
    gNandFlashChipCnt = gHW_LayoutParam.mChannelsPerArray *
                        gHW_LayoutParam.mChipsPerChannel *
                        gHW_LayoutParam.mDiesPerChip;

    iIndex = 0;
    for (iChannel=0; iChannel<gHW_LayoutParam.mChannelsPerArray; iChannel++)
    {
        for (iChip=0; iChip<gHW_LayoutParam.mChipsPerChannel; iChip++)
        {
            for (iDie=0; iDie<gHW_LayoutParam.mDiesPerChip; iDie++)
            {
                NandLoadChip(CHIP_ADDR(iChannel, iChip, iDie), &gNandFlashChips[iIndex]);
                iIndex++;
            }
        }
    }
}

static uint32_t NandFilesInit()
{
    uint8_t  iDie;
    uint8_t  index;
    uint8_t  iChannel;
    uint16_t iChip;

    memset(gTempPageDataBuffer, PAGE_META_DATA, sizeof(gTempPageDataBuffer));
    gNandFiles = (NAND_FILE_S*)malloc(sizeof(NAND_FILE_S)*gNandFlashChipCnt);
    memset(gNandFiles, 0xFF, sizeof(NAND_FILE_S)*gNandFlashChipCnt);

    index = 0;
    for (iChannel=0; iChannel<gHW_LayoutParam.mChannelsPerArray; iChannel++)
    {
        for (iChip=0; iChip<gHW_LayoutParam.mChipsPerChannel; iChip++)
        {
            for (iDie=0; iDie<gHW_LayoutParam.mDiesPerChip; iDie++)
            {
                gNandFiles[index].mChipAddr = CHIP_ADDR(iChannel, iChip, iDie);
                if (ChipFileOpen(&gNandFiles[index]) == FAILURE)
                {
                    return 1;
                }
                index++;
            }
        }
    }

    return 0;

}

void NandSimInit()
{
    NandFlashSetup();
    NandFilesInit();
    //BasicNand_InitBlockList();
    //SuperNand_InitChipFiles();
    //NandEraseCntLogOpen();
}
