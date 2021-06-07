#ifndef __ESCODE_ESHEAD_H__
#define __ESCODE_ESHEAD_H__


#include <stdint.h>
#include <math.h>
#include "htonll.h"
#include "intlib.h"

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
 * ENCODE
 ****************************************************************************/


#define ESHEAD_ENCODENONE(eshead, type)                                 \
  ({ESHEAD_SETINFO(eshead, type, 0);                                    \
    (eshead)->ops |= OP_ESINDEXHEAD;                                    \
    eshead;})

#define ESHEAD_ENCODEBOOL(eshead, type, bval)                           \
  ({ESHEAD_SETINFO(eshead, type, B(bval) << 3);                         \
    (eshead)->ops |= OP_ESINDEXHEAD;                                    \
    eshead;})

#define ESHEAD_ENCODEINT(eshead, type, pos)                             \
  ({bool _pos = B(pos);                                                 \
    _ESHEAD_ENCODENUM(eshead, type, _pos, _pos);                        \
    (eshead)->ops |= OP_ESINDEXHEAD | OP_ESINDEXNUM;                    \
    eshead;})

#define ESHEAD_ENCODELEN(eshead, type, bit)                             \
  _ESHEAD_ENCODENUM(eshead, type, B(bit), 1)

// Floats are similar to Sign Represented Ints with MSB sign and an abs
// value. Reverse MSB so that +ve is 1 and -ve is 0. Flip the abs value
// for -ve numbers because lower abs value means a higher number
#define ESHEAD_ENCODEFLOAT(eshead, type)                                \
  ({bool _pos = !((eshead)->val.u64 >> 63);                             \
    (eshead)->enc.width = sizeof(double);                               \
    (eshead)->enc.off = 0;                                              \
    (eshead)->enc.num.b64 = htonll(SINT64UINT((eshead)->val.u64));      \
    (eshead)->ops = OP_ESHASNUM | OP_ESINDEXNUM;                        \
    _ESHEAD_SETNUMINFO(eshead, type, _pos, 0, _pos);                    \
    eshead;})

#define ESHEAD_ENCODEEXP(eshead, type, pos)                             \
  ({bool _pos = B(pos);                                                 \
    bool _epos = !((eshead)->val.u64 >> 63);                            \
    byte _lg2width = _NUMLG2WIDTH((eshead)->val.u64, _epos);            \
    (eshead)->enc.width = 1 << _lg2width;                               \
    (eshead)->enc.off = 8-(eshead)->enc.width;                          \
    (eshead)->enc.num.b64  = FLIPIF(htonll((eshead)->val.u64), !_pos);  \
    (eshead)->ops = OP_ESHASNUM | OP_ESINDEXHEAD | OP_ESINDEXNUM;       \
    _ESHEAD_SETEXPINFO(eshead, type, _pos, _epos, _lg2width);           \
    eshead;})


// HELPERS:These helpers assume bools have been !! converted

#define ESHEAD_SETINFO(eshead, type, info)                              \
  ({(eshead)->headbyte = ((byte)(((type) << 4) | ((info) & 0x0F)));     \
    eshead;})

// offset is 0->7 implying width of 8->1 bytes needed to store
// +ve: lower offset means higher width number: flip the offset
// -ve: lower offset means more negative i.e. lower: keep the offset
#define _ESHEAD_SETNUMINFO(eshead, type, bit, offset, pos)              \
  ({byte _info = ((bit << 3) | (FLIPIF(offset, pos) & 0x07));           \
    ESHEAD_SETINFO(eshead, type, _info);})


#define _ESHEAD_SETEXPINFO(eshead, type, pos, epos, lg2width)           \
  ({byte _eposwidth = (epos << 2) | (FLIPIF(lg2width, !epos) & 0x03);   \
    byte _info = (pos << 3) | (FLIPIF(_eposwidth, !pos) & 0x07);        \
    ESHEAD_SETINFO(eshead, type, _info);})

#define _ESHEAD_ENCODENUM(eshead, type, bit, pos)                       \
  ({(eshead)->enc.off = _NUMOFFSET((eshead)->val.u64, pos);             \
    (eshead)->enc.width = 8-((eshead)->enc.off);                        \
    (eshead)->enc.num.b64  = htonll((eshead)->val.u64);                 \
    (eshead)->ops = OP_ESHASNUM;                                        \
    _ESHEAD_SETNUMINFO(eshead, type, bit, (eshead)->enc.off, pos);      \
    eshead;})


/****************************************************************************
 * DECODE
 ****************************************************************************/

#define ESHEAD_GETBOOL(eshead) \
  ESHEAD_GETBIT(eshead)

#define ESHEAD_DECODEINT(eshead, bytes)                                 \
  ({bool _pos = ESHEAD_GETBIT(eshead);                                  \
    _ESHEAD_DECODENUM(eshead, bytes, _pos);})

#define ESHEAD_DECODELEN(eshead, bytes)                                 \
  _ESHEAD_DECODENUM(eshead, bytes, 1)

#define ESHEAD_DECODEFLOAT(eshead, bytes)                               \
  ({memcpy((eshead)->enc.num.bytes, bytes, sizeof(double));             \
    (eshead)->val.u64 = UINT64SINT(ntohll((eshead)->enc.num.b64));      \
    0;})

#define _ESHEAD_DECODENUM(eshead, bytes, pos)                           \
  ({(eshead)->enc.width = ESHEAD_GETNUMWIDTH(eshead, pos);              \
    (eshead)->enc.num.b64 = 0-(!pos);                                   \
    (eshead)->enc.off = 8-(eshead)->enc.width;                          \
    byte* _cursor = (eshead)->enc.num.bytes + (eshead)->enc.off;        \
    memcpy(_cursor, bytes, (eshead)->enc.width);                        \
    (eshead)->val.u64 = ntohll((eshead)->enc.num.b64);                  \
    ESHEAD_GETBIT(eshead);})

#define ESHEAD_DECODEEXP(eshead, bytes)                                 \
  ({bool _pos = ESHEAD_GETBIT(eshead);                                  \
    bool _epos = ESHEAD_GETEXPBIT(eshead);                              \
    (eshead)->enc.width = ESHEAD_GETEXPWIDTH(eshead, _epos);            \
    (eshead)->enc.off = 8-(eshead)->enc.width;                          \
    (eshead)->enc.num.b64 = 0-(!epos);                                  \
    byte* _cursor = (eshead)->enc.num.bytes + (eshead)->enc.off;        \
    memcpy(_cursor, bytes, (eshead)->enc.width);                        \
    (eshead)->val.i64 = FLIPIF(ntohll((eshead)->enc.num.b64), !_pos);   \
    _pos;})


// (headbyte >> 4) & 0000.1111
#define ESHEAD_GETTYPE(eshead) (((eshead)->headbyte >> 4) & 0x0F)

// headbyte & 0000.1111
#define ESHEAD_GETINFO(eshead) ((eshead)->headbyte & 0x0F)

// (headbyte & 0000.1000) >> 3
#define ESHEAD_GETBIT(eshead) (((eshead)->headbyte & 0x08)>>3)

// (headbyte & 0000.1000) >> 3
#define ESHEAD_GETEXPBIT(eshead) (((eshead)->headbyte & 0x04)>>2)


// HELPERS:These helpers assume bools have been !! converted
#define ESHEAD_GETNUMWIDTH(eshead, pos)                 \
  ((FLIPIF((eshead)->headbyte, !pos) & 0x07) + 1)

#define ESHEAD_GETEXPWIDTH(eshead, epos)                 \
  (1 << (FLIPIF((eshead)->headbyte, !epos) & 0x03))




#endif //__ESCODE_ESHEAD_H__
