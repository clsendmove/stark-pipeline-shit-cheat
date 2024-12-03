#pragma once
#include "../../math.hpp"
//#include "interfaces.hpp"
#include "math.hpp"
#include "Windows.h"
/*class bf_read {
public:
  unsigned long* m_pData;
  int            m_nDataBytes;
  int            m_nDataBits;
  int            m_iCurBit;
  bool           m_bOverflow;
  bool           m_bAssertOnOverflow;
  const char*    m_pDebugName;
};

class bf_write {
public:
  int StartWriting(void* pData, int nBytes, int iStartBit = 0, int nBits = -1);

public:
  unsigned long* m_pData;
  int            m_nDataBytes;
  int            m_nDataBits;
  int            m_iCurBit;
  bool           m_bOverflow;
  bool           m_bAssertOnOverflow;
  const char*    m_pDebugName;
};*/





#include <cassert>
#include <cstdint>

#define nullAssert assert

#if _DEBUG
#define BITBUF_INLINE inline
#else
#define BITBUF_INLINE FORCEINLINE
#endif

#define RESTRICT

//-----------------------------------------------------------------------------
// You can define a handler function that will be called in case of
// out-of-range values and overruns here.
//
// NOTE: the handler is only called in debug mode.
//
// Call SetBitBufErrorHandler to install a handler.
//-----------------------------------------------------------------------------

typedef enum
{
    BITBUFERROR_VALUE_OUT_OF_RANGE = 0,		// Tried to write a value with too few bits.
    BITBUFERROR_BUFFER_OVERRUN,				// Was about to overrun a buffer.

    BITBUFERROR_NUM_ERRORS
} BitBufErrorType;


typedef void(*BitBufErrorHandler)(BitBufErrorType errorType, const char* pDebugName);


#if defined( _DEBUG )
extern void InternalBitBufErrorHandler(BitBufErrorType errorType, const char* pDebugName);
#define CallErrorHandler( errorType, pDebugName ) InternalBitBufErrorHandler( errorType, pDebugName );
#else
#define CallErrorHandler( errorType, pDebugName )
#endif


// Use this to install the error handler. Call with NULL to uninstall your error handler.
void SetBitBufErrorHandler(BitBufErrorHandler fn);

#define BITS_PER_INT 32

//-----------------------------------------------------------------------------
// Helpers.
//-----------------------------------------------------------------------------
template <typename T>
inline T WordSwapC(T w)
{
    uint16_t temp;

    temp = ((*((uint16_t*)&w) & 0xff00) >> 8);
    temp |= ((*((uint16_t*)&w) & 0x00ff) << 8);

    return *((T*)&temp);
}

template <typename T>
inline T DWordSwapC(T dw)
{
    uint32_t temp;

    temp = *((uint32_t*)&dw) >> 24;
    temp |= ((*((uint32_t*)&dw) & 0x00FF0000) >> 8);
    temp |= ((*((uint32_t*)&dw) & 0x0000FF00) << 8);
    temp |= ((*((uint32_t*)&dw) & 0x000000FF) << 24);

    return *((T*)&temp);
}

template <typename T>
inline T QWordSwapC(T dw)
{
    // Assert sizes passed to this are already correct, otherwise
    // the cast to uint64_t * below is unsafe and may have wrong results
    // or even crash.
    //PLAT_COMPILE_TIME_nullAssert(sizeof(dw) == sizeof(uint64_t));

    uint64_t temp;

    temp = *((uint64_t*)&dw) >> 56;
    temp |= ((*((uint64_t*)&dw) & 0x00FF000000000000ull) >> 40);
    temp |= ((*((uint64_t*)&dw) & 0x0000FF0000000000ull) >> 24);
    temp |= ((*((uint64_t*)&dw) & 0x000000FF00000000ull) >> 8);
    temp |= ((*((uint64_t*)&dw) & 0x00000000FF000000ull) << 8);
    temp |= ((*((uint64_t*)&dw) & 0x0000000000FF0000ull) << 24);
    temp |= ((*((uint64_t*)&dw) & 0x000000000000FF00ull) << 40);
    temp |= ((*((uint64_t*)&dw) & 0x00000000000000FFull) << 56);

    return *((T*)&temp);
}

inline int BitByte(int bits)
{
    // return PAD_NUMBER( bits, 8 ) >> 3;
    return (bits + 7) >> 3;
}

inline unsigned GetEndMask(int numBits)
{
    static unsigned bitStringEndMasks[] =
    {
        0xffffffff,
        0x00000001,
        0x00000003,
        0x00000007,
        0x0000000f,
        0x0000001f,
        0x0000003f,
        0x0000007f,
        0x000000ff,
        0x000001ff,
        0x000003ff,
        0x000007ff,
        0x00000fff,
        0x00001fff,
        0x00003fff,
        0x00007fff,
        0x0000ffff,
        0x0001ffff,
        0x0003ffff,
        0x0007ffff,
        0x000fffff,
        0x001fffff,
        0x003fffff,
        0x007fffff,
        0x00ffffff,
        0x01ffffff,
        0x03ffffff,
        0x07ffffff,
        0x0fffffff,
        0x1fffffff,
        0x3fffffff,
        0x7fffffff,
    };

    return bitStringEndMasks[numBits % BITS_PER_INT];
}


inline int GetBitForBitnum(int bitNum)
{
    static int bitsForBitnum[] =
    {
        (1 << 0),
        (1 << 1),
        (1 << 2),
        (1 << 3),
        (1 << 4),
        (1 << 5),
        (1 << 6),
        (1 << 7),
        (1 << 8),
        (1 << 9),
        (1 << 10),
        (1 << 11),
        (1 << 12),
        (1 << 13),
        (1 << 14),
        (1 << 15),
        (1 << 16),
        (1 << 17),
        (1 << 18),
        (1 << 19),
        (1 << 20),
        (1 << 21),
        (1 << 22),
        (1 << 23),
        (1 << 24),
        (1 << 25),
        (1 << 26),
        (1 << 27),
        (1 << 28),
        (1 << 29),
        (1 << 30),
        (1 << 31),
    };

    return bitsForBitnum[(bitNum) & (BITS_PER_INT - 1)];
}

inline int GetBitForBitnumByte(int bitNum)
{
    static int bitsForBitnum[] =
    {
        (1 << 0),
        (1 << 1),
        (1 << 2),
        (1 << 3),
        (1 << 4),
        (1 << 5),
        (1 << 6),
        (1 << 7),
    };

    return bitsForBitnum[bitNum & 7];
}

inline int CalcNumIntsForBits(int numBits) { return (numBits + (BITS_PER_INT - 1)) / BITS_PER_INT; }



#define WordSwap  WordSwapC
#define DWordSwap DWordSwapC
#define QWordSwap QWordSwapC


#define SafeSwapFloat( pOut, pIn )	(*((unsigned int*)pOut) = DWordSwap( *((unsigned int*)pIn) ))
inline short BigShort(short val) { int test = 1; return (*(char*)&test == 1) ? WordSwap(val) : val; }
inline uint16_t BigWord(uint16_t val) { int test = 1; return (*(char*)&test == 1) ? WordSwap(val) : val; }
inline long BigLong(long val) { int test = 1; return (*(char*)&test == 1) ? DWordSwap(val) : val; }
inline uint32_t BigDWord(uint32_t val) { int test = 1; return (*(char*)&test == 1) ? DWordSwap(val) : val; }
inline short LittleShort(short val) { int test = 1; return (*(char*)&test == 1) ? val : WordSwap(val); }
inline uint16_t LittleWord(uint16_t val) { int test = 1; return (*(char*)&test == 1) ? val : WordSwap(val); }
inline long LittleLong(long val) { int test = 1; return (*(char*)&test == 1) ? val : DWordSwap(val); }
inline uint32_t LittleDWord(uint32_t val) { int test = 1; return (*(char*)&test == 1) ? val : DWordSwap(val); }
inline uint64_t LittleQWord(uint64_t val) { int test = 1; return (*(char*)&test == 1) ? val : QWordSwap(val); }
inline short SwapShort(short val) { return WordSwap(val); }
inline uint16_t SwapWord(uint16_t val) { return WordSwap(val); }
inline long SwapLong(long val) { return DWordSwap(val); }
inline uint32_t SwapDWord(uint32_t val) { return DWordSwap(val); }

// Pass floats by pointer for swapping to avoid truncation in the fpu
inline void BigFloat(float* pOut, const float* pIn) { int test = 1; (*(char*)&test == 1) ? SafeSwapFloat(pOut, pIn) : (*pOut = *pIn); }
inline void LittleFloat(float* pOut, const float* pIn) { int test = 1; (*(char*)&test == 1) ? (*pOut = *pIn) : SafeSwapFloat(pOut, pIn); }
inline void SwapFloat(float* pOut, const float* pIn) { SafeSwapFloat(pOut, pIn); }


#define FORCEINLINE __forceinline

FORCEINLINE unsigned long LoadLittleDWord(const unsigned long* base, unsigned int dwordIndex)
{
    return LittleDWord(base[dwordIndex]);
}

FORCEINLINE void StoreLittleDWord(unsigned long* base, unsigned int dwordIndex, unsigned long dword)
{
    base[dwordIndex] = LittleDWord(dword);
}



//-----------------------------------------------------------------------------
// namespaced helpers
//-----------------------------------------------------------------------------
namespace bitbuf
{
    // ZigZag Transform:  Encodes signed integers so that they can be
    // effectively used with varint encoding.
    //
    // varint operates on unsigned integers, encoding smaller numbers into
    // fewer bytes.  If you try to use it on a signed integer, it will treat
    // this number as a very large unsigned integer, which means that even
    // small signed numbers like -1 will take the maximum number of bytes
    // (10) to encode.  ZigZagEncode() maps signed integers to unsigned
    // in such a way that those with a small absolute value will have smaller
    // encoded values, making them appropriate for encoding using varint.
    //
    //       int32_t ->     uint32_t
    // -------------------------
    //           0 ->          0
    //          -1 ->          1
    //           1 ->          2
    //          -2 ->          3
    //         ... ->        ...
    //  2147483647 -> 4294967294
    // -2147483648 -> 4294967295
    //
    //        >> encode >>
    //        << decode <<

    inline uint32_t ZigZagEncode32(int32_t n)
    {
        // Note:  the right-shift must be arithmetic
        return(n << 1) ^ (n >> 31);
    }

    inline int32_t ZigZagDecode32(uint32_t n)
    {
        return(n >> 1) ^ -static_cast<int32_t>(n & 1);
    }

    inline uint64_t ZigZagEncode64(int64_t n)
    {
        // Note:  the right-shift must be arithmetic
        return(n << 1) ^ (n >> 63);
    }

    inline int64_t ZigZagDecode64(uint64_t n)
    {
        return(n >> 1) ^ -static_cast<int64_t>(n & 1);
    }

    const int kMaxVarintBytes = 10;
    const int kMaxVarint32Bytes = 5;
}

class bf_write
{
public:
    bf_write();
    bf_write(void* pData, int nBytes, int nMaxBits = -1);
    bf_write(const char* pDebugName, void* pData, int nBytes, int nMaxBits = -1);
    void			StartWriting(void* pData, int nBytes, int iStartBit = 0, int nMaxBits = -1);
    void			Reset();
    unsigned char* GetBasePointer() { return (unsigned char*)m_pData; }
    void			SetAssertOnOverflow(bool bAssert);
    const char* GetDebugName();
    void			SetDebugName(const char* pDebugName);
    void			SeekToBit(int bitPos);
    void			WriteOneBit(int nValue);
    void			WriteOneBitNoCheck(int nValue);
    void			WriteOneBitAt(int iBit, int nValue);
    void			WriteUBitLong(unsigned int data, int numbits, bool bCheckRange = true);
    void			WriteSBitLong(int data, int numbits);
    void			WriteBitLong(unsigned int data, int numbits, bool bSigned);
    bool			WriteBits(const void* pIn, int nBits);
    void			WriteUBitVar(unsigned int data);
    void			WriteVarInt32(uint32_t data);
    void			WriteVarInt64(uint64_t data);
    void			WriteSignedVarInt32(int32_t data);
    void			WriteSignedVarInt64(int64_t data);
    int				ByteSizeVarInt32(uint32_t data);
    int				ByteSizeVarInt64(uint64_t data);
    int				ByteSizeSignedVarInt32(int32_t data);
    int				ByteSizeSignedVarInt64(int64_t data);
    bool			WriteBitsFromBuffer(class bf_read* pIn, int nBits);
    void			WriteBitAngle(float fAngle, int numbits);
    void			WriteBitCoord(const float f);
    void			WriteBitCoordMP(const float f, bool bIntegral, bool bLowPrecision);
    void			WriteBitFloat(float val);
    void			WriteBitVec3Coord(const vector& fa);
    void			WriteBitNormal(float f);
    void			WriteBitVec3Normal(const vector& fa);
    void			WriteBitAngles(const vector& fa);
    void			WriteChar(int val);
    void			WriteByte(int val);
    void			WriteShort(int val);
    void			WriteWord(int val);
    void			WriteLong(long val);
    void			WriteLongLong(int64_t val);
    void			WriteFloat(float val);
    bool			WriteBytes(const void* pBuf, int nBytes);
    bool			WriteString(const char* pStr);
    int				GetNumBytesWritten() const;
    int				GetNumBitsWritten() const;
    int				GetMaxNumBits();
    int				GetNumBitsLeft();
    int				GetNumBytesLeft();
    unsigned char* GetData();
    const unsigned char* GetData() const;
    bool			CheckForOverflow(int nBits);
    inline bool		IsOverflowed() const { return m_bOverflow; }
    void			SetOverflowFlag();

public:
    unsigned long* m_pData;
    int				m_nDataBytes;
    int				m_nDataBits;
    int				m_iCurBit;
    bool			m_bOverflow;
    bool			m_bAssertOnOverflow;
    const char* m_pDebugName;
};


//-----------------------------------------------------------------------------
// Inlined methods
//-----------------------------------------------------------------------------

// How many bytes are filled in?
inline int bf_write::GetNumBytesWritten() const
{
    return BitByte(m_iCurBit);
}

inline int bf_write::GetNumBitsWritten() const
{
    return m_iCurBit;
}

inline int bf_write::GetMaxNumBits()
{
    return m_nDataBits;
}

inline int bf_write::GetNumBitsLeft()
{
    return m_nDataBits - m_iCurBit;
}

inline int bf_write::GetNumBytesLeft()
{
    return GetNumBitsLeft() >> 3;
}

inline unsigned char* bf_write::GetData()
{
    return (unsigned char*)m_pData;
}

inline const unsigned char* bf_write::GetData()	const
{
    return (unsigned char*)m_pData;
}

BITBUF_INLINE bool bf_write::CheckForOverflow(int nBits)
{
    if (m_iCurBit + nBits > m_nDataBits)
    {
        SetOverflowFlag();
        CallErrorHandler(BITBUFERROR_BUFFER_OVERRUN, GetDebugName());
    }

    return m_bOverflow;
}

BITBUF_INLINE void bf_write::SetOverflowFlag()
{
#ifdef DBGFLAG_ASSERT
    if (m_bAssertOnOverflow)
    {
        nullAssert(false);
    }
#endif
    m_bOverflow = true;
}

BITBUF_INLINE void bf_write::WriteOneBitNoCheck(int nValue)
{
#if __i386__
    if (nValue)
        m_pData[m_iCurBit >> 5] |= 1u << (m_iCurBit & 31);
    else
        m_pData[m_iCurBit >> 5] &= ~(1u << (m_iCurBit & 31));
#else
    extern unsigned long g_LittleBits[32];
    if (nValue)
        m_pData[m_iCurBit >> 5] |= g_LittleBits[m_iCurBit & 31];
    else
        m_pData[m_iCurBit >> 5] &= ~g_LittleBits[m_iCurBit & 31];
#endif

    ++m_iCurBit;
}

inline void bf_write::WriteOneBit(int nValue)
{
    if (m_iCurBit >= m_nDataBits)
    {
        SetOverflowFlag();
        CallErrorHandler(BITBUFERROR_BUFFER_OVERRUN, GetDebugName());
        return;
    }
    WriteOneBitNoCheck(nValue);
}


inline void	bf_write::WriteOneBitAt(int iBit, int nValue)
{
    if (iBit >= m_nDataBits)
    {
        SetOverflowFlag();
        CallErrorHandler(BITBUFERROR_BUFFER_OVERRUN, GetDebugName());
        return;
    }

#if __i386__
    if (nValue)
        m_pData[iBit >> 5] |= 1u << (iBit & 31);
    else
        m_pData[iBit >> 5] &= ~(1u << (iBit & 31));
#else
    extern unsigned long g_LittleBits[32];
    if (nValue)
        m_pData[iBit >> 5] |= g_LittleBits[iBit & 31];
    else
        m_pData[iBit >> 5] &= ~g_LittleBits[iBit & 31];
#endif
}

BITBUF_INLINE void bf_write::WriteUBitLong(unsigned int curData, int numbits, bool bCheckRange) RESTRICT
{
#ifdef _DEBUG
    // Make sure it doesn't overflow.
    if (bCheckRange && numbits < 32)
    {
        if (curData >= (unsigned long)(1 << numbits))
        {
            CallErrorHandler(BITBUFERROR_VALUE_OUT_OF_RANGE, GetDebugName());
        }
    }
    nullAssert(numbits >= 0 && numbits <= 32);
#endif

    if (GetNumBitsLeft() < numbits)
    {
        m_iCurBit = m_nDataBits;
        SetOverflowFlag();
        CallErrorHandler(BITBUFERROR_BUFFER_OVERRUN, GetDebugName());
        return;
    }

    int iCurBitMasked = m_iCurBit & 31;
    int iDWord = m_iCurBit >> 5;
    m_iCurBit += numbits;

    // Mask in a dword.
    nullAssert((iDWord * 4 + sizeof(long)) <= (unsigned int)m_nDataBytes);
    unsigned long* RESTRICT pOut = &m_pData[iDWord];

    // Rotate data into dword alignment
    curData = (curData << iCurBitMasked) | (curData >> (32 - iCurBitMasked));

    // Calculate bitmasks for first and second word
    unsigned int temp = 1 << (numbits - 1);
    unsigned int mask1 = (temp * 2 - 1) << iCurBitMasked;
    unsigned int mask2 = (temp - 1) >> (31 - iCurBitMasked);

    // Only look beyond current word if necessary (avoid access violation)
    int i = mask2 & 1;
    unsigned long dword1 = LoadLittleDWord(pOut, 0);
    unsigned long dword2 = LoadLittleDWord(pOut, i);

    // Drop bits into place
    dword1 ^= (mask1 & (curData ^ dword1));
    dword2 ^= (mask2 & (curData ^ dword2));

    // Note reversed order of writes so that dword1 wins if mask2 == 0 && i == 0
    StoreLittleDWord(pOut, i, dword2);
    StoreLittleDWord(pOut, 0, dword1);
}

// writes an unsigned integer with variable bit length
BITBUF_INLINE void bf_write::WriteUBitVar(unsigned int data)
{
    /* Reference:
    if ( data < 0x10u )
            WriteUBitLong( 0, 2 ), WriteUBitLong( data, 4 );
    else if ( data < 0x100u )
            WriteUBitLong( 1, 2 ), WriteUBitLong( data, 8 );
    else if ( data < 0x1000u )
            WriteUBitLong( 2, 2 ), WriteUBitLong( data, 12 );
    else
            WriteUBitLong( 3, 2 ), WriteUBitLong( data, 32 );
    */
    // a < b ? -1 : 0 translates into a CMP, SBB instruction pair
    // with no flow control. should also be branchless on consoles.
    int n = (data < 0x10u ? -1 : 0) + (data < 0x100u ? -1 : 0) + (data < 0x1000u ? -1 : 0);
    WriteUBitLong(data * 4 + n + 3, 6 + n * 4 + 12);
    if (data >= 0x1000u)
    {
        WriteUBitLong(data >> 16, 16);
    }
}

// write raw IEEE float bits in little endian form
BITBUF_INLINE void bf_write::WriteBitFloat(float val)
{
    long intVal;

    nullAssert(sizeof(long) == sizeof(float));
    nullAssert(sizeof(float) == 4);

    intVal = *((long*)&val);
    WriteUBitLong(intVal, 32);
}

//-----------------------------------------------------------------------------
// This is useful if you just want a buffer to write into on the stack.
//-----------------------------------------------------------------------------

template<int SIZE>
class old_bf_write_static : public bf_write
{
public:
    inline old_bf_write_static() : bf_write(m_StaticData, SIZE) {}

    char	m_StaticData[SIZE];
};


class bf_read
{
public:
    bf_read();
    bf_read(const void* pData, int nBytes, int nBits = -1);
    bf_read(const char* pDebugName, const void* pData, int nBytes, int nBits = -1);
    void			StartReading(const void* pData, int nBytes, int iStartBit = 0, int nBits = -1);
    void			Reset();
    void			SetAssertOnOverflow(bool bAssert);
    const char* GetDebugName() const { return m_pDebugName; }
    void			SetDebugName(const char* pName);
    void			ExciseBits(int startbit, int bitstoremove);
    int				ReadOneBit();
protected:
    unsigned int	CheckReadUBitLong(int numbits);		// For debugging.
    int				ReadOneBitNoCheck();				// Faster version, doesn't check bounds and is inlined.
    bool			CheckForOverflow(int nBits);
public:
    const unsigned long* GetBasePointer() { return m_pData; }
    BITBUF_INLINE int TotalBytesAvailable(void) const
    {
        return m_nDataBytes;
    }
    void            ReadBits(void* pOut, int nBits);
    int             ReadBitsClamped_ptr(void* pOut, size_t outSizeBytes, size_t nBits);
    template <typename T, size_t N>
    int             ReadBitsClamped(T(&pOut)[N], size_t nBits)
    {
        return ReadBitsClamped_ptr(pOut, N * sizeof(T), nBits);
    }
    float			ReadBitAngle(int numbits);
    unsigned int	ReadUBitLong(int numbits) RESTRICT;
    unsigned int	ReadUBitLongNoInline(int numbits) RESTRICT;
    unsigned int	PeekUBitLong(int numbits);
    int				ReadSBitLong(int numbits);
    unsigned int	ReadUBitVar();
    unsigned int	ReadUBitVarInternal(int encodingType);
    uint32_t			ReadVarInt32();
    uint64_t			ReadVarInt64();
    int32_t			ReadSignedVarInt32();
    int64_t			ReadSignedVarInt64();
    unsigned int	ReadBitLong(int numbits, bool bSigned);
    float			ReadBitCoord();
    float			ReadBitCoordMP(bool bIntegral, bool bLowPrecision);
    float			ReadBitFloat();
    float			ReadBitNormal();
    void			ReadBitVec3Coord(vector& fa);
    void			ReadBitVec3Normal(vector& fa);
    void			ReadBitAngles(vector& fa);
    unsigned int	ReadBitCoordBits();
    unsigned int	ReadBitCoordMPBits(bool bIntegral, bool bLowPrecision);
    BITBUF_INLINE int	ReadChar() { return (char)ReadUBitLong(8); }
    BITBUF_INLINE int	ReadByte() { return ReadUBitLong(8); }
    BITBUF_INLINE int	ReadShort() { return (short)ReadUBitLong(16); }
    BITBUF_INLINE int	ReadWord() { return ReadUBitLong(16); }
    BITBUF_INLINE long ReadLong() { return ReadUBitLong(32); }
    int64_t			ReadLongLong();
    float			ReadFloat();
    bool			ReadBytes(void* pOut, int nBytes);
    bool			ReadString(char* pStr, int bufLen, bool bLine = false, int* pOutNumChars = NULL);
    char* ReadAndAllocateString(bool* pOverflow = 0);
    int				CompareBits(bf_read* RESTRICT other, int bits) RESTRICT;
    int				CompareBitsAt(int offset, bf_read* RESTRICT other, int otherOffset, int bits) RESTRICT;
    int				GetNumBytesLeft();
    int				GetNumBytesRead();
    int				GetNumBitsLeft();
    int				GetNumBitsRead() const;
    inline bool		IsOverflowed() const { return m_bOverflow; }
    inline bool		Seek(int iBit);					// Seek to a specific bit.
    inline bool		SeekRelative(int iBitDelta);	// Seek to an offset from the current position.
    void			SetOverflowFlag();
public:
    unsigned long* m_pData;
    int					m_nDataBytes;
    int					m_nDataBits;
    int					m_iCurBit;
    bool				m_bOverflow;
    bool				m_bAssertOnOverflow;
    const char* m_pDebugName;
};

//-----------------------------------------------------------------------------
// Inlines.
//-----------------------------------------------------------------------------

inline int bf_read::GetNumBytesRead()
{
    return BitByte(m_iCurBit);
}

inline int bf_read::GetNumBitsLeft()
{
    return m_nDataBits - m_iCurBit;
}

inline int bf_read::GetNumBytesLeft()
{
    return GetNumBitsLeft() >> 3;
}

inline int bf_read::GetNumBitsRead() const
{
    return m_iCurBit;
}

inline bool bf_read::Seek(int iBit)
{
    if (iBit < 0 || iBit > m_nDataBits)
    {
        SetOverflowFlag();
        m_iCurBit = m_nDataBits;
        return false;
    }
    else
    {
        m_iCurBit = iBit;
        return true;
    }
}

// Seek to an offset from the current position.
inline bool	bf_read::SeekRelative(int iBitDelta)
{
    return Seek(m_iCurBit + iBitDelta);
}

inline bool bf_read::CheckForOverflow(int nBits)
{
    if (m_iCurBit + nBits > m_nDataBits)
    {
        SetOverflowFlag();
        CallErrorHandler(BITBUFERROR_BUFFER_OVERRUN, GetDebugName());
    }

    return m_bOverflow;
}

inline int bf_read::ReadOneBitNoCheck()
{
#if VALVE_LITTLE_ENDIAN
    unsigned int value = ((unsigned long* RESTRICT)m_pData)[m_iCurBit >> 5] >> (m_iCurBit & 31);
#else
    unsigned char value = m_pData[m_iCurBit >> 3] >> (m_iCurBit & 7);
#endif
    ++m_iCurBit;
    return value & 1;
}

inline int bf_read::ReadOneBit()
{
    if (GetNumBitsLeft() <= 0)
    {
        SetOverflowFlag();
        CallErrorHandler(BITBUFERROR_BUFFER_OVERRUN, GetDebugName());
        return 0;
    }
    return ReadOneBitNoCheck();
}

inline float bf_read::ReadBitFloat()
{
    union { uint32_t u; float f; } c = { ReadUBitLong(32) };
    return c.f;
}

BITBUF_INLINE unsigned int bf_read::ReadUBitVar()
{
    // six bits: low 2 bits for encoding + first 4 bits of value
    unsigned int sixbits = ReadUBitLong(6);
    unsigned int encoding = sixbits & 3;
    if (encoding)
    {
        // this function will seek back four bits and read the full value
        return ReadUBitVarInternal(encoding);
    }
    return sixbits >> 2;
}

BITBUF_INLINE unsigned int bf_read::ReadUBitLong(int numbits) RESTRICT
{
    nullAssert(numbits > 0 && numbits <= 32);

    if (GetNumBitsLeft() < numbits)
    {
        m_iCurBit = m_nDataBits;
        SetOverflowFlag();
        CallErrorHandler(BITBUFERROR_BUFFER_OVERRUN, GetDebugName());
        return 0;
    }

    unsigned int iStartBit = m_iCurBit & 31u;
    int iLastBit = m_iCurBit + numbits - 1;
    unsigned int iWordOffset1 = m_iCurBit >> 5;
    unsigned int iWordOffset2 = iLastBit >> 5;
    m_iCurBit += numbits;

#if __i386__
    unsigned int bitmask = (2 << (numbits - 1)) - 1;
#else
    extern unsigned long g_ExtraMasks[33];
    unsigned int bitmask = g_ExtraMasks[numbits];
#endif

    unsigned int dw1 = LoadLittleDWord((unsigned long* RESTRICT)m_pData, iWordOffset1) >> iStartBit;
    unsigned int dw2 = LoadLittleDWord((unsigned long* RESTRICT)m_pData, iWordOffset2) << (32 - iStartBit);

    return (dw1 | dw2) & bitmask;
}

BITBUF_INLINE int bf_read::CompareBits(bf_read* RESTRICT other, int numbits) RESTRICT
{
    return (ReadUBitLong(numbits) != other->ReadUBitLong(numbits));
}



// Largest # of commands to send in a packet
#define NUM_NEW_COMMAND_BITS    4
#define MAX_NEW_COMMANDS        ((1 << NUM_NEW_COMMAND_BITS) - 1)
// Max number of history commands to send ( 2 by default ) in case of dropped packets
#define NUM_BACKUP_COMMAND_BITS 3
#define MAX_BACKUP_COMMANDS     ((1 << NUM_BACKUP_COMMAND_BITS) - 1)
#define NETMSG_TYPE_BITS        6 // must be 2^NETMSG_TYPE_BITS > SVC_LASTMSG
#define FLOW_OUTGOING           0
#define FLOW_INCOMING           1
#define MAX_FLOWS               2 // in & out
#define MAX_STREAMS             2
#define MAX_SUBCHANNELS         8 // we have 8 alternative send&wait bits
#define SUBCHANNEL_FREE         0 // subchannel is free to use
#define SUBCHANNEL_TOSEND       1 // subchannel has data, but not send yet
#define SUBCHANNEL_WAITING      2 // sbuchannel sent data, waiting for ACK
#define SUBCHANNEL_DIRTY        3 // subchannel is marked as dirty during changelevel
#define clc_Move                9

typedef struct netadr_s {
public:
    netadr_s() {
        SetIP(0);
        SetPort(0);
    }
    netadr_s(unsigned int unIP, unsigned short usPort) {
        SetIP(unIP);
        SetPort(usPort);
    }
    netadr_s(const char* pch) { SetFromString(pch); }
    void Clear(); // invalids Address

    void SetPort(unsigned short port);
    bool SetFromSockadr(const struct sockaddr* s);
    void SetIP(unsigned char b1, unsigned char  b2, unsigned char  b3, unsigned char  b4);
    void SetIP(unsigned int unIP); // Sets IP.  unIP is in host order (little-endian)
    void SetIPAndPort(unsigned int unIP, unsigned short usPort) {
        SetIP(unIP);
        SetPort(usPort);
    }
    bool           SetFromString(const char* pch,
        bool        bUseDNS = false); // if bUseDNS is true then do a DNS lookup if needed

    bool           CompareAdr(const netadr_s& a, bool onlyBase = false) const;
    bool           CompareClassBAdr(const netadr_s& a) const;
    bool           CompareClassCAdr(const netadr_s& a) const;

    // netadrtype_t	GetType() const;
    unsigned short GetPort() const;

    // DON'T CALL THIS
    const char* ToString(bool onlyBase = false) const; // returns xxx.xxx.xxx.xxx:ppppp

    void           ToString(char* pchBuffer, unsigned int unBufferSize,
        bool onlyBase = false) const; // returns xxx.xxx.xxx.xxx:ppppp
    template <size_t maxLenInChars>
    void ToString_safe(char(&pDest)[maxLenInChars], bool onlyBase = false) const {
        ToString(&pDest[0], maxLenInChars, onlyBase);
    }

    void         ToSockadr(struct sockaddr* s) const;

    // Returns 0xAABBCCDD for AA.BB.CC.DD on all platforms, which is the same format used by SetIP().
    // (So why isn't it just named GetIP()?  Because previously there was a fucntion named GetIP(), and
    // it did NOT return back what you put into SetIP().  So we nuked that guy.)
    unsigned int GetIPHostByteOrder() const;

    // Returns a number that depends on the platform.  In most cases, this probably should not be used.
    unsigned int GetIPNetworkByteOrder() const;

    bool         IsLocalhost() const;    // true, if this is the localhost IP
    bool         IsLoopback() const;     // true if engine loopback buffers are used
    bool         IsReservedAdr() const;  // true, if this is a private LAN IP
    bool         IsValid() const;        // ip & port != 0
    bool         IsBaseAdrValid() const; // ip != 0

    void         SetFromSocket(int hSocket);

    bool         Unserialize(bf_read& readBuf);
    bool         Serialize(bf_write& writeBuf);

    bool         operator==(const netadr_s& netadr) const { return (CompareAdr(netadr)); }
    bool         operator!=(const netadr_s& netadr) const { return !(CompareAdr(netadr)); }
    bool         operator<(const netadr_s& netadr) const;

public: // members are public to avoid to much changes
    // netadrtype_t	type;
    unsigned char  ip[4];
    unsigned short port;
} netadr_t;

class INetChannelInfo {
public:
    enum {
        GENERIC = 0,  // must be first and is default group
        LOCALPLAYER,  // bytes for local player entity update
        OTHERPLAYERS, // bytes for other players update
        ENTITIES,     // all other entity bytes
        SOUNDS,       // game sounds
        EVENTS,       // event messages
        USERMESSAGES, // user messages
        ENTMESSAGES,  // entity messages
        VOICE,        // voice data
        STRINGTABLE,  // a stringtable update
        MOVE,         // client move cmds
        STRINGCMD,    // string command
        SIGNON,       // various signondata
        TOTAL,        // must be last and is not a real group
    };

    virtual const char* GetName(void) const = 0;          // get channel name
    virtual const char* GetAddress(void) const = 0;       // get channel IP address as string
    virtual float       GetTime(void) const = 0;          // current net time
    virtual float       GetTimeConnected(void) const = 0; // get connection time in seconds
    virtual int         GetBufferSize(void) const = 0;    // netchannel packet history size
    virtual int         GetDataRate(void) const = 0;      // send data rate in byte/sec

    virtual bool        IsLoopback(void) const = 0;  // true if loopback channel
    virtual bool        IsTimingOut(void) const = 0; // true if timing out
    virtual bool        IsPlayback(void) const = 0;  // true if demo playback

    virtual float       GetLatency(int flow) const = 0;    // current latency (RTT), more accurate but jittering
    virtual float       GetAvgLatency(int flow) const = 0; // average packet latency in seconds
    virtual float       GetAvgLoss(int flow) const = 0;    // avg packet loss[0..1]
    virtual float       GetAvgChoke(int flow) const = 0;   // avg packet choke[0..1]
    virtual float       GetAvgData(int flow) const = 0;    // data flow in bytes/sec
    virtual float       GetAvgPackets(int flow) const = 0; // avg packets/sec
    virtual int         GetTotalData(int flow) const = 0;  // total flow in/out in bytes
    virtual int         GetSequenceNr(int flow) const = 0; // last send seq number
    virtual bool
        IsValidPacket(int flow, int frame_number) const = 0; // true if packet was not lost/dropped/chocked/flushed
    virtual float GetPacketTime(int flow, int frame_number) const = 0;             // time when packet was send
    virtual int   GetPacketBytes(int flow, int frame_number, int group) const = 0; // group size of this packet
    virtual bool  GetStreamProgress(int flow, int* received,
        int* total) const = 0;  // TCP progress if transmitting
    virtual float GetTimeSinceLastReceived(void) const = 0; // get time since last recieved packet in seconds
    virtual float GetCommandInterpolationAmount(int flow, int frame_number) const = 0;
    virtual void  GetPacketResponseLatency(int flow, int frame_number, int* pnLatencyMsecs,
        int* pnChoke) const = 0;
    virtual void  GetRemoteFramerate(float* pflFrameTime, float* pflFrameTimeStdDeviation) const = 0;

    virtual float GetTimeoutSeconds() const = 0;
};

class INetMessage; // forward declare
class INetChannelHandler;
class INetChannel : public INetChannelInfo {
public:
    virtual ~INetChannel(void) {};

    virtual void            SetDataRate(float rate) = 0;
    virtual bool            RegisterMessage(INetMessage* msg) = 0;
    virtual bool            StartStreaming(unsigned int challengeNr) = 0;
    virtual void            ResetStreaming(void) = 0;
    virtual void            SetTimeout(float seconds) = 0;
    // virtual void	SetDemoRecorder(IDemoRecorder *recorder) = 0;
    virtual void            SetChallengeNr(unsigned int chnr) = 0;

    virtual void            Reset(void) = 0;
    virtual void            Clear(void) = 0;
    virtual void            Shutdown(const char* reason) = 0;

    virtual void            ProcessPlayback(void) = 0;
    virtual bool            ProcessStream(void) = 0;
    virtual void            ProcessPacket(struct netpacket_s* packet, bool bHasHeader) = 0;

    virtual bool            SendNetMsg(INetMessage& msg, bool bForceReliable = false, bool bVoice = false) = 0;
    virtual bool            SendData(bf_write& msg, bool bReliable = true) = 0;
    virtual bool            SendFile(const char* filename, unsigned int transferID) = 0;
    virtual void            DenyFile(const char* filename, unsigned int transferID) = 0;
    virtual void            RequestFile_OLD(const char* filename,
        unsigned int transferID) = 0; // get rid of this function when we version the
    virtual void            SetChoked(void) = 0;
    virtual int             SendDatagram(bf_write* data) = 0;
    virtual bool            Transmit(bool onlyReliable = false) = 0;

    virtual const netadr_t& GetRemoteAddress(void) const = 0;
    virtual INetChannelHandler* GetMsgHandler(void) const = 0;
    virtual int                 GetDropNumber(void) const = 0;
    virtual int                 GetSocket(void) const = 0;
    virtual unsigned int        GetChallengeNr(void) const = 0;
    virtual void         GetSequenceData(int& nOutSequenceNr, int& nInSequenceNr, int& nOutSequenceNrAck) = 0;
    virtual void         SetSequenceData(int nOutSequenceNr, int nInSequenceNr, int nOutSequenceNrAck) = 0;

    virtual void         UpdateMessageStats(int msggroup, int bits) = 0;
    virtual bool         CanPacket(void) const = 0;
    virtual bool         IsOverflowed(void) const = 0;
    virtual bool         IsTimedOut(void) const = 0;
    virtual bool         HasPendingReliableData(void) = 0;

    virtual void         SetFileTransmissionMode(bool bBackgroundMode) = 0;
    virtual void         SetCompressionMode(bool bUseCompression) = 0;
    virtual unsigned int RequestFile(const char* filename) = 0;
    virtual float GetTimeSinceLastReceived(void) const = 0; // get time since last received packet in seconds

    virtual void  SetMaxBufferSize(bool bReliable, int nBytes, bool bVoice = false) = 0;

    virtual bool  IsNull() const = 0;
    virtual int   GetNumBitsWritten(bool bReliable) = 0;
    virtual void  SetInterpolationAmount(float flInterpolationAmount) = 0;
    virtual void  SetRemoteFramerate(float flFrameTime, float flFrameTimeStdDeviation) = 0;

    // Max # of payload bytes before we must split/fragment the packet
    virtual void  SetMaxRoutablePayloadSize(int nSplitSize) = 0;
    virtual int   GetMaxRoutablePayloadSize() = 0;

    virtual int   GetProtocolVersion() = 0;
};

class INetMessage {
public:
    virtual ~INetMessage() {};

    // Use these to setup who can hear whose voice.
    // Pass in client indices (which are their ent indices - 1).

    virtual void         SetNetChannel(INetChannel* netchan) = 0; // netchannel this message is from/for
    virtual void         SetReliable(bool state) = 0;             // set to true if it's a reliable message

    virtual bool         Process(void) = 0; // calles the recently set handler to process this message

    virtual bool         ReadFromBuffer(bf_read& buffer) = 0; // returns true if parsing was OK
    virtual bool         WriteToBuffer(bf_write& buffer) = 0; // returns true if writing was OK

    virtual bool         IsReliable(void) const = 0; // true, if message needs reliable handling

    virtual int          GetType(void) const = 0;  // returns module specific header tag eg svc_serverinfo
    virtual int          GetGroup(void) const = 0; // returns net message group of this message
    virtual const char* GetName(void) const = 0;  // returns network message name, eg "svc_serverinfo"
    virtual INetChannel* GetNetChannel(void) const = 0;
    virtual const char* ToString(void) const = 0; // returns a human readable string about message content
};

typedef void* FileHandle_t;
#define MAX_OSPATH      260
#define UDP_HEADER_SIZE 28

class CNetChannel : public INetChannel {

public: // netchan structurs
    typedef struct dataFragments_s {
        FileHandle_t file;                 // open file handle
        char         filename[MAX_OSPATH]; // filename
        char* buffer;               // if NULL it's a file
        unsigned int BYTEs;                // size in BYTEs
        unsigned int bits;                 // size in bits
        unsigned int transferID;           // only for files
        bool         isCompressed;         // true if data is bzip compressed
        unsigned int nUncompressedSize;    // full size in BYTEs
        bool         asTCP;                // send as TCP stream
        int          numFragments;         // number of total fragments
        int          ackedFragments;       // number of fragments send & acknowledged
        int          pendingFragments;     // number of fragments send, but not acknowledged yet
    } dataFragments_t;

    struct subChannel_s {
        int  startFraggment[MAX_STREAMS];
        int  numFragments[MAX_STREAMS];
        int  sendSeqNr;
        int  state; // 0 = free, 1 = scheduled to send, 2 = send & waiting, 3 = dirty
        int  index; // index in m_SubChannels[]

        void Free() {
            state = SUBCHANNEL_FREE;
            sendSeqNr = -1;
            for (int i = 0; i < MAX_STREAMS; i++) {
                numFragments[i] = 0;
                startFraggment[i] = -1;
            }
        }
    };

    typedef struct netframe_s {
        float          time;        // net_time received/send
        int            size;        // total size in BYTEs
        float          latency;     // raw ping for this packet, not cleaned. set when acknowledged otherwise -1.
        float          avg_latency; // averaged ping for this packet
        bool           valid;       // false if dropped, lost, flushed
        int            choked;      // number of previously chocked packets
        int            dropped;
        float          m_flInterpolationAmount;
        unsigned short msggroups[INetChannelInfo::TOTAL]; // received BYTEs for each message group
    } netframe_t;
    typedef struct {
        float       nextcompute;      // Time when we should recompute k/sec data
        float       avgBYTEspersec;   // average BYTEs/sec
        float       avgpacketspersec; // average packets/sec
        float       avgloss;          // average packet loss [0..1]
        float       avgchoke;         // average packet choke [0..1]
        float       avglatency;       // average ping, not cleaned
        float       latency;          // current ping, more accurate also more jittering
        int         totalpackets;     // total processed packets
        int         totalBYTEs;       // total processed BYTEs
        int         currentindex;     // current frame index
        netframe_t  frames[64];       // frame history
        netframe_t* currentframe;     // current frame
    } netflow_t;
    typedef struct netpacket_s {
        netadr_t            from;     // sender IP
        int                 source;   // received source
        double              received; // received time
        unsigned char* data;     // pointer to raw packet data
        bf_read             message;  // easy bitbuf data access
        int                 size;     // size in bytes
        int                 wiresize; // size in bytes before decompression
        bool                stream;   // was send as stream
        struct netpacket_s* pNext;    // for internal use, should be NULL in public
    } netpacket_t;

public:
    bool                         ProcessingMessages;
    bool                         ShouldDelete;

    int                          OutSequenceNr;
    int                          InSequenceNr;
    int                          OutSequenceNrAck;
    int                          OutReliableState;
    int                          InReliableState;
    int                          ChokedPackets;

    // Reliable data buffer, send which each packet (or put in waiting list)
    bf_write                     m_StreamReliable;
    CUtlMemory<byte>             m_ReliableDataBuffer;

    // unreliable message buffer, cleared which each packet
    bf_write                     m_StreamUnreliable;
    CUtlMemory<byte>             m_UnreliableDataBuffer;

    bf_write                     m_StreamVoice;
    CUtlMemory<byte>             m_VoiceDataBuffer;

    int                          m_Socket;       // NS_SERVER or NS_CLIENT index, depending on channel.
    int                          m_StreamSocket; // TCP socket handle

    unsigned int                 m_MaxReliablePayloadSize; // max size of reliable payload in a single packet

    // Address this channel is talking to.
    netadr_t                     remote_address;

    // For timeouts.  Time last message was received.
    float                        last_received;
    // Time when channel was connected.
    double                       connect_time;

    // Bandwidth choke
    // Bytes per second
    int                          m_Rate;
    // If realtime > cleartime, free to send next packet
    double                       m_fClearTime;

    CUtlVector<dataFragments_t*> m_WaitingList[MAX_STREAMS]; // waiting list for reliable data and file transfer
    dataFragments_t              m_ReceiveList[MAX_STREAMS]; // receive buffers for streams
    subChannel_s                 m_SubChannels[MAX_SUBCHANNELS];

    unsigned int                 m_FileRequestCounter;         // increasing counter with each file request
    bool                         m_bFileBackgroundTranmission; // if true, only send 1 fragment per packet
    bool                         m_bUseCompression; // if true, larger reliable data will be bzip compressed

    // TCP stream state maschine:
    bool                         m_StreamActive;          // true if TCP is active
    int                          m_SteamType;             // STREAM_CMD_*
    int                          m_StreamSeqNr;           // each blob send of TCP as an increasing ID
    int                          m_StreamLength;          // total length of current stream blob
    int                          m_StreamReceived;        // length of already received bytes
    char                         m_SteamFile[MAX_OSPATH]; // if receiving file, this is it's name
    CUtlMemory<byte>
        m_StreamData; // Here goes the stream data (if not file). Only allocated if we're going to use it.

    // packet history
    netflow_t m_DataFlow[MAX_FLOWS];

    int       m_MsgStats[INetChannelInfo::TOTAL]; // total bytes for each message group

    int       m_PacketDrop; // packets lost before getting last update (was global net_drop)

    char      m_Name[32]; // channel name

    unsigned int             m_ChallengeNr; // unique, random challenge number

    float                    m_Timeout; // in seconds

    INetChannelHandler* m_MessageHandler; // who registers and processes messages
    CUtlVector<INetMessage*> m_NetMessages;    // list of registered message
    void* m_DemoRecorder;   // if != NULL points to a recording/playback demo object
    int                      m_nQueuedPackets;

    float                    m_flInterpolationAmount;
    float                    m_flRemoteFrameTime;
    float                    m_flRemoteFrameTimeStdDeviation;
    int                      m_nMaxRoutablePayloadSize;

    int                      m_nSplitPacketSequence;
    bool m_bStreamContainsChallenge; // true if PACKET_FLAG_CHALLENGE was set when receiving packets from the
    // sender

    int  m_nProtocolVersion;
};

class CNetMessage : public INetMessage {
public:
    CNetMessage() {
        m_bReliable = true;
        m_NetChannel = nullptr;
    }

    virtual ~CNetMessage() {};

    virtual int  GetGroup() const { return INetChannelInfo::GENERIC; }
    INetChannel* GetNetChannel() const { return m_NetChannel; }

    virtual void SetReliable(bool state) { m_bReliable = state; };
    virtual bool IsReliable() const { return m_bReliable; };
    virtual void SetNetChannel(INetChannel* netchan) { m_NetChannel = netchan; }
    virtual bool Process() { return false; }; // no handler set

protected:
    bool         m_bReliable;  // true if message should be send reliable
    INetChannel* m_NetChannel; // netchannel this message is from/for
};

class CLC_Move : public CNetMessage {
public:
    bool ReadFromBuffer(bf_read& buffer) {
        m_nNewCommands = buffer.ReadUBitLong(NUM_NEW_COMMAND_BITS);
        m_nBackupCommands = buffer.ReadUBitLong(NUM_BACKUP_COMMAND_BITS);
        m_nLength = buffer.ReadWord();
        m_DataIn = buffer;
        return buffer.SeekRelative(m_nLength);
    }

    bool WriteToBuffer(bf_write& buffer) {
        buffer.WriteUBitLong(GetType(), NETMSG_TYPE_BITS);
        m_nLength = m_DataOut.GetNumBitsWritten();

        buffer.WriteUBitLong(m_nNewCommands, NUM_NEW_COMMAND_BITS);
        buffer.WriteUBitLong(m_nBackupCommands, NUM_BACKUP_COMMAND_BITS);

        buffer.WriteWord(m_nLength);

        return buffer.WriteBits(m_DataOut.GetData(), m_nLength);
    }

    const char* ToString() const { return "CLC_Move"; }
    int         GetType() const { return clc_Move; }
    const char* GetName() const { return "clc_Move"; }
    void* m_pMessageHandler;
    int         GetGroup() const { return INetChannelInfo::MOVE; }
    CLC_Move() { m_bReliable = false; }

public:
    int      m_nBackupCommands;
    int      m_nNewCommands;
    int      m_nLength;
    bf_read  m_DataIn;
    bf_write m_DataOut;
};

#define NET_TICK_SCALEUP 100000.0f
#define net_Tick         3 // send last world tick

class NET_Tick : public CNetMessage {
public:
    bool WriteToBuffer(bf_write& buffer) {
        buffer.WriteUBitLong(GetType(), NETMSG_TYPE_BITS);
        buffer.WriteLong(m_nTick);
        buffer.WriteUBitLong(math::clamp((int)(NET_TICK_SCALEUP * m_flHostFrameTime), 0, 65535), 16);
        buffer.WriteUBitLong(math::clamp((int)(NET_TICK_SCALEUP * m_flHostFrameTimeStdDeviation), 0, 65535), 16);
        return !buffer.IsOverflowed();
    }

    bool ReadFromBuffer(bf_read& buffer) {
        m_nTick = buffer.ReadLong();
        m_flHostFrameTime = (float)buffer.ReadUBitLong(16) / NET_TICK_SCALEUP;
        m_flHostFrameTimeStdDeviation = (float)buffer.ReadUBitLong(16) / NET_TICK_SCALEUP;
        return !buffer.IsOverflowed();
    }

    const char* ToString(void) const { return "NET_Tick"; }
    int         GetType() const { return net_Tick; }
    const char* GetName() const { return "net_Tick"; }

    NET_Tick() {
        m_bReliable = false;
        m_flHostFrameTime = 0;
        m_flHostFrameTimeStdDeviation = 0;
    };

    NET_Tick(int tick, float hostFrametime, float hostFrametime_stddeviation) {
        m_bReliable = false;
        m_nTick = tick;
        m_flHostFrameTime = hostFrametime;
        m_flHostFrameTimeStdDeviation = hostFrametime_stddeviation;
    };

public:
    int   m_nTick;
    float m_flHostFrameTime;
    float m_flHostFrameTimeStdDeviation;
};
//constexpr int TEAM_RED_COLOR[4] = { 255, 59, 59, 255 };
//constexpr int TEAM_RED_NOVIS_COLOR[4] = { 143, 28, 28, 255 };
//constexpr int TEAM_BLU_COLOR[4] = { 59, 130, 255, 255 };
//constexpr int TEAM_BLU_NOVIS_COLOR[4] = { 33, 84, 163, 255 };
//constexpr int TEAM_SPEC_COLOR[4] = { 216, 216, 216, 216 };

#define TEAM_RED_COLOR D3DCOLOR_RGBA(255,59,59,255)
#define TEAM_RED_NOVIS_COLOR D3DCOLOR_RGBA(143, 28, 28, 255)
#define TEAM_BLU_COLOR D3DCOLOR_RGBA(59, 130, 255, 255)
#define TEAM_BLU_NOVIS_COLOR D3DCOLOR_RGBA(33, 84, 163, 255)
#define TEAM_SPEC_COLOR D3DCOLOR_RGBA(216, 216, 216, 216);
#define TEAM_UNASSIGNED 0
#define TEAM_SPECTATOR 1
#define TEAM_RED 2
#define TEAM_BLU 3

#define MAX_CMD_BUFFER          4000

#define MAX_NEW_COMMANDS        ((1 << NUM_NEW_COMMAND_BITS) - 1)
#define NUM_BACKUP_COMMAND_BITS 3
#define MAX_BACKUP_COMMANDS     ((1 << NUM_BACKUP_COMMAND_BITS) - 1)
#define MAX_COMMANDS            MAX_NEW_COMMANDS + MAX_BACKUP_COMMANDS

#define MAX_PLAYER_NAME_LENGTH  32
#define SIGNED_GUID_LEN         32 // Hashed CD Key (32 hex alphabetic chars + 0 terminator )

#define SENTRYGUN_MAX_SHELLS_1  150
#define SENTRYGUN_MAX_SHELLS_2  200
#define SENTRYGUN_MAX_SHELLS_3  200
#define SENTRYGUN_MAX_ROCKETS   20

#define DISPSURF_FLAG_SURFACE   (1 << 0)
#define DISPSURF_FLAG_WALKABLE  (1 << 1)
#define DISPSURF_FLAG_BUILDABLE (1 << 2)
#define DISPSURF_FLAG_SURFPROP1 (1 << 3)
#define DISPSURF_FLAG_SURFPROP2 (1 << 4)

#define IN_ATTACK               (1 << 0)
#define IN_JUMP                 (1 << 1)
#define IN_DUCK                 (1 << 2)
#define IN_FORWARD              (1 << 3)
#define IN_BACK                 (1 << 4)
#define IN_USE                  (1 << 5)
#define IN_CANCEL               (1 << 6)
#define IN_LEFT                 (1 << 7)
#define IN_RIGHT                (1 << 8)
#define IN_MOVELEFT             (1 << 9)
#define IN_MOVERIGHT            (1 << 10)
#define IN_ATTACK2              (1 << 11)
#define IN_RUN                  (1 << 12)
#define IN_RELOAD               (1 << 13)
#define IN_ALT1                 (1 << 14)
#define IN_ALT2                 (1 << 15)
#define IN_SCORE                (1 << 16)
#define IN_SPEED                (1 << 17)
#define IN_WALK                 (1 << 18)
#define IN_ZOOM                 (1 << 19)
#define IN_WEAPON1              (1 << 20)
#define IN_WEAPON2              (1 << 21)
#define IN_BULLRUSH             (1 << 22)
#define IN_GRENADE1             (1 << 23)
#define IN_GRENADE2             (1 << 24)
#define IN_ATTACK3              (1 << 25)

#define FL_ONGROUND             (1 << 0)
#define FL_DUCKING              (1 << 1)
#define FL_WATERJUMP            (1 << 2)
#define FL_ONTRAIN              (1 << 3)
#define FL_INRAIN               (1 << 4)
#define FL_FROZEN               (1 << 5)
#define FL_ATCONTROLS           (1 << 6)
#define FL_CLIENT               (1 << 7)
#define FL_FAKECLIENT           (1 << 8)
#define FL_INWATER              (1 << 9)
#define FL_CONVEYOR             (1 << 12)

enum {
    TF_CLASS_UNDEFINED = 0,

    TF_CLASS_SCOUT, // TF_FIRST_NORMAL_CLASS
    TF_CLASS_SNIPER,
    TF_CLASS_SOLDIER,
    TF_CLASS_DEMOMAN,
    TF_CLASS_MEDIC,
    TF_CLASS_HEAVYWEAPONS,
    TF_CLASS_PYRO,
    TF_CLASS_SPY,
    TF_CLASS_ENGINEER,

    // Add any new classes after Engineer
    TF_CLASS_CIVILIAN, // TF_LAST_NORMAL_CLASS
    TF_CLASS_COUNT_ALL,

    TF_CLASS_RANDOM
};

//-----------------------------------------------------------------------------
// Player Classes.
//-----------------------------------------------------------------------------
#define TF_CLASS_COUNT        (TF_CLASS_COUNT_ALL)

#define TF_FIRST_NORMAL_CLASS (TF_CLASS_UNDEFINED + 1)
#define TF_LAST_NORMAL_CLASS  (TF_CLASS_CIVILIAN)

#define TF_CLASS_MENU_BUTTONS (TF_CLASS_RANDOM + 1)

enum e_hitboxes {
    HITBOX_HEAD,
    HITBOX_PELVIS,
    HITBOX_SPINE_0, // stomach
    HITBOX_SPINE_1, // stomach
    HITBOX_SPINE_2, // chest
    HITBOX_SPINE_3, // chest
    HITBOX_UPPERARM_L,
    HITBOX_LOWERARM_L,
    HITBOX_HAND_L,
    HITBOX_UPPERARM_R,
    HITBOX_LOWERARM_R,
    HITBOX_HAND_R,
    HITBOX_HIP_L,
    HITBOX_KNEE_L,
    HITBOX_FOOT_L,
    HITBOX_HIP_R,
    HITBOX_KNEE_R,
    HITBOX_FOOT_R,
    HITBOX_MAX
};

#define BONE_CALCULATE_MASK        0x1F
#define BONE_PHYSICALLY_SIMULATED  0x01 // bone is physically simulated when physics are active
#define BONE_PHYSICS_PROCEDURAL    0x02 // procedural when physics is active
#define BONE_ALWAYS_PROCEDURAL     0x04 // bone is always procedurally animated
#define BONE_SCREEN_ALIGN_SPHERE   0x08 // bone aligns to the screen, not constrained in motion.
#define BONE_SCREEN_ALIGN_CYLINDER 0x10 // bone aligns to the screen, constrained by it's own axis.

#define BONE_USED_MASK             0x0007FF00
#define BONE_USED_BY_ANYTHING      0x0007FF00
#define BONE_USED_BY_HITBOX        0x00000100 // bone (or child) is used by a hit box
#define BONE_USED_BY_ATTACHMENT    0x00000200 // bone (or child) is used by an attachment point
#define BONE_USED_BY_VERTEX_MASK   0x0003FC00
#define BONE_USED_BY_VERTEX_LOD0 \
  0x00000400 // bone (or child) is used by the toplevel model via skinned vertex
#define BONE_USED_BY_VERTEX_LOD1 0x00000800
#define BONE_USED_BY_VERTEX_LOD2 0x00001000
#define BONE_USED_BY_VERTEX_LOD3 0x00002000
#define BONE_USED_BY_VERTEX_LOD4 0x00004000
#define BONE_USED_BY_VERTEX_LOD5 0x00008000
#define BONE_USED_BY_VERTEX_LOD6 0x00010000
#define BONE_USED_BY_VERTEX_LOD7 0x00020000
#define BONE_USED_BY_BONE_MERGE  0x00040000 // bone is available for bone merge to occur against it

//
// NOTE: Inserting to most or all of the enums in this file will BREAK DEMOS -
// please add to the end instead.
//
enum e_weapon_type {
    TF_WEAPON_NONE = 0,
    TF_WEAPON_BAT,
    TF_WEAPON_BAT_WOOD,
    TF_WEAPON_BOTTLE,
    TF_WEAPON_FIREAXE,
    TF_WEAPON_CLUB,
    TF_WEAPON_CROWBAR,
    TF_WEAPON_KNIFE,
    TF_WEAPON_FISTS,
    TF_WEAPON_SHOVEL,
    TF_WEAPON_WRENCH,
    TF_WEAPON_BONESAW,
    TF_WEAPON_SHOTGUN_PRIMARY,
    TF_WEAPON_SHOTGUN_SOLDIER,
    TF_WEAPON_SHOTGUN_HWG,
    TF_WEAPON_SHOTGUN_PYRO,
    TF_WEAPON_SCATTERGUN,
    TF_WEAPON_SNIPERRIFLE,
    TF_WEAPON_MINIGUN,
    TF_WEAPON_SMG,
    TF_WEAPON_SYRINGEGUN_MEDIC,
    TF_WEAPON_TRANQ,
    TF_WEAPON_ROCKETLAUNCHER,
    TF_WEAPON_GRENADELAUNCHER,
    TF_WEAPON_PIPEBOMBLAUNCHER,
    TF_WEAPON_FLAMETHROWER,
    TF_WEAPON_GRENADE_NORMAL,
    TF_WEAPON_GRENADE_CONCUSSION,
    TF_WEAPON_GRENADE_NAIL,
    TF_WEAPON_GRENADE_MIRV,
    TF_WEAPON_GRENADE_MIRV_DEMOMAN,
    TF_WEAPON_GRENADE_NAPALM,
    TF_WEAPON_GRENADE_GAS,
    TF_WEAPON_GRENADE_EMP,
    TF_WEAPON_GRENADE_CALTROP,
    TF_WEAPON_GRENADE_PIPEBOMB,
    TF_WEAPON_GRENADE_SMOKE_BOMB,
    TF_WEAPON_GRENADE_HEAL,
    TF_WEAPON_GRENADE_STUNBALL,
    TF_WEAPON_GRENADE_JAR,
    TF_WEAPON_GRENADE_JAR_MILK,
    TF_WEAPON_PISTOL,
    TF_WEAPON_PISTOL_SCOUT,
    TF_WEAPON_REVOLVER,
    TF_WEAPON_NAILGUN,
    TF_WEAPON_PDA,
    TF_WEAPON_PDA_ENGINEER_BUILD,
    TF_WEAPON_PDA_ENGINEER_DESTROY,
    TF_WEAPON_PDA_SPY,
    TF_WEAPON_BUILDER,
    TF_WEAPON_MEDIGUN,
    TF_WEAPON_GRENADE_MIRVBOMB,
    TF_WEAPON_FLAMETHROWER_ROCKET,
    TF_WEAPON_GRENADE_DEMOMAN,
    TF_WEAPON_SENTRY_BULLET,
    TF_WEAPON_SENTRY_ROCKET,
    TF_WEAPON_DISPENSER,
    TF_WEAPON_INVIS,
    TF_WEAPON_FLAREGUN,
    TF_WEAPON_LUNCHBOX,
    TF_WEAPON_JAR,
    TF_WEAPON_COMPOUND_BOW,
    TF_WEAPON_BUFF_ITEM,
    TF_WEAPON_PUMPKIN_BOMB,
    TF_WEAPON_SWORD,
    TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT,
    TF_WEAPON_LIFELINE,
    TF_WEAPON_LASER_POINTER,
    TF_WEAPON_DISPENSER_GUN,
    TF_WEAPON_SENTRY_REVENGE,
    TF_WEAPON_JAR_MILK,
    TF_WEAPON_HANDGUN_SCOUT_PRIMARY,
    TF_WEAPON_BAT_FISH,
    TF_WEAPON_CROSSBOW,
    TF_WEAPON_STICKBOMB,
    TF_WEAPON_HANDGUN_SCOUT_SECONDARY,
    TF_WEAPON_SODA_POPPER,
    TF_WEAPON_SNIPERRIFLE_DECAP,
    TF_WEAPON_RAYGUN,
    TF_WEAPON_PARTICLE_CANNON,
    TF_WEAPON_MECHANICAL_ARM,
    TF_WEAPON_DRG_POMSON,
    TF_WEAPON_BAT_GIFTWRAP,
    TF_WEAPON_GRENADE_ORNAMENT_BALL,
    TF_WEAPON_FLAREGUN_REVENGE,
    TF_WEAPON_PEP_BRAWLER_BLASTER,
    TF_WEAPON_CLEAVER,
    TF_WEAPON_GRENADE_CLEAVER,
    TF_WEAPON_STICKY_BALL_LAUNCHER,
    TF_WEAPON_GRENADE_STICKY_BALL,
    TF_WEAPON_SHOTGUN_BUILDING_RESCUE,
    TF_WEAPON_CANNON,
    TF_WEAPON_THROWABLE,
    TF_WEAPON_GRENADE_THROWABLE,
    TF_WEAPON_PDA_SPY_BUILD,
    TF_WEAPON_GRENADE_WATERBALLOON,
    TF_WEAPON_HARVESTER_SAW,
    TF_WEAPON_SPELLBOOK,
    TF_WEAPON_SPELLBOOK_PROJECTILE,
    TF_WEAPON_SNIPERRIFLE_CLASSIC,
    TF_WEAPON_PARACHUTE,
    TF_WEAPON_GRAPPLINGHOOK,
    TF_WEAPON_PASSTIME_GUN,
    TF_WEAPON_CHARGED_SMG,
    //
    // ADD NEW WEAPONS HERE TO AVOID BREAKING DEMOS
    //
    TF_WEAPON_COUNT
};

enum struct e_class_ids : int {
    CAI_BaseNPC,
    CBaseAnimating,
    CBaseAnimatingOverlay,
    CBaseAttributableItem,
    CBaseCombatCharacter,
    CBaseCombatWeapon,
    CBaseDoor,
    CBaseEntity,
    CBaseFlex,
    CBaseGrenade,
    CBaseObject,
    CBaseObjectUpgrade,
    CBaseParticleEntity,
    CBasePlayer,
    CBaseProjectile,
    CBasePropDoor,
    CBaseTeamObjectiveResource,
    CBaseTempEntity,
    CBaseViewModel,
    CBeam,
    CBoneFollower,
    CBonusDuckPickup,
    CBonusPack,
    CBonusRoundLogic,
    CBreakableProp,
    CBreakableSurface,
    CCaptureFlag,
    CCaptureFlagReturnIcon,
    CCaptureZone,
    CColorCorrection,
    CColorCorrectionVolume,
    CCurrencyPack,
    CDynamicLight,
    CDynamicProp,
    CEconEntity,
    CEconWearable,
    CEmbers,
    CEntityDissolve,
    CEntityFlame,
    CEntityParticleTrail,
    CEnvDetailController,
    CEnvParticleScript,
    CEnvProjectedTexture,
    CEnvQuadraticBeam,
    CEnvScreenEffect,
    CEnvScreenOverlay,
    CEnvTonemapController,
    CEnvWind,
    CEyeballBoss,
    CFireSmoke,
    CFireTrail,
    CFish,
    CFogController,
    CFunc_Dust,
    CFunc_LOD,
    CFuncAreaPortalWindow,
    CFuncConveyor,
    CFuncForceField,
    CFuncLadder,
    CFuncMonitor,
    CFuncOccluder,
    CFuncPasstimeGoal,
    CFuncReflectiveGlass,
    CFuncRespawnRoom,
    CFuncRespawnRoomVisualizer,
    CFuncRotating,
    CFuncSmokeVolume,
    CFuncTrackTrain,
    CGameRulesProxy,
    CHalloweenGiftPickup,
    CHalloweenPickup,
    CHalloweenSoulPack,
    CHandleTest,
    CHeadlessHatman,
    CHightower_TeleportVortex,
    CInfoLadderDismount,
    CInfoLightingRelative,
    CInfoOverlayAccessor,
    CLaserDot,
    CLightGlow,
    CMannVsMachineStats,
    CMaterialModifyControl,
    CMerasmus,
    CMerasmusDancer,
    CMonsterResource,
    CObjectCartDispenser,
    CObjectDispenser,
    CObjectSapper,
    CObjectSentrygun,
    CObjectTeleporter,
    CParticleFire,
    CParticlePerformanceMonitor,
    CParticleSystem,
    CPasstimeBall,
    CPasstimeGun,
    CPhysBox,
    CPhysBoxMultiplayer,
    CPhysicsProp,
    CPhysicsPropMultiplayer,
    CPhysMagnet,
    CPlasma,
    CPlayerDestructionDispenser,
    CPlayerResource,
    CPointCamera,
    CPointCommentaryNode,
    CPointWorldText,
    CPoseController,
    CPrecipitation,
    CPropVehicleDriveable,
    CRagdollManager,
    CRagdollProp,
    CRagdollPropAttached,
    CRobotDispenser,
    CRopeKeyframe,
    CSceneEntity,
    CShadowControl,
    CSlideshowDisplay,
    CSmokeStack,
    CSniperDot,
    CSpotlightEnd,
    CSprite,
    CSpriteOriented,
    CSpriteTrail,
    CSteamJet,
    CSun,
    CTeam,
    CTeamplayRoundBasedRulesProxy,
    CTeamRoundTimer,
    CTeamTrainWatcher,
    CTEArmorRicochet,
    CTEBaseBeam,
    CTEBeamEntPoint,
    CTEBeamEnts,
    CTEBeamFollow,
    CTEBeamLaser,
    CTEBeamPoints,
    CTEBeamRing,
    CTEBeamRingPoint,
    CTEBeamSpline,
    CTEBloodSprite,
    CTEBloodStream,
    CTEBreakModel,
    CTEBSPDecal,
    CTEBubbles,
    CTEBubbleTrail,
    CTEClientProjectile,
    CTEDecal,
    CTEDust,
    CTEDynamicLight,
    CTEEffectDispatch,
    CTEEnergySplash,
    CTEExplosion,
    CTEFireBullets,
    CTEFizz,
    CTEFootprintDecal,
    CTEGaussExplosion,
    CTEGlowSprite,
    CTEImpact,
    CTEKillPlayerAttachments,
    CTELargeFunnel,
    CTeleportVortex,
    CTEMetalSparks,
    CTEMuzzleFlash,
    CTEParticleSystem,
    CTEPhysicsProp,
    CTEPlayerAnimEvent,
    CTEPlayerDecal,
    CTEProjectedDecal,
    CTEShatterSurface,
    CTEShowLine,
    CTesla,
    CTESmoke,
    CTESparks,
    CTESprite,
    CTESpriteSpray,
    CTest_ProxyToggle_Networkable,
    CTestTraceline,
    CTETFBlood,
    CTETFExplosion,
    CTETFParticleEffect,
    CTEWorldDecal,
    CTFAmmoPack,
    CTFBall_Ornament,
    CTFBaseBoss,
    CTFBaseProjectile,
    CTFBaseRocket,
    CTFBat,
    CTFBat_Fish,
    CTFBat_Giftwrap,
    CTFBat_Wood,
    CTFBonesaw,
    CTFBotHintEngineerNest,
    CTFBottle,
    CTFBreakableMelee,
    CTFBreakableSign,
    CTFBuffItem,
    CTFCannon,
    CTFChargedSMG,
    CTFCleaver,
    CTFClub,
    CTFCompoundBow,
    CTFCrossbow,
    CTFDRGPomson,
    CTFDroppedWeapon,
    CTFFireAxe,
    CTFFists,
    CTFFlameManager,
    CTFFlameRocket,
    CTFFlameThrower,
    CTFFlareGun,
    CTFFlareGun_Revenge,
    CTFGameRulesProxy,
    CTFGasManager,
    CTFGenericBomb,
    CTFGlow,
    CTFGrapplingHook,
    CTFGrenadeLauncher,
    CTFGrenadePipebombProjectile,
    CTFHalloweenMinigame,
    CTFHalloweenMinigame_FallingPlatforms,
    CTFHellZap,
    CTFItem,
    CTFJar,
    CTFJarGas,
    CTFJarMilk,
    CTFKatana,
    CTFKnife,
    CTFLaserPointer,
    CTFLunchBox,
    CTFLunchBox_Drink,
    CTFMechanicalArm,
    CTFMedigunShield,
    CTFMiniGame,
    CTFMinigameLogic,
    CTFMinigun,
    CTFObjectiveResource,
    CTFParachute,
    CTFParachute_Primary,
    CTFParachute_Secondary,
    CTFParticleCannon,
    CTFPasstimeLogic,
    CTFPEPBrawlerBlaster,
    CTFPipebombLauncher,
    CTFPistol,
    CTFPistol_Scout,
    CTFPistol_ScoutPrimary,
    CTFPistol_ScoutSecondary,
    CTFPlayer,
    CTFPlayerDestructionLogic,
    CTFPlayerResource,
    CTFPointManager,
    CTFPowerupBottle,
    CTFProjectile_Arrow,
    CTFProjectile_BallOfFire,
    CTFProjectile_Cleaver,
    CTFProjectile_EnergyBall,
    CTFProjectile_EnergyRing,
    CTFProjectile_Flare,
    CTFProjectile_GrapplingHook,
    CTFProjectile_HealingBolt,
    CTFProjectile_Jar,
    CTFProjectile_JarGas,
    CTFProjectile_JarMilk,
    CTFProjectile_MechanicalArmOrb,
    CTFProjectile_Rocket,
    CTFProjectile_SentryRocket,
    CTFProjectile_SpellBats,
    CTFProjectile_SpellFireball,
    CTFProjectile_SpellKartBats,
    CTFProjectile_SpellKartOrb,
    CTFProjectile_SpellLightningOrb,
    CTFProjectile_SpellMeteorShower,
    CTFProjectile_SpellMirv,
    CTFProjectile_SpellPumpkin,
    CTFProjectile_SpellSpawnBoss,
    CTFProjectile_SpellSpawnHorde,
    CTFProjectile_SpellSpawnZombie,
    CTFProjectile_SpellTransposeTeleport,
    CTFProjectile_Throwable,
    CTFProjectile_ThrowableBreadMonster,
    CTFProjectile_ThrowableBrick,
    CTFProjectile_ThrowableRepel,
    CTFPumpkinBomb,
    CTFRagdoll,
    CTFRaygun,
    CTFReviveMarker,
    CTFRevolver,
    CTFRobotArm,
    CTFRobotDestruction_Robot,
    CTFRobotDestruction_RobotGroup,
    CTFRobotDestruction_RobotSpawn,
    CTFRobotDestructionLogic,
    CTFRocketLauncher,
    CTFRocketLauncher_AirStrike,
    CTFRocketLauncher_DirectHit,
    CTFRocketLauncher_Mortar,
    CTFRocketPack,
    CTFScatterGun,
    CTFShotgun,
    CTFShotgun_HWG,
    CTFShotgun_Pyro,
    CTFShotgun_Revenge,
    CTFShotgun_Soldier,
    CTFShotgunBuildingRescue,
    CTFShovel,
    CTFSlap,
    CTFSMG,
    CTFSniperRifle,
    CTFSniperRifleClassic,
    CTFSniperRifleDecap,
    CTFSodaPopper,
    CTFSpellBook,
    CTFStickBomb,
    CTFStunBall,
    CTFSword,
    CTFSyringeGun,
    CTFTankBoss,
    CTFTauntProp,
    CTFTeam,
    CTFThrowable,
    CTFViewModel,
    CTFWeaponBase,
    CTFWeaponBaseGrenadeProj,
    CTFWeaponBaseGun,
    CTFWeaponBaseMelee,
    CTFWeaponBaseMerasmusGrenade,
    CTFWeaponBuilder,
    CTFWeaponFlameBall,
    CTFWeaponInvis,
    CTFWeaponPDA,
    CTFWeaponPDA_Engineer_Build,
    CTFWeaponPDA_Engineer_Destroy,
    CTFWeaponPDA_Spy,
    CTFWeaponPDAExpansion_Dispenser,
    CTFWeaponPDAExpansion_Teleporter,
    CTFWeaponSapper,
    CTFWearable,
    CTFWearableCampaignItem,
    CTFWearableDemoShield,
    CTFWearableItem,
    CTFWearableLevelableItem,
    CTFWearableRazorback,
    CTFWearableRobotArm,
    CTFWearableVM,
    CTFWrench,
    CVGuiScreen,
    CVoteController,
    CWaterBullet,
    CWaterLODControl,
    CWeaponIFMBase,
    CWeaponIFMBaseCamera,
    CWeaponIFMSteadyCam,
    CWeaponMedigun,
    CWorld,
    CZombie,
    DustTrail,
    MovieExplosion,
    NextBotCombatCharacter,
    ParticleSmokeGrenade,
    RocketTrail,
    SmokeTrail,
    SporeExplosion,
    SporeTrail,
};

#define DISPENSER_MAX_METAL_AMMO      400
#define MAX_DISPENSER_HEALING_TARGETS 32
#define MINI_DISPENSER_MAX_METAL      200

enum {
    OBS_MODE_NONE = 0,  // not in spectator mode
    OBS_MODE_DEATHCAM,  // special mode for death cam animation
    OBS_MODE_FREEZECAM, // zooms to a target, and freeze-frames on them
    OBS_MODE_FIXED,     // view from a fixed camera position
    OBS_MODE_IN_EYE,    // follow a player in first person view
    OBS_MODE_CHASE,     // follow a player in third person view
    OBS_MODE_POI, // PASSTIME point of interest - game objective, big fight, anything interesting; added in the
    // middle of the enum due to tons of hard-coded "<ROAMING" enum compares
    OBS_MODE_ROAMING, // free roaming

    NUM_OBSERVER_MODES
};

enum ETFConds {
    TFCond_Slowed = (1 << 0),      // Toggled when a player is slowed down.
    TFCond_Zoomed = (1 << 1),      // Toggled when a player is zoomed.
    TFCond_Disguising = (1 << 2),  // Toggled when a Spy is disguising.
    TFCond_Disguised = (1 << 3),   // Toggled when a Spy is disguised.
    TFCond_Cloaked = (1 << 4),     // Toggled when a Spy is invisible.
    TFCond_Ubercharged = (1 << 5), // Toggled when a player is UberCharged.
    TFCond_TeleportedGlow =
    (1 << 6),               // Toggled when someone leaves a teleporter and has glow beneath their feet.
    TFCond_Taunting = (1 << 7), // Toggled when a player is taunting.
    TFCond_UberchargeFading = (1 << 8), // Toggled when the UberCharge is fading.
    TFCond_CloakFlicker = (1 << 9),     // Toggled when a Spy is visible during cloak.
    TFCond_Teleporting =
    (1 << 10), // Only activates for a brief second when the player is being teleported; not very useful.
    TFCond_Kritzkrieged = (1 << 11),   // Toggled when a player is being crit buffed by the KritzKrieg.
    TFCond_TmpDamageBonus = (1 << 12), // Unknown what this is for. Name taken from the AlliedModders SDK.
    TFCond_DeadRingered = (1 << 13),   // Toggled when a player is taking reduced damage from the Deadringer.
    TFCond_Bonked = (1 << 14),         // Toggled when a player is under the effects of The Bonk! Atomic Punch.
    TFCond_Stunned = (1 << 15),  // Toggled when a player's speed is reduced from airblast or a Sandman ball.
    TFCond_Buffed = (1 << 16),   // Toggled when a player is within range of an activated Buff Banner.
    TFCond_Charging = (1 << 17), // Toggled when a Demoman charges with the shield.
    TFCond_DemoBuff = (1 << 18), // Toggled when a Demoman has heads from the Eyelander.
    TFCond_CritCola = (1 << 19), // Toggled when the player is under the effect of The Crit-a-Cola.
    TFCond_InHealRadius = (1 << 20), // Unused condition, name taken from AlliedModders SDK.
    TFCond_Healing = (1 << 21),      // Toggled when someone is being healed by a medic or a dispenser.
    TFCond_OnFire = (1 << 22),       // Toggled when a player is on fire.
    TFCond_Overhealed = (1 << 23),   // Toggled when a player has >100% health.
    TFCond_Jarated = (1 << 24),      // Toggled when a player is hit with a Sniper's Jarate.
    TFCond_Bleeding = (1 << 25),     // Toggled when a player is taking bleeding damage.
    TFCond_DefenseBuffed =
    (1 << 26),                  // Toggled when a player is within range of an activated Battalion's Backup.
    TFCond_Milked = (1 << 27),      // Player was hit with a jar of Mad Milk.
    TFCond_MegaHeal = (1 << 28),    // Player is under the effect of Quick-Fix charge.
    TFCond_RegenBuffed = (1 << 29), // Toggled when a player is within a Concheror's range.
    TFCond_MarkedForDeath =
    (1 << 30), // Player is marked for death by a Fan O'War hit. Effects are similar to TFCond_Jarated.
    TFCond_NoHealingDamageBuff = (1 << 31),      // Unknown what this is used for.
    TFCondEx_SpeedBuffAlly = (1 << 0),           // Toggled when a player gets hit with the disciplinary action.
    TFCondEx_HalloweenCritCandy = (1 << 1),      // Only for Scream Fortress event maps that drop crit candy.
    TFCondEx_CritCanteen = (1 << 2),             // Player is getting a crit boost from a MVM canteen.
    TFCondEx_CritDemoCharge = (1 << 3),          // From demo's shield
    TFCondEx_CritHype = (1 << 4),                // Soda Popper crits.
    TFCondEx_CritOnFirstBlood = (1 << 5),        // Arena first blood crit buff.
    TFCondEx_CritOnWin = (1 << 6),               // End of round crits.
    TFCondEx_CritOnFlagCapture = (1 << 7),       // CTF intelligence capture crits.
    TFCondEx_CritOnKill = (1 << 8),              // Unknown what this is for.
    TFCondEx_RestrictToMelee = (1 << 9),         // Unknown what this is for.
    TFCondEx_DefenseBuffNoCritBlock = (1 << 10), // MvM Buff.
    TFCondEx_Reprogrammed = (1 << 11),           // MvM Bot has been reprogrammed.
    TFCondEx_PyroCrits = (1 << 12),              // Player is getting crits from the Mmmph charge.
    TFCondEx_PyroHeal = (1 << 13),               // Player is being healed from the Mmmph charge.
    TFCondEx_FocusBuff = (1 << 14),              // Player is getting a focus buff.
    TFCondEx_DisguisedRemoved = (1 << 15),       // Disguised remove from a bot.
    TFCondEx_MarkedForDeathSilent =
    (1 << 16), // Player is under the effects of the Escape Plan/Equalizer or GRU.
    TFCondEx_DisguisedAsDispenser = (1 << 17), // Bot is disguised as dispenser.
    TFCondEx_InKart = (1 << 18),               // MvM bot is being sapped.
    TFCondEx_UberchargedHidden = (1 << 19),    // MvM Related
    TFCondEx_UberchargedCanteen = (1 << 20),   // Player is receiving UberCharge from a canteen.
    TFCondEx_HalloweenBombHead = (1 << 21),    // Player has a bomb on their head from Merasmus.
    TFCondEx_HalloweenThriller = (1 << 22),    // Players are forced to dance from Merasmus.
    TFCondEx_BulletCharge = (1 << 26),         // Player is receiving 75% reduced damage from bullets.
    TFCondEx_ExplosiveCharge = (1 << 27),      // Player is receiving 75% reduced damage from explosives.
    TFCondEx_FireCharge = (1 << 28),           // Player is receiving 75% reduced damage from fire.
    TFCondEx_BulletResistance = (1 << 29),     // Player is receiving 10% reduced damage from bullets.
    TFCondEx_ExplosiveResistance = (1 << 30),  // Player is receiving 10% reduced damage from explosives.
    TFCondEx_FireResistance = (1 << 31),       // Player is receiving 10% reduced damage from fire.
    TFCondEx2_Stealthed = (1 << 0),
    TFCondEx2_MedigunDebuff = (1 << 1),
    TFCondEx2_StealthedUserBuffFade = (1 << 2),
    TFCondEx2_BulletImmune = (1 << 3),
    TFCondEx2_BlastImmune = (1 << 4),
    TFCondEx2_FireImmune = (1 << 5),
    TFCondEx2_PreventDeath = (1 << 6),
    TFCondEx2_MVMBotRadiowave = (1 << 7),
    TFCondEx2_HalloweenSpeedBoost = (1 << 8), // Wheel has granted player speed boost.
    TFCondEx2_HalloweenQuickHeal = (1 << 9),  // Wheel has granted player quick heal.
    TFCondEx2_HalloweenGiant = (1 << 10),     // Wheel has granted player giant mode.
    TFCondEx2_HalloweenTiny = (1 << 11),      // Wheel has granted player tiny mode.
    TFCondEx2_HalloweenInHell = (1 << 12),    // Wheel has granted player in hell mode.
    TFCondEx2_HalloweenGhostMode = (1 << 13), // Wheel has granted player ghost mode.
    TFCondEx2_Parachute = (1 << 16),          // Player has deployed the BASE Jumper.
    TFCondEx2_BlastJumping = (1 << 17),       // Player has sticky or rocket jumped.
    TFCond_MiniCrits = (TFCond_Buffed | TFCond_CritCola),
    TFCond_IgnoreStates = (TFCond_Ubercharged | TFCond_Bonked),
    TFCondEx_IgnoreStates = (TFCondEx_PyroHeal)
};

enum ETFCond {
    TF_COND_INVALID = -1,
    TF_COND_AIMING,
    TF_COND_ZOOMED,
    TF_COND_DISGUISING,
    TF_COND_DISGUISED,
    TF_COND_STEALTHED,
    TF_COND_INVULNERABLE,
    TF_COND_TELEPORTED,
    TF_COND_TAUNTING,
    TF_COND_INVULNERABLE_WEARINGOFF,
    TF_COND_STEALTHED_BLINK,
    TF_COND_SELECTED_TO_TELEPORT,
    TF_COND_CRITBOOSTED,
    TF_COND_TMPDAMAGEBONUS,
    TF_COND_FEIGN_DEATH,
    TF_COND_PHASE,
    TF_COND_STUNNED,
    TF_COND_OFFENSEBUFF,
    TF_COND_SHIELD_CHARGE,
    TF_COND_DEMO_BUFF,
    TF_COND_ENERGY_BUFF,
    TF_COND_RADIUSHEAL,
    TF_COND_HEALTH_BUFF,
    TF_COND_BURNING,
    TF_COND_HEALTH_OVERHEALED,
    TF_COND_URINE,
    TF_COND_BLEEDING,
    TF_COND_DEFENSEBUFF,
    TF_COND_MAD_MILK,
    TF_COND_MEGAHEAL,
    TF_COND_REGENONDAMAGEBUFF,
    TF_COND_MARKEDFORDEATH,
    TF_COND_NOHEALINGDAMAGEBUFF,
    TF_COND_SPEED_BOOST,
    TF_COND_CRITBOOSTED_PUMPKIN,
    TF_COND_CRITBOOSTED_USER_BUFF,
    TF_COND_CRITBOOSTED_DEMO_CHARGE,
    TF_COND_SODAPOPPER_HYPE,
    TF_COND_CRITBOOSTED_FIRST_BLOOD,
    TF_COND_CRITBOOSTED_BONUS_TIME,
    TF_COND_CRITBOOSTED_CTF_CAPTURE,
    TF_COND_CRITBOOSTED_ON_KILL,
    TF_COND_CANNOT_SWITCH_FROM_MELEE,
    TF_COND_DEFENSEBUFF_NO_CRIT_BLOCK,
    TF_COND_REPROGRAMMED,
    TF_COND_CRITBOOSTED_RAGE_BUFF,
    TF_COND_DEFENSEBUFF_HIGH,
    TF_COND_SNIPERCHARGE_RAGE_BUFF,
    TF_COND_DISGUISE_WEARINGOFF,
    TF_COND_MARKEDFORDEATH_SILENT,
    TF_COND_DISGUISED_AS_DISPENSER,
    TF_COND_SAPPED,
    TF_COND_INVULNERABLE_HIDE_UNLESS_DAMAGE,
    TF_COND_INVULNERABLE_USER_BUFF,
    TF_COND_HALLOWEEN_BOMB_HEAD,
    TF_COND_HALLOWEEN_THRILLER,
    TF_COND_RADIUSHEAL_ON_DAMAGE,
    TF_COND_CRITBOOSTED_CARD_EFFECT,
    TF_COND_INVULNERABLE_CARD_EFFECT,
    TF_COND_MEDIGUN_UBER_BULLET_RESIST,
    TF_COND_MEDIGUN_UBER_BLAST_RESIST,
    TF_COND_MEDIGUN_UBER_FIRE_RESIST,
    TF_COND_MEDIGUN_SMALL_BULLET_RESIST,
    TF_COND_MEDIGUN_SMALL_BLAST_RESIST,
    TF_COND_MEDIGUN_SMALL_FIRE_RESIST,
    TF_COND_STEALTHED_USER_BUFF,
    TF_COND_MEDIGUN_DEBUFF,
    TF_COND_STEALTHED_USER_BUFF_FADING,
    TF_COND_BULLET_IMMUNE,
    TF_COND_BLAST_IMMUNE,
    TF_COND_FIRE_IMMUNE,
    TF_COND_PREVENT_DEATH,
    TF_COND_MVM_BOT_STUN_RADIOWAVE,
    TF_COND_HALLOWEEN_SPEED_BOOST,
    TF_COND_HALLOWEEN_QUICK_HEAL,
    TF_COND_HALLOWEEN_GIANT,
    TF_COND_HALLOWEEN_TINY,
    TF_COND_HALLOWEEN_IN_HELL,
    TF_COND_HALLOWEEN_GHOST_MODE,
    TF_COND_MINICRITBOOSTED_ON_KILL,
    TF_COND_OBSCURED_SMOKE,
    TF_COND_PARACHUTE_ACTIVE,
    TF_COND_BLASTJUMPING,
    TF_COND_HALLOWEEN_KART,
    TF_COND_HALLOWEEN_KART_DASH,
    TF_COND_BALLOON_HEAD,
    TF_COND_MELEE_ONLY,
    TF_COND_SWIMMING_CURSE,
    TF_COND_FREEZE_INPUT,
    TF_COND_HALLOWEEN_KART_CAGE,
    TF_COND_DONOTUSE_0,
    TF_COND_RUNE_STRENGTH,
    TF_COND_RUNE_HASTE,
    TF_COND_RUNE_REGEN,
    TF_COND_RUNE_RESIST,
    TF_COND_RUNE_VAMPIRE,
    TF_COND_RUNE_REFLECT,
    TF_COND_RUNE_PRECISION,
    TF_COND_RUNE_AGILITY,
    TF_COND_GRAPPLINGHOOK,
    TF_COND_GRAPPLINGHOOK_SAFEFALL,
    TF_COND_GRAPPLINGHOOK_LATCHED,
    TF_COND_GRAPPLINGHOOK_BLEEDING,
    TF_COND_AFTERBURN_IMMUNE,
    TF_COND_RUNE_KNOCKOUT,
    TF_COND_RUNE_IMBALANCE,
    TF_COND_CRITBOOSTED_RUNE_TEMP,
    TF_COND_PASSTIME_INTERCEPTION,
    TF_COND_SWIMMING_NO_EFFECTS,
    TF_COND_PURGATORY,
    TF_COND_RUNE_KING,
    TF_COND_RUNE_PLAGUE,
    TF_COND_RUNE_SUPERNOVA,
    TF_COND_PLAGUE,
    TF_COND_KING_BUFFED,
    TF_COND_TEAM_GLOWS,
    TF_COND_KNOCKED_INTO_AIR,
    TF_COND_COMPETITIVE_WINNER,
    TF_COND_COMPETITIVE_LOSER,
    TF_COND_HEALING_DEBUFF,
    TF_COND_PASSTIME_PENALTY_DEBUFF,
    TF_COND_GRAPPLED_TO_PLAYER,
    TF_COND_GRAPPLED_BY_PLAYER,
    TF_COND_PARACHUTE_DEPLOYED,
    TF_COND_GAS,
    TF_COND_BURNING_PYRO,
    TF_COND_ROCKETPACK,
    TF_COND_LOST_FOOTING,
    TF_COND_AIR_CURRENT,
    TF_COND_HALLOWEEN_HELL_HEAL,
    TF_COND_POWERUPMODE_DOMINANT
};

// contents flags are seperate bits
// a given brush can contribute multiple content bits
// multiple brushes can be in a single leaf

// these definitions also need to be in q_shared.h!

// lower bits are stronger, and will eat weaker brushes completely
#define CONTENTS_EMPTY                0 // No contents

#define CONTENTS_SOLID                0x1 // an eye is never valid in a solid
#define CONTENTS_WINDOW               0x2 // translucent, but not watery (glass)
#define CONTENTS_AUX                  0x4
#define CONTENTS_GRATE                0x8 // alpha-tested "grate" textures.  Bullets/sight pass through, but solids don't
#define CONTENTS_SLIME                0x10
#define CONTENTS_WATER                0x20
#define CONTENTS_BLOCKLOS             0x40 // block AI line of sight
#define CONTENTS_OPAQUE               0x80 // things that cannot be seen through (may be non-solid though)
#define LAST_VISIBLE_CONTENTS         0x80

#define ALL_VISIBLE_CONTENTS          (LAST_VISIBLE_CONTENTS | (LAST_VISIBLE_CONTENTS - 1))

#define CONTENTS_TESTFOGVOLUME        0x100
#define CONTENTS_UNUSED               0x200

// unused
// NOTE: If it's visible, grab from the top + update LAST_VISIBLE_CONTENTS
// if not visible, then grab from the bottom.
#define CONTENTS_UNUSED6              0x400

#define CONTENTS_TEAM1                0x800  // per team contents used to differentiate collisions
#define CONTENTS_TEAM2                0x1000 // between players and objects on different teams

// ignore CONTENTS_OPAQUE on surfaces that have SURF_NODRAW
#define CONTENTS_IGNORE_NODRAW_OPAQUE 0x2000

// hits entities which are MOVETYPE_PUSH (doors, plats, etc.)
#define CONTENTS_MOVEABLE             0x4000

// remaining contents are non-visible, and don't eat brushes
#define CONTENTS_AREAPORTAL           0x8000

#define CONTENTS_PLAYERCLIP           0x10000
#define CONTENTS_MONSTERCLIP          0x20000

// currents can be added to any other contents, and may be mixed
#define CONTENTS_CURRENT_0            0x40000
#define CONTENTS_CURRENT_90           0x80000
#define CONTENTS_CURRENT_180          0x100000
#define CONTENTS_CURRENT_270          0x200000
#define CONTENTS_CURRENT_UP           0x400000
#define CONTENTS_CURRENT_DOWN         0x800000

#define CONTENTS_ORIGIN               0x1000000 // removed before bsping an entity

#define CONTENTS_MONSTER              0x2000000 // should never be on a brush, only in game
#define CONTENTS_DEBRIS               0x4000000
#define CONTENTS_DETAIL               0x8000000  // brushes to be added after vis leafs
#define CONTENTS_TRANSLUCENT          0x10000000 // auto set if any surface has trans
#define CONTENTS_LADDER               0x20000000
#define CONTENTS_HITBOX               0x40000000 // use accurate hitboxes on trace

// -----------------------------------------------------
// spatial content masks - used for spatial queries (traceline,etc.)
// -----------------------------------------------------
#define MASK_ALL                      (0xFFFFFFFF)
// everything that is normally solid
#define MASK_SOLID                    (CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_WINDOW | CONTENTS_MONSTER | CONTENTS_GRATE)
// everything that blocks player movement
#define MASK_PLAYERSOLID                                                                           \
  (CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_PLAYERCLIP | CONTENTS_WINDOW | CONTENTS_MONSTER | \
   CONTENTS_GRATE)
// blocks npc movement
#define MASK_NPCSOLID                                                                               \
  (CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_MONSTERCLIP | CONTENTS_WINDOW | CONTENTS_MONSTER | \
   CONTENTS_GRATE)
// water physics in these contents
#define MASK_WATER             (CONTENTS_WATER | CONTENTS_MOVEABLE | CONTENTS_SLIME)
// everything that blocks lighting
#define MASK_OPAQUE            (CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_OPAQUE)
// everything that blocks lighting, but with monsters added.
#define MASK_OPAQUE_AND_NPCS   (MASK_OPAQUE | CONTENTS_MONSTER)
// everything that blocks line of sight for AI
#define MASK_BLOCKLOS          (CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_BLOCKLOS)
// everything that blocks line of sight for AI plus NPCs
#define MASK_BLOCKLOS_AND_NPCS (MASK_BLOCKLOS | CONTENTS_MONSTER)
// everything that blocks line of sight for players
#define MASK_VISIBLE           (MASK_OPAQUE | CONTENTS_IGNORE_NODRAW_OPAQUE)
// everything that blocks line of sight for players, but with monsters added.
#define MASK_VISIBLE_AND_NPCS  (MASK_OPAQUE_AND_NPCS | CONTENTS_IGNORE_NODRAW_OPAQUE)
// bullets see these as solid
#define MASK_SHOT                                                                              \
  (CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_MONSTER | CONTENTS_WINDOW | CONTENTS_DEBRIS | \
   CONTENTS_HITBOX)
// non-raycasted weapons see this as solid (includes grates)
#define MASK_SHOT_HULL \
  (CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_MONSTER | CONTENTS_WINDOW | CONTENTS_DEBRIS | CONTENTS_GRATE)
// hits solids (not grates) and passes through everything else
#define MASK_SHOT_PORTAL     (CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_WINDOW | CONTENTS_MONSTER)
// everything normally solid, except monsters (world+brush only)
#define MASK_SOLID_BRUSHONLY (CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_WINDOW | CONTENTS_GRATE)
// everything normally solid for player movement, except monsters (world+brush only)
#define MASK_PLAYERSOLID_BRUSHONLY \
  (CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_WINDOW | CONTENTS_PLAYERCLIP | CONTENTS_GRATE)
// everything normally solid for npc movement, except monsters (world+brush only)
#define MASK_NPCSOLID_BRUSHONLY \
  (CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_WINDOW | CONTENTS_MONSTERCLIP | CONTENTS_GRATE)
// just the world, used for route rebuilding
#define MASK_NPCWORLDSTATIC  (CONTENTS_SOLID | CONTENTS_WINDOW | CONTENTS_MONSTERCLIP | CONTENTS_GRATE)
// These are things that can split areaportals
#define MASK_SPLITAREAPORTAL (CONTENTS_WATER | CONTENTS_SLIME)

// UNDONE: This is untested, any moving water
#define MASK_CURRENT                                                                        \
  (CONTENTS_CURRENT_0 | CONTENTS_CURRENT_90 | CONTENTS_CURRENT_180 | CONTENTS_CURRENT_270 | \
   CONTENTS_CURRENT_UP | CONTENTS_CURRENT_DOWN)

// everything that blocks corpse movement
// UNDONE: Not used yet / may be deleted
#define MASK_DEADSOLID (CONTENTS_SOLID | CONTENTS_PLAYERCLIP | CONTENTS_WINDOW | CONTENTS_GRATE)

#include <cassert>

namespace why
{
    void* gameAlloc(size_t size);
    void* gameRealloc(void* mem, size_t size);
    void gameFree(void* mem);
}

//-----------------------------------------------------------------------------
// The CUtlMemory class:
// A growable memory class which doubles in size by default.
//-----------------------------------------------------------------------------
template <class T> class CUtlMemory {
public:
    // constructor, destructor
    CUtlMemory(int nGrowSize = 0, int nInitSize = 0);
    CUtlMemory(T* pMemory, int numElements);
    ~CUtlMemory();

    // element access
    T& operator[](int i);
    T const& operator[](int i) const;
    T& Element(int i);
    T const& Element(int i) const;

    // Can we use this index?
    bool     IsIdxValid(int i) const;

    // Gets the base address (can change when adding elements!)
    T* Base();
    T const* Base() const;

    // Attaches the buffer to external memory....
    void     SetExternalBuffer(T* pMemory, int numElements);

    // Size
    int      NumAllocated() const;
    int      Count() const;

    // Grows the memory, so that at least allocated + num elements are allocated
    void     Grow(int num = 1);

    // Memory deallocation
    void     Purge();

    // is the memory externally allocated?
    bool     IsExternallyAllocated() const;

    // Set the size by which the memory grows
    void     SetGrowSize(int size);

    enum {
        EXTERNAL_BUFFER_MARKER = -1,
    };

    T* m_pMemory;
    int m_nAllocationCount;
    int m_nGrowSize;
};

//-----------------------------------------------------------------------------
// constructor, destructor
//-----------------------------------------------------------------------------
template< class T >
CUtlMemory<T>::CUtlMemory(int nGrowSize, int nInitAllocationCount) : m_pMemory(0),
m_nAllocationCount(nInitAllocationCount), m_nGrowSize(nGrowSize)
{
    //Assert((nGrowSize >= 0) && (nGrowSize != EXTERNAL_BUFFER_MARKER));
    if (m_nAllocationCount)
    {
        m_pMemory = (T*)why::gameAlloc(m_nAllocationCount * sizeof(T));
    }
}

template< class T >
CUtlMemory<T>::CUtlMemory(T* pMemory, int numElements) : m_pMemory(pMemory),
m_nAllocationCount(numElements)
{
    // Special marker indicating externally supplied memory
    m_nGrowSize = EXTERNAL_BUFFER_MARKER;
}

template< class T >
CUtlMemory<T>::~CUtlMemory()
{
    Purge();
}


//-----------------------------------------------------------------------------
// Attaches the buffer to external memory....
//-----------------------------------------------------------------------------
template< class T >
void CUtlMemory<T>::SetExternalBuffer(T* pMemory, int numElements)
{
    // Blow away any existing allocated memory
    Purge();

    m_pMemory = pMemory;
    m_nAllocationCount = numElements;

    // Indicate that we don't own the memory
    m_nGrowSize = EXTERNAL_BUFFER_MARKER;
}


//-----------------------------------------------------------------------------
// element access
//-----------------------------------------------------------------------------
template< class T >
inline T& CUtlMemory<T>::operator[](int i)
{
    //Assert(IsIdxValid(i));
    return m_pMemory[i];
}

template< class T >
inline T const& CUtlMemory<T>::operator[](int i) const
{
    //Assert(IsIdxValid(i));
    return m_pMemory[i];
}

template< class T >
inline T& CUtlMemory<T>::Element(int i)
{
    //Assert(IsIdxValid(i));
    return m_pMemory[i];
}

template< class T >
inline T const& CUtlMemory<T>::Element(int i) const
{
    //Assert(IsIdxValid(i));
    return m_pMemory[i];
}


//-----------------------------------------------------------------------------
// is the memory externally allocated?
//-----------------------------------------------------------------------------
template< class T >
bool CUtlMemory<T>::IsExternallyAllocated() const
{
    return m_nGrowSize == EXTERNAL_BUFFER_MARKER;
}


template< class T >
void CUtlMemory<T>::SetGrowSize(int nSize)
{
    //Assert((nSize >= 0) && (nSize != EXTERNAL_BUFFER_MARKER));
    m_nGrowSize = nSize;
}


//-----------------------------------------------------------------------------
// Gets the base address (can change when adding elements!)
//-----------------------------------------------------------------------------
template< class T >
inline T* CUtlMemory<T>::Base()
{
    return m_pMemory;
}

template< class T >
inline T const* CUtlMemory<T>::Base() const
{
    return m_pMemory;
}


//-----------------------------------------------------------------------------
// Size
//-----------------------------------------------------------------------------
template< class T >
inline int CUtlMemory<T>::NumAllocated() const
{
    return m_nAllocationCount;
}

template< class T >
inline int CUtlMemory<T>::Count() const
{
    return m_nAllocationCount;
}


//-----------------------------------------------------------------------------
// Is element index valid?
//-----------------------------------------------------------------------------
template< class T >
inline bool CUtlMemory<T>::IsIdxValid(int i) const
{
    return (i >= 0) && (i < m_nAllocationCount);
}

#pragma warning (push)
#pragma warning (disable : 6308)

//-----------------------------------------------------------------------------
// Grows the memory
//-----------------------------------------------------------------------------
template< class T >
void CUtlMemory<T>::Grow(int num)
{
    assert(num > 0);

    if (IsExternallyAllocated())
    {
        // Can't grow a buffer whose memory was externally allocated
        assert(0);
        return;
    }

    // Make sure we have at least nuwhy::gameAllocated + num allocations.
    // Use the grow rules specified for this memory (in m_nGrowSize)
    int nAllocationRequested = m_nAllocationCount + num;
    while (m_nAllocationCount < nAllocationRequested)
    {
        if (m_nAllocationCount != 0)
        {
            if (m_nGrowSize)
            {
                m_nAllocationCount += m_nGrowSize;
            }
            else
            {
                m_nAllocationCount += m_nAllocationCount;
            }
        }
        else
        {
            // Compute an allocation which is at least as big as a cache line...
            m_nAllocationCount = (31 + sizeof(T)) / sizeof(T);
            assert(m_nAllocationCount != 0);
        }
    }

    if (m_pMemory)
    {
        m_pMemory = (T*)why::gameRealloc(m_pMemory, m_nAllocationCount * sizeof(T));
    }
    else
    {
        m_pMemory = (T*)why::gameAlloc(m_nAllocationCount * sizeof(T));
    }
}

#pragma warning (pop)

//-----------------------------------------------------------------------------
// Memory deallocation
//-----------------------------------------------------------------------------
template< class T >
void CUtlMemory<T>::Purge()
{
    if (!IsExternallyAllocated())
    {
        if (m_pMemory)
        {
            why::gameFree((void*)m_pMemory);
            m_pMemory = 0;
        }
        m_nAllocationCount = 0;
    }
}

template<class T>
class CUtlVector
{
public:
    typedef T ElemType_t;

    // constructor, destructor
    CUtlVector(int growSize = 0, int initSize = 0);
    CUtlVector(T* pMemory, int numElements);
    ~CUtlVector();

    // Copy the array.
    CUtlVector<T>& operator=(const CUtlVector<T>& other);

    // element access
    T& operator[](int i);
    T const& operator[](int i) const;
    T& Element(int i);
    T const& Element(int i) const;

    // Gets the base address (can change when adding elements!)
    T* Base();
    T const* Base() const;

    // Returns the number of elements in the vector
    // SIZE IS DEPRECATED!
    int Count() const;
    int Size() const;	// don't use me!

    // Is element index valid?
    bool IsValidIndex(int i) const;
    static int InvalidIndex(void);

    // Adds an element, uses default constructor
    int AddToHead();
    int AddToTail();
    int InsertBefore(int elem);
    int InsertAfter(int elem);

    // Adds an element, uses copy constructor
    int AddToHead(T const& src);
    int AddToTail(T const& src);
    int InsertBefore(int elem, T const& src);
    int InsertAfter(int elem, T const& src);

    // Adds multiple elements, uses default constructor
    int AddMultipleToHead(int num);
    int AddMultipleToTail(int num, const T* pToCopy = NULL);
    int InsertMultipleBefore(int elem, int num, const T* pToCopy = NULL);	// If pToCopy is set, then it's an array of length 'num' and
    int InsertMultipleAfter(int elem, int num);

    // Calls RemoveAll() then AddMultipleToTail.
    void SetSize(int size);
    void SetCount(int count);

    // Calls SetSize and copies each element.
    void CopyArray(T const* pArray, int size);

    // Add the specified array to the tail.
    int AddVectorToTail(CUtlVector<T> const& src);

    // Finds an element (element needs operator== defined)
    int Find(T const& src) const;

    bool HasElement(T const& src);

    // Makes sure we have at least this many elements
    void EnsureCount(int num);

    // Element removal
    void FastRemove(int elem);	// doesn't preserve order
    void Remove(int elem);		// preserves order, shifts elements
    void FindAndRemove(T const& src);	// removes first occurrence of src, preserves order, shifts elements
    bool FindAndFastRemove(const T& src);
    void RemoveMultiple(int elem, int num);	// preserves order, shifts elements
    void RemoveAll();				// doesn't deallocate memory

    // Memory deallocation
    void Purge();

    // Purges the list and calls delete on each element in it.
    void PurgeAndDeleteElements();

    // Set the size by which it grows when it needs to allocate more memory.
    void SetGrowSize(int size);

protected:
    // Can't copy this unless we explicitly do it!
    CUtlVector(CUtlVector const& vec)
    {
        assert(0);
    }

    // Grows the vector
    void GrowVector(int num = 1);

    // Shifts elements....
    void ShiftElementsRight(int elem, int num = 1);
    void ShiftElementsLeft(int elem, int num = 1);

    // For easier access to the elements through the debugger
    void ResetDbgInfo();

    CUtlMemory<T> m_Memory;
    int m_Size;

    // For easier access to the elements through the debugger
    // it's in release builds so this can be used in libraries correctly
    T* m_pElements;
};

//-----------------------------------------------------------------------------
// For easier access to the elements through the debugger
//-----------------------------------------------------------------------------

template< class T >
inline void CUtlVector<T>::ResetDbgInfo()
{
    m_pElements = m_Memory.Base();
}

//-----------------------------------------------------------------------------
// constructor, destructor
//-----------------------------------------------------------------------------

template< class T >
inline CUtlVector<T>::CUtlVector(int growSize, int initSize) :
    m_Memory(growSize, initSize), m_Size(0)
{
    ResetDbgInfo();
}

template< class T >
inline CUtlVector<T>::CUtlVector(T* pMemory, int numElements) :
    m_Memory(pMemory, numElements), m_Size(0)
{
    ResetDbgInfo();
}

template< class T >
inline CUtlVector<T>::~CUtlVector()
{
    Purge();
}

template<class T>
inline CUtlVector<T>& CUtlVector<T>::operator=(const CUtlVector<T>& other)
{
    CopyArray(other.Base(), other.Count());
    return *this;
}

//-----------------------------------------------------------------------------
// element access
//-----------------------------------------------------------------------------

template< class T >
inline T& CUtlVector<T>::operator[](int i)
{
    assert(IsValidIndex(i));
    return m_Memory[i];
}

template< class T >
inline T const& CUtlVector<T>::operator[](int i) const
{
    assert(IsValidIndex(i));
    return m_Memory[i];
}

template< class T >
inline T& CUtlVector<T>::Element(int i)
{
    assert(IsValidIndex(i));
    return m_Memory[i];
}

template< class T >
inline T const& CUtlVector<T>::Element(int i) const
{
    assert(IsValidIndex(i));
    return m_Memory[i];
}

//-----------------------------------------------------------------------------
// Gets the base address (can change when adding elements!)
//-----------------------------------------------------------------------------

template< class T >
inline T* CUtlVector<T>::Base()
{
    return m_Memory.Base();
}

template< class T >
inline T const* CUtlVector<T>::Base() const
{
    return m_Memory.Base();
}

//-----------------------------------------------------------------------------
// Count
//-----------------------------------------------------------------------------

template< class T >
inline int CUtlVector<T>::Size() const
{
    return m_Size;
}

template< class T >
inline int CUtlVector<T>::Count() const
{
    return m_Size;
}

//-----------------------------------------------------------------------------
// Is element index valid?
//-----------------------------------------------------------------------------

template< class T >
inline bool CUtlVector<T>::IsValidIndex(int i) const
{
    return (i >= 0) && (i < m_Size);
}

//-----------------------------------------------------------------------------
// Returns in invalid index
//-----------------------------------------------------------------------------
template< class T >
inline int CUtlVector<T>::InvalidIndex(void)
{
    return -1;
}

//-----------------------------------------------------------------------------
// Grows the vector
//-----------------------------------------------------------------------------
template< class T >
void CUtlVector<T>::GrowVector(int num)
{
    if (m_Size + num - 1 >= m_Memory.NumAllocated())
    {
        m_Memory.Grow(m_Size + num - m_Memory.NumAllocated());
    }

    m_Size += num;
    ResetDbgInfo();
}

//-----------------------------------------------------------------------------
// Makes sure we have at least this many elements
//-----------------------------------------------------------------------------
template< class T >
void CUtlVector<T>::EnsureCount(int num)
{
    if (Count() < num)
        AddMultipleToTail(num - Count());
}

//-----------------------------------------------------------------------------
// Shifts elements
//-----------------------------------------------------------------------------
template< class T >
void CUtlVector<T>::ShiftElementsRight(int elem, int num)
{
    assert(IsValidIndex(elem) || (m_Size == 0) || (num == 0));
    int numToMove = m_Size - elem - num;
    if ((numToMove > 0) && (num > 0))
        memmove(&Element(elem + num), &Element(elem), numToMove * sizeof(T));
}

template< class T >
void CUtlVector<T>::ShiftElementsLeft(int elem, int num)
{
    assert(IsValidIndex(elem) || (m_Size == 0) || (num == 0));
    int numToMove = m_Size - elem - num;
    if ((numToMove > 0) && (num > 0))
    {
        memmove(&Element(elem), &Element(elem + num), numToMove * sizeof(T));

#ifdef _DEBUG
        memset(&Element(m_Size - num), 0xDD, num * sizeof(T));
#endif
    }
}

//-----------------------------------------------------------------------------
// Adds an element, uses default constructor
//-----------------------------------------------------------------------------

template< class T >
inline int CUtlVector<T>::AddToHead()
{
    return InsertBefore(0);
}

template< class T >
inline int CUtlVector<T>::AddToTail()
{
    return InsertBefore(m_Size);
}

template< class T >
inline int CUtlVector<T>::InsertAfter(int elem)
{
    return InsertBefore(elem + 1);
}

template< class T >
int CUtlVector<T>::InsertBefore(int elem)
{
    // Can insert at the end
    assert((elem == Count()) || IsValidIndex(elem));

    GrowVector();
    ShiftElementsRight(elem);
    Construct(&Element(elem));
    return elem;
}

//-----------------------------------------------------------------------------
// Adds an element, uses copy constructor
//-----------------------------------------------------------------------------

template< class T >
inline int CUtlVector<T>::AddToHead(T const& src)
{
    return InsertBefore(0, src);
}

template< class T >
inline int CUtlVector<T>::AddToTail(T const& src)
{
    return InsertBefore(m_Size, src);
}

template< class T >
inline int CUtlVector<T>::InsertAfter(int elem, T const& src)
{
    return InsertBefore(elem + 1, src);
}

template< class T >
int CUtlVector<T>::InsertBefore(int elem, T const& src)
{
    // Can insert at the end
    assert((elem == Count()) || IsValidIndex(elem));

    GrowVector();
    ShiftElementsRight(elem);
    CopyConstruct(&Element(elem), src);
    return elem;
}


//-----------------------------------------------------------------------------
// Adds multiple elements, uses default constructor
//-----------------------------------------------------------------------------

template< class T >
inline int CUtlVector<T>::AddMultipleToHead(int num)
{
    return InsertMultipleBefore(0, num);
}

template< class T >
inline int CUtlVector<T>::AddMultipleToTail(int num, const T* pToCopy)
{
    return InsertMultipleBefore(m_Size, num, pToCopy);
}

template< class T >
int CUtlVector<T>::InsertMultipleAfter(int elem, int num)
{
    return InsertMultipleBefore(elem + 1, num);
}


template< class T >
void CUtlVector<T>::SetCount(int count)
{
    RemoveAll();
    AddMultipleToTail(count);
}

template< class T >
inline void CUtlVector<T>::SetSize(int size)
{
    SetCount(size);
}

template< class T >
void CUtlVector<T>::CopyArray(T const* pArray, int size)
{
    SetSize(size);
    for (int i = 0; i < size; i++)
        (*this)[i] = pArray[i];
}

template< class T >
int CUtlVector<T>::AddVectorToTail(CUtlVector const& src)
{
    int base = Count();

    // Make space.
    AddMultipleToTail(src.Count());

    // Copy the elements.
    for (int i = 0; i < src.Count(); i++)
        (*this)[base + i] = src[i];

    return base;
}

template< class T >
inline int CUtlVector<T>::InsertMultipleBefore(int elem, int num, const T* pToInsert)
{
    if (num == 0)
        return elem;

    // Can insert at the end
    assert((elem == Count()) || IsValidIndex(elem));

    GrowVector(num);
    ShiftElementsRight(elem, num);

    // Invoke default constructors
    for (int i = 0; i < num; ++i)
        Construct(&Element(elem + i));

    // Copy stuff in?
    if (pToInsert)
    {
        for (int i = 0; i < num; i++)
        {
            Element(elem + i) = pToInsert[i];
        }
    }

    return elem;
}

//-----------------------------------------------------------------------------
// Finds an element (element needs operator== defined)
//-----------------------------------------------------------------------------
template< class T >
int CUtlVector<T>::Find(T const& src) const
{
    for (int i = 0; i < Count(); ++i)
    {
        if (Element(i) == src)
            return i;
    }
    return -1;
}

template< class T >
bool CUtlVector<T>::HasElement(T const& src)
{
    return (Find(src) >= 0);
}

//-----------------------------------------------------------------------------
// Element removal
//-----------------------------------------------------------------------------

template< class T >
void CUtlVector<T>::FastRemove(int elem)
{
    assert(IsValidIndex(elem));

    Destruct(&Element(elem));
    if (m_Size > 0)
    {
        Q_memcpy(&Element(elem), &Element(m_Size - 1), sizeof(T));
        --m_Size;
    }
}

template< class T >
void CUtlVector<T>::Remove(int elem)
{
    Destruct(&Element(elem));
    ShiftElementsLeft(elem);
    --m_Size;
}

template< class T >
void CUtlVector<T>::FindAndRemove(T const& src)
{
    int elem = Find(src);
    if (elem != -1)
    {
        Remove(elem);
    }
}

template <class T>
bool CUtlVector<T>::FindAndFastRemove(const T& src)
{
    int elem = Find(src);
    if (elem != -1);
    {
        FastRemove(elem);
        return true;
    }
    return false;
}

template< class T >
void CUtlVector<T>::RemoveMultiple(int elem, int num)
{
    assert(IsValidIndex(elem));
    assert(elem + num <= Count());

    for (int i = elem + num; --i >= elem;)
        Destruct(&Element(i));

    ShiftElementsLeft(elem, num);
    m_Size -= num;
}

template< class T >
void CUtlVector<T>::RemoveAll()
{
    for (int i = m_Size; --i >= 0;)
        Destruct(&Element(i));

    m_Size = 0;
}

//-----------------------------------------------------------------------------
// Memory deallocation
//-----------------------------------------------------------------------------

template< class T >
void CUtlVector<T>::Purge()
{
    RemoveAll();
    m_Memory.Purge();
    ResetDbgInfo();
}

template<class T>
inline void CUtlVector<T>::PurgeAndDeleteElements()
{
    for (int i = 0; i < m_Size; i++)
        delete Element(i);

    Purge();
}

template< class T >
void CUtlVector<T>::SetGrowSize(int size)
{
    m_Memory.SetGrowSize(size);
}


// 16 bytes == 128 bit digest
#define MD5_DIGEST_LENGTH 16
#define MD5_BIT_LENGTH ( MD5_DIGEST_LENGTH * sizeof(unsigned char) )
struct MD5Value_t
{
    unsigned char bits[MD5_DIGEST_LENGTH];

    void Zero();
    bool IsZero() const;

    bool operator==(const MD5Value_t& src) const;
    bool operator!=(const MD5Value_t& src) const;

};

// MD5 Hash
typedef struct
{
    unsigned int	buf[4];
    unsigned int	bits[2];
    unsigned char	in[64];
} MD5Context_t;

void MD5Init(MD5Context_t* context);
void MD5Update(MD5Context_t* context, unsigned char const* buf, unsigned int len);
void MD5Final(unsigned char digest[MD5_DIGEST_LENGTH], MD5Context_t* context);

char* MD5_Print(unsigned char* digest, int hashlen);

/// Convenience wrapper to calculate the MD5 for a buffer, all in one step, without
/// bothering with the context object.
void MD5_ProcessSingleBuffer(const void* p, int len, MD5Value_t& md5Result);

unsigned int MD5_PseudoRandom(unsigned int nSeed);

/// Returns true if the values match.
bool MD5_Compare(const MD5Value_t& data, const MD5Value_t& compare);

inline bool MD5Value_t::operator==(const MD5Value_t& src) const
{
    return MD5_Compare(*this, src);
}

inline bool MD5Value_t::operator!=(const MD5Value_t& src) const
{
    return !MD5_Compare(*this, src);
}

//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef COLOR_H
#define COLOR_H

#ifdef _WIN32
#pragma once
#endif
//-----------------------------------------------------------------------------
// Purpose: Basic handler for an rgb set of colors
//			This class is fully inline
//-----------------------------------------------------------------------------
class color
{
public:
    // constructors
    color()
    {
        *((int*)this) = 0;
    }
    color(int _r, int _g, int _b)
    {
        SetColor(_r, _g, _b, 0);
    }
    color(int _r, int _g, int _b, int _a)
    {
        SetColor(_r, _g, _b, _a);
    }

    // set the color
    // r - red component (0-255)
    // g - green component (0-255)
    // b - blue component (0-255)
    // a - alpha component, controls transparency (0 - transparent, 255 - opaque);
    void SetColor(int _r, int _g, int _b, int _a = 0)
    {
        _color[0] = (unsigned char)_r;
        _color[1] = (unsigned char)_g;
        _color[2] = (unsigned char)_b;
        _color[3] = (unsigned char)_a;
    }

    void GetColor(int& _r, int& _g, int& _b, int& _a) const
    {
        _r = _color[0];
        _g = _color[1];
        _b = _color[2];
        _a = _color[3];
    }

    void SetRawColor(int color32)
    {
        *((int*)this) = color32;
    }

    int GetRawColor() const
    {
        return *((int*)this);
    }

    int r = _color[0];
    int g = _color[1];
    int b = _color[2];
    int a = _color[3];

    unsigned char& operator[](int index)
    {
        return _color[index];
    }

    const unsigned char& operator[](int index) const
    {
        return _color[index];
    }

    bool operator == (const color& rhs) const
    {
        return (*((int*)this) == *((int*)&rhs));
    }

    bool operator != (const color& rhs) const
    {
        return !(operator==(rhs));
    }

    color& operator=(const color& rhs)
    {
        SetRawColor(rhs.GetRawColor());
        return *this;
    }

private:
    unsigned char _color[4];
};

#define COLOR_RED		D3DCOLOR_RGBA(255, 64, 64, 255)
#define COLOR_BLUE		D3DCOLOR_RGBA(153, 204, 255, 255)
#define COLOR_YELLOW	D3DCOLOR_RGBA(255, 178, 0, 255)
#define COLOR_GREEN		D3DCOLOR_RGBA(153, 255, 153, 255)
#define COLOR_GREY		D3DCOLOR_RGBA(204, 204, 204, 255)
#define COLOR_WHITE		D3DCOLOR_RGBA(255, 255, 255, 255)
#define COLOR_BLACK		D3DCOLOR_RGBA(0, 0, 0, 255)

#endif // COLOR_H
