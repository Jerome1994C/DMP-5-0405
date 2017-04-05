/*********************************************************
*     File Name : FTL_GC_WL.h
*     YIN  @ 2017.04.04
*     Description  : Initial create
**********************************************************/
#ifndef _FTL_GC_WL_H_
#define _FTL_GC_WL_H_
/*********************************************************
*  Include
**********************************************************/
#include <stdint.h>
#include "Hardware.h"
#include "FTL_pageOP.h"
#include "FTL_Struct.h"
//#include "FTL_DPM.h"
extern uint16_t *DPM_Table;

extern uint8_t DPM_Change_Cfg;

uint32_t Get_FreeDPage(uint32_t Sector);

uint8_t Static_WL(void);

uint32_t Dynamic_WL(void);

uint8_t FTL_GC(void);

uint32_t ANTI_Dynamic_WL(void);

#endif
