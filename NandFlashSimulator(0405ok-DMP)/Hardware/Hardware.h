/*********************************************************
*     File Name : Hardware.h
*     YIN  @ 2016.12.03
*     Description  : Initial create
**********************************************************/
#ifndef _HARDWARE_H_
#define _HARDWARE_H_

#include <stdint.h>

#define HARDWARE_INTERFACE

/*
【NAND Flash 结构定义 —— 三星HY27UF081G2A】
     备用区有16x4字节，每page 1024字节，每512字节一个扇区，每个扇区对应16自己的备用区：

	 每个PAGE的逻辑结构，前面512Bx4是主数据区，后面16Bx4是备用区
	┌──────┐┌──────┐┌──────┐┌──────┐┌──────┐┌──────┐┌──────┐┌──────┐
	│ Main area  ││ Main area  ││ Main area  ││Main area   ││ Spare area ││ Spare area ││ Spare area ││Spare area  │
	│            ││            ││            ││            ││            ││            ││            ││            │
	│   512B     ││    512B    ││    512B    ││    512B    ││    16B     ││     16B    ││     16B    ││    16B     │
	└──────┘└──────┘└──────┘└──────┘└──────┘└──────┘└──────┘└──────┘

	 每16B的备用区的逻辑结构如下:(三星推荐标准）
	┌───┐┌───┐┌──┐┌──┐┌──┐┌───┐┌───┐┌───┐┌──┐┌──┐┌──┐┌──┐┌───┐┌───┐┌───┐┌───┐┌───┐
	│  BI  ││RESER ││LSN0││LSN1││LSN2││RESER ││RESER ││RESER ││ECC0││ECC1││ECC2││ECC0││S-ECC1││S-ECC0││RESER ││RESER ││RESER │
	│      ││ VED  ││    ││    ││    ││ VED  ││ VED  ││ VED  ││    ││    ││    ││    ││      ││      ││ VED  ││ VED  ││ VED  │
	└───┘└───┘└──┘└──┘└──┘└───┘└───┘└───┘└──┘└──┘└──┘└──┘└───┘└───┘└───┘└───┘└───┘

	K9F1G08U0A 和 HY27UF081G2A 是兼容的。芯片出厂时，厂商保证芯片的第1个块是好块。如果是坏块，则在该块的第1个PAGE的第1个字节
	或者第2个PAGE（当第1个PAGE坏了无法标记为0xFF时）的第1个字节写入非0xFF值。坏块标记值是随机的，软件直接判断是否等于0xFF即可。

	注意：网上有些资料说NAND Flash厂商的默认做法是将坏块标记定在第1个PAGE的第6个字节处。这个说法是错误。坏块标记在第6个字节仅针对部分小扇区（512字节）的NAND Flash
	并不是所有的NAND Flash都是这个标准。大家在更换NAND Flash时，请仔细阅读芯片的数据手册。


	为了便于在NAND Flash 上移植Fat文件系统，我们对16B的备用区采用以下分配方案:
	┌──┐┌──┐┌──┐┌──┐┌──┐┌──┐┌──┐┌──┐┌──┐┌──┐┌───┐┌───┐┌──┐┌──┐┌──┐┌──┐
	│ BI ││USED││LBN0││LBN1││ECC0││ECC1││ECC2││ECC3││ECC4││ECC5││S-ECC1││S-ECC0││RSVD││RSVD││RSVD││RSVD│
	│    ││    ││    ││    ││    ││    ││    ││    ││    ││    ││      ││      ││    ││    ││    ││    │
	└──┘└──┘└──┘└──┘└──┘└──┘└──┘└──┘└──┘└──┘└───┘└───┘└──┘└──┘└──┘└──┘
    - BI : 坏块标志(Bad Block Identifier)。每个BLOCK的第1个PAGE或者第2个PAGE的第1个字节指示该块是否坏块。0xFF表示好块，不是0xFF表示坏块。
    - USED : 该块使用标志。0xFF表示空闲块；0xF0表示已用块。
    - LBN0 LBN1 : 逻辑块号(Logic Block No) 。从0开始编码。只在每个BLOCK的第1个PAGE有效，其它PAGE该字段固定为0xFF FF
    - ECC0 ~ ECC6 : 512B主数据区的ECC校验 （按照三星提供ECC算法，256字节对应3个字节的ECC)
    - S-ECC1 S-ECC0 : LSN0和LSN2的ECC校验
    - RSVD : 保留字节，Reserved
*/

#define BYTE_PER_MDSECTOR           (512)
#define MAX_NAND_ID_LENGTH          (6)

#define MAX_PAGES_PER_BLOCK         (64)
#define MAX_BLOCKS_PER_CHIP         (2*1024)

#define MAX_BYTES_PER_PAGEDATA      (2*1024)
#define MAX_BYTES_PER_PAGE          (2048 + 64)

#define MAX_BYTES_PER_PAGESPARE     (64)
#define MAX_BYTES_PER_SECTORSPARE   (16)

#define MAX_NANDFLASH_CHIP_CNT      (32)

#define PAGE_META_DATA   0x55

#define DATA_PAGE      0
#define SPARE_PAGE     1
#define FULL_PAGE      2

#define BYTE_PER_FILESECTOR   (4) //simulator (data)
#define BYTE_PER_FILEROWPAGE  (BYTE_PER_FILESECTOR*MAX_BLOCKS_PER_CHIP/512) // simulator bytes of a phy page(16)
#define BYTE_PER_FILESPARE    (64)  //spare bytes per phy page
#define BYTE_PER_FILEPAGE     (BYTE_PER_FILEROWPAGE+BYTE_PER_FILESPARE)  // data byte+spare byte per phy page(16+64)
#define BYTE_PER_FILEBLOCK    (BYTE_PER_FILEPAGE*MAX_PAGES_PER_BLOCK)//(16+64)*64
#define BYTE_PER_FILECHIP     (BYTE_PER_FILEBLOCK*MAX_BLOCKS_PER_CHIP)//(16+64)*64*2048

#define NAND_OK   0
#define NAND_FAIL 1

#define  NAND_BLOCK_COUNT (MAX_NANDFLASH_CHIP_CNT * MAX_BLOCKS_PER_CHIP)  /* 32个chip ，每个chip 有 2个 plane ， 一个plane 有 1024 个block */
#define  PAGES_PER_CHIP   (MAX_BLOCKS_PER_CHIP * MAX_PAGES_PER_BLOCK)
#define  BLOCKS_PER_CHIP  (MAX_BLOCKS_PER_CHIP)

#define  MAX_BAD_BLOCKS_COUNT (512)

#define BB_OFFSET				0		/* 块内第1个page备用区的第1个字节是坏块标志 */
//#define USED_OFFSET				1		/* 块内第1个page备用区的第2个字节是已用标志 */
//#define LBN0_OFFSET				2		/* 块内第1个page备用区的第3个字节表示逻辑块号低8bit */
//#define LBN1_OFFSET				3		/* 块内第1个page备用区的第4个字节表示逻辑块号高8bit */

#define CHECK_SPARE_SIZE		4		/* 实际使用的备用区大小,用于函数内部声明数据缓冲区大小 */

#define CHIP_ADDR(aChannel, aChip, aDie) ((aChannel<<24) + (aChip<<8) + aDie)

//#define DPM_PAGES_PER_BLOCK   2

typedef struct _NANDFLASH_PARAM
{
    uint16_t mSizePerPage;
    uint16_t mBytesPerPage;
    uint16_t mBytesPerSpare;
    uint16_t mPagesPerBlock;
    uint16_t mBlocksPerChip;
    uint16_t mPlanesPerChip;
    uint16_t mBlocksPerPlane;
    uint16_t mMultiPlaneEnable;
    uint16_t mSectorsPerPage;
} NANDFLASH_PARAM_S;

typedef struct _LAYOUT_PARAM
{
    uint8_t  mDiesPerChip;
    uint16_t mChipsPerChannel;
    uint8_t  mChannelsPerArray;
} LAYOUT_PARAM_S;

LAYOUT_PARAM_S    gHW_LayoutParam;
NANDFLASH_PARAM_S gHW_NandFlashParam;

extern HARDWARE_INTERFACE   LAYOUT_PARAM_S   gHW_LayoutParam;

uint16_t BBT_BlkNum[MAX_BAD_BLOCKS_COUNT];

uint32_t DPM_Pages_Per_Blk;
#endif
