/*
 * Copyright (C) 2014 Akhil Wable
 * Author: Akhil Wable <awable@gmail.com>
 */

#ifndef __INTLIB_H__
#define __INTLIB_H__


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
