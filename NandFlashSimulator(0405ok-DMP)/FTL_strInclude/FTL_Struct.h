/*********************************************************
*     File Name : FTL_Struct.h
*     YIN  @ 2017.04.01
*     Description  : Initial create
**********************************************************/
#ifndef _FTL_STRUCT_H_
#define _FTL_STRUCT_H_
/*********************************************************
*  Include
**********************************************************/
#include <stdint.h>
/**
 * 扩展状态区域结构(每一页)
 * 为了简单起见，不设计块状态，由读取页状态进行判断.
 * 16B，不考虑字节对齐.
 */
typedef struct
{
    uint8_t spare_Flg_Bi;		  // 坏块标志（是否废掉了）
    uint8_t spare_Flg_Used;	      // 使用标志（页是否有数据）
    uint8_t spare_Flg_Dirty;	  // 脏数据页标志（数据是否脏）
    uint8_t ecc[4];		          // ECC校验码
    uint32_t spare_Page_Num;	  // 页映射表号，只有页表使用,且只在初始化和退出操作的时候
    uint32_t spare_Erase_Time;	  // 擦除计数（块）
    uint8_t Reserved;		      // 预留一个字节
} SPARE_TAG_TYPE;

/**
*DPM-Dynamic Page Mapping Struct
*/
uint16_t *DPM_Table;

#endif
