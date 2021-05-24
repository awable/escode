/*
 * Copyright (C) 2014 Akhil Wable
 * Author: Akhil Wable <awable@gmail.com>
 *
 * Constants for ESCODE
 *
 */

#ifndef __ESCODE_CONSTANTS_H__
#define __ESCODE_CONSTANTS_H__

#include <stdint.h>
#include <time.h>
#include <datetime.h>
#include <bytesobject.h>
#include "strbuf.h"

// The order int and bool types below must be like this
// since we use the type code for sorting.

#define _ESCODE_TYPE_NONE 1
#define _ESCODE_TYPE_TRUE 2
#define _ESCODE_TYPE_FALSE 3
#define _ESCODE_TYPE_LONG 4  // -(1<<63) <= val < -(1<<31)
#define _ESCODE_TYPE_INT 5   // -(1<<31) <= val < (1<<31)
#define _ESCODE_TYPE_UINT 6  //    1<<31 <= val < 1<<32
#define _ESCODE_TYPE_ULONG 7 //    1<<32 <= val < 1<<64
#define _ESCODE_TYPE_FLOAT 8
#define _ESCODE_TYPE_BYTES 9
#define _ESCODE_TYPE_UNICODE 10
#define _ESCODE_TYPE_LIST 11
#define _ESCODE_TYPE_DICT 12
#define _ESCODE_TYPE_DATETIME 13

static PyObject *ESCODE_Error;
static PyObject *ESCODE_EncodeError;
static PyObject *ESCODE_DecodeError;
static PyObject *ESCODE_UnsupportedError;

typedef char byte;
typedef char bool;
static const byte ESCODE_TYPE_NONE = _ESCODE_TYPE_NONE;
static const byte ESCODE_TYPE_TRUE = _ESCODE_TYPE_TRUE;
static const byte ESCODE_TYPE_FALSE = _ESCODE_TYPE_FALSE;
static const byte ESCODE_TYPE_LONG = _ESCODE_TYPE_LONG;
static const byte ESCODE_TYPE_INT = _ESCODE_TYPE_INT;
static const byte ESCODE_TYPE_UINT = _ESCODE_TYPE_UINT;
static const byte ESCODE_TYPE_ULONG = _ESCODE_TYPE_ULONG;
static const byte ESCODE_TYPE_FLOAT = _ESCODE_TYPE_FLOAT;
static const byte ESCODE_TYPE_BYTES = _ESCODE_TYPE_BYTES;
static const byte ESCODE_TYPE_UNICODE = _ESCODE_TYPE_UNICODE;
static const byte ESCODE_TYPE_LIST = _ESCODE_TYPE_LIST;
static const byte ESCODE_TYPE_DICT = _ESCODE_TYPE_DICT;
static const byte ESCODE_TYPE_DATETIME = _ESCODE_TYPE_DATETIME;

#define assertLen(_len)                                                 \
  if (_len > UINT16_MAX) {                                              \
    PyErr_SetString(ESCODE_EncodeError, "value too long to encode");    \
    return 0;                                                           \
  }                                                                     \
  uint16_t len = (uint16_t)_len;                                        \

#if PY_MAJOR_VERSION >= 3
#define PyIntObject PyLongObject
#define PyInt_Type PyLong_Type
#define PyInt_CheckExact PyLong_CheckExact
#endif

/**************************************************************
                  NUMBER MANIPULATION
****************************************************************/

#define MSB64_MASK 0x8000000000000000
#define MSB32_MASK 0x80000000

#define MSB64_FLIP(x) (x ^ MSB64_MASK)
#define MSB32_FLIP(x) (x ^ MSB32_MASK)
#define MSB64_FILL(x) ((int64_t)(x & MSB64_MASK) >> 63)
#define MSB32_FILL(x) ((int32_t)(x & MSB64_MASK) >> 31)

// SRINT is MSB.MAGNITUDE where the MSB is 1 for neg. So to get ordered bytes
// we need to flip the MSB, and flip the MAGNITUDE only for neg
//
// (MSB64_FILL(x) | MSB) means we get 0xFF...FF or neg, and 0x80...00 for pos
// (<res> ^ x) means neg will be inverted while pos will have its MSB flipped
#define SRINT64_TOUINT(x) ((MSB64_FILL(x) | MSB64_MASK) ^ x)
#define SRINT64_FROMUINT(x) ((~MSB64_FILL(x) | MSB64_MASK) ^ x)
#define SRINT32_TOUINT(x) ((MSB32_FILL(x) | MSB32_MASK) ^ x)
#define SRINT32_FROMUINT(x) ((~MSB32_FILL(x) | MSB32_MASK) ^ x)

/**************************************************************
                  PYTHON DATETIME HELPERS
****************************************************************/

#define PyDateTime_GET_TIMESTAMP(ob, pu64)      \
  struct tm time = {                            \
    .tm_year=PyDateTime_GET_YEAR(ob)-1900,      \
    .tm_mon=PyDateTime_GET_MONTH(ob)-1,         \
    .tm_mday=PyDateTime_GET_DAY(ob),            \
    .tm_hour=PyDateTime_DATE_GET_HOUR(ob),      \
    .tm_min=PyDateTime_DATE_GET_MINUTE(ob),     \
    .tm_sec=PyDateTime_DATE_GET_SECOND(ob)};    \
  *pu64 = (uint64_t)mktime(&time);              \


#define PyDateTime_FROM_TIMESTAMP(pob, val)                             \
  struct tm* time = localtime((time_t*)&val);                           \
  *pob = PyDateTime_FromDateAndTime(                                    \
    time->tm_year+1900, time->tm_mon+1, time->tm_mday,                  \
    time->tm_hour, time->tm_min, time->tm_sec, 0);                      \


/**************************************************************
                  htonll/ntohll
****************************************************************/

#ifndef htonll
#define htonll
#if __BIG_ENDIAN__
#define htonll(x) (x)
#define ntohll(x) (x)
#else
#define htonll(x) ((uint64_t)htonl((x) & 0xFFFFFFFF) << 32) | htonl((x) >> 32))
#define ntohll(x) ((uint64_t)ntohl((x) & 0xFFFFFFFF) << 32) | ntohl((x) >> 32))
#endif
#endif //htonll


#endif //__ESCODE_CONSTANTS_H__
