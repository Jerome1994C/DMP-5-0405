/*********************************************************
*     File Name : MyFile.c
*     YIN  @ 2016.11.30
*     Description  : Initial create
**********************************************************/
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "MyFile.h"
#include "File_lib.h"

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

static uint8_t MFileOpen(FILE_HANDLE_S* pFileHandle)
{
    uint64_t  fileSize = 0;
    FILE_INFO_S* pFileInfo = pFileHandle->priv;
    pFileInfo->mView = 0;
    pFileInfo->hFile = open(pFileInfo->mFileName, O_RDWR|O_CREAT, S_IRWXU|S_IRWXG|S_IRWXO);

    if (pFileInfo->hFile < 0)
    {
        printf("Open File Failure\n");
        return FAILURE;
    }

    if ((pFileInfo->mFileSize % VIEW_SIZE) == 0)
    {
        fileSize = pFileInfo->mFileSize;
    }
    else
    {
        fileSize = (pFileInfo->mFileSize / VIEW_SIZE + 1) * VIEW_SIZE;
    }

    lseek(pFileInfo->hFile, fileSize, SEEK_SET);
    write(pFileInfo->hFile, "", 1);
    pFileInfo->pBuffer = mmap(NULL, VIEW_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, pFileInfo->hFile, 0);

    if (pFileInfo->pBuffer == MAP_FAILED)
    {
        printf("ViewOfFile Failure\n");
        return FAILURE;
    }

    return SUCCESS;
}

static uint8_t MFileFlush(FILE_HANDLE_S* pFileHandle)
{
    FILE_INFO_S* pFileInfo = pFileHandle->priv;

    if (pFileInfo->pBuffer != MAP_FAILED)
    {
        if (munmap(pFileInfo->pBuffer, VIEW_SIZE) == -1)
        {
            printf("munmap Failure\n");
            return FAILURE;
        }
    }

    return SUCCESS;
}

static uint8_t MFileClose(FILE_HANDLE_S* pFileHandle)
{
    FILE_INFO_S* pFileInfo = pFileHandle->priv;

    if (pFileInfo->pBuffer != MAP_FAILED)
    {
        if (munmap(pFileInfo->pBuffer, VIEW_SIZE) == -1)
        {
            printf("munmap Failure\n");
        }
    }

    if (pFileInfo->hFile > 0)
    {
        close(pFileInfo->hFile);
        pFileInfo->hFile = -1;
    }

    return SUCCESS;
}

static uint8_t MFileRead(FILE_HANDLE_S* pFileHandle, uint64_t aAddr, uint32_t aSize, uint8_t* pBuf)
{
    uint32_t mView;
    uint32_t offset;
    uint32_t m32SecondSize;
    uint64_t viewOffset;

    FILE_INFO_S* pFileInfo = pFileHandle->priv;

    mView = (uint32_t)(aAddr / (uint64_t)VIEW_SIZE);
    offset = (uint32_t)(aAddr % (uint64_t)VIEW_SIZE);

    if (pFileInfo->mView != mView)
    {
        pFileInfo->mView = mView;
        if (munmap(pFileInfo->pBuffer, VIEW_SIZE) == -1)
        {
            printf("munmap Failure\n");
            return FAILURE;
        }

        viewOffset = (uint64_t)mView * (uint64_t)VIEW_SIZE;
        pFileInfo->pBuffer = mmap(NULL, VIEW_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, pFileInfo->hFile, viewOffset);
        if (pFileInfo->pBuffer == NULL)
        {
            printf("MapViewOfFile Failure\n");
            return FAILURE;
        }
    }

    if (pFileInfo->pBuffer != NULL)
    {
        if((aSize + offset) <= VIEW_SIZE)
        {
            memcpy(pBuf, pFileInfo->pBuffer+offset, aSize);
        }
        else     // accross boundary
        {
            m32SecondSize = aSize + offset - VIEW_SIZE;
            memcpy(pBuf,pFileInfo->pBuffer+offset, (VIEW_SIZE - offset));
            pFileInfo->mView += 1;
            if (munmap(pFileInfo->pBuffer, VIEW_SIZE) == -1)
            {
                printf("munmap Failure\n");
                return FAILURE;
            }

            viewOffset = (uint64_t)(pFileInfo->mView)*(uint64_t)VIEW_SIZE;
            pFileInfo->pBuffer = mmap(NULL, VIEW_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, pFileInfo->hFile, viewOffset);
            if (pFileInfo->pBuffer == NULL)
            {
                printf("MapViewOfFile Failure\n");
                return FAILURE;
            }
            memcpy(pBuf +(VIEW_SIZE - offset), pFileInfo->pBuffer,m32SecondSize);
        }
    }

    return SUCCESS;
}

static uint8_t MFileWrite(FILE_HANDLE_S* pFileHandle, uint64_t aAddr, uint32_t aSize, uint8_t* pBuf)
{
    uint32_t mView =0;
    uint32_t offset =0;
    uint32_t m32SecondSize;
    uint64_t viewOffset;

    FILE_INFO_S* pFileInfo = pFileHandle->priv;

    mView = (uint32_t)(aAddr / (uint64_t)VIEW_SIZE);
    offset = (uint32_t)(aAddr % (uint64_t)VIEW_SIZE);

    if (pFileInfo->mView != mView)
    {
        pFileInfo->mView = mView;
        if (munmap(pFileInfo->pBuffer, VIEW_SIZE) == -1)  // error
        {
            printf("munmap Failure\n");
            return FAILURE;
        }

        viewOffset = (uint64_t)mView*(uint64_t)VIEW_SIZE;
        pFileInfo->pBuffer = mmap(NULL, VIEW_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, pFileInfo->hFile, viewOffset);
        if (pFileInfo->pBuffer == NULL)
        {
            printf("MapViewOfFile Failure\n");
            return FAILURE;
        }
    }

    if(pFileInfo->pBuffer != NULL)
    {
        if((aSize + offset) <= VIEW_SIZE)
        {
            memcpy(pFileInfo->pBuffer+offset, pBuf, aSize);
        }
        else     // accross boundary
        {
            m32SecondSize = aSize + offset - VIEW_SIZE;
            memcpy(pFileInfo->pBuffer+offset, pBuf, (VIEW_SIZE - offset));
            pFileInfo->mView += 1;

            if (munmap(pFileInfo->pBuffer, VIEW_SIZE) == -1)
            {
                printf("munmap Failure\n");
                return FAILURE;
            }

            viewOffset = (uint64_t)(pFileInfo->mView)*(uint64_t)VIEW_SIZE;
            pFileInfo->pBuffer = mmap(NULL, VIEW_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, pFileInfo->hFile, viewOffset);
            if (pFileInfo->pBuffer == NULL)
            {
                printf("MapViewOfFile Failure\n");
                return FAILURE;
            }
            memcpy(pFileInfo->pBuffer, pBuf +(VIEW_SIZE - offset), m32SecondSize);
        }
    }

    return SUCCESS;
}

FILE_HANDLE_S* MyFileCreate(char* pFileName, uint64_t aFileSize)
{
    FILE_HANDLE_S*  pFileHandle = malloc(sizeof(FILE_HANDLE_S));
    FILE_INFO_S* pFileInfo = malloc(sizeof(FILE_INFO_S));

    memcpy(pFileInfo->mFileName, pFileName, 64);
    pFileInfo->mFileSize = aFileSize;

    pFileHandle->priv = pFileInfo;
    pFileHandle->file_open = MFileOpen;
    pFileHandle->file_read  = MFileRead;
    pFileHandle->file_write = MFileWrite;
    pFileHandle->file_flush = MFileFlush;
    pFileHandle->file_close = MFileClose;

    return pFileHandle;
}

uint8_t MyFileDestroy(FILE_HANDLE_S* pFileHandle)
{
    FILE_INFO_S* pFileInfo = pFileHandle->priv;

    free(pFileInfo);
    free(pFileHandle);

    return SUCCESS;
}
