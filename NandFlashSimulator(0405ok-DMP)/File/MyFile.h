/*********************************************************
*     File Name : MyFile.h
*     YIN  @ 2016.11.30
*     Description  : Initial create
**********************************************************/
#ifndef _MY_FILE_H
#define _MY_FILE_H

#define  VIEW_SIZE  ((uint64_t)(256*(16*1024))) //4M
/*********************************************************
*  Include
**********************************************************/
#include "FileAccess.h"

typedef struct _FILE_INFO
{
    char     mFileName[64];
    uint64_t mFileSize;
    int 	 hFile; //file discreptor
    void*	 pBuffer;
    uint32_t mView;
} FILE_INFO_S;

extern uint8_t MyFileDestroy(FILE_HANDLE_S* pFileHandle);
extern FILE_HANDLE_S* MyFileCreate(char* pFileName, uint64_t aFileSize);

#endif
