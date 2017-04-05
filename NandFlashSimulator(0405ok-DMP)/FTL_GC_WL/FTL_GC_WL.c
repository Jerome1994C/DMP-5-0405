/*********************************************************
*     File Name : FTL_GC_WL.c
*     YIN  @ 2017.04.04
*     Description  : Initial create(FTL-GC & WL)
**********************************************************/

/*********************************************************
*  Include
**********************************************************/
#include <stdint.h>
#include "Hardware.h"
#include "FTL_GC_WL.h"
#include "FTL_Struct.h"

/**
*Get_FreePage
*para1 : Sector
*/
uint32_t Get_FreeDPage(uint32_t Sector)
{
    uint32_t DPageAddrBase;

    //call Dynamic-WL
    DPageAddrBase = Dynamic_WL();
    DPM_Table[Sector] = DPageAddrBase;//return blkNo
    DPM_Change_Cfg = 1;

    return DPageAddrBase;
}

/************************************************
 * 以下三个操作说明:
 *
 * 一、静态操作:为了保证长期死数据不占用存储块的擦除寿命
 *
 * 二、动态操作:为了保证已有空闲块中擦除次数最少的先被使用
 *
 * 三、垃圾回收:用于产生空闲块，且会大量导致性能下降，所以该操作最好是有更好的调用机制
 *
 * 静态操作需要有一定的策略完成才可以发挥大量的作用
 *
 * 动态操作由页分配函数调用，获得年轻页
 *
 * 垃圾回收操作可以由分配函数调用和闲置时间调用
 *
 * 核心都是在围绕块转，操作却是以页为目标,平衡操作不产生空闲页，而是产生无效数据页，和垃圾回收效果相反
 ************************************************/


/**
 * Static_WL : 就是将长期不变的数据读取出来，放到擦写次数最少的里面
 *             （关键在于对块擦写次数的控制）
 * 参数解释: Degree表示静态平衡的程度，值越大，平衡效果越强但是效率越慢，推荐选择2
 * 返回值:0正常，1失败
 */
uint8_t Static_WL(void)
{
    uint16_t  EraseTime_MIN;
    uint16_t  EraseTime_MIN_BLK = 0xFFFF;

    uint16_t  m16i_Blk, m16j_Page;
    uint32_t  DBlk_Src, DBlk_Dst;

    SPARE_TAG_TYPE  SpareTag;

// find first
    for (m16i_Blk = 0; m16i_Blk < MAX_BLOCKS_PER_CHIP; m16i_Blk++)
    {
        for (m16j_Page = 0; m16j_Page < MAX_PAGES_PER_BLOCK; m16j_Page++)
        {
            pageOP_ReadSpare(m16i_Blk * MAX_PAGES_PER_BLOCK + m16j_Page, &SpareTag);

            if (SpareTag.spare_Flg_Bi != 0xFF)
            {
                NAND_MarkBadBlock(m16i_Blk);
                NAND_BuildBBT();
                break;
            }

            if ((SpareTag.spare_Flg_Bi == 0xFF)	                    // 好页
                    && (SpareTag.spare_Flg_Used != 0xFF)	            // 有数据
                    && (SpareTag.spare_Flg_Dirty == 0xFF))	// 有效数据
            {
                EraseTime_MIN = SpareTag.spare_Erase_Time;
                EraseTime_MIN_BLK = m16i_Blk;
                break;
            }
        }
        if (EraseTime_MIN_BLK != 0xFFFF)
            break;
    }

    // 不可能的情况，除非全盘数据损坏
    if (EraseTime_MIN_BLK == 0xFFFF)
        return 1;

    // 查找出有有效数据且擦除次数最少的块
    for (m16i_Blk = 0; m16i_Blk < MAX_BLOCKS_PER_CHIP; m16i_Blk++)
    {
        for (m16j_Page = 0; m16j_Page < MAX_PAGES_PER_BLOCK; m16j_Page++)
        {
            pageOP_ReadSpare(m16i_Blk * MAX_PAGES_PER_BLOCK + m16j_Page, &SpareTag);

            if (SpareTag.spare_Flg_Bi != 0xFF)
            {
                NAND_MarkBadBlock(m16i_Blk);
                NAND_BuildBBT();
                break;
            }

            if (SpareTag.spare_Erase_Time > EraseTime_MIN)
                break;

            if ((SpareTag.spare_Flg_Bi == 0xFF)	                    // 好页
                    && (SpareTag.spare_Flg_Used != 0xFF)	            // 有数据
                    && (SpareTag.spare_Flg_Dirty == 0xFF))	// 有效数据
            {
                EraseTime_MIN = SpareTag.spare_Erase_Time;
                EraseTime_MIN_BLK = m16i_Blk;
                break;
            }
        }
    }

    // 对找到的块进行必要手段处理
    for (m16j_Page = 0; m16j_Page < MAX_PAGES_PER_BLOCK; m16j_Page++)
    {
        pageOP_ReadSpare(EraseTime_MIN_BLK * MAX_PAGES_PER_BLOCK + m16j_Page, &SpareTag);

        if ((SpareTag.spare_Flg_Bi == 0xFF)	                        // 好页
                && (SpareTag.spare_Flg_Used != 0xFF)	            // 有数据
                && (SpareTag.spare_Flg_Dirty == 0xFF))	// 有效数据
        {
            DBlk_Src = (uint32_t)(EraseTime_MIN * MAX_PAGES_PER_BLOCK + m16j_Page) / DPM_Pages_Per_Blk;

            while (1)
            {
                DBlk_Dst = ANTI_Dynamic_WL();//return free-D-Blk
                if (DPM_Date_Move(DBlk_Src, DBlk_Dst) == 0)
                    break;
            }
        }
    }

    return 0;
}

/***************************************************************
 * 动态负载平衡
 * 作用: 返回可用页，产生无效页
 *
 * 策略:
 *	一、	找到可用页且记下该页擦除次数
 *	二、	依次找到最年轻的页
 *	三、	返回该页
 *	四、	如果都找不到，调用垃圾回收
 *	五、	返回第一步
 *
 * 机制:	每一次获取闲置页的时候都先使用擦除次数最少的块进行分配
 * 返回值:0xFFFF 表示动态平衡无法获得闲置页,其他值表示返回的闲置页地址
 ***************************************************************/
uint32_t Dynamic_WL(void)
{
    uint16_t Page_Addr = 0xFFFF;

    uint32_t DPM_BlkNo;

    uint16_t EraseTime_MIN;
    uint16_t m16i_Blk, m16j_Page;

    SPARE_TAG_TYPE  SpareTag;

LOOP:
    for (m16i_Blk = 0; m16i_Blk < MAX_BLOCKS_PER_CHIP; m16i_Blk++)
    {
        for (m16j_Page = 0; m16j_Page < MAX_PAGES_PER_BLOCK; m16j_Page++)
        {
            pageOP_ReadSpare(m16i_Blk * MAX_PAGES_PER_BLOCK + m16j_Page, &SpareTag);

            if (SpareTag.spare_Flg_Bi != 0xFF)
            {
                NAND_MarkBadBlock(m16i_Blk);
                NAND_BuildBBT();
                break;
            }

            if ((SpareTag.spare_Flg_Bi == 0xFF) && (SpareTag.spare_Flg_Used == 0xFF))
            {
                Page_Addr = m16i_Blk * MAX_PAGES_PER_BLOCK + m16j_Page;

                DPM_BlkNo = (uint32_t)Page_Addr / DPM_Pages_Per_Blk;// Get Free Page

                EraseTime_MIN = SpareTag.spare_Erase_Time;
                break;
            }
        }
        if (Page_Addr != 0xFFFF)
            break;
    }

    // 没有可用的东西，调用垃圾回收
    if (Page_Addr == 0xFFFF)
    {
        FTL_GC();
        goto LOOP;
    }

// Because the uint Erase is Block, check page can know one Block
    for (m16i_Blk = 0; m16i_Blk < MAX_BLOCKS_PER_CHIP; m16i_Blk++)
    {
        for (m16j_Page = 0; m16j_Page < MAX_PAGES_PER_BLOCK; m16j_Page++)
        {
            pageOP_ReadSpare(m16i_Blk * MAX_PAGES_PER_BLOCK + m16j_Page, &SpareTag);

            if (SpareTag.spare_Flg_Bi != 0xFF)
            {
                NAND_MarkBadBlock(m16i_Blk);
                NAND_BuildBBT();
                break;
            }

            if (SpareTag.spare_Erase_Time >= EraseTime_MIN)
                break;

            if ((SpareTag.spare_Flg_Bi == 0xFF) && (SpareTag.spare_Flg_Used == 0xFF))
            {
                Page_Addr = m16i_Blk * MAX_PAGES_PER_BLOCK + m16j_Page;

                DPM_BlkNo = (uint32_t)Page_Addr / DPM_Pages_Per_Blk;

                EraseTime_MIN = SpareTag.spare_Erase_Time;
                break;
            }
        }
    }
    return DPM_BlkNo;
}

/**************************************************************************
* 垃圾回收机制(回收废旧页)分三步
*
* 1、回收所有废块;
* 2、若上一步没有回收到垃圾，则进行这一步。回收每一块里脏数据量比较大的块;
* 3、若上一步没有可回收的块，则进行这一步，也即是将所有脏数据全部回收;
*
* 返回值: 1 不正常，一般表示没有可以回收的资源; 0 正常,表示有资源释放且释放了资源.
***************************************************************************/
uint8_t FTL_GC(void)
{
    uint8_t clean_cfg;			//	每一位代表一个页是否是脏的
    uint16_t m16i_Blk, m16j_Page;
    uint16_t k_page_map, k_Dblk_map;
    uint16_t dirty_page_num, clean_page_num;
    uint16_t bad_page_num;		// 记录坏页数量

    uint32_t  DBlk_Src, DBlk_Dst;

    SPARE_TAG_TYPE  SpareTag;
    uint8_t tick_cfg = 0; 		// 代表策略是否进行成功

    /*************
     * 机制第1步
     *************
     *  这里的机制第一步比较简单:
     * 	其实就是将所有没有有效数据的块全部擦干净来用;
     *	必须得注意有的块里面所有页都废了.
     */
    for (m16i_Blk = 0; m16i_Blk < MAX_BLOCKS_PER_CHIP; m16i_Blk++)
    {
        bad_page_num = 0;

        for (m16j_Page = 0; m16j_Page < MAX_PAGES_PER_BLOCK; m16j_Page++)
        {
            pageOP_ReadSpare(m16i_Blk * MAX_PAGES_PER_BLOCK + m16j_Page, &SpareTag);

            if (SpareTag.spare_Flg_Bi != 0xFF)
            {
                NAND_MarkBadBlock(m16i_Blk);
                NAND_BuildBBT();
                break;
            }

            if (SpareTag.spare_Flg_Dirty != 0xFF)
            {
                bad_page_num++;
            }

            else if ((SpareTag.spare_Flg_Bi == 0xFF) && (SpareTag.spare_Flg_Dirty == 0xFF))
                break;
        }

        if (bad_page_num == MAX_PAGES_PER_BLOCK)
        {
            pageOP_EraseBlk(m16i_Blk);
            tick_cfg = 1;
        }
    }

    if (tick_cfg == 1)// 已经回收到可用空间，返回
        return 0;

    /*************
    * 机制第2步
    **************
    * 处理一个块里面的情况
    * 简介策略为:
    *	1、脏页和有效数据页数量对比
    *	2、脏 > 有效(将数据搬离，擦除此块)
    *	3、脏 < 有效,不处理此块
    */
    for (m16i_Blk = 0; m16i_Blk < MAX_BLOCKS_PER_CHIP; m16i_Blk++)
    {

        dirty_page_num = 0;
        clean_page_num = 0;
        clean_cfg = 0;

        for (m16j_Page = 0; m16j_Page < MAX_PAGES_PER_BLOCK; m16j_Page++)
        {
            pageOP_ReadSpare(m16i_Blk * MAX_PAGES_PER_BLOCK + m16j_Page, &SpareTag);

            if (SpareTag.spare_Flg_Bi != 0xFF)
            {
                NAND_MarkBadBlock(m16i_Blk);
                NAND_BuildBBT();
                break;
            }

            if ((SpareTag.spare_Flg_Bi == 0xFF) && (SpareTag.spare_Flg_Dirty != 0xFF))	// 好页,数据脏
            {
                dirty_page_num++;
            }

            else if ((SpareTag.spare_Flg_Bi == 0xFF) && (SpareTag.spare_Flg_Used != 0xFF) && (SpareTag.spare_Flg_Dirty == 0xFF))
            {
                clean_page_num++;
                clean_cfg |= (0x0001 << m16j_Page);
            }
        }

        if (dirty_page_num > clean_page_num) // 可以回收此块
        {
            for (m16j_Page = 0; m16j_Page < MAX_PAGES_PER_BLOCK; m16j_Page++) // 转移有效数据
            {
                if ((clean_cfg & (0x0001 << m16j_Page)) == (0x0001 << m16j_Page))// 发现是哪个页数据有效
                {
                    for (k_page_map = 0; k_page_map < MAX_PAGES_PER_BLOCK; k_page_map++)
                    {
                        k_Dblk_map = k_page_map / DPM_Pages_Per_Blk;

                        if (DPM_Table[k_Dblk_map] == (m16i_Blk * MAX_PAGES_PER_BLOCK + m16j_Page) / DPM_Pages_Per_Blk)// 发现和映射里面哪一个项相同
                        {
                            DBlk_Src = DPM_Table[k_Dblk_map];
                            while (1)
                            {
                                DBlk_Dst = Dynamic_WL();// 移动成功

                                if (DPM_Date_Move(DBlk_Src, DBlk_Dst) == 0)
                                    break;
                            }
                        }
                    }
                }
            }

            // 擦除此块
            pageOP_EraseBlk(m16i_Blk);

            tick_cfg = 1;	// 表示不用进行第二个回收策略
        }
    }

    // 已经回收了空间，返回
    if (tick_cfg == 1)
        return 0;

    /**************
     * 机制第3步
     **************
     * 处理一个块里面的情况
     * 简介策略为:
     *	1、检查每一块里面有无脏数据
     *	2、将有效数据搬离
     *	3、擦除此块
     */
    for (m16i_Blk = 0; m16i_Blk  < MAX_BLOCKS_PER_CHIP; m16i_Blk ++)
    {
        dirty_page_num = 0;
        clean_cfg = 0;

        for (m16j_Page = 0; m16j_Page < MAX_PAGES_PER_BLOCK; m16j_Page++)
        {
            pageOP_ReadSpare(m16i_Blk * MAX_PAGES_PER_BLOCK + m16j_Page, &SpareTag);

            if (SpareTag.spare_Flg_Bi != 0xFF)
            {
                NAND_MarkBadBlock(m16i_Blk);
                NAND_BuildBBT();
                break;
            }

            if ((SpareTag.spare_Flg_Bi == 0xFF) && (SpareTag.spare_Flg_Dirty == 0xFF) && (SpareTag.spare_Flg_Used != 0xFF))
            {
                clean_cfg |= (0x0001 << m16j_Page);
            }

            else if ((SpareTag.spare_Flg_Bi == 0xFF) && (SpareTag.spare_Flg_Dirty != 0xFF))
            {
                dirty_page_num++;
            }
        }

        if ((dirty_page_num != 0) && (clean_cfg != 0)) // 可以回收此块
        {
            for (m16j_Page = 0; m16j_Page < MAX_PAGES_PER_BLOCK; m16j_Page++)// 转移有效数据
            {
                if ((clean_cfg & (0x0001 << m16j_Page)) == (0x0001 << m16j_Page)) // 发现是哪个页数据有效
                {
                    k_Dblk_map = k_page_map / DPM_Pages_Per_Blk;

                    for (k_page_map = 0; k_page_map < MAX_PAGES_PER_BLOCK; k_page_map++)// 发现和映射里面哪一个项相同
                    {
                        if (DPM_Table[k_Dblk_map] == (m16i_Blk * MAX_PAGES_PER_BLOCK + m16j_Page) / DPM_Pages_Per_Blk)
                        {
                            DPM_Date_Move(k_Dblk_map, (m16i_Blk * MAX_PAGES_PER_BLOCK + m16j_Page) / DPM_Pages_Per_Blk);
                            break;
                        }
                    }
                }
            }

            pageOP_EraseBlk(m16i_Blk);
        }

        else if ((dirty_page_num != 0) && (clean_cfg == 0))	// 没有有效数据
        {
            pageOP_EraseBlk(m16i_Blk);// 直接擦除此块
        }
    }

    return 0;
}

/**
 * ANTI_Dynamic_WL : 专门弄出一个最老最老的页，放静态数据; 反动态负载平衡，与动态负载平衡效果相反
 * 返回值: 0xFFFF 表示错误返回，其他值表示返回的可用页号;
 */
uint32_t ANTI_Dynamic_WL(void)
{
    uint8_t EraseTime_MAX;
    uint16_t m16i_Blk, m16j_Page;
    uint32_t page_ret = 0xFFFFFFFF;

    SPARE_TAG_TYPE SpareTag;

LOOP:
    for (m16i_Blk = 0; m16i_Blk < MAX_BLOCKS_PER_CHIP; m16i_Blk++)
    {
        for (m16j_Page = 0; m16j_Page < MAX_PAGES_PER_BLOCK; m16j_Page++)
        {
            pageOP_ReadSpare(m16i_Blk * MAX_PAGES_PER_BLOCK + m16j_Page, &SpareTag);

            if (SpareTag.spare_Flg_Bi != 0xFF)
            {
                NAND_MarkBadBlock(m16i_Blk);
                NAND_BuildBBT();
                break;
            }

            if ((SpareTag.spare_Flg_Bi == 0xFF)	&& (SpareTag.spare_Flg_Used == 0xFF))
            {
                EraseTime_MAX = SpareTag.spare_Erase_Time;
                page_ret = (uint32_t)m16i_Blk * MAX_PAGES_PER_BLOCK + m16j_Page;
                break;
            }
        }
        if (page_ret != 0xFFFFFFFF)
            break;
    }

    if (page_ret == 0xFFFFFFFF) // 表示没有可用页
    {
        FTL_GC();   // 调用垃圾回收
        goto LOOP;	// 上去重新分配
    }

    for (m16i_Blk = 0; m16i_Blk < MAX_BLOCKS_PER_CHIP; m16i_Blk++)
    {
        for (m16j_Page = 0; m16j_Page < MAX_PAGES_PER_BLOCK; m16j_Page++)
        {
            pageOP_ReadSpare(m16i_Blk * MAX_PAGES_PER_BLOCK + m16j_Page, &SpareTag);

            if (SpareTag.spare_Flg_Bi != 0xFF)
            {
                NAND_MarkBadBlock(m16i_Blk);
                NAND_BuildBBT();
                break;
            }

            if ((SpareTag.spare_Flg_Bi == 0xFF)	&& (SpareTag.spare_Flg_Used == 0xFF))
            {
                if (SpareTag.spare_Erase_Time > EraseTime_MAX)
                {
                    EraseTime_MAX = SpareTag.spare_Erase_Time;
                    page_ret = (uint32_t)m16i_Blk * MAX_PAGES_PER_BLOCK + m16j_Page;
                    break;
                }
            }
        }
    }

    return page_ret;
}
