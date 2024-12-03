#include "shared.hpp"

void bf_write::StartWriting(void* pData, int nBytes, int iStartBit, int nBits) {
    if (!(nBytes % 4 == 0))
        return;

    if (!(((unsigned long)pData & 3) == 0))
        return;

    nBytes &= ~3;

    m_pData = (unsigned long*)pData;
    m_nDataBytes = nBytes;

    if (nBits == -1) {
        m_nDataBits = nBytes << 3;
    }
    else {
        if (nBits <= nBytes * 8) {
            m_nDataBits = nBits;
        }
    }

    m_iCurBit = iStartBit;
    m_bOverflow = false;
}

//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//
//=============================================================================//

// FIXME: Can't use this until we get multithreaded allocations in tier0 working for tools
// This is used by VVIS and fails to link
// NOTE: This must be the last file included!!!
// #include "tier0/memdbgon.h"

#define COORD_INTEGER_BITS                    14
#define COORD_FRACTIONAL_BITS                 5
#define COORD_DENOMINATOR                     (1 << (COORD_FRACTIONAL_BITS))
#define COORD_RESOLUTION                      (1.0 / (COORD_DENOMINATOR))

// Special threshold for networking multiplayer origins
#define COORD_INTEGER_BITS_MP                 11
#define COORD_FRACTIONAL_BITS_MP_LOWPRECISION 3
#define COORD_DENOMINATOR_LOWPRECISION        (1 << (COORD_FRACTIONAL_BITS_MP_LOWPRECISION))
#define COORD_RESOLUTION_LOWPRECISION         (1.0 / (COORD_DENOMINATOR_LOWPRECISION))

#define NORMAL_FRACTIONAL_BITS                11
#define NORMAL_DENOMINATOR                    ((1 << (NORMAL_FRACTIONAL_BITS)) - 1)
#define NORMAL_RESOLUTION                     (1.0 / (NORMAL_DENOMINATOR))

// this is limited by the network fractional bits used for coords
// because net coords will be only be accurate to 5 bits fractional
// Standard collision test epsilon
// 1/32nd inch collision epsilon

#define FAST_BIT_SCAN                         1
#include <intrin.h>
#include <stdlib.h>
#pragma intrinsic(_BitScanReverse)
#pragma intrinsic(_BitScanForward)

inline unsigned int CountLeadingZeros(unsigned int x) {
    unsigned long firstBit;
    if (_BitScanReverse(&firstBit, x))
        return 31 - firstBit;
    return 32;
}
inline unsigned int CountTrailingZeros(unsigned int elem) {
    unsigned long out;
    if (_BitScanForward(&out, elem))
        return out;
    return 32;
}

static BitBufErrorHandler g_BitBufErrorHandler = 0;

inline int                BitForBitnum(int bitnum) {
    return GetBitForBitnum(bitnum);
}

void InternalBitBufErrorHandler(BitBufErrorType errorType, const char* pDebugName) {
    if (g_BitBufErrorHandler)
        g_BitBufErrorHandler(errorType, pDebugName);
}

void SetBitBufErrorHandler(BitBufErrorHandler fn) {
    g_BitBufErrorHandler = fn;
}

// #define BB_PROFILING

unsigned long g_LittleBits[32];

// Precalculated bit masks for WriteUBitLong. Using these tables instead of
// doing the calculations gives a 33% speedup in WriteUBitLong.
unsigned long g_BitWriteMasks[32][33];

// (1 << i) - 1
unsigned long g_ExtraMasks[33];

class CBitWriteMasksInit {
public:
    CBitWriteMasksInit() {
        for (unsigned int startbit = 0; startbit < 32; startbit++) {
            for (unsigned int nBitsLeft = 0; nBitsLeft < 33; nBitsLeft++) {
                unsigned int endbit = startbit + nBitsLeft;
                g_BitWriteMasks[startbit][nBitsLeft] = BitForBitnum(startbit) - 1;
                if (endbit < 32)
                    g_BitWriteMasks[startbit][nBitsLeft] |= ~(BitForBitnum(endbit) - 1);
            }
        }

        for (unsigned int maskBit = 0; maskBit < 32; maskBit++)
            g_ExtraMasks[maskBit] = BitForBitnum(maskBit) - 1;
        g_ExtraMasks[32] = ~0ul;

        for (unsigned int littleBit = 0; littleBit < 32; littleBit++)
            StoreLittleDWord(&g_LittleBits[littleBit], 0, 1u << littleBit);
    }
};
static CBitWriteMasksInit g_BitWriteMasksInit;

// ---------------------------------------------------------------------------------------- //
// bf_write
// ---------------------------------------------------------------------------------------- //

bf_write::bf_write() {
    m_pData = NULL;
    m_nDataBytes = 0;
    m_nDataBits = -1; // set to -1 so we generate overflow on any operation
    m_iCurBit = 0;
    m_bOverflow = false;
    m_bAssertOnOverflow = true;
    m_pDebugName = NULL;
}

bf_write::bf_write(const char* pDebugName, void* pData, int nBytes, int nBits) {
    m_bAssertOnOverflow = true;
    m_pDebugName = pDebugName;
    StartWriting(pData, nBytes, 0, nBits);
}

bf_write::bf_write(void* pData, int nBytes, int nBits) {
    m_bAssertOnOverflow = true;
    m_pDebugName = NULL;
    StartWriting(pData, nBytes, 0, nBits);
}

void bf_write::Reset() {
    m_iCurBit = 0;
    m_bOverflow = false;
}

void bf_write::SetAssertOnOverflow(bool bAssert) {
    m_bAssertOnOverflow = bAssert;
}

const char* bf_write::GetDebugName() {
    return m_pDebugName;
}

void bf_write::SetDebugName(const char* pDebugName) {
    m_pDebugName = pDebugName;
}

void bf_write::SeekToBit(int bitPos) {
    m_iCurBit = bitPos;
}

// Sign bit comes first
void bf_write::WriteSBitLong(int data, int numbits) {
    // Force the sign-extension bit to be correct even in the case of overflow.
    int nValue = data;
    int nPreserveBits = (0x7FFFFFFF >> (32 - numbits));
    int nSignExtension = (nValue >> 31) & ~nPreserveBits;
    nValue &= nPreserveBits;
    nValue |= nSignExtension;

    // nullAssert(nValue == data, "WriteSBitLong: 0x%08x does not fit in %d bits", data, numbits);

    WriteUBitLong(nValue, numbits, false);
}

void bf_write::WriteVarInt32(uint32_t data) {
    // Check if align and we have room, slow path if not
    if ((m_iCurBit & 7) == 0 && (m_iCurBit + bitbuf::kMaxVarint32Bytes * 8) <= m_nDataBits) {
        uint8_t* target = ((uint8_t*)m_pData) + (m_iCurBit >> 3);

        target[0] = static_cast<uint8_t>(data | 0x80);
        if (data >= (1 << 7)) {
            target[1] = static_cast<uint8_t>((data >> 7) | 0x80);
            if (data >= (1 << 14)) {
                target[2] = static_cast<uint8_t>((data >> 14) | 0x80);
                if (data >= (1 << 21)) {
                    target[3] = static_cast<uint8_t>((data >> 21) | 0x80);
                    if (data >= (1 << 28)) {
                        target[4] = static_cast<uint8_t>(data >> 28);
                        m_iCurBit += 5 * 8;
                        return;
                    }
                    else {
                        target[3] &= 0x7F;
                        m_iCurBit += 4 * 8;
                        return;
                    }
                }
                else {
                    target[2] &= 0x7F;
                    m_iCurBit += 3 * 8;
                    return;
                }
            }
            else {
                target[1] &= 0x7F;
                m_iCurBit += 2 * 8;
                return;
            }
        }
        else {
            target[0] &= 0x7F;
            m_iCurBit += 1 * 8;
            return;
        }
    }
    else // Slow path
    {
        while (data > 0x7F) {
            WriteUBitLong((data & 0x7F) | 0x80, 8);
            data >>= 7;
        }
        WriteUBitLong(data & 0x7F, 8);
    }
}

void bf_write::WriteVarInt64(uint64_t data) {
    // Check if align and we have room, slow path if not
    if ((m_iCurBit & 7) == 0 && (m_iCurBit + bitbuf::kMaxVarintBytes * 8) <= m_nDataBits) {
        uint8_t* target = ((uint8_t*)m_pData) + (m_iCurBit >> 3);

        // Splitting into 32-bit pieces gives better performance on 32-bit
        // processors.
        uint32_t part0 = static_cast<uint32_t>(data);
        uint32_t part1 = static_cast<uint32_t>(data >> 28);
        uint32_t part2 = static_cast<uint32_t>(data >> 56);

        int      size;

        // Here we can't really optimize for small numbers, since the data is
        // split into three parts.  Cheking for numbers < 128, for instance,
        // would require three comparisons, since you'd have to make sure part1
        // and part2 are zero.  However, if the caller is using 64-bit integers,
        // it is likely that they expect the numbers to often be very large, so
        // we probably don't want to optimize for small numbers anyway.  Thus,
        // we end up with a hardcoded binary search tree...
        if (part2 == 0) {
            if (part1 == 0) {
                if (part0 < (1 << 14)) {
                    if (part0 < (1 << 7)) {
                        size = 1;
                        goto size1;
                    }
                    else {
                        size = 2;
                        goto size2;
                    }
                }
                else {
                    if (part0 < (1 << 21)) {
                        size = 3;
                        goto size3;
                    }
                    else {
                        size = 4;
                        goto size4;
                    }
                }
            }
            else {
                if (part1 < (1 << 14)) {
                    if (part1 < (1 << 7)) {
                        size = 5;
                        goto size5;
                    }
                    else {
                        size = 6;
                        goto size6;
                    }
                }
                else {
                    if (part1 < (1 << 21)) {
                        size = 7;
                        goto size7;
                    }
                    else {
                        size = 8;
                        goto size8;
                    }
                }
            }
        }
        else {
            if (part2 < (1 << 7)) {
                size = 9;
                goto size9;
            }
            else {
                size = 10;
                goto size10;
            }
        }

        // nullAssert(false, "Can't get here.");

    size10:
        target[9] = static_cast<uint8_t>((part2 >> 7) | 0x80);
    size9:
        target[8] = static_cast<uint8_t>((part2) | 0x80);
    size8:
        target[7] = static_cast<uint8_t>((part1 >> 21) | 0x80);
    size7:
        target[6] = static_cast<uint8_t>((part1 >> 14) | 0x80);
    size6:
        target[5] = static_cast<uint8_t>((part1 >> 7) | 0x80);
    size5:
        target[4] = static_cast<uint8_t>((part1) | 0x80);
    size4:
        target[3] = static_cast<uint8_t>((part0 >> 21) | 0x80);
    size3:
        target[2] = static_cast<uint8_t>((part0 >> 14) | 0x80);
    size2:
        target[1] = static_cast<uint8_t>((part0 >> 7) | 0x80);
    size1:
        target[0] = static_cast<uint8_t>((part0) | 0x80);

        target[size - 1] &= 0x7F;
        m_iCurBit += size * 8;
    }
    else // slow path
    {
        while (data > 0x7F) {
            WriteUBitLong((data & 0x7F) | 0x80, 8);
            data >>= 7;
        }
        WriteUBitLong(data & 0x7F, 8);
    }
}

void bf_write::WriteSignedVarInt32(int32_t data) {
    WriteVarInt32(bitbuf::ZigZagEncode32(data));
}

void bf_write::WriteSignedVarInt64(int64_t data) {
    WriteVarInt64(bitbuf::ZigZagEncode64(data));
}

int bf_write::ByteSizeVarInt32(uint32_t data) {
    int size = 1;
    while (data > 0x7F) {
        size++;
        data >>= 7;
    }
    return size;
}

int bf_write::ByteSizeVarInt64(uint64_t data) {
    int size = 1;
    while (data > 0x7F) {
        size++;
        data >>= 7;
    }
    return size;
}

// int bf_write::ByteSizeSignedVarInt32(int32_t data)
//{
//	return ByteSizeVarInt32(bitbuf::ZigZagEncode32(data));
// }
//
// int bf_write::ByteSizeSignedVarInt64(int64_t data)
//{
//	return ByteSizeVarInt64(bitbuf::ZigZagEncode64(data));
// }

void bf_write::WriteBitLong(unsigned int data, int numbits, bool bSigned) {
    if (bSigned)
        WriteSBitLong((int)data, numbits);
    else
        WriteUBitLong(data, numbits);
}

void Q_memcpy(void* dest, const void* src, int count) {
    int i;
    if ((((long)dest | (long)src | count) & 3) == 0) {
        count >>= 2;
        for (i = 0; i < count; i++)
            ((int*)dest)[i] = ((int*)src)[i];
    }
    else
        for (i = 0; i < count; i++)
            ((char*)dest)[i] = ((char*)src)[i];
}

bool bf_write::WriteBits(const void* pInData, int nBits) {
#if defined(BB_PROFILING)
    VPROF("bf_write::WriteBits");
#endif

    unsigned char* pOut = (unsigned char*)pInData;
    int            nBitsLeft = nBits;

    // Bounds checking..
    if ((m_iCurBit + nBits) > m_nDataBits) {
        SetOverflowFlag();
        CallErrorHandler(BITBUFERROR_BUFFER_OVERRUN, GetDebugName());
        return false;
    }

    // Align output to dword boundary
    while (((unsigned long)pOut & 3) != 0 && nBitsLeft >= 8) {

        WriteUBitLong(*pOut, 8, false);
        ++pOut;
        nBitsLeft -= 8;
    }

    if ((nBitsLeft >= 32) && (m_iCurBit & 7) == 0) {
        // current bit is char aligned, do block copy
        int numbytes = nBitsLeft >> 3;
        int numbits = numbytes << 3;

        Q_memcpy((char*)m_pData + (m_iCurBit >> 3), pOut, numbytes);
        pOut += numbytes;
        nBitsLeft -= numbits;
        m_iCurBit += numbits;
    }

    // X360TBD: Can't write dwords in WriteBits because they'll get swapped
    if (nBitsLeft >= 32) {
        unsigned long  iBitsRight = (m_iCurBit & 31);
        unsigned long  iBitsLeft = 32 - iBitsRight;
        unsigned long  bitMaskLeft = g_BitWriteMasks[iBitsRight][32];
        unsigned long  bitMaskRight = g_BitWriteMasks[0][iBitsRight];

        unsigned long* pData = &m_pData[m_iCurBit >> 5];

        // Read dwords.
        while (nBitsLeft >= 32) {
            unsigned long curData = *(unsigned long*)pOut;
            pOut += sizeof(unsigned long);

            *pData &= bitMaskLeft;
            *pData |= curData << iBitsRight;

            pData++;

            if (iBitsLeft < 32) {
                curData >>= iBitsLeft;
                *pData &= bitMaskRight;
                *pData |= curData;
            }

            nBitsLeft -= 32;
            m_iCurBit += 32;
        }
    }

    // write remaining bytes
    while (nBitsLeft >= 8) {
        WriteUBitLong(*pOut, 8, false);
        ++pOut;
        nBitsLeft -= 8;
    }

    // write remaining bits
    if (nBitsLeft) {
        WriteUBitLong(*pOut, nBitsLeft, false);
    }

    return !IsOverflowed();
}

bool bf_write::WriteBitsFromBuffer(bf_read* pIn, int nBits) {
    // This could be optimized a little by
    while (nBits > 32) {
        WriteUBitLong(pIn->ReadUBitLong(32), 32);
        nBits -= 32;
    }

    WriteUBitLong(pIn->ReadUBitLong(nBits), nBits);
    return !IsOverflowed() && !pIn->IsOverflowed();
}

void bf_write::WriteBitAngle(float fAngle, int numbits) {
    int          d;
    unsigned int mask;
    unsigned int shift;

    shift = BitForBitnum(numbits);
    mask = shift - 1;

    d = (int)((fAngle / 360.0) * shift);
    d &= mask;

    WriteUBitLong((unsigned int)d, numbits);
}

void bf_write::WriteBitCoordMP(const float f, bool bIntegral, bool bLowPrecision) {
#if defined(BB_PROFILING)
    VPROF("bf_write::WriteBitCoordMP");
#endif
    int          signbit = (f <= -(bLowPrecision ? COORD_RESOLUTION_LOWPRECISION : COORD_RESOLUTION));
    int          intval = (int)fabsf(f);
    int          fractval = bLowPrecision
        ? (abs((int)(f * COORD_DENOMINATOR_LOWPRECISION)) & (COORD_DENOMINATOR_LOWPRECISION - 1))
        : (abs((int)(f * COORD_DENOMINATOR)) & (COORD_DENOMINATOR - 1));

    bool         bInBounds = intval < (1 << COORD_INTEGER_BITS_MP);

    unsigned int bits, numbits;

    if (bIntegral) {
        // Integer encoding: in-bounds bit, nonzero bit, optional sign bit + integer value bits
        if (intval) {
            // Adjust the integers from [1..MAX_COORD_VALUE] to [0..MAX_COORD_VALUE-1]
            --intval;
            bits = intval * 8 + signbit * 4 + 2 + bInBounds;
            numbits = 3 + (bInBounds ? COORD_INTEGER_BITS_MP : COORD_INTEGER_BITS);
        }
        else {
            bits = bInBounds;
            numbits = 2;
        }
    }
    else {
        // Float encoding: in-bounds bit, integer bit, sign bit, fraction value bits, optional integer value bits
        if (intval) {
            // Adjust the integers from [1..MAX_COORD_VALUE] to [0..MAX_COORD_VALUE-1]
            --intval;
            bits = intval * 8 + signbit * 4 + 2 + bInBounds;
            bits += bInBounds ? (fractval << (3 + COORD_INTEGER_BITS_MP)) : (fractval << (3 + COORD_INTEGER_BITS));
            numbits = 3 + (bInBounds ? COORD_INTEGER_BITS_MP : COORD_INTEGER_BITS) +
                (bLowPrecision ? COORD_FRACTIONAL_BITS_MP_LOWPRECISION : COORD_FRACTIONAL_BITS);
        }
        else {
            bits = fractval * 8 + signbit * 4 + 0 + bInBounds;
            numbits = 3 + (bLowPrecision ? COORD_FRACTIONAL_BITS_MP_LOWPRECISION : COORD_FRACTIONAL_BITS);
        }
    }

    WriteUBitLong(bits, numbits);
}

void bf_write::WriteBitCoord(const float f) {
#if defined(BB_PROFILING)
    VPROF("bf_write::WriteBitCoord");
#endif
    int signbit = (f <= -COORD_RESOLUTION);
    int intval = (int)fabsf(f);
    int fractval = abs((int)(f * COORD_DENOMINATOR)) & (COORD_DENOMINATOR - 1);

    // Send the bit flags that indicate whether we have an integer part and/or a fraction part.
    WriteOneBit(intval);
    WriteOneBit(fractval);

    if (intval || fractval) {
        // Send the sign bit
        WriteOneBit(signbit);

        // Send the integer if we have one.
        if (intval) {
            // Adjust the integers from [1..MAX_COORD_VALUE] to [0..MAX_COORD_VALUE-1]
            intval--;
            WriteUBitLong((unsigned int)intval, COORD_INTEGER_BITS);
        }

        // Send the fraction if we have one
        if (fractval) {
            WriteUBitLong((unsigned int)fractval, COORD_FRACTIONAL_BITS);
        }
    }
}

void bf_write::WriteBitVec3Coord(const vector& fa) {
    int xflag, yflag, zflag;

    xflag = (fa[0] >= COORD_RESOLUTION) || (fa[0] <= -COORD_RESOLUTION);
    yflag = (fa[1] >= COORD_RESOLUTION) || (fa[1] <= -COORD_RESOLUTION);
    zflag = (fa[2] >= COORD_RESOLUTION) || (fa[2] <= -COORD_RESOLUTION);

    WriteOneBit(xflag);
    WriteOneBit(yflag);
    WriteOneBit(zflag);

    if (xflag)
        WriteBitCoord(fa[0]);
    if (yflag)
        WriteBitCoord(fa[1]);
    if (zflag)
        WriteBitCoord(fa[2]);
}

void bf_write::WriteBitNormal(float f) {
    int          signbit = (f <= -NORMAL_RESOLUTION);

    // NOTE: Since +/-1 are valid values for a normal, I'm going to encode that as all ones
    unsigned int fractval = abs((int)(f * NORMAL_DENOMINATOR));

    // clamp..
    if (fractval > NORMAL_DENOMINATOR)
        fractval = NORMAL_DENOMINATOR;

    // Send the sign bit
    WriteOneBit(signbit);

    // Send the fractional component
    WriteUBitLong(fractval, NORMAL_FRACTIONAL_BITS);
}

void bf_write::WriteBitVec3Normal(const vector& fa) {
    int xflag, yflag;

    xflag = (fa[0] >= NORMAL_RESOLUTION) || (fa[0] <= -NORMAL_RESOLUTION);
    yflag = (fa[1] >= NORMAL_RESOLUTION) || (fa[1] <= -NORMAL_RESOLUTION);

    WriteOneBit(xflag);
    WriteOneBit(yflag);

    if (xflag)
        WriteBitNormal(fa[0]);
    if (yflag)
        WriteBitNormal(fa[1]);

    // Write z sign bit
    int signbit = (fa[2] <= -NORMAL_RESOLUTION);
    WriteOneBit(signbit);
}

void bf_write::WriteBitAngles(const vector& fa) {
    // FIXME:
    vector tmp(fa.x, fa.y, fa.z);
    WriteBitVec3Coord(tmp);
}

void bf_write::WriteChar(int val) {
    WriteSBitLong(val, sizeof(char) << 3);
}

void bf_write::WriteByte(int val) {
    WriteUBitLong(val, sizeof(unsigned char) << 3);
}

void bf_write::WriteShort(int val) {
    WriteSBitLong(val, sizeof(short) << 3);
}

void bf_write::WriteWord(int val) {
    WriteUBitLong(val, sizeof(unsigned short) << 3);
}

void bf_write::WriteLong(long val) {
    WriteSBitLong(val, sizeof(long) << 3);
}

void bf_write::WriteLongLong(int64_t val) {
    unsigned int* pLongs = (unsigned int*)&val;

    // Insert the two DWORDS according to network endian
    const short   endianIndex = 0x0100;
    char* idx = (char*)&endianIndex;
    WriteUBitLong(pLongs[*idx++], sizeof(long) << 3);
    WriteUBitLong(pLongs[*idx], sizeof(long) << 3);
}

void bf_write::WriteFloat(float val) {
    // Pre-swap the float, since WriteBits writes raw data
    LittleFloat(&val, &val);

    WriteBits(&val, sizeof(val) << 3);
}

bool bf_write::WriteBytes(const void* pBuf, int nBytes) {
    return WriteBits(pBuf, nBytes << 3);
}

bool bf_write::WriteString(const char* pStr) {
    if (pStr) {
        do {
            WriteChar(*pStr);
            ++pStr;
        } while (*(pStr - 1) != 0);
    }
    else {
        WriteChar(0);
    }

    return !IsOverflowed();
}

// ---------------------------------------------------------------------------------------- //
// bf_read
// ---------------------------------------------------------------------------------------- //

bf_read::bf_read() {
    m_pData = NULL;
    m_nDataBytes = 0;
    m_nDataBits = -1; // set to -1 so we overflow on any operation
    m_iCurBit = 0;
    m_bOverflow = false;
    m_bAssertOnOverflow = true;
    m_pDebugName = NULL;
}

bf_read::bf_read(const void* pData, int nBytes, int nBits) {
    m_bAssertOnOverflow = true;
    StartReading(pData, nBytes, 0, nBits);
}

bf_read::bf_read(const char* pDebugName, const void* pData, int nBytes, int nBits) {
    m_bAssertOnOverflow = true;
    m_pDebugName = pDebugName;
    StartReading(pData, nBytes, 0, nBits);
}

void bf_read::StartReading(const void* pData, int nBytes, int iStartBit, int nBits) {
    // Make sure we're dword aligned.
    nullAssert(((size_t)pData & 3) == 0);

    m_pData = (unsigned long*)pData;
    m_nDataBytes = nBytes;

    if (nBits == -1) {
        m_nDataBits = m_nDataBytes << 3;
    }
    else {
        nullAssert(nBits <= nBytes * 8);
        m_nDataBits = nBits;
    }

    m_iCurBit = iStartBit;
    m_bOverflow = false;
}

void bf_read::Reset() {
    m_iCurBit = 0;
    m_bOverflow = false;
}

void bf_read::SetAssertOnOverflow(bool bAssert) {
    m_bAssertOnOverflow = bAssert;
}

void bf_read::SetDebugName(const char* pName) {
    m_pDebugName = pName;
}

void bf_read::SetOverflowFlag() {
    if (m_bAssertOnOverflow) {
        nullAssert(false);
    }
    m_bOverflow = true;
}

unsigned int bf_read::CheckReadUBitLong(int numbits) {
    // Ok, just read bits out.
    int          i, nBitValue;
    unsigned int r = 0;

    for (i = 0; i < numbits; i++) {
        nBitValue = ReadOneBitNoCheck();
        r |= nBitValue << i;
    }
    m_iCurBit -= numbits;

    return r;
}

void bf_read::ReadBits(void* pOutData, int nBits) {
#if defined(BB_PROFILING)
    VPROF("bf_read::ReadBits");
#endif

    unsigned char* pOut = (unsigned char*)pOutData;
    int            nBitsLeft = nBits;

    // align output to dword boundary
    while (((size_t)pOut & 3) != 0 && nBitsLeft >= 8) {
        *pOut = (unsigned char)ReadUBitLong(8);
        ++pOut;
        nBitsLeft -= 8;
    }

    // read dwords
    while (nBitsLeft >= 32) {
        *((unsigned long*)pOut) = ReadUBitLong(32);
        pOut += sizeof(unsigned long);
        nBitsLeft -= 32;
    }

    // read remaining bytes
    while (nBitsLeft >= 8) {
        *pOut = ReadUBitLong(8);
        ++pOut;
        nBitsLeft -= 8;
    }

    // read remaining bits
    if (nBitsLeft) {
        *pOut = ReadUBitLong(nBitsLeft);
    }
}

int bf_read::ReadBitsClamped_ptr(void* pOutData, size_t outSizeBytes, size_t nBits) {
    size_t outSizeBits = outSizeBytes * 8;
    size_t readSizeBits = nBits;
    int    skippedBits = 0;
    if (readSizeBits > outSizeBits) {
        // Should we print a message when we clamp the data being read? Only
        // in debug builds I think.
        // nullAssert(0, "Oversized network packet received, and clamped.");
        readSizeBits = outSizeBits;
        skippedBits = (int)(nBits - outSizeBits);
        // What should we do in this case, which should only happen if nBits
        // is negative for some reason?
        // if ( skippedBits < 0 )
        //	return 0;
    }

    ReadBits(pOutData, readSizeBits);
    SeekRelative(skippedBits);

    // Return the number of bits actually read.
    return (int)readSizeBits;
}

float bf_read::ReadBitAngle(int numbits) {
    float fReturn;
    int   i;
    float shift;

    shift = (float)(BitForBitnum(numbits));

    i = ReadUBitLong(numbits);
    fReturn = (float)i * (360.0 / shift);

    return fReturn;
}

unsigned int bf_read::PeekUBitLong(int numbits) {
    unsigned int r;
    int          i, nBitValue;
#ifdef BIT_VERBOSE
    int nShifts = numbits;
#endif

    bf_read savebf;

    savebf = *this; // Save current state info

    r = 0;
    for (i = 0; i < numbits; i++) {
        nBitValue = ReadOneBit();

        // Append to current stream
        if (nBitValue) {
            r |= BitForBitnum(i);
        }
    }

    *this = savebf;

#ifdef BIT_VERBOSE
    Con_Printf("PeekBitLong:  %i %i\n", nShifts, (unsigned int)r);
#endif

    return r;
}

unsigned int bf_read::ReadUBitLongNoInline(int numbits) {
    return ReadUBitLong(numbits);
}

unsigned int bf_read::ReadUBitVarInternal(int encodingType) {
    m_iCurBit -= 4;
    // int bits = { 4, 8, 12, 32 }[ encodingType ];
    int bits = 4 + encodingType * 4 + (((2 - encodingType) >> 31) & 16);
    return ReadUBitLong(bits);
}

// Append numbits least significant bits from data to the current bit stream
int bf_read::ReadSBitLong(int numbits) {
    unsigned int r = ReadUBitLong(numbits);
    unsigned int s = 1 << (numbits - 1);
    if (r >= s) {
        // sign-extend by removing sign bit and then subtracting sign bit again
        r = r - s - s;
    }
    return r;
}

uint32_t bf_read::ReadVarInt32() {
    uint32_t result = 0;
    int      count = 0;
    uint32_t b;

    do {
        if (count == bitbuf::kMaxVarint32Bytes) {
            return result;
        }
        b = ReadUBitLong(8);
        result |= (b & 0x7F) << (7 * count);
        ++count;
    } while (b & 0x80);

    return result;
}

uint64_t bf_read::ReadVarInt64() {
    uint64_t result = 0;
    int      count = 0;
    uint64_t b;

    do {
        if (count == bitbuf::kMaxVarintBytes) {
            return result;
        }
        b = ReadUBitLong(8);
        result |= static_cast<uint64_t>(b & 0x7F) << (7 * count);
        ++count;
    } while (b & 0x80);

    return result;
}

int32_t bf_read::ReadSignedVarInt32() {
    uint32_t value = ReadVarInt32();
    return bitbuf::ZigZagDecode32(value);
}

int64_t bf_read::ReadSignedVarInt64() {
    uint32_t value = ReadVarInt64();
    return bitbuf::ZigZagDecode64(value);
}

unsigned int bf_read::ReadBitLong(int numbits, bool bSigned) {
    if (bSigned)
        return (unsigned int)ReadSBitLong(numbits);
    else
        return ReadUBitLong(numbits);
}

// Basic Coordinate Routines (these contain bit-field size AND fixed point scaling constants)
float bf_read::ReadBitCoord(void) {
#if defined(BB_PROFILING)
    VPROF("bf_read::ReadBitCoord");
#endif
    int   intval = 0, fractval = 0, signbit = 0;
    float value = 0.0;

    // Read the required integer and fraction flags
    intval = ReadOneBit();
    fractval = ReadOneBit();

    // If we got either parse them, otherwise it's a zero.
    if (intval || fractval) {
        // Read the sign bit
        signbit = ReadOneBit();

        // If there's an integer, read it in
        if (intval) {
            // Adjust the integers from [0..MAX_COORD_VALUE-1] to [1..MAX_COORD_VALUE]
            intval = ReadUBitLong(COORD_INTEGER_BITS) + 1;
        }

        // If there's a fraction, read it in
        if (fractval) {
            fractval = ReadUBitLong(COORD_FRACTIONAL_BITS);
        }

        // Calculate the correct floating point value
        value = intval + ((float)fractval * COORD_RESOLUTION);

        // Fixup the sign if negative.
        if (signbit)
            value = -value;
    }

    return value;
}

float bf_read::ReadBitCoordMP(bool bIntegral, bool bLowPrecision) {
#if defined(BB_PROFILING)
    VPROF("bf_read::ReadBitCoordMP");
#endif
    // BitCoordMP float encoding: inbounds bit, integer bit, sign bit, optional int bits, float bits
    // BitCoordMP integer encoding: inbounds bit, integer bit, optional sign bit, optional int bits.
    // int bits are always encoded as (value - 1) since zero is handled by the integer bit

    // With integer-only encoding, the presence of the third bit depends on the second
    int flags = ReadUBitLong(3 - bIntegral);
    enum {
        INBOUNDS = 1,
        INTVAL = 2,
        SIGN = 4
    };

    if (bIntegral) {
        if (flags & INTVAL) {
            // Read the third bit and the integer portion together at once
            unsigned int bits =
                ReadUBitLong((flags & INBOUNDS) ? COORD_INTEGER_BITS_MP + 1 : COORD_INTEGER_BITS + 1);
            // Remap from [0,N] to [1,N+1]
            int intval = (bits >> 1) + 1;
            return (bits & 1) ? -intval : intval;
        }
        return 0.f;
    }

    static const float mul_table[4] = { 1.f / (1 << COORD_FRACTIONAL_BITS), -1.f / (1 << COORD_FRACTIONAL_BITS),
                                       1.f / (1 << COORD_FRACTIONAL_BITS_MP_LOWPRECISION),
                                       -1.f / (1 << COORD_FRACTIONAL_BITS_MP_LOWPRECISION) };
    // equivalent to: float multiply = mul_table[ ((flags & SIGN) ? 1 : 0) + bLowPrecision*2 ];
    float              multiply = *(float*)((uintptr_t)&mul_table[0] + (flags & 4) + bLowPrecision * 8);

    static const unsigned char numbits_table[8] = { COORD_FRACTIONAL_BITS,
                                                   COORD_FRACTIONAL_BITS,
                                                   COORD_FRACTIONAL_BITS + COORD_INTEGER_BITS,
                                                   COORD_FRACTIONAL_BITS + COORD_INTEGER_BITS_MP,
                                                   COORD_FRACTIONAL_BITS_MP_LOWPRECISION,
                                                   COORD_FRACTIONAL_BITS_MP_LOWPRECISION,
                                                   COORD_FRACTIONAL_BITS_MP_LOWPRECISION + COORD_INTEGER_BITS,
                                                   COORD_FRACTIONAL_BITS_MP_LOWPRECISION +
                                                       COORD_INTEGER_BITS_MP };
    unsigned int bits = ReadUBitLong(numbits_table[(flags & (INBOUNDS | INTVAL)) + bLowPrecision * 4]);

    if (flags & INTVAL) {
        // Shuffle the bits to remap the integer portion from [0,N] to [1,N+1]
        // and then paste in front of the fractional parts so we only need one
        // int-to-float conversion.

        unsigned int fracbitsMP = bits >> COORD_INTEGER_BITS_MP;
        unsigned int fracbits = bits >> COORD_INTEGER_BITS;

        unsigned int intmaskMP = ((1 << COORD_INTEGER_BITS_MP) - 1);
        unsigned int intmask = ((1 << COORD_INTEGER_BITS) - 1);

        unsigned int selectNotMP = (flags & INBOUNDS) - 1;

        fracbits -= fracbitsMP;
        fracbits &= selectNotMP;
        fracbits += fracbitsMP;

        intmask -= intmaskMP;
        intmask &= selectNotMP;
        intmask += intmaskMP;

        unsigned int intpart = (bits & intmask) + 1;
        unsigned int intbitsLow = intpart << COORD_FRACTIONAL_BITS_MP_LOWPRECISION;
        unsigned int intbits = intpart << COORD_FRACTIONAL_BITS;
        unsigned int selectNotLow = (unsigned int)bLowPrecision - 1;

        intbits -= intbitsLow;
        intbits &= selectNotLow;
        intbits += intbitsLow;

        bits = fracbits | intbits;
    }

    return (int)bits * multiply;
}

unsigned int bf_read::ReadBitCoordBits(void) {
#if defined(BB_PROFILING)
    VPROF("bf_read::ReadBitCoordBits");
#endif

    unsigned int flags = ReadUBitLong(2);
    if (flags == 0)
        return 0;

    static const int numbits_table[3] = { COORD_INTEGER_BITS + 1, COORD_FRACTIONAL_BITS + 1,
                                         COORD_INTEGER_BITS + COORD_FRACTIONAL_BITS + 1 };
    return ReadUBitLong(numbits_table[flags - 1]) * 4 + flags;
}

unsigned int bf_read::ReadBitCoordMPBits(bool bIntegral, bool bLowPrecision) {
#if defined(BB_PROFILING)
    VPROF("bf_read::ReadBitCoordMPBits");
#endif

    unsigned int flags = ReadUBitLong(2);
    enum {
        INBOUNDS = 1,
        INTVAL = 2
    };
    int numbits = 0;

    if (bIntegral) {
        if (flags & INTVAL) {
            numbits = (flags & INBOUNDS) ? (1 + COORD_INTEGER_BITS_MP) : (1 + COORD_INTEGER_BITS);
        }
        else {
            return flags; // no extra bits
        }
    }
    else {
        static const unsigned char numbits_table[8] = {
            1 + COORD_FRACTIONAL_BITS,
            1 + COORD_FRACTIONAL_BITS,
            1 + COORD_FRACTIONAL_BITS + COORD_INTEGER_BITS,
            1 + COORD_FRACTIONAL_BITS + COORD_INTEGER_BITS_MP,
            1 + COORD_FRACTIONAL_BITS_MP_LOWPRECISION,
            1 + COORD_FRACTIONAL_BITS_MP_LOWPRECISION,
            1 + COORD_FRACTIONAL_BITS_MP_LOWPRECISION + COORD_INTEGER_BITS,
            1 + COORD_FRACTIONAL_BITS_MP_LOWPRECISION + COORD_INTEGER_BITS_MP };
        numbits = numbits_table[flags + bLowPrecision * 4];
    }

    return flags + ReadUBitLong(numbits) * 4;
}

void bf_read::ReadBitVec3Coord(vector& fa) {
    int xflag, yflag, zflag;

    // This Vector3 must be initialized! Otherwise, If any of the flags aren't set,
    // the corresponding component will not be read and will be stack garbage.
    fa = vector(0, 0, 0);

    xflag = ReadOneBit();
    yflag = ReadOneBit();
    zflag = ReadOneBit();

    if (xflag)
        fa[0] = ReadBitCoord();
    if (yflag)
        fa[1] = ReadBitCoord();
    if (zflag)
        fa[2] = ReadBitCoord();
}

float bf_read::ReadBitNormal(void) {
    // Read the sign bit
    int          signbit = ReadOneBit();

    // Read the fractional part
    unsigned int fractval = ReadUBitLong(NORMAL_FRACTIONAL_BITS);

    // Calculate the correct floating point value
    float        value = (float)fractval * NORMAL_RESOLUTION;

    // Fixup the sign if negative.
    if (signbit)
        value = -value;

    return value;
}

void bf_read::ReadBitVec3Normal(vector& fa) {
    int xflag = ReadOneBit();
    int yflag = ReadOneBit();

    if (xflag)
        fa[0] = ReadBitNormal();
    else
        fa[0] = 0.0f;

    if (yflag)
        fa[1] = ReadBitNormal();
    else
        fa[1] = 0.0f;

    // The first two imply the third (but not its sign)
    int   znegative = ReadOneBit();

    float fafafbfb = fa[0] * fa[0] + fa[1] * fa[1];
    if (fafafbfb < 1.0f)
        fa[2] = sqrt(1.0f - fafafbfb);
    else
        fa[2] = 0.0f;

    if (znegative)
        fa[2] = -fa[2];
}

void bf_read::ReadBitAngles(vector& fa) {
    vector tmp;
    ReadBitVec3Coord(tmp);
    // fa(tmp.x, tmp.y, tmp.z);
}

int64_t bf_read::ReadLongLong() {
    int64_t       retval;
    unsigned int* pLongs = (unsigned int*)&retval;

    // Read the two DWORDs according to network endian
    const short   endianIndex = 0x0100;
    char* idx = (char*)&endianIndex;
    pLongs[*idx++] = ReadUBitLong(sizeof(long) << 3);
    pLongs[*idx] = ReadUBitLong(sizeof(long) << 3);

    return retval;
}

float bf_read::ReadFloat() {
    float ret;
    nullAssert(sizeof(ret) == 4);
    ReadBits(&ret, 32);

    // Swap the float, since ReadBits reads raw data
    LittleFloat(&ret, &ret);
    return ret;
}

bool bf_read::ReadBytes(void* pOut, int nBytes) {
    ReadBits(pOut, nBytes << 3);
    return !IsOverflowed();
}

bool bf_read::ReadString(char* pStr, int maxLen, bool bLine, int* pOutNumChars) {
    nullAssert(maxLen != 0);

    bool bTooSmall = false;
    int  iChar = 0;
    while (1) {
        char val = ReadChar();
        if (val == 0)
            break;
        else if (bLine && val == '\n')
            break;

        if (iChar < (maxLen - 1)) {
            pStr[iChar] = val;
            ++iChar;
        }
        else {
            bTooSmall = true;
        }
    }

    // Make sure it's null-terminated.
    nullAssert(iChar < maxLen);
    pStr[iChar] = 0;

    if (pOutNumChars)
        *pOutNumChars = iChar;

    return !IsOverflowed() && !bTooSmall;
}

char* bf_read::ReadAndAllocateString(bool* pOverflow) {
    char str[2048];

    int  nChars;
    bool bOverflow = !ReadString(str, sizeof(str), false, &nChars);
    if (pOverflow)
        *pOverflow = bOverflow;

    // Now copy into the output and return it;
    char* pRet = new char[nChars + 1];
    for (int i = 0; i <= nChars; i++)
        pRet[i] = str[i];

    return pRet;
}

void bf_read::ExciseBits(int startbit, int bitstoremove) {
    int      endbit = startbit + bitstoremove;
    int      remaining_to_end = m_nDataBits - endbit;

    bf_write temp;
    temp.StartWriting((void*)m_pData, m_nDataBits << 3, startbit);

    Seek(endbit);

    for (int i = 0; i < remaining_to_end; i++) {
        temp.WriteOneBit(ReadOneBit());
    }

    Seek(startbit);

    m_nDataBits -= bitstoremove;
    m_nDataBytes = m_nDataBits >> 3;
}

int bf_read::CompareBitsAt(int offset, bf_read* RESTRICT other, int otherOffset, int numbits) RESTRICT {
    extern unsigned long g_ExtraMasks[33];

    if (numbits == 0)
        return 0;

    int overflow1 = offset + numbits > m_nDataBits;
    int overflow2 = otherOffset + numbits > other->m_nDataBits;

    int x = overflow1 | overflow2;
    if (x != 0)
        return x;

    unsigned int   iStartBit1 = offset & 31u;
    unsigned int   iStartBit2 = otherOffset & 31u;
    unsigned long* pData1 = (unsigned long*)m_pData + (offset >> 5);
    unsigned long* pData2 = (unsigned long*)other->m_pData + (otherOffset >> 5);
    unsigned long* pData1End = pData1 + ((offset + numbits - 1) >> 5);
    unsigned long* pData2End = pData2 + ((otherOffset + numbits - 1) >> 5);

    while (numbits > 32) {
        x = LoadLittleDWord((unsigned long*)pData1, 0) >> iStartBit1;
        x ^= LoadLittleDWord((unsigned long*)pData1, 1) << (32 - iStartBit1);
        x ^= LoadLittleDWord((unsigned long*)pData2, 0) >> iStartBit2;
        x ^= LoadLittleDWord((unsigned long*)pData2, 1) << (32 - iStartBit2);
        if (x != 0) {
            return x;
        }
        ++pData1;
        ++pData2;
        numbits -= 32;
    }

    x = LoadLittleDWord((unsigned long*)pData1, 0) >> iStartBit1;
    x ^= LoadLittleDWord((unsigned long*)pData1End, 0) << (32 - iStartBit1);
    x ^= LoadLittleDWord((unsigned long*)pData2, 0) >> iStartBit2;
    x ^= LoadLittleDWord((unsigned long*)pData2End, 0) << (32 - iStartBit2);
    return (x & g_ExtraMasks[numbits]);
}

#include <Windows.h>

// The four core functions - F1 is optimized somewhat
// #define F1(x, y, z) (x & y | ~x & z)
#define F1(x, y, z)                     (z ^ (x & (y ^ z)))
#define F2(x, y, z)                     F1(z, x, y)
#define F3(x, y, z)                     (x ^ y ^ z)
#define F4(x, y, z)                     (y ^ (x | ~z))

// This is the central step in the MD5 algorithm.
#define MD5STEP(f, w, x, y, z, data, s) (w += f(x, y, z) + data, w = w << s | w >> (32 - s), w += x)

//-----------------------------------------------------------------------------
// Purpose: The core of the MD5 algorithm, this alters an existing MD5 hash to
//  reflect the addition of 16 longwords of new data.  MD5Update blocks
//  the data and converts bytes into longwords for this routine.
// Input  : buf[4] -
//			in[16] -
// Output : static void
//-----------------------------------------------------------------------------

static void MD5Transform(unsigned int buf[4], unsigned int const in[16]) {
    unsigned int a, b, c, d;

    a = buf[0];
    b = buf[1];
    c = buf[2];
    d = buf[3];

    MD5STEP(F1, a, b, c, d, in[0] + 0xd76aa478, 7);
    MD5STEP(F1, d, a, b, c, in[1] + 0xe8c7b756, 12);
    MD5STEP(F1, c, d, a, b, in[2] + 0x242070db, 17);
    MD5STEP(F1, b, c, d, a, in[3] + 0xc1bdceee, 22);
    MD5STEP(F1, a, b, c, d, in[4] + 0xf57c0faf, 7);
    MD5STEP(F1, d, a, b, c, in[5] + 0x4787c62a, 12);
    MD5STEP(F1, c, d, a, b, in[6] + 0xa8304613, 17);
    MD5STEP(F1, b, c, d, a, in[7] + 0xfd469501, 22);
    MD5STEP(F1, a, b, c, d, in[8] + 0x698098d8, 7);
    MD5STEP(F1, d, a, b, c, in[9] + 0x8b44f7af, 12);
    MD5STEP(F1, c, d, a, b, in[10] + 0xffff5bb1, 17);
    MD5STEP(F1, b, c, d, a, in[11] + 0x895cd7be, 22);
    MD5STEP(F1, a, b, c, d, in[12] + 0x6b901122, 7);
    MD5STEP(F1, d, a, b, c, in[13] + 0xfd987193, 12);
    MD5STEP(F1, c, d, a, b, in[14] + 0xa679438e, 17);
    MD5STEP(F1, b, c, d, a, in[15] + 0x49b40821, 22);

    MD5STEP(F2, a, b, c, d, in[1] + 0xf61e2562, 5);
    MD5STEP(F2, d, a, b, c, in[6] + 0xc040b340, 9);
    MD5STEP(F2, c, d, a, b, in[11] + 0x265e5a51, 14);
    MD5STEP(F2, b, c, d, a, in[0] + 0xe9b6c7aa, 20);
    MD5STEP(F2, a, b, c, d, in[5] + 0xd62f105d, 5);
    MD5STEP(F2, d, a, b, c, in[10] + 0x02441453, 9);
    MD5STEP(F2, c, d, a, b, in[15] + 0xd8a1e681, 14);
    MD5STEP(F2, b, c, d, a, in[4] + 0xe7d3fbc8, 20);
    MD5STEP(F2, a, b, c, d, in[9] + 0x21e1cde6, 5);
    MD5STEP(F2, d, a, b, c, in[14] + 0xc33707d6, 9);
    MD5STEP(F2, c, d, a, b, in[3] + 0xf4d50d87, 14);
    MD5STEP(F2, b, c, d, a, in[8] + 0x455a14ed, 20);
    MD5STEP(F2, a, b, c, d, in[13] + 0xa9e3e905, 5);
    MD5STEP(F2, d, a, b, c, in[2] + 0xfcefa3f8, 9);
    MD5STEP(F2, c, d, a, b, in[7] + 0x676f02d9, 14);
    MD5STEP(F2, b, c, d, a, in[12] + 0x8d2a4c8a, 20);

    MD5STEP(F3, a, b, c, d, in[5] + 0xfffa3942, 4);
    MD5STEP(F3, d, a, b, c, in[8] + 0x8771f681, 11);
    MD5STEP(F3, c, d, a, b, in[11] + 0x6d9d6122, 16);
    MD5STEP(F3, b, c, d, a, in[14] + 0xfde5380c, 23);
    MD5STEP(F3, a, b, c, d, in[1] + 0xa4beea44, 4);
    MD5STEP(F3, d, a, b, c, in[4] + 0x4bdecfa9, 11);
    MD5STEP(F3, c, d, a, b, in[7] + 0xf6bb4b60, 16);
    MD5STEP(F3, b, c, d, a, in[10] + 0xbebfbc70, 23);
    MD5STEP(F3, a, b, c, d, in[13] + 0x289b7ec6, 4);
    MD5STEP(F3, d, a, b, c, in[0] + 0xeaa127fa, 11);
    MD5STEP(F3, c, d, a, b, in[3] + 0xd4ef3085, 16);
    MD5STEP(F3, b, c, d, a, in[6] + 0x04881d05, 23);
    MD5STEP(F3, a, b, c, d, in[9] + 0xd9d4d039, 4);
    MD5STEP(F3, d, a, b, c, in[12] + 0xe6db99e5, 11);
    MD5STEP(F3, c, d, a, b, in[15] + 0x1fa27cf8, 16);
    MD5STEP(F3, b, c, d, a, in[2] + 0xc4ac5665, 23);

    MD5STEP(F4, a, b, c, d, in[0] + 0xf4292244, 6);
    MD5STEP(F4, d, a, b, c, in[7] + 0x432aff97, 10);
    MD5STEP(F4, c, d, a, b, in[14] + 0xab9423a7, 15);
    MD5STEP(F4, b, c, d, a, in[5] + 0xfc93a039, 21);
    MD5STEP(F4, a, b, c, d, in[12] + 0x655b59c3, 6);
    MD5STEP(F4, d, a, b, c, in[3] + 0x8f0ccc92, 10);
    MD5STEP(F4, c, d, a, b, in[10] + 0xffeff47d, 15);
    MD5STEP(F4, b, c, d, a, in[1] + 0x85845dd1, 21);
    MD5STEP(F4, a, b, c, d, in[8] + 0x6fa87e4f, 6);
    MD5STEP(F4, d, a, b, c, in[15] + 0xfe2ce6e0, 10);
    MD5STEP(F4, c, d, a, b, in[6] + 0xa3014314, 15);
    MD5STEP(F4, b, c, d, a, in[13] + 0x4e0811a1, 21);
    MD5STEP(F4, a, b, c, d, in[4] + 0xf7537e82, 6);
    MD5STEP(F4, d, a, b, c, in[11] + 0xbd3af235, 10);
    MD5STEP(F4, c, d, a, b, in[2] + 0x2ad7d2bb, 15);
    MD5STEP(F4, b, c, d, a, in[9] + 0xeb86d391, 21);

    buf[0] += a;
    buf[1] += b;
    buf[2] += c;
    buf[3] += d;
}

//-----------------------------------------------------------------------------
// Purpose: Start MD5 accumulation.  Set bit count to 0 and buffer to mysterious initialization constants.

// Input  : *ctx -
//-----------------------------------------------------------------------------
void MD5Init(MD5Context_t* ctx) {
    ctx->buf[0] = 0x67452301;
    ctx->buf[1] = 0xefcdab89;
    ctx->buf[2] = 0x98badcfe;
    ctx->buf[3] = 0x10325476;

    ctx->bits[0] = 0;
    ctx->bits[1] = 0;
}

//-----------------------------------------------------------------------------
// Purpose: Update context to reflect the concatenation of another buffer full of bytes.
// Input  : *ctx -
//			*buf -
//			len -
//-----------------------------------------------------------------------------
void MD5Update(MD5Context_t* ctx, unsigned char const* buf, unsigned int len) {
    unsigned int t;

    /* Update bitcount */

    t = ctx->bits[0];
    if ((ctx->bits[0] = t + ((unsigned int)len << 3)) < t)
        ctx->bits[1]++; /* Carry from low to high */
    ctx->bits[1] += len >> 29;

    t = (t >> 3) & 0x3f; /* Bytes already in shsInfo->data */

    /* Handle any leading odd-sized chunks */

    if (t) {
        unsigned char* p = (unsigned char*)ctx->in + t;

        t = 64 - t;
        if (len < t) {
            memcpy(p, buf, len);
            return;
        }
        memcpy(p, buf, t);
        // byteReverse(ctx->in, 16);
        MD5Transform(ctx->buf, (unsigned int*)ctx->in);
        buf += t;
        len -= t;
    }
    /* Process data in 64-byte chunks */

    while (len >= 64) {
        memcpy(ctx->in, buf, 64);
        // byteReverse(ctx->in, 16);
        MD5Transform(ctx->buf, (unsigned int*)ctx->in);
        buf += 64;
        len -= 64;
    }

    /* Handle any remaining bytes of data. */
    memcpy(ctx->in, buf, len);
}

//-----------------------------------------------------------------------------
// Purpose: Final wrapup - pad to 64-byte boundary with the bit pattern
// 1 0* (64-bit count of bits processed, MSB-first)
// Input  : digest[MD5_DIGEST_LENGTH] -
//			*ctx -
//-----------------------------------------------------------------------------
void MD5Final(unsigned char digest[MD5_DIGEST_LENGTH], MD5Context_t* ctx) {
    unsigned       count;
    unsigned char* p;

    /* Compute number of bytes mod 64 */
    count = (ctx->bits[0] >> 3) & 0x3F;

    /* Set the first char of padding to 0x80.  This is safe since there is
       always at least one byte free */
    p = ctx->in + count;
    *p++ = 0x80;

    /* Bytes of padding needed to make 64 bytes */
    count = 64 - 1 - count;

    /* Pad out to 56 mod 64 */
    if (count < 8) {
        /* Two lots of padding:  Pad the first block to 64 bytes */
        memset(p, 0, count);
        // byteReverse(ctx->in, 16);
        MD5Transform(ctx->buf, (unsigned int*)ctx->in);

        /* Now fill the next block with 56 bytes */
        memset(ctx->in, 0, 56);
    }
    else {
        /* Pad block to 56 bytes */
        memset(p, 0, count - 8);
    }
    // byteReverse(ctx->in, 14);

    /* Append length in bits and transform */
    ((unsigned int*)ctx->in)[14] = ctx->bits[0];
    ((unsigned int*)ctx->in)[15] = ctx->bits[1];

    MD5Transform(ctx->buf, (unsigned int*)ctx->in);
    // byteReverse((unsigned char *) ctx->buf, 4);
    memcpy(digest, ctx->buf, MD5_DIGEST_LENGTH);
    memset(ctx, 0, sizeof(*ctx)); /* In case it's sensitive */
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  : *hash -
//			hashlen -
// Output : char
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Purpose: generate pseudo random number from a seed number
// Input  : seed number
// Output : pseudo random number
//-----------------------------------------------------------------------------
unsigned int MD5_PseudoRandom(unsigned int nSeed) {
    MD5Context_t  ctx;
    unsigned char digest[MD5_DIGEST_LENGTH]; // The MD5 Hash

    memset(&ctx, 0, sizeof(ctx));

    MD5Init(&ctx);
    MD5Update(&ctx, (unsigned char*)&nSeed, sizeof(nSeed));
    MD5Final(digest, &ctx);

    return *(unsigned int*)(digest + 6); // use 4 middle bytes for random value
}

//-----------------------------------------------------------------------------
bool MD5_Compare(const MD5Value_t& data, const MD5Value_t& compare) {
    return memcmp(data.bits, compare.bits, MD5_DIGEST_LENGTH) == 0;
}

//-----------------------------------------------------------------------------
void MD5Value_t::Zero() {
    memset(bits, 0, sizeof(bits));
}

//-----------------------------------------------------------------------------

#undef min
#undef max
void c_tf2::updatew2s()
{
    c_view_setup view_setup = {};
    if (client->get_player_view(view_setup)) {
        static D3DMATRIX w2view = {};
        static D3DMATRIX view2proj = {};
        static D3DMATRIX w2pixels = {};

        render_view->get_matrices_for_view(&view_setup, &w2view, &view2proj, &m_w2s_mat, &w2pixels);
    }

}

bool c_tf2::w2s(const vector& origin, vec2& screen)
{
    const auto& w2s = engine_client->world_to_screen_matrix();

    const float w = (w2s[3][0] * origin[0] + w2s[3][1] * origin[1] + w2s[3][2] * origin[2] + w2s[3][3]);

    if (w > 0.001f)
    {
        const float flsw = static_cast<float>(screenwidth);
        const float flsh = static_cast<float>(screenheight);
        const float fl1dbw = 1.0f / w;

        screen.x = (flsw / 2.0f) + (0.5f * ((w2s[0][0] * origin[0] + w2s[0][1] * origin[1] + w2s[0][2] * origin[2] + w2s[0][3]) * fl1dbw) * flsw + 0.5f);
        screen.y = (flsh / 2.0f) - (0.5f * ((w2s[1][0] * origin[0] + w2s[1][1] * origin[1] + w2s[1][2] * origin[2] + w2s[1][3]) * fl1dbw) * flsh + 0.5f);

        return true;
    }

    return false;
}


void c_tf2::getlocal()
{
}
static bool once = false;
std::string c_tf2::randomstringfromlist() {
    static bool initialized = false; // Static variable to check if the function has been called before
    static std::string result; // Static variable to store the random string

    if (!initialized) { // Check if this is the first call
        std::string arr[] = {
            "Know thy self, know thy enemy. A thousand battles, a thousand victories. - Sun Tzu",
            "If you know the enemy and know yourself you need not fear the results of a hundred battles. - Sun Tzu",
            "Pretend inferiority and encourage his arrogance. - Sun Tzu",
            "Know your enemy and know yourself and you can fight a hundred battles without disaster. - Sun Tzu",
            "In peace, sons bury their fathers. In war, fathers bury their sons. - Herodotus",
            "Lord, make me an instrument of thy peace. Where there is hatred, let me sow love. - Francis of Assisi",
            "To love and be loved is to feel the sun from both sides. - David Viscott",
            "The fact that an opinion has been widely held is no evidence whatever that it is not utterly absurd. - Bertrand Russell",
            "It is no measure of health to be well adjusted to a profoundly sick society. - Jiddu Krishnamurti",
            "hawk tuah",
            "Your body hears everything your mind says. - Naomi Judd",
            "joguei mvm ganhei merda - shootluckynicetap ",
            "I don't see anything bad in pasting something you completely understand. - kgb"
        };
        int arrLength = sizeof(arr) / sizeof(arr[0]);  // Get the length of the array

        srand(static_cast<unsigned int>(time(0))); // Seed for random number generation
        int randomNumber = rand() % arrLength; // Generate a random index

        result = arr[randomNumber]; // Store the result
        initialized = true; // Mark as initialized
    }

    return result; // Return the stored result
}

void c_tf2::trace(const vector& start, const vector& end, unsigned int mask, c_trace_filter* pFilter, c_game_trace* pTrace)
{
    ray_t ray;
    ray.init(start, end);
    engine_trace->trace_ray(ray, mask, pFilter, pTrace);
}

bool c_tf2::computeboundingbox(tf_entity* entity, vector* transformed, rect_t& bbox)
{
    const auto& Mins = entity->mins();
    const auto& Maxs = entity->maxs();

    vector Points[8] = {
        vector(Mins.x, Mins.y, Mins.z),
        vector(Mins.x, Maxs.y, Mins.z),
        vector(Maxs.x, Maxs.y, Mins.z),
        vector(Maxs.x, Mins.y, Mins.z),
        vector(Maxs.x, Maxs.y, Maxs.z),
        vector(Mins.x, Maxs.y, Maxs.z),
        vector(Mins.x, Mins.y, Maxs.z),
        vector(Maxs.x, Mins.y, Maxs.z)
    };

    auto& Transform = (matrix3x4&)entity->renderable_to_world_transform();



    auto Left = std::numeric_limits< float >::max();
    auto Top = std::numeric_limits< float >::max();
    auto Right = std::numeric_limits< float >::lowest();
    auto Bottom = std::numeric_limits< float >::lowest();

    vec2 Screen[8];

    for (std::size_t i = 0; i < 8; i++) {
        math::vector_transform(Points[i], Transform, transformed[i]);
        if (!w2s(transformed[i], Screen[i]))
        {
            return false;
        }

        Left = std::min(Left, Screen[i].x);
        Top = std::min(Top, Screen[i].y);
        Right = std::max(Right, Screen[i].x);
        Bottom = std::max(Bottom, Screen[i].y);
    }

    auto x_ = Left;
    auto y_ = Top;
    auto w_ = Right - Left;
    auto h_ = Bottom - Top;

    if (entity->get_class_id() == e_class_ids::CTFPlayer) {
        x_ += ((Right - Left) / 8.0f);
        w_ -= (((Right - Left) / 8.0f) * 2.0f);
    }



    bbox.x = static_cast<int>(x_);
    bbox.y = static_cast<int>(y_);
    bbox.w = static_cast<int>(w_);
    bbox.h = static_cast<int>(h_); //z is h

    return true;
}

vec2 c_tf2::getscreensize()
{
    engine_client->get_screen_size(screenwidth, screenheight);
    return vec2(screenwidth, screenheight);
}

bool c_tf2::vis_check(tf_player* skip, const tf_player* ent, const vector& from, const vector& to, unsigned int mask)
{
    c_game_trace trace = {};
    c_trace_filter_hitscan filter = {};
    filter.skip = skip;
    tf2.trace(from, to, mask, &filter, &trace);
    if (trace.did_hit())
        return trace.ent && trace.ent == ent;
    return true;
}

void c_tf2::fixmovement(c_user_cmd* cmd, const vector& target_angle)
{
    auto forward = vector(), right = vector(), up = vector();
    math::angle_vectors(target_angle, &forward, &right, &up);

    forward.z = right.z = up.x = up.y = 0.0f;

    math::normalize(forward.length());
    math::normalize(right.length());
    math::normalize(up.length());

    auto OldForward = vector(), OldRight = vector(), OldUp = vector();
    math::angle_vectors(cmd->viewangles, &OldForward, &OldRight, &OldUp);

    OldForward.z = OldRight.z = OldUp.x = OldUp.y = 0.0f;

    math::normalize(OldForward.length());
    math::normalize(OldRight.length());
    math::normalize(OldUp.length());

    const float pForward = forward.x * cmd->forwardmove;
    const float yForward = forward.y * cmd->forwardmove;
    const float pSide = right.x * cmd->sidemove;
    const float ySide = right.y * cmd->sidemove;
    const float rUp = up.z * cmd->upmove;

    const float x = OldForward.x * pSide + OldForward.y * ySide + OldForward.x * pForward + OldForward.y * yForward + OldForward.z * rUp;
    const float y = OldRight.x * pSide + OldRight.y * ySide + OldRight.x * pForward + OldRight.y * yForward + OldRight.z * rUp;
    const float z = OldUp.x * ySide + OldUp.y * pSide + OldUp.x * yForward + OldUp.y * pForward + OldUp.z * rUp;

    cmd->forwardmove = math::clamp(x, -450.f, 450.f);
    cmd->sidemove = math::clamp(y, -450.f, 450.f);
    cmd->upmove = math::clamp(z, -450.f, 450.f);
}

std::wstring c_tf2::ConvertUtf8ToWide(const std::string& str)
{
    int cnt = MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.length()), NULL, 0);

    std::wstring s(cnt, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.length()), &s[0], cnt);

    return s;
}
