// https://nvlpubs.nist.gov/nistpubs/FIPS/NIST.FIPS.180-4.pdf

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

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

uint32_t K[] =
{
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 
    0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
    0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
    0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967, 
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
    0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
    0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3, 
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

bool isLittleEndian()
{
    uint32_t n = 0x00000001;
    uint8_t *p = (uint8_t *)&n;
    return *p == 0x01;
}

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

uint32_t rotl(uint32_t x, uint8_t n)
{
    return (x << n) | (x >> (32 - n));
}

uint32_t rotr(uint32_t x, uint8_t n)
{
    return (x >> n) | (x << (32 - n));
}

uint32_t shr(uint32_t x, uint8_t n)
{
    return x >> n;
}

uint32_t ch(uint32_t x, uint32_t y, uint32_t z)
{
    return (x & y) ^ (~x & z);
}

uint32_t maj(uint32_t x, uint32_t y, uint32_t z)
{
    return (x & y) ^ (x & z) ^ (y & z);
}

uint32_t Sig0(uint32_t x)
{
    return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22);
}

uint32_t Sig1(uint32_t x)
{
    return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25);
}

uint32_t sig0(uint32_t x)
{
    return rotr(x, 7) ^ rotr(x, 18) ^ shr(x, 3);
}

uint32_t sig1(uint32_t x)
{
    return rotr(x, 17) ^ rotr(x, 19) ^ shr(x, 10);
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

    int byteIndex;

    createBuffers();
    
    fp = fopen(filePath,"rb+");
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
            byteIndex += 8;
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
        current->buffer[bufferIndex] = (union buf*)malloc(sizeof(union buf));
        for (int i = 0; i < 7; i++)
        {
            current->buffer[bufferIndex]->qwords[i] = 0x0000000000000000;
        }
        current->buffer[bufferIndex]->qwords[7] = chgEndUInt64(totalBits);
    }

    fclose(fp);

    return;
}

void printBytes()
{
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
            return;
        }
        current = current->next;
    }
    return;
}

void freeBuffers()
{
    current = head;
    for (int i = 0; i < totalNodes; i++)
    {
        if (i < totalNodes - 1){        
            for(int j = 0; j < 64; j++)
            {
                free(current->buffer[j]);
            }
        }
        else
        {
            for(int j = 0; j < bufferIndex + 1; j++)
            {
                free(current->buffer[j]);
            }
        }
        current = current->next;
    }
}

void sha256()
{
    uint32_t H[8] = 
    {
        0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 
        0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
    };

    uint32_t a, b, c, d, e, f, g, h, T1, T2;
    uint32_t W[64];

    int si = 64;

    current = head;
    struct bufNode *temp;

    for (uint64_t n = 0; n < totalNodes; n++)
    {
        if (n == totalNodes - 1)
        {
            si = bufferIndex + 1;
        }

        for (int bu = 0; bu < si; bu++)
        {
            for (int i = 0; i < 16; i++)
            {
                W[i] = chgEndUInt32(current->buffer[bu]->dwords[i]);
            }

            for (int i = 16; i < 64; i++)
            {
                W[i] = sig1(W[i - 2]) + W[i - 7] + sig0(W[i - 15]) + W[i - 16];
            }

            a = H[0];
            b = H[1];
            c = H[2];
            d = H[3];
            e = H[4];
            f = H[5];
            g = H[6];
            h = H[7];

            for (int i = 0; i < 64; i++)
            {
                T1 = h + Sig1(e) + ch(e, f, g) + K[i] + W[i];
                T2 = Sig0(a) + maj(a, b, c);
                h = g;
                g = f;
                f = e;
                e = d + T1;
                d = c;
                c = b;
                b = a;
                a = T1 + T2;
            }

            H[0] += a;
            H[1] += b;
            H[2] += c;
            H[3] += d;
            H[4] += e;
            H[5] += f;
            H[6] += g;
            H[7] += h;

            free(current->buffer[bu]);
        }
        temp = current->next;
        free(current);
        current = temp;
    }

    for (int i = 0; i < 8; i++)
    {
        printf("%08llx", H[i]);
    }

    return;
}

int main()
{
    // Add command line args for file path.
    char path[] = "C:/Users/Aurora/Desktop/thesis/data/sha.txt";

    bool littleEndian = isLittleEndian();

    buffGen(path);

    sha256();

    // printBytes();
    // freeBuffers();

    return 0;
}