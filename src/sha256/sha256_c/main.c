// https://nvlpubs.nist.gov/nistpubs/FIPS/NIST.FIPS.180-4.pdf

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#pragma clang diagnostic ignored "-Wdeprecated-declarations"

#define NODE_SIZE 4096

union buf
{
    uint8_t bytes[128];
    uint64_t qwords[16];
};

struct bufNode
{
    union buf *buffer[NODE_SIZE];
    struct bufNode *next;
};

enum bufferStatus
{
    Continue,
    AddPadBlock,
    Done
};

struct bufNode *head, *current;
uint64_t totalNodes, bufferIndex;
__uint128_t totalBits;

uint64_t K[] =
    {
        0x428a2f98d728ae22, 0x7137449123ef65cd, 0xb5c0fbcfec4d3b2f, 0xe9b5dba58189dbbc,
        0x3956c25bf348b538, 0x59f111f1b605d019, 0x923f82a4af194f9b, 0xab1c5ed5da6d8118,
        0xd807aa98a3030242, 0x12835b0145706fbe, 0x243185be4ee4b28c, 0x550c7dc3d5ffb4e2,
        0x72be5d74f27b896f, 0x80deb1fe3b1696b1, 0x9bdc06a725c71235, 0xc19bf174cf692694,
        0xe49b69c19ef14ad2, 0xefbe4786384f25e3, 0x0fc19dc68b8cd5b5, 0x240ca1cc77ac9c65,
        0x2de92c6f592b0275, 0x4a7484aa6ea6e483, 0x5cb0a9dcbd41fbd4, 0x76f988da831153b5,
        0x983e5152ee66dfab, 0xa831c66d2db43210, 0xb00327c898fb213f, 0xbf597fc7beef0ee4,
        0xc6e00bf33da88fc2, 0xd5a79147930aa725, 0x06ca6351e003826f, 0x142929670a0e6e70,
        0x27b70a8546d22ffc, 0x2e1b21385c26c926, 0x4d2c6dfc5ac42aed, 0x53380d139d95b3df,
        0x650a73548baf63de, 0x766a0abb3c77b2a8, 0x81c2c92e47edaee6, 0x92722c851482353b,
        0xa2bfe8a14cf10364, 0xa81a664bbc423001, 0xc24b8b70d0f89791, 0xc76c51a30654be30,
        0xd192e819d6ef5218, 0xd69906245565a910, 0xf40e35855771202a, 0x106aa07032bbd1b8,
        0x19a4c116b8d2d0c8, 0x1e376c085141ab53, 0x2748774cdf8eeb99, 0x34b0bcb5e19b48a8,
        0x391c0cb3c5c95a63, 0x4ed8aa4ae3418acb, 0x5b9cca4f7763e373, 0x682e6ff3d6b2b8a3,
        0x748f82ee5defb2fc, 0x78a5636f43172f60, 0x84c87814a1f0ab72, 0x8cc702081a6439ec,
        0x90befffa23631e28, 0xa4506cebde82bde9, 0xbef9a3f7b2c67915, 0xc67178f2e372532b,
        0xca273eceea26619c, 0xd186b8c721c0c207, 0xeada7dd6cde0eb1e, 0xf57d4f7fee6ed178,
        0x06f067aa72176fba, 0x0a637dc5a2c898a6, 0x113f9804bef90dae, 0x1b710b35131c471b,
        0x28db77f523047d84, 0x32caab7b40c72493, 0x3c9ebe0a15c9bebc, 0x431d67c49c100d4c,
        0x4cc5d4becb3e42b6, 0x597f299cfc657e2a, 0x5fcb6fab3ad6faec, 0x6c44198c4a475817
    };

#define rotr(x, n) ((x >> n) | (x << (64 - n)))

#define ch(x, y, z) ((x & y) ^ (~x & z))

#define maj(x, y, z) ((x & y) ^ (x & z) ^ (y & z))

#define Sig0(x) (rotr(x, 28) ^ rotr(x, 34) ^ rotr(x, 39))

#define Sig1(x) (rotr(x, 14) ^ rotr(x, 18) ^ rotr(x, 41))

#define sig0(x) (rotr(x, 1) ^ rotr(x, 8) ^ (x >> 7))

#define sig1(x) (rotr(x, 19) ^ rotr(x, 61) ^ (x >> 6))

#ifdef __x86_64__
uint64_t chgEndUInt64(uint64_t n)
{
    uint64_t r =
        (n >> 56) | (n >> 40) & 0x000000000000FF00 |
        (n >> 24) & 0x0000000000FF0000 | (n >> 8) & 0x00000000FF000000 |
        (n << 8) & 0x000000FF00000000 | (n << 24) & 0x0000FF0000000000 |
        (n << 40) & 0x00FF000000000000 | (n << 56);

    return r;
}
#endif

void createBuffers()
{
    head = (struct bufNode *)malloc(sizeof(struct bufNode));
    current = head;

    if (head == NULL)
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

    if (next == NULL)
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

    fp = fopen(filePath, "rb+");
    if (fp == NULL)
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
        current->buffer[bufferIndex] = (union buf *)malloc(sizeof(union buf));

        if (current->buffer[bufferIndex] == NULL)
        {
            printf("Unable to allocate memory.");
            exit(0);
        }

        byteIndex = fread(current->buffer[bufferIndex]->bytes, 1, 128, fp);
        totalBits += byteIndex * 8;

        if (byteIndex < 112)
        {
            current->buffer[bufferIndex]->bytes[byteIndex] = 0x80;
            byteIndex += 1;
            while (byteIndex < 112)
            {
                current->buffer[bufferIndex]->bytes[byteIndex] = 0x00;
                byteIndex += 1;
            }

#ifdef __x86_64__
            current->buffer[bufferIndex]->qwords[14] = chgEndUInt64((uint64_t)(totalBits >> 64));
            current->buffer[bufferIndex]->qwords[15] = chgEndUInt64((uint64_t)(totalBits));
#endif
#ifdef _M_ARM64
            current->buffer[bufferIndex]->qwords[14] = (uint64_t)(totalBits >> 64);
            current->buffer[bufferIndex]->qwords[15] = (uint64_t)(totalBits);
#endif

            byteIndex += 16;
            bs = Done;
        }
        else if (byteIndex < 128)
        {
            current->buffer[bufferIndex]->bytes[byteIndex] = 0x80;
            byteIndex += 1;
            while (byteIndex < 128)
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

        if (bufferIndex == NODE_SIZE)
        {
            addBuffers();
        }
    }

    if (bs == AddPadBlock)
    {
        current->buffer[bufferIndex] = (union buf *)malloc(sizeof(union buf));
        for (int i = 0; i < 14; i++)
        {
            current->buffer[bufferIndex]->qwords[i] = 0x0000000000000000;
        }

#ifdef __x86_64__
        current->buffer[bufferIndex]->qwords[14] = chgEndUInt64((uint64_t)(totalBits >> 64));
        current->buffer[bufferIndex]->qwords[15] = chgEndUInt64((uint64_t)(totalBits));
#endif
#ifdef _M_ARM64
        current->buffer[bufferIndex]->qwords[14] = (uint64_t)(totalBits >> 64);
        current->buffer[bufferIndex]->qwords[15] = (uint64_t)(totalBits);
#endif
    }

    fclose(fp);

    return;
}

void printBytes()
{
    current = head;
    for (int i = 0; i < totalNodes; i++)
    {
        if (i < totalNodes - 1)
        {
            for (int j = 0; j < NODE_SIZE; j++)
            {
                for (int k = 0; k < 128; k++)
                {
                    printf("\n%02x", current->buffer[j]->bytes[k]);
                }
            }
        }
        else
        {
            for (int j = 0; j < bufferIndex + 1; j++)
            {
                for (int k = 0; k < 128; k++)
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
        if (i < totalNodes - 1)
        {
            for (int j = 0; j < NODE_SIZE; j++)
            {
                free(current->buffer[j]);
            }
        }
        else
        {
            for (int j = 0; j < bufferIndex + 1; j++)
            {
                free(current->buffer[j]);
            }
        }
        current = current->next;
    }
}

void sha512()
{
    uint64_t H[8] =
        {
            0x6a09e667f3bcc908, 0xbb67ae8584caa73b, 0x3c6ef372fe94f82b, 0xa54ff53a5f1d36f1,
            0x510e527fade682d1, 0x9b05688c2b3e6c1f, 0x1f83d9abfb41bd6b, 0x5be0cd19137e2179};

    uint64_t a, b, c, d, e, f, g, h, T1, T2;
    uint64_t W[80];

    int si = NODE_SIZE;

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
#ifdef __x86_64__
                W[i] = chgEndUInt64(current->buffer[bu]->qwords[i]);
#endif
#ifdef _M_ARM64
                W[i] = current->buffer[bu]->qwords[i];
#endif
            }

            for (int i = 16; i < 80; i++)
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

            for (int i = 0; i < 80; i++)
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
        printf("%016llx", H[i]);
    }

    return;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Invalid number of arguments.\nPlease enter a file path.\n");
        exit(0);
    }

    buffGen(argv[1]);

    sha512();

    // printBytes();
    // freeBuffers();

    return 0;
}