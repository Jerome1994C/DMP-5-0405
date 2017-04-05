/*********************************************************
*     File Name : Driver.c
*     YIN  @ 2016.12.15
*     Description  : Initial create
**********************************************************/
#include <string.h>
#include <stdio.h>
#include "Driver.h"
#include "Hardware.h"

/* �߼����ӳ����ÿ�������2%���ڱ��������������ά������1024�� LUT = Look Up Table */
//static uint16_t m_LUT[NAND_BLOCK_COUNT];

static uint8_t  gDataPageBuffer[MAX_BYTES_PER_PAGE];

uint32_t Nand_ReadID(uint32_t PageNo, uint8_t* pID)
{
    uint32_t ret;
    uint32_t mChipNo;
    uint32_t mChipAddr;

    mChipNo = PageNo / PAGES_PER_CHIP;
    mChipAddr = ((mChipNo / 8)<<24) + ((mChipNo % 8)<<8);

    ret = NandReadID(mChipAddr, pID);

    return ret;
}

uint32_t Nand_Read(uint8_t* pDataBuffer, uint32_t PageNo, uint16_t AddrInPage, uint16_t ByteCount)
{
    uint16_t i;
    uint32_t ret;
    uint32_t mChipNo;
    uint32_t mChipAddr;
    uint32_t mPageAddr;

    memset(gDataPageBuffer, 0x00, MAX_BYTES_PER_PAGE);

    /*�߼�ҳ�� ת��Ϊ ʵ�ʴ洢��ַ*/
    mChipNo = PageNo / PAGES_PER_CHIP;
    mChipAddr = ((mChipNo / 8)<<24) + ((mChipNo % 8)<<8);

    mPageAddr = PageNo - (mChipNo * PAGES_PER_CHIP);
    mPageAddr += AddrInPage;

    ret = NandSimRead(mChipAddr, mPageAddr, gDataPageBuffer, FULL_PAGE);

    for(i = 0; i < ByteCount; i++)
    {
        pDataBuffer[i] = gDataPageBuffer[i];
    }

    return ret;
}

uint32_t Nand_ReadData(uint8_t* pDataBuffer, uint32_t PageNo, uint16_t AddrInPage, uint16_t ByteCount)
{
    uint16_t i;
    uint32_t ret;
    uint32_t mChipNo;
    uint32_t mChipAddr;
    uint32_t mPageAddr;

    memset(gDataPageBuffer, 0x00, MAX_BYTES_PER_PAGE);

    /*�߼�ҳ�� ת��Ϊ ʵ�ʴ洢��ַ*/
    mChipNo = PageNo / PAGES_PER_CHIP;
    mChipAddr = ((mChipNo / 8)<<24) + ((mChipNo % 8)<<8);

    mPageAddr = PageNo - (mChipNo * PAGES_PER_CHIP);
    mPageAddr += AddrInPage;

    ret = NandSimRead(mChipAddr, mPageAddr, gDataPageBuffer, DATA_PAGE);

    for(i = 0; i < ByteCount; i++)
    {
        pDataBuffer[i] = gDataPageBuffer[i];
    }

    return ret;
}

/**
*PS: ���� AddrInPage ͨ������Ϊ0����ʾ��ҳ��ƫ��Ϊ 0
*ע�⣺�����ȡ SPARE �����ݣ�AddrInPage += BYTE_PER_FILEROWPAGE��
*/

uint32_t Nand_ReadSpare(uint8_t* pDataBuffer, uint32_t PageNo, uint16_t AddrInPage, uint16_t ByteCount)
{
    uint16_t i;
    uint32_t ret;
    uint32_t mChipNo;
    uint32_t mChipAddr;
    uint32_t mPageAddr;

    memset(gDataPageBuffer, 0x00, MAX_BYTES_PER_PAGE);

    mChipNo = PageNo / PAGES_PER_CHIP;
    mChipAddr = ((mChipNo / 8)<<24) + ((mChipNo % 8)<<8);

    mPageAddr = PageNo - (mChipNo * PAGES_PER_CHIP);
    mPageAddr += AddrInPage;

    ret = NandSimRead(mChipAddr, mPageAddr, gDataPageBuffer, SPARE_PAGE);//1�����ȡSPARE����

    for(i = 0; i < ByteCount; i++)
    {
        pDataBuffer[i] = gDataPageBuffer[i];
    }

    return ret;
}

uint32_t Nand_Write(uint8_t* pDataBuffer, uint32_t PageNo, uint16_t AddrInPage, uint16_t ByteCount)
{
    uint16_t i;
    uint32_t ret;
    uint32_t mChipNo;
    uint32_t mChipAddr;
    uint32_t mPageAddr;

    memset(gDataPageBuffer, 0x00, MAX_BYTES_PER_PAGE);

    mChipNo = PageNo / PAGES_PER_CHIP;
    mChipAddr = ((mChipNo / 8)<<24) + ((mChipNo % 8)<<8);

    mPageAddr = PageNo - (mChipNo * PAGES_PER_CHIP);
    mPageAddr += AddrInPage;

    ret = NandSimWrite(mChipAddr, mPageAddr, pDataBuffer, FULL_PAGE);

    for(i = 0; i < ByteCount; i++)
    {
        gDataPageBuffer[i] = pDataBuffer[i];
    }

    return ret;
}

uint32_t Nand_WriteData(uint8_t* pDataBuffer, uint32_t PageNo, uint16_t AddrInPage, uint16_t ByteCount)
{
    uint16_t i;
    uint32_t ret;
    uint32_t mChipNo;
    uint32_t mChipAddr;
    uint32_t mPageAddr;

    memset(gDataPageBuffer, 0x00, MAX_BYTES_PER_PAGE);

    mChipNo = PageNo / PAGES_PER_CHIP;
    mChipAddr = ((mChipNo / 8)<<24) + ((mChipNo % 8)<<8);

    mPageAddr = PageNo - (mChipNo * PAGES_PER_CHIP);
    mPageAddr += AddrInPage;

    ret = NandSimWrite(mChipAddr, mPageAddr, pDataBuffer, DATA_PAGE);

    for(i = 0; i < ByteCount; i++)
    {
        gDataPageBuffer[i] = pDataBuffer[i];
    }

    return ret;
}

uint32_t Nand_WriteSpare(uint8_t* pDataBuffer, uint32_t PageNo, uint16_t AddrInPage, uint16_t ByteCount)
{
    uint16_t i;
    uint32_t ret;
    uint32_t mChipNo;
    uint32_t mChipAddr;
    uint32_t mPageAddr;

    memset(gDataPageBuffer, 0x00, MAX_BYTES_PER_PAGE);

    mChipNo = PageNo / PAGES_PER_CHIP;
    mChipAddr = ((mChipNo / 8)<<24) + ((mChipNo % 8)<<8);

    mPageAddr = PageNo - (mChipNo * PAGES_PER_CHIP);
    mPageAddr += AddrInPage;

    ret = NandSimWrite(mChipAddr, mPageAddr, pDataBuffer, SPARE_PAGE);

    for(i = 0; i < ByteCount; i++)
    {
        gDataPageBuffer[i] = pDataBuffer[i];
    }

    return ret;
}

/**
PS: �ҵ���ҳ��Ӧ�Ŀ飬ɾ���˿�
*/
uint32_t Nand_EraseBlock(uint32_t PageNo)
{
    uint32_t ret;
    uint32_t mChipNo;
    uint32_t mChipAddr;
    uint32_t mPageAddr;

    mChipNo = PageNo / BLOCKS_PER_CHIP;
    mChipAddr = ((mChipNo / 8)<<24) + ((mChipNo % 8)<<8);

    mPageAddr = PageNo - (mChipNo * PAGES_PER_CHIP);

    ret = NandSimErase(mChipAddr, mPageAddr);

    return ret;
}

uint32_t Nand_PageCopy(uint8_t* pDataBuffer, uint32_t SrcPageNo, uint32_t TarPageNo, uint16_t AddrInPage, uint16_t ByteCount)
{
    uint16_t m16i;
    uint32_t ret;
    uint32_t mrChipNo, mrChipAddr, mrPageAddr;
    uint32_t mwChipNo, mwChipAddr, mwPageAddr;

    memset(gDataPageBuffer, 0x00, MAX_BYTES_PER_PAGE);

    mrChipNo = SrcPageNo / PAGES_PER_CHIP;
    mrChipAddr = ((mrChipNo / 8)<<24) + ((mrChipNo % 8)<<8);
    mrPageAddr = SrcPageNo - (mrChipNo * PAGES_PER_CHIP);
    mrPageAddr += AddrInPage;

    mwChipNo = TarPageNo / PAGES_PER_CHIP;
    mwChipAddr = ((mwChipNo / 8)<<24) + ((mwChipNo % 8)<<8);

    mwPageAddr = TarPageNo - (mwChipNo * PAGES_PER_CHIP);

    ret = NandSimRead(mrChipAddr, mrPageAddr, gDataPageBuffer, 0);

    for(m16i = 0; m16i < ByteCount; m16i++)
    {
        pDataBuffer[m16i] = gDataPageBuffer[m16i];
    }

    if(ret == 0)
    {
        memset(gDataPageBuffer, 0x00, MAX_BYTES_PER_PAGE);

        ret = NandSimWrite(mwChipAddr, mwPageAddr, pDataBuffer, 0);

        for(m16i = 0; m16i < ByteCount; m16i++)
        {
            gDataPageBuffer[m16i] = pDataBuffer[m16i];
        }
    }

    return ret;
}
