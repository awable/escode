#ifndef __ESCODE_ESHEAD_H__
#define __ESCODE_ESHEAD_H__


#include <stdint.h>
#include <math.h>
#include "htonll.h"
#include "intlib.h"

typedef uint8_t bool;
typedef uint8_t byte;

typedef struct eshead_t {
  byte headbyte;
  byte ops;

  struct {
    uint8_t width;
    uint8_t off;
    union {
      uint64_t b64;
      byte bytes[8];
    } num;
  } enc;

  union  {
    uint64_t u64;
    int64_t i64;
    double flt;
    byte bytes[8];
  } val;

} eshead_t;

#define OP_ESHASNUM 0x01
#define OP_ESINDEXHEAD 0x02
#define OP_ESINDEXNUM 0x04



#define ESHEAD_INITENCODE(eshead)               \
  ({memset((eshead), 0, sizeof(eshead_t));      \
    eshead;})

#define ESHEAD_INITDECODE(eshead, headbyte)     \
  ({memset((eshead), 0, sizeof(eshead_t));      \
    (eshead)->headbyte = headbyte;              \
    eshead;})


/****************************************************************************
 * ENCODE STAGE
 ****************************************************************************/


#define ESHEAD_SETBOOL(eshead, type, bval)                              \
  ({ESHEAD_SETINFO(eshead, type, !!(bval));                             \
    (eshead)->ops |= OP_ESINDEXHEAD;                                    \
    eshead;})


#define ESHEAD_ENCODEINT(eshead, type, pos)                             \
  ({_ESHEAD_ENCODENUM(eshead, type, pos);                               \
    ESHEAD_SETINFO(eshead, type, _INTINFO(pos, (eshead)->enc.width));   \
    (eshead)->ops |= OP_ESINDEXHEAD | OP_ESINDEXNUM;                    \
    eshead;})

#define ESHEAD_ENCODELEN(eshead, type, bit)                             \
  ({_ESHEAD_ENCODENUM(eshead, type, 1);                                 \
    ESHEAD_SETINFO(eshead, type, _LENINFO(bit, (eshead)->enc.width));   \
    eshead;})

#define _ESHEAD_ENCODENUM(eshead, type, pos)                            \
  ({(eshead)->enc.num.b64  = htonll((eshead)->val.u64);                 \
    (eshead)->enc.off = _NUMOFFSET((eshead)->enc.num.bytes, pos);       \
    (eshead)->enc.width = 8-((eshead)->enc.off);                        \
    (eshead)->ops = OP_ESHASNUM;                                        \
    eshead;})

// Floats are similar to Sign Represented Ints with MSB sign and an abs value
// Reverse MSB so that +ve is 1 and -ve is 0. Flip the magnitude for -ve numbers
// because lower abs value means a higher number
#define ESHEAD_ENCODEFLOAT(eshead, type)                                \
  ({(eshead)->enc.num.b64 = htonll(SINT64UINT((eshead)->val.u64));      \
    (eshead)->enc.width = sizeof(double);                               \
    (eshead)->enc.off = 0;                                              \
    (eshead)->ops = OP_ESHASNUM | OP_ESINDEXNUM;                        \
    ESHEAD_SETINFO(eshead, type, sizeof(double)-1);                     \
    eshead;})


#define ESHEAD_SETINFO(eshead, type, info)                              \
  ({(eshead)->headbyte = ((byte)(((type) << 4) | ((info) & 0x0F)));     \
    eshead;})

// width-1 = 0 -> 7 = 0000 -> 0111
#define _LENINFO(bit, width) (((!!(bit)) << 3) | ((width)-1))

// width-1 = 0 -> 7 = 0000 -> 0111. upper bit is 0.
// +ve xor mask = 8-!1 = 8-0 = 8 = 1000 which keeps bottom 3, sets bit to 1
// -ve xor mask = 8-!0 = 8-1 = 7 = 0111 which flips bottom 3, keeps bit at 0
#define _INTINFO(pos, width) (((width)-1) ^ (0x08 - !(pos)))

#define _NUMOFFSET(bytes, pos)                      \
  ({uint8_t off = 0;                                \
    byte skip = ((byte)(0-!(pos)));                 \
    for(;off < 7 && bytes[off] == skip; off++);     \
    off;})


/****************************************************************************
 * DECODE STAGE
 ****************************************************************************/

#define ESHEAD_GETBOOL(eshead) \
  ESHEAD_GETINFO(eshead)

#define ESHEAD_DECODEINT(eshead, bytes)                                 \
  ({(eshead)->enc.width = ESHEAD_GETINTWIDTH(eshead);                   \
    bool _pos = ESHEAD_GETBIT(eshead);                                  \
    _ESHEAD_DECODENUM(eshead, bytes, _pos);                             \
    _pos;})

#define ESHEAD_DECODELEN(eshead, bytes)                                 \
  ({(eshead)->enc.width = ESHEAD_GETWIDTH(eshead);                      \
    _ESHEAD_DECODENUM(eshead, bytes, 1);                                \
    ESHEAD_GETBIT(eshead);})

#define _ESHEAD_DECODENUM(eshead, bytes, pos)                           \
  ({(eshead)->enc.num.b64 = (uint64_t)((int64_t)(0-(!pos)));            \
    (eshead)->enc.off = 8-(eshead)->enc.width;                          \
    byte* _cursor = (eshead)->enc.num.bytes + (eshead)->enc.off;        \
    memcpy(_cursor, bytes, (eshead)->enc.width);                        \
    (eshead)->val.u64 = ntohll((eshead)->enc.num.b64);                  \
    ESHEAD_GETBIT(eshead);})

#define ESHEAD_DECODEFLOAT(eshead, bytes)                               \
  ({memcpy((eshead)->enc.num.bytes, bytes, sizeof(double));             \
    (eshead)->val.u64 = UINT64SINT(ntohll((eshead)->enc.num.b64));      \
    0;})


// (headbyte >> 4) & 0000.1111
#define ESHEAD_GETTYPE(eshead) (((eshead)->headbyte >> 4) & 0x0F)

// headbyte & 0000.1111
#define ESHEAD_GETINFO(eshead) ((eshead)->headbyte & 0x0F)

// (headbyte & 0000.1000) >> 3
#define ESHEAD_GETBIT(eshead) (((eshead)->headbyte & 0x08)>>3)

// headbyte & 0000.0111
#define ESHEAD_GETWIDTH(eshead) (((eshead)->headbyte & 0x07) + 1)

// +ve xor mask = 8-!1 = 8-0 = 8 = 1000 which keeps bottom 3
// -ve xor mask = 8-!0 = 8-1 = 7 = 0111 which flips bottom 3
// & 0x07 returns the bottom 3 bits which represent width
#define ESHEAD_GETINTWIDTH(eshead)                              \
  ((((eshead)->headbyte ^ (0x08-!ESHEAD_GETBIT(eshead))) & 0x07) + 1)

#endif //__ESCODE_ESHEAD_H__
