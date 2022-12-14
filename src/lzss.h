#ifndef H_LZSS
#define H_LZSS

// The code by Patrice Mandin
// https://github.com/pmandin/reevengi-tools/wiki/.ADT-(Resident-Evil-2-PC)

/* Uncompressed data */

/* Note: the game allocates 32KB, then allocate more 32KB */
/* if the dstOffset reach the 32KB limit */

#include "types.h"

uint8* dstPointer;
int32 dstOffset; /* Position where to write data in the dstPointer buffer */

/* Source data */

uint8* srcPointer; /* pointer to source file */
int32 srcOffset; /* position in source file */
int32 srcLength; /* length of source file */
int32 srcNumBit; /* current bit in source file */
uint8 srcByte; /* Current byte read from file */

/* Unpack structure */

typedef struct
{
    int32 start;
    int32 length;
} unpackArray8_t;

typedef struct
{
    uint32 start;
    int32 length;
    uint32* ptr4;
    unpackArray8_t* ptr8;
    uint32* ptr16;
} unpackArray_t;

unpackArray_t array1, array2, array3;

uint8 tmp32k[32768]; /* Temporary 32KB buffer */
uint32 tmp32kOffset; /* Position in temp buffer */

uint8 tmp16k[16384]; /* Temporary 16KB buffer */
int32 tmp16kOffset;

uint16 freqArray[17];

void initTmpArray(unpackArray_t* array, int32 start, int32 length)
{
    array->start = start;

    array->length = length;

    array->ptr16 = (uint32*)&tmp32k[tmp32kOffset];
    tmp32kOffset += length << 5;

    array->ptr8 = (unpackArray8_t*)&tmp32k[tmp32kOffset];
    tmp32kOffset += length << 3;

    array->ptr4 = (uint32*)&tmp32k[tmp32kOffset];
    tmp32kOffset += length << 2;
}

void initTmpArrayData(unpackArray_t* array)
{
    int32 i;

    for (i = 0; i < array->length; i++)
    {
        array->ptr4[i] = array->ptr8[i].start = array->ptr8[i].length = array->ptr16[(i << 2)] = 0;
        array->ptr16[(i << 2) + 1] = array->ptr16[(i << 2) + 2] = array->ptr16[(i << 2) + 3] = 0xffffffff;
    }

    while (i < array->length << 1)
    {
        array->ptr16[(i << 2)] = 0;
        array->ptr16[(i << 2) + 1] = array->ptr16[(i << 2) + 2] = array->ptr16[(i << 2) + 3] = 0xffffffff;
        i++;
    }
}

int32 readSrcBits(int32 numBits)
{
    int32 orMask = 0, andMask;
    int32 finalValue;

    finalValue = srcByte;

    while (numBits > srcNumBit)
    {
        numBits -= srcNumBit;
        andMask = (1 << srcNumBit) - 1;
        andMask &= finalValue;
        andMask <<= numBits;
        if (srcOffset < srcLength)
        {
            finalValue = srcByte = srcPointer[srcOffset++];
        }
        else
        {
            finalValue = srcByte = 0;
        }
        /*finalValue = srcByte = srcPointer[srcOffset++];*/
        srcNumBit = 8;
        orMask |= andMask;
    }

    srcNumBit -= numBits;
    finalValue >>= srcNumBit;
    finalValue = (finalValue & ((1 << numBits) - 1)) | orMask;
    return finalValue;
}

int32 readSrcOneBit(void)
{
    srcNumBit--;
    if (srcNumBit < 0)
    {
        srcNumBit = 7;
        if (srcOffset < srcLength)
        {
            srcByte = srcPointer[srcOffset++];
        }
        else
        {
            srcByte = 0;
        }
    }

    return (srcByte >> srcNumBit) & 1;
}

int32 readSrcBitfieldArray(unpackArray_t* array, int32 curIndex)
{
    do
    {
        if (readSrcOneBit())
        {
            curIndex = array->ptr16[(curIndex << 2) + 3];
        }
        else
        {
            curIndex = array->ptr16[(curIndex << 2) + 2];
        }
    } while (curIndex >= array->length);

    return curIndex;
}

int32 readSrcBitfield(void)
{
    int32 numZeroBits = 0;
    int32 bitfieldValue = 1;

    while (readSrcOneBit() == 0)
    {
        numZeroBits++;
    }

    while (numZeroBits > 0)
    {
        bitfieldValue = readSrcOneBit() + (bitfieldValue << 1);
        numZeroBits--;
    }

    return bitfieldValue;
}

void initUnpackBlockArray(unpackArray_t* array)
{
    uint16 tmp[18];
    int32 i, j;

    memset(tmp, 0, sizeof(tmp));

    for (i = 0; i < 16; i++)
    {
        tmp[i + 2] = (tmp[i + 1] + freqArray[i + 1]) << 1;
    }

    for (i = 0; i < 18; i++)
    {
        int32 startTmp = tmp[i];
        for (j = 0; j < array->length; j++)
        {
            if (array->ptr8[j].length == i)
            {
                array->ptr8[j].start = tmp[i]++ & 0xffff;
            }
        }
    }
}

int32 initUnpackBlockArray2(unpackArray_t* array)
{
    int32 i, j;
    int32 curLength = array->length;
    int32 curArrayIndex = curLength + 1;
    array->ptr16[(curLength << 2) + 2] = 0xffffffff;
    array->ptr16[(curLength << 2) + 3] = 0xffffffff;
    array->ptr16[(curArrayIndex << 2) + 2] = 0xffffffff;
    array->ptr16[(curArrayIndex << 2) + 3] = 0xffffffff;

    for (i = 0; i < array->length; i++)
    {
        curLength = array->length;

        int32 curPtr8Start = array->ptr8[i].start;
        int32 curPtr8Length = array->ptr8[i].length;

        for (j = 0; j < curPtr8Length; j++)
        {
            int32 curMask = 1 << (curPtr8Length - j - 1);
            int32 arrayOffset;

            if ((curMask & curPtr8Start) != 0)
            {
                arrayOffset = 3;
            }
            else
            {
                arrayOffset = 2;
            }

            if (j + 1 == curPtr8Length)
            {
                array->ptr16[(curLength << 2) + arrayOffset] = i;
                break;
            }

            if (array->ptr16[(curLength << 2) + arrayOffset] == -1)
            {
                array->ptr16[(curLength << 2) + arrayOffset] = curArrayIndex;
                array->ptr16[(curArrayIndex << 2) + 2] = array->ptr16[(curArrayIndex << 2) + 3] = -1;
                curLength = curArrayIndex++;
            }
            else
            {
                curLength = array->ptr16[(curLength << 2) + arrayOffset];
            }
        }
    }

    return array->length;
}

void initUnpackBlock(void)
{
    int32 i, j, prevValue, curBit, curBitfield;
    int32 numValues;
    uint16 tmp[512];
    uint32 tmpBufLen;

    /* Initialize array 1 to unpack block */

    prevValue = 0;
    for (i = 0; i < array1.length; i++)
    {
        if (readSrcOneBit())
        {
            array1.ptr8[i].length = readSrcBitfield() ^ prevValue;
        }
        else
        {
            array1.ptr8[i].length = prevValue;
        }
        prevValue = array1.ptr8[i].length;
    }

    /* Count frequency of values in array 1 */
    memset(freqArray, 0, sizeof(freqArray));

    for (i = 0; i < array1.length; i++)
    {
        numValues = array1.ptr8[i].length;
        if (numValues <= 16)
        {
            freqArray[numValues]++;
        }
    }

    initUnpackBlockArray(&array1);
    tmpBufLen = initUnpackBlockArray2(&array1);

    /* Initialize array 2 to unpack block */

    if (array2.length > 0)
    {
        memset(tmp, 0, array2.length);
    }

    curBit = readSrcOneBit();
    j = 0;
    while (j < array2.length)
    {
        if (curBit)
        {
            curBitfield = readSrcBitfield();
            for (i = 0; i < curBitfield; i++)
            {
                tmp[j + i] = readSrcBitfieldArray(&array1, tmpBufLen);
            }
            j += curBitfield;
            curBit = 0;
            continue;
        }

        curBitfield = readSrcBitfield();
        if (curBitfield > 0)
        {
            memset(&tmp[j], 0, curBitfield * sizeof(uint16));
            j += curBitfield;
        }
        curBit = 1;
    }

    j = 0;
    for (i = 0; i < array2.length; i++)
    {
        j = j ^ tmp[i];
        array2.ptr8[i].length = j;
    }

    /* Count frequency of values in array 2 */
    memset(freqArray, 0, sizeof(freqArray));

    for (i = 0; i < array2.length; i++)
    {
        numValues = array2.ptr8[i].length;
        if (numValues <= 16)
        {
            freqArray[numValues]++;
        }
    }

    initUnpackBlockArray(&array2);

    /* Initialize array 3 to unpack block */

    prevValue = 0;
    for (i = 0; i < array3.length; i++)
    {
        if (readSrcOneBit())
        {
            array3.ptr8[i].length = readSrcBitfield() ^ prevValue;
        }
        else
        {
            array3.ptr8[i].length = prevValue;
        }
        prevValue = array3.ptr8[i].length;
    }

    /* Count frequency of values in array 3 */
    memset(freqArray, 0, sizeof(freqArray));

    for (i = 0; i < array3.length; i++)
    {
        numValues = array3.ptr8[i].length;
        if (numValues <= 16)
        {
            freqArray[numValues]++;
        }
    }

    initUnpackBlockArray(&array3);
}

int32 unpackImage(uint8* source, int32 length, uint8* destination)
{
    int32 blockLength, curBlockLength;
    int32 tmpBufLen, tmpBufLen1;
    int32 i;

    srcPointer = source;
    srcOffset = 0;
    srcLength = length;

    dstPointer = destination;
    dstOffset = 0;

    srcNumBit = 0;
    srcByte = 0;

    tmp16kOffset = 0;
    tmp32kOffset = 0;

    initTmpArray(&array1, 8, 16);
    initTmpArray(&array2, 8, 512);
    initTmpArray(&array3, 8, 16);

    initTmpArrayData(&array1);
    initTmpArrayData(&array2);
    initTmpArrayData(&array3);

    memset(tmp16k, 0, sizeof(tmp16k));

    blockLength = readSrcBits(8);
    blockLength |= readSrcBits(8) << 8;
    while (blockLength > 0)
    {
        initUnpackBlock();

        tmpBufLen = initUnpackBlockArray2(&array2);
        tmpBufLen1 = initUnpackBlockArray2(&array3);

        curBlockLength = 0;
        while (curBlockLength < blockLength)
        {
            int32 curBitfield = readSrcBitfieldArray(&array2, tmpBufLen);

            if (curBitfield < 256)
            {
                dstPointer[dstOffset++] = tmp16k[tmp16kOffset++] = curBitfield;
                tmp16kOffset &= 0x3fff;
            }
            else
            {
                int32 i;
                int32 numValues = curBitfield - 0xfd;
                int32 startOffset;
                curBitfield = readSrcBitfieldArray(&array3, tmpBufLen1);
                if (curBitfield != 0)
                {
                    int32 numBits = curBitfield - 1;
                    curBitfield = readSrcBits(numBits) & 0xffff;
                    curBitfield += 1 << numBits;
                }

                startOffset = (tmp16kOffset - curBitfield - 1) & 0x3fff;
                for (i = 0; i < numValues; i++)
                {
                    dstPointer[dstOffset++] = tmp16k[tmp16kOffset++] = tmp16k[startOffset++];
                    startOffset &= 0x3fff;
                    tmp16kOffset &= 0x3fff;
                }
            }

            curBlockLength++;
        }

        blockLength = readSrcBits(8);
        blockLength |= readSrcBits(8) << 8;
    }

    return dstOffset;
}

#endif
