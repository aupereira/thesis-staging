// To-Do List:
// Switch-case statemtents instead of if-else to make it more readable.
// Compact the bulky function code.
// Revamp memory management because it's a mess.
// Maybe some level of parallelism, though how much has yet to be decided.

// https://nvlpubs.nist.gov/nistpubs/FIPS/NIST.FIPS.180-4.pdf

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#pragma clang diagnostic ignored "-Wswitch-bool"

#include <inttypes.h>

#define NODE_SIZE 4096

enum bufferStatus {Continue, AddPadBlock, Done} bs;

FILE *fp;
size_t s_buffers;
bool littleEndian;
uint64_t totalNodes, bufferIndex, totalBits, totalBytes;
uint64_t fileSize;

uint32_t H[8] =
{
    0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 
    0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
};
uint32_t a, b, c, d, e, f, g, h, T1, T2;
uint32_t W[64];

union buf
{
    uint8_t bytes[64];
    uint32_t dwords[16];
    uint64_t qwords[8];
};

union buf* buffers;

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
    return (*p == 0x01);
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

uint64_t fillBuffer()
{
    union buf* bufPtr = buffers;
    
    size_t rBytes = fread(bufPtr->bytes, 1, 64 * NODE_SIZE, fp);
    totalBytes += rBytes;

    // Move this into the part where total bits are added.
    
    uint64_t blocks;
    uint64_t r = rBytes % 64;
    switch (r)
    {
        case 0:
            blocks = rBytes / 64;
            break;
        default:
            blocks = rBytes / 64 + 1;
            break;
    }

    if (feof(fp))
    {
        // totalBlocks = byte_i / 64;
        
        bufPtr += blocks - 1;

        if (r < 56)
        {
            bufPtr->bytes[r] = 0x80;
            r += 1;
            while (r < 56)
            {
                bufPtr->bytes[r] = 0x00;
                r += 1;
            }
            totalBits = totalBytes * 8;
            switch (littleEndian)
            {
                case 1:
                    bufPtr->qwords[7] = chgEndUInt64(totalBits);
                    break;
                default: 
                    bufPtr->qwords[7] = totalBits;
                    break;
            }
            // r += 8;
            bs = Done;
        }
        else if (r < 64)
        {
            bufPtr->bytes[r] = 0x80;
            r += 1;
            while (r < 64)
            {
                bufPtr->bytes[r] = 0x00;
                r += 1;
            }
            bs = AddPadBlock;
            blocks += 1; //remember to handle this correctly if overflow to new buffer.
        }
        else
        {
            bs = AddPadBlock;
            blocks += 1; //remember to handle this correctly if overflow to new buffer.
        }
    }
    if (bs == AddPadBlock && blocks != NODE_SIZE)
    {
        bufPtr += 1;
        for (int i = 0; i < 7; i++)
        {
            bufPtr->qwords[i] = 0x0000000000000000;
        }
        totalBits = totalBytes * 8;
        switch (littleEndian)
        {
            case 1:
                bufPtr->qwords[7] = chgEndUInt64(totalBits);
                break;
            default: 
                bufPtr->qwords[7] = totalBits;
                break;
        }
    }

    printf("\n");
    bufPtr = buffers;
    for (int i = 0; i < blocks; i++)
    {
        for (int j = 0; j < 64; j++)
        {
            //printf("%p\n", &bufPtr->bytes[j]);
            printf("\n%02x", bufPtr->bytes[j]);
        }
        bufPtr += 1;
    }
    printf("\n");

    return blocks;
    // Need to add something to handle the case where the block is full and we need a new buffer with just a pad block.
}

void hashGen(uint64_t blocks)
{
    union buf* bufPtr = buffers;

    for (int b = 0; b < blocks; b++)
    {
        printf("\nB %d", b);
        printf("\nPtr %p", bufPtr);
        for (int i = 0; i < 16; i++)
        {
            switch (littleEndian)
            {
                case 1:
                    W[i] = chgEndUInt32(bufPtr->dwords[i]);
                    break;
                default:
                    W[i] = bufPtr->dwords[i];
                    break;
            }
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

        bufPtr += 1;
        printf("\nPtr %p", bufPtr);
    }
    return;
}

void sha256(char filePath[])
{
    uint64_t bytes;
    totalBits = 0;
    
    fp = fopen(filePath,"rb+");
    if (fp == NULL)
    {
        printf("Error opening file.");
        exit(0);
    }

    size_t s_buffers = sizeof(union buf) * NODE_SIZE;
    buffers = (union buf*) malloc(s_buffers);

    if (buffers == NULL)
    {
        printf("Error allocating memory.");
        exit(0);
    }

    // FSEEK BROKEN METHOD FOR USE OF FREAD_S
        // if (fseek(fp, 0L, SEEK_END) != 0)
        // {
        //     printf("Error seeking to the end of the file.");
        //     exit(0);
        // }

        // fileSize = _ftelli64(fp);
        // if (fileSize == -1)
        // {
        //     printf("Error telling file size.");
        //     exit(0);
        // }
        // printf("FS%lld", fileSize);

        // if (fseek(fp, 0L, SEEK_SET) != 0)
        // {
        //     printf("Error seeking to the beginning of the file.");
        //     exit(0);
        // }
        // rewind(fp);

//    uint32_t H[8] =



    bs = Continue;

    while (bs == Continue)
    {
        bytes = fillBuffer();
        hashGen(bytes);
    }

    fclose(fp);

    printf("\n\n");
    for (int i = 0; i < 8; i++)
    {
        printf("%08llx", H[i]);
    }

    free(buffers);

    return;
}

int main(int argc, char *argv[])
{
    if (argc != 2){
        printf("Invalid number of arguments.\nPlease enter a file path.\n");
        exit(0);
    }

    littleEndian = isLittleEndian();

    sha256(argv[1]);

    //sha256();

    // printBytes();
    // freeBuffers();

    return 0;
}