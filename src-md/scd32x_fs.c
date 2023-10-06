
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#define MD_WORDRAM          (void*)0x600000
#define MCD_WORDRAM         (void*)((uintptr_t)0x0C0000)

#define BLOCK_SIZE 2048
#define CHUNK_SHIFT 3
#define CHUNK_BLOCKS (1<<CHUNK_SHIFT) 
#define CHUNK_SIZE (CHUNK_BLOCKS*BLOCK_SIZE)

#define MCD_DISC_BUFFER (void*)(MCD_WORDRAM + 0x20000 - CHUNK_SIZE)
#define MD_DISC_BUFFER (void*)(MD_WORDRAM + 0x20000 - CHUNK_SIZE)

typedef struct CDFileHandle {
    int32_t  (*Seek)(struct CDFileHandle *handle, int32_t offset, int32_t whence);
    int32_t  (*Tell)(struct CDFileHandle *handle);
    int32_t  (*Read)(struct CDFileHandle *handle, void *ptr, int32_t size);
    uint8_t  (*Eof)(struct CDFileHandle *handle);
    int32_t  offset; // start block of file
    int32_t  length; // length of file
    int32_t  blk; // current block in buffer
    int32_t  pos; // current position in file
} CDFileHandle_t;

extern CDFileHandle_t *cd_handle_from_name(CDFileHandle_t *handle, const char *name);
extern CDFileHandle_t *cd_handle_from_offset(CDFileHandle_t *handle, int32_t offset, int32_t length);

extern int64_t scd_open_file(const char *name);
extern void scd_read_block(void *ptr, int lba, int len, void (*wait)(void));
extern void bump_fm(void);

static uint8_t cd_Eof(CDFileHandle_t *handle)
{
    if (!handle)
        return 1;

    if (handle->pos >= handle->length)
        return 1;

    return 0;
}

static int32_t cd_Read(CDFileHandle_t *handle, void *ptr, int32_t size)
{
    int32_t pos, blk, len, read = 0;
    uint8_t *dst = ptr;

    if (!handle)
        return -1;

    while (size > 0 && !handle->Eof(handle))
    {
        pos = handle->pos;
        len = CHUNK_SIZE - (pos & (CHUNK_SIZE-1));
        if (len > size)
            len = size;
        if (len > (handle->length - pos))
            len = (handle->length - pos);

        blk = pos / CHUNK_SIZE;
        if (blk != handle->blk)
        {
            // keep the FM music going by calling bump_fm
            scd_read_block((void *)MCD_DISC_BUFFER, blk*CHUNK_BLOCKS + handle->offset, CHUNK_BLOCKS, bump_fm);
            handle->blk = blk;
        }

        memcpy(dst, (char *)MD_DISC_BUFFER + (pos & (CHUNK_SIZE-1)), len);

        handle->pos += len;
        dst += len;
        read += len;
        size -= len;
    }

    return read;
}

static int32_t cd_Seek(CDFileHandle_t *handle, int32_t offset, int32_t whence)
{
    int32_t pos;

    if (!handle)
        return -1;

    pos = handle->pos;
    switch(whence)
    {
        case SEEK_CUR:
            pos += offset;
            break;
        case SEEK_SET:
            pos = offset;
            break;
        case SEEK_END:
            pos = handle->length - offset - 1;
            break;
    }
    if (pos < 0)
        handle->pos = 0;
    else if (pos > handle->length)
        handle->pos = handle->length;
    else
        handle->pos = pos;

    return handle->pos;
}

static int32_t cd_Tell(CDFileHandle_t *handle)
{
    return handle ? handle->pos : 0;
}

CDFileHandle_t *cd_handle_from_offset(CDFileHandle_t *handle, int32_t offset, int32_t length)
{
    if (handle)
    {
        handle->Eof  = &cd_Eof;
        handle->Read = &cd_Read;
        handle->Seek = &cd_Seek;
        handle->Tell = &cd_Tell;
        handle->offset = offset;
        handle->length = length;
        handle->blk = -1; // nothing read yet
        handle->pos = 0;
    }
    return handle;
}

///

CDFileHandle_t gfh;

int scd32x_open_file(char *name)
{
    int64_t lo;
    CDFileHandle_t *handle = &gfh;
    int length, offset;

    lo = scd_open_file(name);
    length = lo >> 32;
    if (length < 0)
        return -1;
    offset = lo & 0xffffffff;
    
    handle->Eof  = &cd_Eof;
    handle->Read = &cd_Read;
    handle->Seek = &cd_Seek;
    handle->Tell = &cd_Tell;
    handle->offset = offset;
    handle->length = length;
    handle->blk = -1; // nothing read yet
    handle->pos = 0;
    return length;
}

int scd32x_read_file(void *ptr, int length)
{
    return cd_Read(&gfh, ptr, length);
}

int scd32x_seek_file(int offset, int whence)
{
    return cd_Seek(&gfh, offset, whence);
}