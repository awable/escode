/*
 * Copyright (C) 2014 Akhil Wable
 * Author: Akhil Wable <awable@gmail.com>
 *
 * Number/INT/LONG encoding for ESCODE
 *
 */

#ifndef __ESCODE_NUMBER_H__
#define __ESCODE_NUMBER_H__

#include <Python.h>
#include <datetime.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include "escode.h"


int
encode_num(PyObject *object, strbuf* buf) {

  uint64_t u64 = 0;

  if (PyInt_CheckExact(object) || PyLong_CheckExact(object)) {
    int overflow;
    int64_t i64 = PyLong_AsLongLongAndOverflow(object, &overflow);
    if (PyErr_Occurred()) { return 0; }

    if (overflow < 0) {
      PyErr_SetString(ESCODE_EncodeError, "negative int out of range");
      return 0;
    }

    if (overflow > 0) {
      // Set u64 since this number is larger than INT64_MAX
      u64 = PyLong_AsUnsignedLongLong(object);
      if (PyErr_Occurred()) { return 0; }
    }

    // have to check u64 first because i64 is no longer
    // a reliable indicator of whats up if overflow > 0


    // UINT64: UINT32_MAX < val <= UINT64_MAX
    if (u64 || i64 > UINT32_MAX) {
      u64 = htonll(u64 ? u64 : i64);
      strbuf_put_meta(buf, ESCODE_TYPE_ULONG, NULL, /*index=*/1);
      strbuf_put_content(buf, &u64, sizeof(uint64_t));


      // UINT32: INT32_MAX < val <= UINT32_MAX
    } else if (i64 > INT32_MAX) {
      uint32_t u32 = htonl(i64);
      strbuf_put_meta(buf, ESCODE_TYPE_UINT, NULL, /*index=*/1);
      strbuf_put_content(buf, &u32, sizeof(uint32_t));


      // -INT64: INT64_MIN <= val < INT32_MIN
    } else if (i64 < INT32_MIN) {
      u64 = htonll(i64);
      strbuf_put_meta(buf, ESCODE_TYPE_LONG, NULL, /*index=*/1);
      strbuf_put_content(buf, &u64, sizeof(uint64_t));


      // INT32: INT32_MIN <= val <= INT32_MAX
    } else if (i64 >= INT32_MIN && i64 <= INT32_MAX) {
      // Downcast i64 to i32 since we are in range and add INT32_MAX+1 (MSB32_FLIP)
      uint32_t u32 = htonl(MSB32_FLIP((int32_t)i64));
      strbuf_put_meta(buf, ESCODE_TYPE_INT, NULL, /*index=*/1);
      strbuf_put_content(buf, &u32, sizeof(uint32_t));

    } else { return 0;}

  } else if (PyFloat_CheckExact(object)) {
    double val = PyFloat_AsDouble(object);
    if (PyErr_Occurred()) { return 0; }

    // Doubles are essentially Sign Represented INTs for ordering
    u64 = htonll(SRINT64_TOUINT(*(uint64_t*)&val));
    strbuf_put_meta(buf, ESCODE_TYPE_FLOAT, NULL, /*index=*/0);
    strbuf_put_content(buf, &u64, sizeof(uint64_t));

  } else if (PyDateTime_CheckExact(object)) {
    PyDateTime_GET_TIMESTAMP(object, &u64);
    u64 = htonll(u64);
    strbuf_put_meta(buf, ESCODE_TYPE_DATETIME, NULL, /*index=*/0);
    strbuf_put_content(buf, &u64, sizeof(uint64_t));
  }

  return 1;
}

#endif //__ESCODE_NUMBER_H__
