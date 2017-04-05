/*********************************************************
*     File Name : Driver.h
*     YIN  @ 2016.12.15
*     Description  : Initial create
**********************************************************/
#ifndef _DRIVER_H_
#define _DRIVER_H_
/*********************************************************
*  Include
**********************************************************/
#include "Hardware.h"

extern uint32_t Nand_ReadID(uint32_t PageNo, uint8_t* pID);

extern uint32_t Nand_ReadData(uint8_t* pDataBuffer, uint32_t PageNo, uint16_t AddrInPage, uint16_t ByteCount);
extern uint32_t Nand_ReadSpare(uint8_t* pDataBuffer, uint32_t PageNo, uint16_t AddrInPage, uint16_t ByteCount);

extern uint32_t Nand_WriteData(uint8_t* pDataBuffer, uint32_t PageNo, uint16_t AddrInPage, uint16_t ByteCount);
extern uint32_t Nand_WriteSpare(uint8_t* pDataBuffer, uint32_t PageNo, uint16_t AddrInPage, uint16_t ByteCount);

extern uint32_t Nand_EraseBlock(uint32_t PageNo);

#endif
