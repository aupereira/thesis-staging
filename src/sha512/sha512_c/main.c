// https://nvlpubs.nist.gov/nistpubs/FIPS/NIST.FIPS.180-4.pdf

#ifdef __linux__
#define _FILE_OFFSET_BITS 64
#endif

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#pragma clang diagnostic ignored "-Wdeprecated-declarations"

#define BUFFER_SIZE 1024

union Block
{
    uint8_t bytes[128];
    uint64_t qwords[16];
};

union BlockBuffer
{
    uint8_t bytes[BUFFER_SIZE * 128];
    union Block blocks[BUFFER_SIZE];
};

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

uint64_t chgEndUInt64(uint64_t n)
{
    uint64_t r =
        (n >> 56) | (n >> 40) & 0x000000000000FF00 |
        (n >> 24) & 0x0000000000FF0000 | (n >> 8) & 0x00000000FF000000 |
        (n << 8) & 0x000000FF00000000 | (n << 24) & 0x0000FF0000000000 |
        (n << 40) & 0x00FF000000000000 | (n << 56);

    return r;
}

void finalBlock(union BlockBuffer *buffer, size_t bytes, __uint128_t totalBits)
{
    size_t block = ((bytes - 1) / 128);
    size_t byte = bytes % 128;

    if (byte < 112 && byte > 0)
    {
        buffer->blocks[block].bytes[byte] = 0x80;
        for (size_t i = byte + 1; i < 112; i++)
        {
            buffer->blocks[block].bytes[i] = 0x00;
        }
        buffer->blocks[block].qwords[14] = chgEndUInt64((uint64_t)(totalBits >> 64));
        buffer->blocks[block].qwords[15] = chgEndUInt64((uint64_t)(totalBits));
    }

    else if (byte < 128 && byte > 0)
    {
        buffer->blocks[block].bytes[byte] = 0x80;
        for (size_t i = byte + 1; i < 128; i++)
        {
            buffer->blocks[block].bytes[i] = 0x00;
        }
        if (block < BUFFER_SIZE - 1)
        {
            for (size_t i = 0; i < 14; i++)
            {
                buffer->blocks[block + 1].qwords[i] = 0;
            }
            buffer->blocks[block + 1].qwords[14] = chgEndUInt64((uint64_t)(totalBits >> 64));
            buffer->blocks[block + 1].qwords[15] = chgEndUInt64((uint64_t)(totalBits));
        }
    }

    else
    {
        if (block < BUFFER_SIZE - 1)
        {
            buffer->blocks[block + 1].qwords[0] = 0x0000000000000080;
            for (size_t i = 1; i < 14; i++)
            {
                buffer->blocks[block + 1].qwords[i] = 0;
            }
            buffer->blocks[block + 1].qwords[14] = chgEndUInt64((uint64_t)(totalBits >> 64));
            buffer->blocks[block + 1].qwords[15] = chgEndUInt64((uint64_t)(totalBits));
        }
    }
}

void addPadding(union BlockBuffer *buffer, __uint128_t totalBits, size_t addEOS)
{
    for (size_t i = 0; i < 14; i++)
    {
        buffer->blocks[0].qwords[i] = 0;
    }

    if (addEOS == 1)
    {
        buffer->blocks[0].qwords[0] = 0x0000000000000080;
    }

    buffer->blocks[0].qwords[14] = chgEndUInt64((uint64_t)(totalBits >> 64));
    buffer->blocks[0].qwords[15] = chgEndUInt64((uint64_t)(totalBits));
}

void sha512Rounds(union BlockBuffer *buffer, uint64_t state[8], size_t iter)
{
    uint64_t temp[10];
    uint64_t w[80];

    for (size_t block = 0; block < iter; block++)
    {
        for (size_t i = 0; i < 16; i++)
        {
            w[i] = chgEndUInt64(buffer->blocks[block].qwords[i]);
        }

        for (size_t i = 16; i < 80; i++)
        {
            w[i] = sig1(w[i - 2]) + w[i - 7] + sig0(w[i - 15]) + w[i - 16];
        }

        temp[0] = state[0];
        temp[1] = state[1];
        temp[2] = state[2];
        temp[3] = state[3];
        temp[4] = state[4];
        temp[5] = state[5];
        temp[6] = state[6];
        temp[7] = state[7];

        for (size_t i = 0; i < 80; i++)
        {
            temp[8] = temp[7] + Sig1(temp[4]) + ch(temp[4], temp[5], temp[6]) + K[i] + w[i];
            temp[9] = Sig0(temp[0]) + maj(temp[0], temp[1], temp[2]);
            temp[7] = temp[6];
            temp[6] = temp[5];
            temp[5] = temp[4];
            temp[4] = temp[3] + temp[8];
            temp[3] = temp[2];
            temp[2] = temp[1];
            temp[1] = temp[0];
            temp[0] = temp[8] + temp[9];
        }

        state[0] += temp[0];
        state[1] += temp[1];
        state[2] += temp[2];
        state[3] += temp[3];
        state[4] += temp[4];
        state[5] += temp[5];
        state[6] += temp[6];
        state[7] += temp[7];
    }
}

void sha512(char *filePath)
{
    int64_t totalBytes;
    __uint128_t totalBits;
    size_t bytes;

    union BlockBuffer buffers;

    uint64_t state[8] =
        {
            0x6a09e667f3bcc908, 0xbb67ae8584caa73b, 0x3c6ef372fe94f82b, 0xa54ff53a5f1d36f1,
            0x510e527fade682d1, 0x9b05688c2b3e6c1f, 0x1f83d9abfb41bd6b, 0x5be0cd19137e2179
        };

    FILE *f;
    f = fopen(filePath, "rb+");
    if (f == NULL)
    {
        printf("Error opening file.");
        exit(1);
    }

#ifdef __linux__
    fseek(f, 0, SEEK_END);
    totalBytes = (size_t)ftell(f);
    fseek(f, 0, SEEK_SET);
#endif
#ifdef __WIN64__
    _fseeki64(f, 0, SEEK_END);
    totalBytes = (size_t)_ftelli64(f);
    _fseeki64(f, 0, SEEK_SET);
#endif

    if (totalBytes == -1)
    {
        printf("Error reading file size.");
        exit(1);
    }

    if (totalBytes == 0)
    {
        buffers.blocks[0].bytes[0] = 0x80;
        for (size_t i = 1; i < 128; i++)
        {
            buffers.blocks[0].bytes[i] = 0x00;
        }
        sha512Rounds(&buffers, state, 1);
        for (size_t i = 0; i < 8; i++)
        {
            printf("%016llx", state[i]);
        }
        exit(0);
    }

    totalBits = (__uint128_t)(totalBytes) * 8;

    for (size_t i = 0; i < ((totalBytes - 1) / (BUFFER_SIZE * 128)); i++)
    {
        bytes = fread(buffers.bytes, 1, BUFFER_SIZE * 128, f);
        sha512Rounds(&buffers, state, BUFFER_SIZE);
    }

    bytes = fread(buffers.bytes, 1, BUFFER_SIZE * 128, f);
    finalBlock(&buffers, bytes, totalBits);

    if (bytes > BUFFER_SIZE * 128 - 17)
    {
        sha512Rounds(&buffers, state, BUFFER_SIZE);
        addPadding(&buffers, totalBits, (bytes % 128 == 0));
        sha512Rounds(&buffers, state, 1);
    }
    else
    {
        sha512Rounds(&buffers, state, (bytes + 144) / 128);
    }

    for (size_t i = 0; i < 8; i++)
    {
        printf("%016llx", state[i]);
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Invalid number of arguments.\nPlease enter a file path.\n");
        exit(1);
    }

    sha512(argv[1]);

    return 0;
}