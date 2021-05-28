/*
 * Copyright (C) 2014 Akhil Wable
 * Author: Akhil Wable <awable@gmail.com>
 *
 * Constants for ESCODE
 *
 */

#ifndef __ESCODE_H__
#define __ESCODE_H__

#include <Python.h>
#include <datetime.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <time.h>
#include <arpa/inet.h>
#include <datetime.h>
#include <py3c/py3c.h>
#include "constants.h"
#include "strbuf.h"

#define MODULE_VERSION "2.0.0"

static PyObject *ESCODE_Error;
static PyObject *ESCODE_EncodeError;
static PyObject *ESCODE_DecodeError;
static PyObject *ESCODE_UnsupportedError;

static PyObject*
ESCODE_encode_index(PyObject *self, PyObject *args);

static PyObject*
ESCODE_encode(PyObject *self, PyObject *object);


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

#define MyPyDateTime_GET_TIMESTAMP(ob)          \
  ({struct tm time = {                          \
    .tm_year=PyDateTime_GET_YEAR(ob)-1900,      \
    .tm_mon=PyDateTime_GET_MONTH(ob)-1,         \
    .tm_mday=PyDateTime_GET_DAY(ob),            \
    .tm_hour=PyDateTime_DATE_GET_HOUR(ob),      \
    .tm_min=PyDateTime_DATE_GET_MINUTE(ob),     \
    .tm_sec=PyDateTime_DATE_GET_SECOND(ob)};    \
    (uint64_t)mktime(&time);})


#define MyPyDateTime_FROM_TIMESTAMP(val)                                \
  ({struct tm* time = localtime((time_t*)&val);                         \
    PyDateTime_FromDateAndTime(                                         \
        time->tm_year+1900, time->tm_mon+1, time->tm_mday,              \
        time->tm_hour, time->tm_min, time->tm_sec, 0);})


#ifndef PyDict_GET_SIZE
#define PyDict_GET_SIZE(mp)  (assert(PyDict_Check(mp)),((PyDictObject *)mp)->ma_used)
#endif

/**************************************************************
                  htonll/ntohll
****************************************************************/

#ifndef htonll
#if __BIG_ENDIAN__
#define htonll(x) (x)
#define ntohll(x) (x)
#else
#define htonll(x) ({((uint64_t)htonl((x) & 0xFFFFFFFF) << 32) | htonl((x) >> 32);})
#define ntohll(x) ({((uint64_t)ntohl((x) & 0xFFFFFFFF) << 32) | ntohl((x) >> 32);})
#endif //__BIG_ENDIAN__
#endif //htonll


#endif //__ESCODE_H__
