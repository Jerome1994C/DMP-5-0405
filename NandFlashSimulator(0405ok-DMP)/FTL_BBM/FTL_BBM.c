/*********************************************************
*     File Name : FTL_BBM.c
*     YIN  @ 2016.12.27
*     Description  : Initial create
*     Revise-01: 2017.3.2
*     Revise-02: 2017.4.1
**********************************************************/

/*********************************************************
*  Include
**********************************************************/
#include <string.h>
#include <stdio.h>
#include "Driver.h"
#include "FTL_BBM.h"
#include "Hardware.h"
#include "NandFlash_sim.h"

/**
*	函 数 名: NAND_BuildBBT
*	功能说明: 上电加载完NAND后启动，创建坏块管理表(记录坏块号)
*	返 回 值: NAND_OK： 成功； 	NAND_FAIL：失败
*/
uint8_t NAND_BuildBBT(void)
{
    uint16_t m32i = 0;
    uint16_t BBN = 0;

    uint32_t ret = 0;

    for (m32i = 0; m32i < NAND_BLOCK_COUNT; m32i++) //对每个块进行判断
    {
        BBT_BlkNum[m32i] = 0xFFFF;//把坏块表初始化
    }

    for (m32i = 0; m32i < NAND_BLOCK_COUNT; m32i++)
    {

        ret = NAND_IsBadBlock((uint32_t)m32i);//好块返回 0

        if (ret) //如果是坏块，进入循环体
        {
            if (BBN < NAND_BLOCK_COUNT)
            {
                if (BBT_BlkNum[BBN] != 0xFF)
                {
                    printf("%s: BBT WRONG!\n", __func__);
                    return NAND_FAIL;
                }

                BBT_BlkNum[BBN] = m32i;//坏块号记录进 BBT_BlkNum，没有记录就是 0xFFF
                BBN++;
                if(BBN >= MAX_BAD_BLOCKS_COUNT)
                {
                    printf("%s: TOO MORE BAD BLOCK!\n", __func__);
                    return NAND_FAIL;
                }
            }
        }
    }

    return NAND_OK;
}

/**
*	函数名: NAND_IsBadBlock
*/
uint8_t NAND_IsBadBlock(uint32_t m32BlkIdx)
{
    uint8_t m32BadBlkIdx;

    Nand_ReadSpare(&m32BadBlkIdx, m32BlkIdx * MAX_PAGES_PER_BLOCK, BLKINFO_BAD_OFFSET, 1);//读取 SPARE 区的第一个字节，即坏块标记

    if(m32BadBlkIdx != 0xFF)
    {
        return BAD_BLK;
    }

    Nand_ReadSpare(&m32BadBlkIdx, m32BlkIdx * MAX_PAGES_PER_BLOCK + 1, BLKINFO_BAD_OFFSET, 1);

    if(m32BadBlkIdx != 0xFF)
    {
        return BAD_BLK;
    }

    return GOOD_BLK;
}

/**
*	函数名: NAND_IsFreeBlock
*/
uint32_t NAND_IsFreeBlock(uint32_t m32BlkIdx)
{
    uint8_t m32FreeBlkIdx;

    if(NAND_IsBadBlock(m32BlkIdx))
    {
        return BUSY_BLK;
    }

    Nand_ReadSpare(&m32FreeBlkIdx, m32BlkIdx * MAX_PAGES_PER_BLOCK, BLKINFO_USED_OFFSET, 1);

    if(m32FreeBlkIdx == 0xFF)
    {
        return FREE_BLK;
    }

    return BUSY_BLK;
}

/**
*	函数名: NAND_MarkUsedBlock
*/
uint8_t NAND_MarkUsedBlock(uint32_t m32BlkIdx)
{
    uint8_t m8UsedBlkIdx;
    uint32_t m32PageIdx;

    m32PageIdx = m32BlkIdx * MAX_PAGES_PER_BLOCK;

    m8UsedBlkIdx = NAND_USED_BLOCK_FLAG;

    if(Nand_WriteSpare(&m8UsedBlkIdx, m32PageIdx, BLKINFO_USED_OFFSET, 1) == NAND_FAIL)
    {
        return NAND_FAIL;
    }

    return NAND_OK;
}

/**
*	函数名: NAND_MarkBadBlock
*/
void NAND_MarkBadBlock(uint32_t m32BlkIdx)
{
    uint8_t m8BadBlkIdx;
    uint32_t m32PageIdx;

    m32PageIdx = m32BlkIdx * MAX_PAGES_PER_BLOCK;

    m8BadBlkIdx = NAND_BAD_BLOCK_FLAG;

    if(Nand_WriteSpare(&m8BadBlkIdx, m32PageIdx, BLKINFO_BAD_OFFSET, 1) == NAND_FAIL)
    {
        Nand_WriteSpare(&m8BadBlkIdx, m32PageIdx + 1, BLKINFO_BAD_OFFSET, 1);
    }
}
