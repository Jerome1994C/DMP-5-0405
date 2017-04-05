/*********************************************************
*     File Name : FileAccess.h
*     YIN  @ 2016.11.30
*     Description  : Initial create
**********************************************************/
#ifndef _FILEACCESS_H
#define _FILEACCESS_H

/*********************************************************
*  Include
**********************************************************/
#include <stdint.h>

typedef struct _FILE_HANDLE
{
    uint8_t (*file_flush)(struct _FILE_HANDLE* pFileHandle);
    uint8_t (*file_close)(struct _FILE_HANDLE* pFileHandle);
    uint8_t (*file_open)(struct _FILE_HANDLE* pFileHandle);
    uint8_t (*file_read)(struct _FILE_HANDLE* pFileHandle, uint64_t aAddr, uint32_t aSize, uint8_t* pBuf);
    uint8_t (*file_write)(struct _FILE_HANDLE* pFileHanlde, uint64_t aAddr, uint32_t aSize, uint8_t* pBuf);

    void *priv;
} FILE_HANDLE_S;

extern uint8_t FileOpen(FILE_HANDLE_S* pFileHandle);
extern uint8_t FileFlush(FILE_HANDLE_S* pFileHandle);
extern uint8_t FileClose(FILE_HANDLE_S* pFileHandle);
extern uint8_t FileRead(FILE_HANDLE_S* pFileHandle, uint64_t aAddr, uint32_t aSize, uint8_t* pBuf);
extern uint8_t FileWrite(FILE_HANDLE_S* pFileHandle, uint64_t aAddr, uint32_t aSize, uint8_t* pBuf);

#endif
