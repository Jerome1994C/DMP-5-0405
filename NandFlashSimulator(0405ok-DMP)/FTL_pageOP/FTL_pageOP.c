/*********************************************************
*     File Name : FTL_pageOP.c
*     YIN  @ 2017.03.31
*     Description  : Initial create
**********************************************************/
#include <string.h>
#include <stdio.h>
#include "Driver.h"
#include "Hardware.h"
#include "FTL_ECC.h"
#include "FTL_pageOP.h"
#include "FTL_Struct.h"

/**
*读取一页的数据
*参数1：@PageAddrBase: 上层写入基地址
*参数2：@Buffer:
*返回值：0 正常， 1 不正常
*/
uint8_t pageOP_Read(uint32_t PageAddrBase, uint8_t *pBuffer)
{
    uint8_t m8i = 0;
    uint8_t ret = 0;
    uint32_t PageNo;
    uint16_t AddrInPage;

    SPARE_TAG_TYPE  SpareTag;
    uint8_t ECC[3];

    PageNo = PageAddrBase;
    AddrInPage = PageNo % BYTE_PER_FILEPAGE;

    for(m8i = 0; m8i < 3; m8i++)
    {
        //读取数据
        Nand_ReadData(pBuffer, PageNo, AddrInPage, BYTE_PER_FILEROWPAGE);

        if(pBuffer == NULL)
        {
            return 1;
        }

        //计算读出来数据的ECC
        nand_calculate_ecc(pBuffer, ECC);

        //读取ECC码
        ret = pageOP_ReadSpare(PageAddrBase, &SpareTag);

        if(ret != 0)
        {
            printf("%s: READ SPARE WRONG!\n", __func__);
            return 1;
        }

        //判断读取是否正确
        if((ECC[0] == SpareTag.ecc[0]) && (ECC[1] == SpareTag.ecc[1]) && (ECC[2] == SpareTag.ecc[2]))
            break;

    }

    //读取错误
    if((ECC[0] != SpareTag.ecc[0]) || (ECC[1] != SpareTag.ecc[1]) || (ECC[2] != SpareTag.ecc[2]))
    {
        return 1;
    }

    return 0;
}

/**
*写入一页的数据
*参数1：@PageAddrBase: 上层写入基地址
*参数2：@Buffer:
*返回值：0 正常， 1 不正常
*/
uint8_t pageOP_Write(uint32_t PageAddrBase, uint8_t *pBuffer)
{
    uint32_t PageNo;
    uint16_t AddrInPage;

    SPARE_TAG_TYPE  SpareTag;
    uint8_t ECC[3];

    PageNo = PageAddrBase;
    AddrInPage = PageNo % BYTE_PER_FILEPAGE;

    //计算出ECC的值
    nand_calculate_ecc(pBuffer, SpareTag.ecc);

    //设计好 SPARE 区的数据
    SpareTag.spare_Erase_Time = 0xFFFFFFFF;
    SpareTag.spare_Flg_Bi = 0xFF;
    SpareTag.spare_Flg_Dirty = 0xFF;
    SpareTag.spare_Flg_Used = 0xF0;
    SpareTag.spare_Page_Num = 0xFFFFFFFF;
    SpareTag.Reserved = 0xFF;

    //写入数据
    Nand_WriteData(pBuffer, PageNo, AddrInPage, BYTE_PER_FILEROWPAGE);
    pageOP_WriteSpare(PageAddrBase, &SpareTag);

    //再读取判断是否写入正常
    Nand_ReadData(pBuffer, PageNo, AddrInPage, BYTE_PER_FILEROWPAGE);
    pageOP_ReadSpare(PageAddrBase, &SpareTag);

    nand_calculate_ecc(pBuffer, ECC);

    //判断写入正确
    if((ECC[0] == SpareTag.ecc[0]) && (ECC[1] == SpareTag.ecc[1]) && (ECC[2] == SpareTag.ecc[2]))
        return 0;

    //若为坏块，则将其标记为坏块，返回错误码
    else
    {
        SpareTag.spare_Flg_Bi = 0x00;
        pageOP_WriteSpare(PageAddrBase, &SpareTag);
        return 1;
    }
}

/**
*读取spare区
*参数1：@PageAddrBase: 上层写入基地址
*参数2：@SpareTag: 装spare区的数据
*/

uint8_t pageOP_ReadSpare(uint32_t PageAddrBase, SPARE_TAG_TYPE *SpareTag)
{
    uint32_t PageNo;
    uint16_t AddrInPage;
    //uint16_t Block_Addr;
    //uint16_t Page_Addr;

    //将 PageAddrBase转化为实际写入的基地址(针对一颗NAND)
    //Block_Addr = (uint16_t)(PageAddrBase / MAX_PAGES_PER_BLOCK);
    //Page_Addr = (uint16_t)(PageAddrBase % MAX_PAGES_PER_BLOCK);
    //PageNo = (uint32_t)(Block_Addr * BYTE_PER_FILEBLOCK) + (Page_Addr * BYTE_PER_FILEPAGE);
    //PageNo += BYTE_PER_FILEROWPAGE;
    PageNo = PageAddrBase;
    AddrInPage = PageNo % BYTE_PER_FILEPAGE;

    Nand_ReadSpare(SpareTag, PageNo, AddrInPage, MAX_BYTES_PER_SECTORSPARE);

    return 0;
}

/**
*写入spare区
*参数1：@PageAddrBase: 上层写入基地址
*参数2：@SpareTag: 装spare区的数据
*/
uint8_t pageOP_WriteSpare(uint32_t PageAddrBase, SPARE_TAG_TYPE *SpareTag)
{
    uint32_t PageNo;
    uint16_t AddrInPage;

    PageNo = PageAddrBase;
    AddrInPage = PageNo % BYTE_PER_FILEPAGE;

    Nand_WriteSpare(SpareTag, PageNo, AddrInPage, MAX_BYTES_PER_SECTORSPARE);

    return 0;
}

/**
*页数据读取
*参数1：@PageAddrBase: 上层写入基地址
*参数2：@Buffer
*/
uint8_t pageOP_ReadData(uint32_t PageAddrBase, uint8_t *pBuffer)
{
    uint8_t m8i = 0;
    uint8_t ret = 0;
    uint32_t PageNo;
    uint16_t AddrInPage;

    uint8_t ECC[3];

    PageNo = PageAddrBase;
    AddrInPage = PageNo % BYTE_PER_FILEPAGE;

    //为保证读取，尝试读取三次
    for(m8i = 0; m8i < 3; m8i++)
    {
        //读取数据
        Nand_Read(pBuffer, PageNo, AddrInPage, BYTE_PER_FILEPAGE);//读取数据区和SPARE区
        nand_calculate_ecc(pBuffer, ECC);

        if((ECC[0] == pBuffer[BYTE_PER_FILEROWPAGE + 3]) && (ECC[1] == pBuffer[BYTE_PER_FILEROWPAGE + 4]) && (ECC[2] == pBuffer[BYTE_PER_FILEROWPAGE + 5]))
            return 0;
    }
    //读取出错
    return 1;
}

/**
*页数据写入（将所有一页数据全部搬移，包括SPARE区）
*参数1：@PageAddrBase: 上层写入基地址
*参数2：@Buffer
*/
uint8_t pageOP_WriteData(uint32_t PageAddrBase, uint8_t *pBuffer)
{
    uint32_t PageNo;
    uint16_t AddrInPage;

    SPARE_TAG_TYPE  SpareTag;

    PageNo = PageAddrBase;
    AddrInPage = PageNo % BYTE_PER_FILEPAGE;

    Nand_Write(pBuffer, PageNo, AddrInPage, BYTE_PER_FILEPAGE);

    //写入成功
    if(pageOP_ReadData(PageAddrBase, pBuffer) == 0)
        return 0;

    //写入不成功，标记为错误页
    else
    {
        SpareTag.spare_Flg_Bi = 0x00;
        pageOP_WriteSpare(PageAddrBase, &SpareTag);
        return 1;
    }
}

/**
*块擦除（擦除一个块里面的有用数据除了 擦除次数会改变，其余字节都设置为0xFF）
*参数1：Blk_Num
*/
uint8_t pageOP_EraseBlk(uint16_t Blk_Num)
{
    uint8_t m8i;
    uint32_t Bad_Blk = 0;
    uint32_t PageNo;

    SPARE_TAG_TYPE  SpareTag;

    for (m8i = 0; m8i < MAX_PAGES_PER_BLOCK; m8i++)
    {
        pageOP_ReadSpare(Blk_Num * MAX_PAGES_PER_BLOCK + m8i, &SpareTag);
        if (SpareTag.spare_Flg_Bi != 0xff)
            Bad_Blk |= (0x0001 << m8i);
    }

    //擦除该块
    PageNo = (uint32_t)Blk_Num * MAX_PAGES_PER_BLOCK;
    Nand_EraseBlock(PageNo);

    //设置SPARE 区
    for(m8i = 0; m8i < 4; m8i++)
    {
        SpareTag.ecc[m8i] = 0xFF;
    }

    SpareTag.spare_Erase_Time++;
    SpareTag.spare_Flg_Dirty = 0xFF;
    SpareTag.spare_Flg_Used = 0xFF;
    SpareTag.spare_Page_Num = 0xFFFFFFFF;
    SpareTag.Reserved = 0xFF;

    //将坏的页进行标记
    for (m8i = 0; m8i < MAX_PAGES_PER_BLOCK; m8i++)
    {
        SpareTag.spare_Flg_Bi = 0xFF;

        if ((Bad_Blk & (0x0001 << m8i)) == (0x0001 << m8i))
            SpareTag.spare_Flg_Bi = 0x00;

        pageOP_WriteSpare(Blk_Num * MAX_PAGES_PER_BLOCK + m8i, &SpareTag);
    }

    return 0;
}
