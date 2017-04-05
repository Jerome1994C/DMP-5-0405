#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include "MyFile.h"
#include "Driver.h"
#include "File_lib.h"
#include "Hardware.h"
#include "FileAccess.h"
#include "FTL_Struct.h"
#include "NandFlash_sim.h"

#define TEST_BUFFER_LEN  256

extern NANDFLASH_CHIP_S  gNandFlashChips[MAX_NANDFLASH_CHIP_CNT];


#if 0
int main()
{
    FILE_HANDLE_S* pFile = NULL;
    uint8_t   readBuf[TEST_BUFFER_LEN];
    uint8_t   writeBuf[TEST_BUFFER_LEN];

    uint64_t  fileSize = (uint64_t)8*(uint64_t)(1024*1024*1024);

    pFile = MyFileCreate("test_file.txt", fileSize);

    memset(readBuf, 0, TEST_BUFFER_LEN);
    memset(writeBuf, 1, TEST_BUFFER_LEN);

    FileOpen(pFile);
    FileWrite(pFile, 0, TEST_BUFFER_LEN, writeBuf);
    FileClose(pFile);

    FileOpen(pFile);
    FileRead(pFile, 0, TEST_BUFFER_LEN, readBuf);
    FileClose(pFile);

    MyFileDestroy(pFile);

    //assert_int_equal(memcmp(readBuf, writeBuf, TEST_BUFFER_LEN), 0);
}
#endif

#if 0
int main()
{
    uint32_t chipaddr;
    uint32_t pageaddr;
    uint8_t FlashID[MAX_NAND_ID_LENGTH];
    uint8_t wrBuff[BYTE_PER_FILEPAGE];
    uint8_t rdBuff[BYTE_PER_FILEROWPAGE];
    uint8_t *p, *pr, *pw;

    chipaddr = 10;
    pageaddr = MAX_PAGES_PER_BLOCK*2;

    NandSetParam();
    NandSimInit();

    printf("Nand load over\n");

    memset(FlashID, 0, sizeof(FlashID));
    NandReadID(gNandFlashChips, FlashID);
    if(FlashID[0] == 0)
    {
        printf("This not my nandflash\n");
        assert(0);
    }

    for(p=FlashID; p<FlashID+MAX_NAND_ID_LENGTH; p++)
    {
        printf("%x\n", *p);
    }
    printf("My nandflash ID is %x\n", FlashID);

    NandSimErase(chipaddr, pageaddr);
    printf("Eraser over\n");

    memset(wrBuff, 0, sizeof(wrBuff));
    NandSimRead(chipaddr, pageaddr, rdBuff, NULL);
    assert(rdBuff);
    printf("Read over!\n");
    for(pr = rdBuff; pr < rdBuff+BYTE_PER_FILEROWPAGE; pr++)
    {
        printf("%o\n", *pr);
    }

    memset(wrBuff, 0x5a, sizeof(wrBuff));
    NandSimWrite(chipaddr, pageaddr, wrBuff, NULL);
    printf("Write over!\n");
    for(pw = wrBuff; pw < wrBuff+BYTE_PER_FILEROWPAGE; pw++)
    {
        printf("%x\n", *pw);
    }

    NandSimRead(chipaddr, pageaddr, rdBuff, NULL);
    printf("Read over!\n");
    for(pr = rdBuff; pr < rdBuff+BYTE_PER_FILEROWPAGE; pr++)
    {
        printf("%x\n", *pr);
    }

    NandSimErase(chipaddr, pageaddr);
    printf("Erase over!\n");

    NandSimRead(chipaddr, pageaddr, rdBuff, NULL);
    for(pr = rdBuff; pr < rdBuff+BYTE_PER_FILEROWPAGE; pr++)
    {
        printf("%x\n", *pr);
    }

    return 0;
    //assert_int_equal(memcmp(readBuf, writeBuf, TEST_BUFFER_LEN), 0);
}
#endif

int main()
{
    uint32_t chipaddr;
    uint32_t pageNo;
    uint8_t FlashID[MAX_NAND_ID_LENGTH];
    uint8_t wrBuff[BYTE_PER_FILEPAGE];
    uint8_t rdBuff[BYTE_PER_FILEROWPAGE];
    uint8_t *p, *pr, *pw;

    //chipaddr = 10;
    pageNo = 100;

    NandSetParam();
    NandSimInit();

    printf("Nand load over\n");

    memset(FlashID, 0, sizeof(FlashID));
    Nand_ReadID(pageNo, FlashID);
    //NandReadID(gNandFlashChips, FlashID);
    if(FlashID[0] == 0)
    {
        printf("This not my nandflash\n");
        assert(0);
    }

    for(p=FlashID; p<FlashID+MAX_NAND_ID_LENGTH; p++)
    {
        printf("%x\n", *p);
    }
    printf("My nandflash ID is %0x\n", FlashID);

    Nand_EraseBlock(pageNo);
    printf("Eraser over\n");

    memset(wrBuff, 0, sizeof(wrBuff));
    //NandSimRead(chipaddr, pageaddr, rdBuff, NULL);
    Nand_ReadData(rdBuff, pageNo, 0, 16);
    //assert(rdBuff);
    printf("Read over!\n");
    for(pr = rdBuff; pr < rdBuff+BYTE_PER_FILEROWPAGE; pr++)
    {
        printf("%o\n", *pr);
    }

    memset(wrBuff, 0x5a, sizeof(wrBuff));
    //NandSimWrite(chipaddr, pageaddr, wrBuff, NULL);
    Nand_WriteData(wrBuff, pageNo, 0, 16);
    printf("Write over!\n");
    for(pw = wrBuff; pw < wrBuff+BYTE_PER_FILEROWPAGE; pw++)
    {
        printf("%x\n", *pw);
    }

    Nand_ReadData(rdBuff, pageNo, 0, 16);
    printf("Read over!\n");
    for(pr = rdBuff; pr < rdBuff+BYTE_PER_FILEROWPAGE; pr++)
    {
        printf("%x\n", *pr);
    }

    Nand_EraseBlock(pageNo);
    printf("Erase over!\n");

    Nand_ReadData(rdBuff, pageNo, 0, 16);
    for(pr = rdBuff; pr < rdBuff+BYTE_PER_FILEROWPAGE; pr++)
    {
        printf("%x\n", *pr);
    }

    return 0;
    //assert_int_equal(memcmp(readBuf, writeBuf, TEST_BUFFER_LEN), 0);
}
