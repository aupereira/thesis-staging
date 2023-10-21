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

struct bufNode
{
    union buf *buffer[64];
    struct bufNode *next;
};

enum bufferStatus {Continue, AddPadBlock, Done};

struct bufNode *head, *current;
uint64_t totalNodes, bufferIndex, totalBits;


// Add function to detect system Endianness.
uint32_t chgEndUInt32(uint32_t n)
{
    uint32_t r = n >> 24 | (n & 0x00FF0000) >> 8 | (n & 0x0000FF00) << 8 | n << 24;
    
    return r;
}

uint64_t chgEndUInt64(uint64_t n)
{
    uint64_t r = 
        (n >> 56) | (n >> 40) & 0x000000000000FF00 |
	    (n >> 24) & 0x0000000000FF0000 | (n >> 8) & 0x00000000FF000000 |
	    (n << 8) & 0x000000FF00000000 | (n << 24) & 0x0000FF0000000000 |
	    (n << 40) & 0x00FF000000000000 | (n << 56)
    ;

    return r;
}

void createBuffers()
{
    head = (struct bufNode *)malloc(sizeof(struct bufNode));
    current = head;

    if(head == NULL)
    {
        printf("Unable to allocate memory.");
        exit(0);
    }

    totalNodes = 1;
    bufferIndex = 0;
    return;
}

void addBuffers()
{
    struct bufNode *next;
    next = (struct bufNode *)malloc(sizeof(struct bufNode));

    if(next == NULL)
    {
        printf("Unable to allocate memory.");
        exit(0);
    }

    current->next = next;
    current = current->next;

    totalNodes += 1;
    bufferIndex = 0;
}

void buffGen(char filePath[])
{
    FILE *fp;
    totalBits = 0;
    enum bufferStatus bs;

    createBuffers();
    
    fp = fopen(filePath,"r");
    if(fp == NULL)
    {
        printf("Error opening file.");
        bs = Done;
        exit(0);
    }
    else
    {
        bs = Continue;
    }

    while (bs == Continue)
    {        
        int byteIndex;
        
        current->buffer[bufferIndex] = (union buf*)malloc(sizeof(union buf));

        if (current->buffer[bufferIndex] == NULL)
        {
            printf("Unable to allocate memory.");
            exit(0);
        }

        byteIndex = fread(current->buffer[bufferIndex]->bytes, 1, 64, fp);
        totalBits += byteIndex * 8;

        if (byteIndex < 56)
        {
            current->buffer[bufferIndex]->bytes[byteIndex] = 0x80;
            byteIndex += 1;
            while (byteIndex < 56)
            {
                current->buffer[bufferIndex]->bytes[byteIndex] = 0x00;
                byteIndex += 1;
            }
            current->buffer[bufferIndex]->qwords[7] = chgEndUInt64(totalBits);
            bs = Done;
        }
        else if (byteIndex < 64)
        {
            current->buffer[bufferIndex]->bytes[byteIndex] = 0x80;
            byteIndex += 1;
            while (byteIndex < 64)
            {
                current->buffer[bufferIndex]->bytes[byteIndex] = 0x00;
                byteIndex += 1;
            }
            bs = AddPadBlock;
        }
        else if (feof(fp))
        {
            bs = AddPadBlock;
        }

        if (bs != Done)
        {
            bufferIndex += 1;
        }
        
        if (bufferIndex == 64)
        {
            addBuffers();
        }
    }

    if (bs == AddPadBlock){
        for (int i = 0; i < 7; i++)
        {
            current->buffer[bufferIndex]->qwords[i] = 0x0000000000000000;
        }
        current->buffer[bufferIndex]->qwords[7] = chgEndUInt64(totalBits);
    }

    fclose(fp);

    return;
}

int main()
{
    // Add command line args for file path.
    char path[] = "C:/Users/Aurora/Desktop/thesis/data/sha.txt";

    buffGen(path);

    current = head;
    for (int i = 0; i < totalNodes; i++)
    {
        if(i < totalNodes - 1)
        {
            for (int j = 0; j < 64; j++)
            {
                for (int k = 0; k < 64; k++)
                {
                    printf("\n%02x", current->buffer[j]->bytes[k]);
                }
            }
        }
        else
        {
            for (int j = 0; j < bufferIndex + 1; j++)
            {
                for (int k = 0; k < 64; k++)
                {
                    printf("\n%02x", current->buffer[j]->bytes[k]);
                }
            }
            break;
        }
        current = current->next;
    }

    // Add freeMem() function that properly handles this.
    current = head;
    for (int i = 0; i < totalNodes; i++)
    {
        free(current->buffer[i % 64]);
        if ((i + 1) % 64 == 0)
        {
            current = current->next;
        }
    }

    return 0;
}