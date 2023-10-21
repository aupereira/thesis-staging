#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include <inttypes.h>

union buf
{
    uint8_t bytes[64];
    uint32_t dwords[16];
    uint64_t qwords[8];
};

uint32_t chgEndUInt32(uint32_t n)
{
    uint32_t r = n >> 24 | (n & 0x00FF0000) >> 8 | (n & 0x0000FF00) << 8 | n << 24;
    
    return r;
}

uint64_t chgEndUInt64(uint64_t n)
{
    uint64_t r = 
        n >> 56 | (n >> 40) & 0x000000000000FF00 |
	    (n >> 24) & 0x0000000000FF0000 | (n >> 8) & 0x00000000FF000000 |
	    (n << 8) & 0x000000FF00000000 | (n << 24) & 0x0000FF0000000000 |
	    (n << 40) & 0x00FF000000000000 | n << 56
    ;

    return r;
}

union buf* buffers[64];
uint64_t totalBuffers = 16;
uint64_t bufferIndex = 0;

enum bufferStatus {Continue, AddPadBlock, Done};

void addBuffers(){
    union buf** nb = (union buf**)malloc((totalBuffers + 16) * sizeof(union buf*));

    for (int i = 0; i < totalBuffers; i++)
    {
        nb[i] = buffers[i];
        // printf("\n%p %p", nb[i], buffers[i]);
    }

    // for (int i = totalBuffers; i < totalBuffers + 16; i++)
    // {
    //     nb[i] = NULL;
    // }

    free(*buffers);

    totalBuffers += 16;

    *buffers = *nb;

    return;
}

int buffGen(char filePath[])
{
    FILE *fp;
    uint64_t totalBits = 0;
    enum bufferStatus bs;
    
    fp = fopen(filePath,"r");
    if(fp == NULL)
    {
        printf("Error opening file.");
        bs = Done;
        return -1;
    }
    else
    {
        bs = Continue;
    }

    while (bs == Continue)
    {
        printf("%" PRId64, bufferIndex);
        
        int byteIndex;
        buffers[bufferIndex] = (union buf*)malloc(sizeof(union buf));
        
        byteIndex = fread(buffers[bufferIndex]->bytes, 1, 64, fp);
        //printf("\n%s", buffers[bufferIndex]->bytes);
        totalBits += byteIndex * 8;
        // for (int i = 0; i < byteIndex; i++){
        //     printf("\n%02x", buffers[bufferIndex]->bytes[i]);
        // }

        if (byteIndex < 56)
        {
            buffers[bufferIndex]->bytes[byteIndex] = 0x80;
            byteIndex += 1;
            while (byteIndex < 56)
            {
                buffers[bufferIndex]->bytes[byteIndex] = 0x00;
                byteIndex += 1;
            }
            buffers[bufferIndex]->qwords[7] = chgEndUInt64(totalBits);
            bs = Done;
        }
        else if (byteIndex < 64)
        {
            buffers[bufferIndex]->bytes[byteIndex] = 0x80;
            byteIndex += 1;
            while (byteIndex < 64)
            {
                buffers[bufferIndex]->bytes[byteIndex] = 0x00;
                byteIndex += 1;
            }
            bs = AddPadBlock;
        }
        else if (feof(fp))
        {
            printf("end of file");
            bs = AddPadBlock;
        }

        if (bs != Done)
        {
            bufferIndex += 1;
        }
        
        if (bufferIndex == totalBuffers)
        {
            addBuffers();
        }
    }

    if (bs == AddPadBlock){
        for (int i = 0; i < 7; i++)
        {
            buffers[bufferIndex]->qwords[i] = 0x0000000000000000;
        }
        buffers[bufferIndex]->qwords[7] = chgEndUInt64(totalBits);
    }

    for (int i = 0; i < bufferIndex + 1; i++)
    {
        for (int j = 0; j < 64; j++)
        {
            printf("\n%02x", buffers[i]->bytes[j]);
        }
    }

    for (uint64_t i = 0; i < totalBuffers; i++)
    {
        printf("\nBuffer %" PRId64, i);
    }

    fclose(fp);

    return 0;
}

int main()
{
    char path[] = "C:/Users/Aurora/Desktop/thesis/data/sha.txt";

    buffGen(path);

    for (int i = 0; i < totalBuffers; i++){
        free(buffers[i]);
    }

    return 0;
}