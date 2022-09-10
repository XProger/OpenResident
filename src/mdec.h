#ifndef H_MDEC
#define H_MDEC

#include "types.h"

// https://psx-spx.consoledev.net/macroblockdecodermdec/
// https://github.com/grumpycoders/pcsx-redux/
// PlayStation1_STR_format1-00.txt
#define AAN_CONST_BITS      12
#define AAN_EXTRA           12
#define SCALE(x, n)         ((x) >> (n))
#define SCALER(x, n)        (((x) + ((1 << (n)) >> 1)) >> (n))
#define SCALE8(c)           SCALER(c, 20)
#define FIX_1_082392200     SCALER(18159528, AAN_CONST_BITS)
#define FIX_1_414213562     SCALER(23726566, AAN_CONST_BITS)
#define FIX_1_847759065     SCALER(31000253, AAN_CONST_BITS)
#define FIX_2_613125930     SCALER(43840978, AAN_CONST_BITS)
#define MULR(a)             ((1434 * (a)))
#define MULB(a)             ((1807 * (a)))
#define MULG2(a, b)         ((-351 * (a)-728 * (b)))
#define MULY(a)             ((a) << 10)
#define CLAMP8(c)           (((c) < -128) ? 0 : (((c) > (255 - 128)) ? 255 : ((c) + 128)))
#define CLAMP_SCALE8(a)     CLAMP8(SCALE8(a))

struct AC_ENTRY
{
    uint8 code;
    uint8 skip;
    uint8 ac;
    uint8 length;
};

// ISO 13818-2 table B-14
const AC_ENTRY MDEC_AC[] = {
    // signBit = (8 + shift) - length
    // AC_LUT_1 (shift = 1)
    { 0XC0 , 1  , 1  , 4  }, // 11000000
    { 0X80 , 0  , 2  , 5  }, // 10000000
    { 0XA0 , 2  , 1  , 5  }, // 10100000
    { 0X50 , 0  , 3  , 6  }, // 01010000
    { 0X60 , 4  , 1  , 6  }, // 01100000
    { 0X70 , 3  , 1  , 6  }, // 01110000
    { 0X20 , 7  , 1  , 7  }, // 00100000
    { 0X28 , 6  , 1  , 7  }, // 00101000
    { 0X30 , 1  , 2  , 7  }, // 00110000
    { 0X38 , 5  , 1  , 7  }, // 00111000
    { 0X10 , 2  , 2  , 8  }, // 00010000
    { 0X14 , 9  , 1  , 8  }, // 00010100
    { 0X18 , 0  , 4  , 8  }, // 00011000
    { 0X1C , 8  , 1  , 8  }, // 00011100
    { 0X40 , 13 , 1  , 9  }, // 01000000
    { 0X42 , 0  , 6  , 9  }, // 01000010
    { 0X44 , 12 , 1  , 9  }, // 01000100
    { 0X46 , 11 , 1  , 9  }, // 01000110
    { 0X48 , 3  , 2  , 9  }, // 01001000
    { 0X4A , 1  , 3  , 9  }, // 01001010
    { 0X4C , 0  , 5  , 9  }, // 01001100
    { 0X4E , 10 , 1  , 9  }, // 01001110
// AC_LUT_6 (shift = 6)
    { 0X80 , 16 , 1  , 11 }, // 10000000
    { 0X90 , 5  , 2  , 11 }, // 10010000
    { 0XA0 , 0  , 7  , 11 }, // 10100000
    { 0XB0 , 2  , 3  , 11 }, // 10110000
    { 0XC0 , 1  , 4  , 11 }, // 11000000
    { 0XD0 , 15 , 1  , 11 }, // 11010000
    { 0XE0 , 14 , 1  , 11 }, // 11100000
    { 0XF0 , 4  , 2  , 11 }, // 11110000
    { 0X40 , 0  , 11 , 13 }, // 01000000
    { 0X44 , 8  , 2  , 13 }, // 01000100
    { 0X48 , 4  , 3  , 13 }, // 01001000
    { 0X4C , 0  , 10 , 13 }, // 01001100
    { 0X50 , 2  , 4  , 13 }, // 01010000
    { 0X54 , 7  , 2  , 13 }, // 01010100
    { 0X58 , 21 , 1  , 13 }, // 01011000
    { 0X5C , 20 , 1  , 13 }, // 01011100
    { 0X60 , 0  , 9  , 13 }, // 01100000
    { 0X64 , 19 , 1  , 13 }, // 01100100
    { 0X68 , 18 , 1  , 13 }, // 01101000
    { 0X6C , 1  , 5  , 13 }, // 01101100
    { 0X70 , 3  , 3  , 13 }, // 01110000
    { 0X74 , 0  , 8  , 13 }, // 01110100
    { 0X78 , 6  , 2  , 13 }, // 01111000
    { 0X7C , 17 , 1  , 13 }, // 01111100
    { 0X20 , 10 , 2  , 14 }, // 00100000
    { 0X22 , 9  , 2  , 14 }, // 00100010
    { 0X24 , 5  , 3  , 14 }, // 00100100
    { 0X26 , 3  , 4  , 14 }, // 00100110
    { 0X28 , 2  , 5  , 14 }, // 00101000
    { 0X2A , 1  , 7  , 14 }, // 00101010
    { 0X2C , 1  , 6  , 14 }, // 00101100
    { 0X2E , 0  , 15 , 14 }, // 00101110
    { 0X30 , 0  , 14 , 14 }, // 00110000
    { 0X32 , 0  , 13 , 14 }, // 00110010
    { 0X34 , 0  , 12 , 14 }, // 00110100
    { 0X36 , 26 , 1  , 14 }, // 00110110
    { 0X38 , 25 , 1  , 14 }, // 00111000
    { 0X3A , 24 , 1  , 14 }, // 00111010
    { 0X3C , 23 , 1  , 14 }, // 00111100
    { 0X3E , 22 , 1  , 14 }, // 00111110
// AC_LUT_9 (shift = 9)
    { 0X80 , 0  , 31 , 15 }, // 10000000
    { 0X88 , 0  , 30 , 15 }, // 10001000
    { 0X90 , 0  , 29 , 15 }, // 10010000
    { 0X98 , 0  , 28 , 15 }, // 10011000
    { 0XA0 , 0  , 27 , 15 }, // 10100000
    { 0XA8 , 0  , 26 , 15 }, // 10101000
    { 0XB0 , 0  , 25 , 15 }, // 10110000
    { 0XB8 , 0  , 24 , 15 }, // 10111000
    { 0XC0 , 0  , 23 , 15 }, // 11000000
    { 0XC8 , 0  , 22 , 15 }, // 11001000
    { 0XD0 , 0  , 21 , 15 }, // 11010000
    { 0XD8 , 0  , 20 , 15 }, // 11011000
    { 0XE0 , 0  , 19 , 15 }, // 11100000
    { 0XE8 , 0  , 18 , 15 }, // 11101000
    { 0XF0 , 0  , 17 , 15 }, // 11110000
    { 0XF8 , 0  , 16 , 15 }, // 11111000
    { 0X40 , 0  , 40 , 16 }, // 01000000
    { 0X44 , 0  , 39 , 16 }, // 01000100
    { 0X48 , 0  , 38 , 16 }, // 01001000
    { 0X4C , 0  , 37 , 16 }, // 01001100
    { 0X50 , 0  , 36 , 16 }, // 01010000
    { 0X54 , 0  , 35 , 16 }, // 01010100
    { 0X58 , 0  , 34 , 16 }, // 01011000
    { 0X5C , 0  , 33 , 16 }, // 01011100
    { 0X60 , 0  , 32 , 16 }, // 01100000
    { 0X64 , 1  , 14 , 16 }, // 01100100
    { 0X68 , 1  , 13 , 16 }, // 01101000
    { 0X6C , 1  , 12 , 16 }, // 01101100
    { 0X70 , 1  , 11 , 16 }, // 01110000
    { 0X74 , 1  , 10 , 16 }, // 01110100
    { 0X78 , 1  , 9  , 16 }, // 01111000
    { 0X7C , 1  , 8  , 16 }, // 01111100
    { 0X20 , 1  , 18 , 17 }, // 00100000
    { 0X22 , 1  , 17 , 17 }, // 00100010
    { 0X24 , 1  , 16 , 17 }, // 00100100
    { 0X26 , 1  , 15 , 17 }, // 00100110
    { 0X28 , 6  , 3  , 17 }, // 00101000
    { 0X2A , 16 , 2  , 17 }, // 00101010
    { 0X2C , 15 , 2  , 17 }, // 00101100
    { 0X2E , 14 , 2  , 17 }, // 00101110
    { 0X30 , 13 , 2  , 17 }, // 00110000
    { 0X32 , 12 , 2  , 17 }, // 00110010
    { 0X34 , 11 , 2  , 17 }, // 00110100
    { 0X36 , 31 , 1  , 17 }, // 00110110
    { 0X38 , 30 , 1  , 17 }, // 00111000
    { 0X3A , 29 , 1  , 17 }, // 00111010
    { 0X3C , 28 , 1  , 17 }, // 00111100
    { 0X3E , 27 , 1  , 17 }  // 00111110
};

const uint8 MDEC_ZSCAN[] = {
     0,  1,  8, 16,  9,  2,  3, 10,
    17, 24, 32, 25, 18, 11,  4,  5,
    12, 19, 26, 33, 40, 48, 41, 34,
    27, 20, 13,  6,  7, 14, 21, 28,
    35, 42, 49, 56, 57, 50, 43, 36,
    29, 22, 15, 23, 30, 37, 44, 51,
    58, 59, 52, 45, 38, 31, 39, 46,
    53, 60, 61, 54, 47, 55, 62, 63
};

const int32 MDEC_QTABLE[] = {
    0x00020000, 0x00163150, 0x00163150, 0x0018D321, 0x001EC830, 0x0018D321, 0x0019DE84, 0x0027DEA0,
    0x0027DEA0, 0x0019DE84, 0x00160000, 0x0023E1B0, 0x002C6282, 0x002724C0, 0x001A0000, 0x001536B1,
    0x00257337, 0x00297B55, 0x0027F206, 0x00241022, 0x00146D8E, 0x000E1238, 0x001D6CAF, 0x002346F9,
    0x00255528, 0x0025E3EF, 0x001F9AA9, 0x000FB1DC, 0x00096162, 0x001985B6, 0x0022E73A, 0x002219AE,
    0x002219AE, 0x001DC539, 0x00144489, 0x000772FB, 0x000B1918, 0x00148191, 0x001D9060, 0x00200000,
    0x001F6966, 0x00180AAA, 0x000E28D8, 0x000DB2B0, 0x00178BD2, 0x001B7FC9, 0x001B7FC9, 0x0015A314,
    0x000C9DD8, 0x000C53EE, 0x001490C8, 0x0018B140, 0x0015A5E0, 0x000CFA08, 0x000D3E30, 0x00146910,
    0x00138F5A, 0x000CB0EE, 0x000C2390, 0x001066E8, 0x000C928C, 0x000A4DA2, 0x000A4DA2, 0x00065187
};

const uint8 AC_LUT_1[] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x0A, 0x0A, 0x0A, 0x0A, 0x0B, 0x0B, 0x0B, 0x0B, 0x0C, 0x0C, 0x0C, 0x0C, 0x0D, 0x0D, 0x0D, 0x0D,
    0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
    0x0E, 0x0E, 0x0F, 0x0F, 0x10, 0x10, 0x11, 0x11, 0x12, 0x12, 0x13, 0x13, 0x14, 0x14, 0x15, 0x15,
    0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
    0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
    0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

const uint8 AC_LUT_6[] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x2E, 0x2E, 0x2F, 0x2F, 0x30, 0x30, 0x31, 0x31, 0x32, 0x32, 0x33, 0x33, 0x34, 0x34, 0x35, 0x35,
    0x36, 0x36, 0x37, 0x37, 0x38, 0x38, 0x39, 0x39, 0x3A, 0x3A, 0x3B, 0x3B, 0x3C, 0x3C, 0x3D, 0x3D,
    0x1E, 0x1E, 0x1E, 0x1E, 0x1F, 0x1F, 0x1F, 0x1F, 0x20, 0x20, 0x20, 0x20, 0x21, 0x21, 0x21, 0x21,
    0x22, 0x22, 0x22, 0x22, 0x23, 0x23, 0x23, 0x23, 0x24, 0x24, 0x24, 0x24, 0x25, 0x25, 0x25, 0x25,
    0x26, 0x26, 0x26, 0x26, 0x27, 0x27, 0x27, 0x27, 0x28, 0x28, 0x28, 0x28, 0x29, 0x29, 0x29, 0x29,
    0x2A, 0x2A, 0x2A, 0x2A, 0x2B, 0x2B, 0x2B, 0x2B, 0x2C, 0x2C, 0x2C, 0x2C, 0x2D, 0x2D, 0x2D, 0x2D,
    0x16, 0x16, 0x16, 0x16, 0x16, 0x16, 0x16, 0x16, 0x16, 0x16, 0x16, 0x16, 0x16, 0x16, 0x16, 0x16,
    0x17, 0x17, 0x17, 0x17, 0x17, 0x17, 0x17, 0x17, 0x17, 0x17, 0x17, 0x17, 0x17, 0x17, 0x17, 0x17,
    0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
    0x19, 0x19, 0x19, 0x19, 0x19, 0x19, 0x19, 0x19, 0x19, 0x19, 0x19, 0x19, 0x19, 0x19, 0x19, 0x19,
    0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A,
    0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x1B,
    0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C,
    0x1D, 0x1D, 0x1D, 0x1D, 0x1D, 0x1D, 0x1D, 0x1D, 0x1D, 0x1D, 0x1D, 0x1D, 0x1D, 0x1D, 0x1D, 0x1D,
};

const uint8 AC_LUT_9[] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x5E, 0x5E, 0x5F, 0x5F, 0x60, 0x60, 0x61, 0x61, 0x62, 0x62, 0x63, 0x63, 0x64, 0x64, 0x65, 0x65,
    0x66, 0x66, 0x67, 0x67, 0x68, 0x68, 0x69, 0x69, 0x6A, 0x6A, 0x6B, 0x6B, 0x6C, 0x6C, 0x6D, 0x6D,
    0x4E, 0x4E, 0x4E, 0x4E, 0x4F, 0x4F, 0x4F, 0x4F, 0x50, 0x50, 0x50, 0x50, 0x51, 0x51, 0x51, 0x51,
    0x52, 0x52, 0x52, 0x52, 0x53, 0x53, 0x53, 0x53, 0x54, 0x54, 0x54, 0x54, 0x55, 0x55, 0x55, 0x55,
    0x56, 0x56, 0x56, 0x56, 0x57, 0x57, 0x57, 0x57, 0x58, 0x58, 0x58, 0x58, 0x59, 0x59, 0x59, 0x59,
    0x5A, 0x5A, 0x5A, 0x5A, 0x5B, 0x5B, 0x5B, 0x5B, 0x5C, 0x5C, 0x5C, 0x5C, 0x5D, 0x5D, 0x5D, 0x5D,
    0x3E, 0x3E, 0x3E, 0x3E, 0x3E, 0x3E, 0x3E, 0x3E, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
    0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41,
    0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x43, 0x43, 0x43, 0x43, 0x43, 0x43, 0x43, 0x43,
    0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x45, 0x45, 0x45, 0x45, 0x45, 0x45, 0x45, 0x45,
    0x46, 0x46, 0x46, 0x46, 0x46, 0x46, 0x46, 0x46, 0x47, 0x47, 0x47, 0x47, 0x47, 0x47, 0x47, 0x47,
    0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x49, 0x49, 0x49, 0x49, 0x49, 0x49, 0x49, 0x49,
    0x4A, 0x4A, 0x4A, 0x4A, 0x4A, 0x4A, 0x4A, 0x4A, 0x4B, 0x4B, 0x4B, 0x4B, 0x4B, 0x4B, 0x4B, 0x4B,
    0x4C, 0x4C, 0x4C, 0x4C, 0x4C, 0x4C, 0x4C, 0x4C, 0x4D, 0x4D, 0x4D, 0x4D, 0x4D, 0x4D, 0x4D, 0x4D,
};

struct BitStream
{
    uint16* data;
    uint32 index;
    uint32 value;

    BitStream(uint8* data) : data((uint16*)data), index(0), value(0) {}

    uint32 getBit()
    {
        if (!index--)
        {
            value = *data++; // TODO BE support
            index = 15;
        }

        return (value >> index) & 1;
    }

    uint32 getU(int32 count)
    {
        uint32 bits = 0;

        while (count--)
        {
            bits = (bits << 1) | getBit();
        }

        return bits;
    }

    int32 getS(int32 count)
    {
        if (getBit())
        {
            return getU(count - 1) + (1 << (count - 1));
        }
        else
        {
            return getU(count - 1) - (1 << count) + 1;
        }
    }

    void skip(int32 count)
    {
        getU(count);
    }

    // http://jpsxdec.blogspot.com/2011/06/decoding-mpeg-like-bitstreams.html
    bool readCode(int32& skipCount, int32& ac)
    {
        if (getBit())
        {
            if (getBit())
            {
                skipCount = 0;
                ac = getBit() ? -1 : 1;
                return true;
            }
            return false; // end of block
        }

        int32 nz = 1;
        while (!getBit())
        {
            nz++;
        }

        if (nz == 5) // escape code == 0b1000001
        {
            uint32 esc = getU(16);
            skipCount = esc >> 10;
            ac = esc & 0x3FF;
            if (ac & 0x200)
                ac -= 0x400;
            return true;
        }

        const uint8* table;
        int32 shift;
        if (nz < 6)
        {
            table = AC_LUT_1;
            shift = 1;
        }
        else if (nz < 9)
        {
            table = AC_LUT_6;
            shift = 6;
        }
        else
        {
            table = AC_LUT_9;
            shift = 9;
        }

        BitStream state = *this;
        uint32 code = (1 << 7) | state.getU(7);

        code >>= nz - shift;

        ASSERT(table);

        int32 idx = table[code];

        ASSERT(idx != 255);

        const AC_ENTRY& e = MDEC_AC[idx];
        skip(e.length - nz - 1);
        skipCount = e.skip;
        ac = (code & (1 << (8 + shift - e.length))) ? -e.ac : e.ac;
        return true;
    }
};

void mdec_IDCT(int* block, int used_col)
{
    int* ptr;

    if (used_col == -1)
    {
        int32 v = block[0];
        for (int32 i = 0; i < 64; i++)
        {
            block[i] = v;
        }
        return;
    }

    ptr = block;
    for (int32 i = 0; i < 8; i++, ptr++)
    {
        if ((used_col & (1 << i)) == 0)
        {
            if (ptr[8 * 0])
            {
                ptr[0 * 8] =
                ptr[1 * 8] =
                ptr[2 * 8] =
                ptr[3 * 8] =
                ptr[4 * 8] =
                ptr[5 * 8] =
                ptr[6 * 8] =
                ptr[7 * 8] = ptr[0];
                used_col |= (1 << i);
            }
            continue;
        }

        int32 z10 = ptr[8 * 0] + ptr[8 * 4];
        int32 z11 = ptr[8 * 0] - ptr[8 * 4];
        int32 z13 = ptr[8 * 2] + ptr[8 * 6];
        int32 z12 = SCALE((ptr[8 * 2] - ptr[8 * 6]) * FIX_1_414213562, AAN_CONST_BITS) - z13;

        int32 tmp0 = z10 + z13;
        int32 tmp3 = z10 - z13;
        int32 tmp1 = z11 + z12;
        int32 tmp2 = z11 - z12;

        z13 = ptr[8 * 3] + ptr[8 * 5];
        z10 = ptr[8 * 3] - ptr[8 * 5];
        z11 = ptr[8 * 1] + ptr[8 * 7];
        z12 = ptr[8 * 1] - ptr[8 * 7];

        int32 tmp7 = z11 + z13;

        int32 z5 = (z12 - z10) * FIX_1_847759065;
        int32 tmp6 = SCALE(z10 * FIX_2_613125930 + z5, AAN_CONST_BITS) - tmp7;
        int32 tmp5 = SCALE((z11 - z13) * FIX_1_414213562, AAN_CONST_BITS) - tmp6;
        int32 tmp4 = SCALE(z12 * FIX_1_082392200 - z5, AAN_CONST_BITS) + tmp5;

        ptr[8 * 0] = (tmp0 + tmp7);
        ptr[8 * 7] = (tmp0 - tmp7);
        ptr[8 * 1] = (tmp1 + tmp6);
        ptr[8 * 6] = (tmp1 - tmp6);
        ptr[8 * 2] = (tmp2 + tmp5);
        ptr[8 * 5] = (tmp2 - tmp5);
        ptr[8 * 4] = (tmp3 + tmp4);
        ptr[8 * 3] = (tmp3 - tmp4);
    }

    ptr = block;
    if (used_col == 1)
    {
        for (int32 i = 0; i < 8; i++)
        {
            ptr[i * 8 + 0] =
            ptr[i * 8 + 1] =
            ptr[i * 8 + 2] =
            ptr[i * 8 + 3] =
            ptr[i * 8 + 4] =
            ptr[i * 8 + 5] =
            ptr[i * 8 + 6] =
            ptr[i * 8 + 7] = ptr[8 * i];
        }
    }
    else
    {
        for (int32 i = 0; i < 8; i++, ptr += 8)
        {
            int32 z10 = ptr[0] + ptr[4];
            int32 z11 = ptr[0] - ptr[4];
            int32 z13 = ptr[2] + ptr[6];
            int32 z12 = SCALE((ptr[2] - ptr[6]) * FIX_1_414213562, AAN_CONST_BITS) - z13;

            int32 tmp0 = z10 + z13;
            int32 tmp3 = z10 - z13;
            int32 tmp1 = z11 + z12;
            int32 tmp2 = z11 - z12;

            z13 = ptr[3] + ptr[5];
            z10 = ptr[3] - ptr[5];
            z11 = ptr[1] + ptr[7];
            z12 = ptr[1] - ptr[7];

            int32 z5 = (z12 - z10) * FIX_1_847759065;
            int32 tmp7 = z11 + z13;
            int32 tmp6 = SCALE(z10 * FIX_2_613125930 + z5, AAN_CONST_BITS) - tmp7;
            int32 tmp5 = SCALE((z11 - z13) * FIX_1_414213562, AAN_CONST_BITS) - tmp6;
            int32 tmp4 = SCALE(z12 * FIX_1_082392200 - z5, AAN_CONST_BITS) + tmp5;

            ptr[0] = tmp0 + tmp7;

            ptr[7] = tmp0 - tmp7;
            ptr[1] = tmp1 + tmp6;
            ptr[6] = tmp1 - tmp6;
            ptr[2] = tmp2 + tmp5;
            ptr[5] = tmp2 - tmp5;
            ptr[4] = tmp3 + tmp4;
            ptr[3] = tmp3 - tmp4;
        }
    }
}

static inline void putQuadRGB24(uint8* image, int* Yblk, int Cr, int Cb)
{
    int Y, R, G, B;

    R = MULR(Cr);
    G = MULG2(Cb, Cr);
    B = MULB(Cb);

    Y = MULY(Yblk[0]);
    image[0 * 3 + 0] = CLAMP_SCALE8(Y + R);
    image[0 * 3 + 1] = CLAMP_SCALE8(Y + G);
    image[0 * 3 + 2] = CLAMP_SCALE8(Y + B);
    Y = MULY(Yblk[1]);
    image[1 * 3 + 0] = CLAMP_SCALE8(Y + R);
    image[1 * 3 + 1] = CLAMP_SCALE8(Y + G);
    image[1 * 3 + 2] = CLAMP_SCALE8(Y + B);
    Y = MULY(Yblk[8]);
    image[16 * 3 + 0] = CLAMP_SCALE8(Y + R);
    image[16 * 3 + 1] = CLAMP_SCALE8(Y + G);
    image[16 * 3 + 2] = CLAMP_SCALE8(Y + B);
    Y = MULY(Yblk[9]);
    image[17 * 3 + 0] = CLAMP_SCALE8(Y + R);
    image[17 * 3 + 1] = CLAMP_SCALE8(Y + G);
    image[17 * 3 + 2] = CLAMP_SCALE8(Y + B);
}

inline void YUV2RGB24(int32* blk, uint8* image)
{
    int32* Yblk = blk + 64 * 2;
    int32* Crblk = blk;
    int32* Cbblk = blk + 64;

    for (int32 y = 0; y < 16; y += 2, Crblk += 4, Cbblk += 4, Yblk += 8, image += 8 * 3 * 3)
    {
        if (y == 8)
        {
            Yblk += 64;
        }

        for (int32 x = 0; x < 4; x++, image += 6, Crblk++, Cbblk++, Yblk += 2)
        {
            putQuadRGB24(image, Yblk, *Crblk, *Cbblk);
            putQuadRGB24(image + 8 * 3, Yblk + 64, *(Crblk + 4), *(Cbblk + 4));
        }
    }
}

int32 mdec_decode(uint8* data, int32 version, int32 width, int32 height, int32 qscale, uint8* dst)
{
    BitStream bs(data);

    int32 prev[3] = { 0, 0, 0 };
    int32 blocks[6][8 * 8]; // Cr, Cb, YTL, YTR, YBL, YBR

    for (int32 bX = 0; bX < width / 16; bX++)
    {
        for (int32 bY = 0; bY < height / 16; bY++)
        {
            memset(blocks, 0, sizeof(blocks));

            for (int i = 0; i < 6; i++)
            {
                int32* block = blocks[i];
                
                int32 dc;

                if (version == 2) // fixed 10-bit DC
                {
                    dc = bs.getU(10);
                    if (dc >> 9)
                    {
                        dc -= 1024;
                    }
                }
                else // variable DC bits
                {
                    int32 nz = 0;
                    while (bs.getBit())
                        nz++;

                    if (i >= 2) // Luma
                    {
                        if (nz == 0)
                        {
                            if (bs.getBit())
                            {
                                dc = bs.getS(2);
                            }
                            else
                            {
                                dc = (bs.getBit() << 1) - 1;
                            }
                        }
                        else if (nz == 1)
                        {
                            dc = bs.getBit() ? bs.getS(3) : 0;
                        }
                        else
                        {
                            dc = bs.getS(nz + 2);
                        }

                        nz = 2;
                    }
                    else // Chroma
                    {
                        if (nz == 0)
                        {
                            if (bs.getBit())
                            {
                                dc = (bs.getBit() << 1) - 1;
                            }
                            else
                            {
                                dc = 0;
                            }
                        }
                        else
                        {
                            dc = bs.getS(nz + 1);
                        }

                        nz = i;
                    }

                    dc <<= 2;
                    dc += prev[nz];
                    prev[nz] = dc;
                    ASSERT(prev[nz] >= -512 && prev[nz] <= 511);
                }

                block[0] = SCALER(dc * MDEC_QTABLE[0], AAN_EXTRA - 3);

                int32 used_col = 0;

                int32 skip, ac;
                int32 index = 0;
                while (bs.readCode(skip, ac))
                {
                    index += skip + 1;
                    ASSERT(index < 64);

                    block[MDEC_ZSCAN[index]] = SCALER(ac * MDEC_QTABLE[index] * qscale, AAN_EXTRA);

                    used_col |= (MDEC_ZSCAN[index] > 7) ? 1 << (MDEC_ZSCAN[index] & 7) : 0;
                }

                if (index == 0) used_col = -1;

                mdec_IDCT(block, used_col);
            }

            uint8 rgb[16 * 16 * 3];
            YUV2RGB24(&blocks[0][0], (uint8*)rgb);

            uint8* src = rgb;
            uint8* ptr = dst + (width * bY * 16 + bX * 16) * 4;
            for (int y = 0; y < 16; y++)
            {
                for (int x = 0; x < 16; x++)
                {
                    *ptr++ = *src++;
                    *ptr++ = *src++;
                    *ptr++ = *src++;
                    *ptr++ = 255;
                }
                ptr += (width - 16) * 4;
            }
        }
    }

    return (uint8*)bs.data - data;
}

#endif
