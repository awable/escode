/*
 * Copyright (C) 2014 Akhil Wable
 * Author: Akhil Wable <awable@gmail.com>
 */

#ifndef __INTLIB_H__
#define __INTLIB_H__

typedef uint8_t bool;
typedef uint8_t byte;

// Count leading 0s (1s if !pos), divide by 8 (>>3) gives us bytes to skip
#define _NUMOFFSET(num, pos) (__builtin_clzll(FLIPIF(num, !B(pos)) | 1) >> 3)
#define _NUMWIDTH(num, pos) (8-_NUMOFFSET(num, pos))

//
// We want ceiling(log2(width))
// - This will result in values 0, 1, 2 or 3
// - 2 raised to the power of that will give us 1, 2, 4 or 8 which is useful for
// figuring out how many bytes we need to store a number.
//
//
// ceiling(log2(x)) = floor(log2(x-1)) + 1, so
// ceiling(log2(width)) = floor(log2(width-1)) + 1
//
// ...and, floor(log2(n)) = w - 1 - clz(n), so with w=64
// ceiling(log2(width)) = (64 - 1 - clz(width-1)) + 1
// ceiling(log2(width)) = 64 - clz(width-1)
//
// ...but clz(0) is not well defined, and width is <=8
// clz((n << 1) + 1) = clz(n) - 1 # shifted 1 left, so lz will reduce
// clz(n) = clz((n << 1) + 1) + 1
//
// ceiling(log2(width)) = 64 - (clz(((width-1) << 1) + 1) + 1)
// ceiling(log2(width)) = 64 - clz(((width-1) << 1) + 1) - 1
// ceiling(log2(width)) = 63 - clz(((width-1) << 1) + 1)
#define _NUMLG2WIDTH(num, pos)  ( 63 - __builtin_clzll(((_NUMWIDTH(num,pos)-1)<<1)+1) )


#define FLIPIF(x, cond) (x ^ (0-B(cond)))
#define B(x) (!!(x))


/**************************************************************
                  NUMBER MANIPULATION
****************************************************************/


#define MSB64_MASK 0x8000000000000000
#define MSB32_MASK 0x80000000

#define MSB64_FILL(x) ((uint64_t)((int64_t)((uint64_t)(x) & MSB64_MASK) >> 63))
#define MSB32_FILL(x) ((uint32_t)((int32_t)((uint32_t)(x) & MSB32_MASK) >> 31))

#define MSB64_FLIP(x) ((uint64_t)(x) ^ MSB64_MASK)
#define MSB32_FLIP(x) ((uint32_t)(x) ^ MSB32_MASK)

//flips bottom bits if MSB is 1
#define IFMSB64_FLIP63(x) ((uint64_t)(x) ^ (~MSB64_MASK & MSB64_FILL(x)))
#define IFMSB32_FLIP31(x) ((uint32_t)(x) ^ (~MSB32_MASK & MSB32_FILL(x)))


// SRINT is MSB.MAGNITUDE where the MSB is 1 for neg. So to get ordered bytes
// we need to flip the MSB, and flip the MAGNITUDE only for neg
//
// (MSB64_FILL(x) | MSB) means we get 0xFF...FF or neg, and 0x80...00 for pos
// (<res> ^ x) means neg will be inverted while pos will have its MSB flipped
#define SINT64UINT(x) ((uint64_t)(x) ^ (MSB64_FILL(x) | MSB64_MASK))
#define SINT32UINT(x) ((uint32_t)(x) ^ (MSB32_FILL(x) | MSB32_MASK))
#define UINT64SINT(x) ((uint64_t)(x) ^ (~MSB64_FILL(x) | MSB64_MASK))
#define UINT32SINT(x) ((uint32_t)(x) ^ (~MSB32_FILL(x) | MSB32_MASK))

#endif //__INTLIB_H__
