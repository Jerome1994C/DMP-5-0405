/*********************************************************
*     File Name : FTL_DPM.c
*     YIN  @ 2017.04.05
*     Description  : Initial create
**********************************************************/

/*********************************************************
*  Include
**********************************************************/
#include <stdint.h>
#include <stdlib.h>
#include "Hardware.h"
#include "FTL_DPM.h"
#include "FTL_GC_WL.h"

/*****************
 *	全局数据
 *页映射表，ftl初始化的时候生成,外部extern进行引用
 *****************/
uint8_t DPM_Change_Cfg;

extern uint16_t *DPM_Table;

uint8_t FTL_pageOP_RDBuffer[BYTE_PER_FILEPAGE];

uint8_t FTL_pageOP_RBuffer[BYTE_PER_FILEROWPAGE];

uint16_t DPM_Size;

/*************
 * 函数接口
 ************/

/**
 * 读取并填装为映射表项
 * 返回值: 0正常返回，1不正常返回,映射表错误
 */
static uint8_t Read_DPM(uint32_t page_addr_base, uint16_t DPMCnt)
{
    uint16_t m16i;
    uint32_t DPM_BlkNo;

    if (pageOP_Read(page_addr_base, FTL_pageOP_RBuffer) == 1)
    {
        return 1;
    }

    DPM_BlkNo = page_addr_base / DPM_Pages_Per_Blk;

    DPM_Table[DPMCnt] = DPM_BlkNo;

    return 0;
}

/**
 * 生成 DPM
 * 返回值: 0生成成功，非0表示页映射表没有生成.
 */
uint16_t Create_DPM(uint32_t DPM_Pages_Per_Blk)
{
    uint16_t i_block, j_page;
    uint16_t PageCnt = 0;
    uint16_t m16i;

    SPARE_TAG_TYPE spare_tag;

    DPM_Size = (uint16_t)(MAX_BLOCKS_PER_CHIP * MAX_PAGES_PER_BLOCK) / DPM_Pages_Per_Blk;
    DPM_Table = (uint16_t *)malloc(DPM_Size);

    memset(DPM_Table, 0xFF, DPM_Size);

LOOP:
    for (i_block = 0; i_block < MAX_BLOCKS_PER_CHIP; i_block++)
    {
        for(m16i = 0; m16i < MAX_BAD_BLOCKS_COUNT; m16i++)
        {
            if(BBT_BlkNum[m16i] == i_block)
            {
                goto LOOP;
            }
        }

        for (j_page = 0; j_page < MAX_PAGES_PER_BLOCK; j_page++)
        {
            pageOP_ReadSpare(i_block * MAX_PAGES_PER_BLOCK + j_page, &spare_tag);

            if ((spare_tag.spare_Flg_Used != 0xFF) && (spare_tag.spare_Flg_Dirty == 0xFF))
            {
                Read_DPM(i_block * MAX_PAGES_PER_BLOCK + j_page, PageCnt);
                PageCnt++;
            }
        }
    }
    // 表示没有被修改过
    DPM_Change_Cfg = 0;

    return 0;
}

/**
 * 获得闲置的页号
 *
 * 入口参数: sector 对应的数组下标
 *
 * 返回:
 *	0xFFFF表示没有可用空间了
 *	其他值表示返回的实际页偏移地址
 */
uint16_t Get_Useful_WPage(uint32_t sector)
{
    uint16_t page_ret = 0xFFFF;
    uint16_t page_addr_old = (uint16_t)DPM_Table[sector];

    SPARE_TAG_TYPE spare_tag;

    // 读取page_addr_old里面的spare区域判断
    pageOP_ReadSpare(page_addr_old, &spare_tag);

    if ((spare_tag.spare_Flg_Used == 0xff) && (page_addr_old != 0xffff))
    {
        page_ret = page_addr_old;
    }

    else if (spare_tag.spare_Flg_Used != 0xff)// 假如页可用但非空，表示数据要被修改
    {
        spare_tag.spare_Flg_Dirty = 0x00;	// 标志为脏数据

        pageOP_ReadSpare(page_addr_old, &spare_tag);

        page_ret = Get_FreeDPage(sector);// 获得一个闲置的page
    }

    else
    {
        page_ret = Get_FreeDPage(sector);// 获得一个闲置的page
    }

    return page_ret;
}

/**
* 获得可读的页号
*/
uint16_t Get_Useful_RPage(uint32_t sector)
{
    return DPM_Table[sector];
}

/**
 * DPM 页数据移动，不是简单的数据区域移动,也即是包含扩展区域数据一起移动
 *
 * 入口参数:
 *	DBlk_Src: 要移动的数据所在的页
 *	DBlk_Dst: 数据将要搬动到的页
 *
 *	DPM_Pos: 页映射数组的下标
 *
 * 返回参数:0正常，1不正常，写入页已坏
 */
uint8_t DPM_Date_Move(uint32_t DBlk_Src, uint32_t DBlk_Dst)
{
    uint16_t DPM_Pos;

    SPARE_TAG_TYPE spare_tag;

    pageOP_ReadData(DBlk_Src, FTL_pageOP_RDBuffer);
    if(pageOP_WriteData(DBlk_Dst, FTL_pageOP_RDBuffer) == 1)
    {
        return 1;
    }

    pageOP_ReadSpare(DBlk_Src, &spare_tag);

    spare_tag.spare_Flg_Dirty = 0X00;
    pageOP_WriteSpare(DBlk_Src, &spare_tag);

    //Updata DPM
    for (DPM_Pos = 0; DPM_Pos < DPM_Size; DPM_Pos++)
    {
        if (DPM_Table[DPM_Pos] == DBlk_Src)
        {
            DPM_Table[DPM_Pos] = DBlk_Dst;// 映射表已经被修改
            DPM_Change_Cfg = 1;
            break;
        }
    }
    return 0;
}
