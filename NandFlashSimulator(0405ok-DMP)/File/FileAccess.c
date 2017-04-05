/*********************************************************
*     File Name : FileAccess.c
*     YIN  @ 2016.11.30
*     Description  : Initial create
**********************************************************/
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include "FileAccess.h"

uint8_t FileOpen(FILE_HANDLE_S* pFileHandle)
{
    return pFileHandle->file_open(pFileHandle);
}

uint8_t FileFlush(FILE_HANDLE_S* pFileHandle)
{
    return pFileHandle->file_flush(pFileHandle);
}

uint8_t FileClose(FILE_HANDLE_S* pFileHandle)
{
    return pFileHandle->file_close(pFileHandle);
}

uint8_t FileRead(FILE_HANDLE_S* pFileHandle, uint64_t aAddr, uint32_t aSize, uint8_t* pBuf)
{
    return pFileHandle->file_read(pFileHandle, aAddr, aSize, pBuf);
}

uint8_t FileWrite(FILE_HANDLE_S* pFileHandle, uint64_t aAddr, uint32_t aSize, uint8_t* pBuf)
{
    return pFileHandle->file_write(pFileHandle, aAddr, aSize, pBuf);
}
