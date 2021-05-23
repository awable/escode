/*
 * Copyright (C) 2014 Akhil Wable
 * Author: Akhil Wable <awable@gmail.com>
 *
 * Decoder ESCODE
 *
 */

#ifndef __ESCODE_DECODER_H__
#define __ESCODE_DECODER_H__

#include <Python.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include "escode.h"
#include "number.h"

PyObject*
decode_object(strbuf *buf) {
  const byte* type = strbuf_pull_type(buf, byte);
  if (type == NULL) { return NULL; }

  switch (*type) {

  case _ESCODE_TYPE_NONE: {
    Py_RETURN_NONE;
  }
  case _ESCODE_TYPE_TRUE: {
    Py_RETURN_TRUE;
  }
  case _ESCODE_TYPE_FALSE: {
    Py_RETURN_FALSE;
  }

  case _ESCODE_TYPE_INT: {
    const uint32_t* pu32 = strbuf_pull_type(buf, uint32_t);
    if (pu32 == NULL) { return NULL; }
    int32_t val = MSB32_FLIP(ntohl(*pu32));
    return PyLong_FromLong(val);
  }

  case _ESCODE_TYPE_UINT: {
    const uint32_t* pu32 = strbuf_pull_type(buf, uint32_t);
    if (pu32 == NULL) { return NULL; }
    uint32_t val = ntohl(*pu32);
    return PyLong_FromUnsignedLong(val);
  }

  case _ESCODE_TYPE_LONG: {
    const uint64_t* pu64 = strbuf_pull_type(buf, uint64_t);
    if (pu64 == NULL) { return NULL; }
    int64_t val = ntohll(*pu64);
    return PyLong_FromLongLong(val);
  }

  case _ESCODE_TYPE_ULONG: {
    const uint64_t* pu64 = strbuf_pull_type(buf, uint64_t);
    if (pu64 == NULL) { return NULL; }
    uint64_t val = ntohll(*pu64);
    return PyLong_FromUnsignedLongLong(val);
  }

  case _ESCODE_TYPE_FLOAT: {
    const uint64_t* pu64 = strbuf_pull_type(buf, uint64_t);
    if (pu64 == NULL) { return NULL; }
    uint64_t repr = SRINT64_FROMUINT(ntohll(*pu64));
    double val = *(double*)&repr;
    return PyFloat_FromDouble(val);
  }

  case _ESCODE_TYPE_DATETIME: {
    const uint64_t* pu64 = strbuf_pull_type(buf, uint64_t);
    if (pu64 == NULL) { return NULL; }
    uint64_t val = ntohll(*pu64);
    PyObject *obj;
    PyDateTime_FROM_TIMESTAMP(&obj, val);
    return obj;
  }

  case _ESCODE_TYPE_BYTES: {
    uint16_t* len = strbuf_pull_type(buf, uint16_t);
    if (len == NULL) { return NULL; }
    const byte* contents = strbuf_pull(buf, *len);
    return PyBytes_FromStringAndSize(contents, *len);
  }

  case _ESCODE_TYPE_STRING: {
    uint16_t* len = strbuf_pull_type(buf, uint16_t);
    if (len == NULL) { return NULL; }
    const byte* contents = strbuf_pull(buf, *len);
    return PyString_FromStringAndSize(contents, *len);
  }

  case _ESCODE_TYPE_UNICODE: {
    uint16_t* len = strbuf_pull_type(buf, uint16_t);
    if (len == NULL) { return NULL; }
    const byte* contents = strbuf_pull(buf, *len);
    return PyUnicode_DecodeUTF8(contents, *len, "strict");
  }

  case _ESCODE_TYPE_LIST: {
    uint16_t* len = strbuf_pull_type(buf, uint16_t);
    if (len == NULL) { return NULL; }
    PyObject* obj = PyList_New(*len);
    if (obj == NULL) { return NULL; }

    for (uint16_t idx = 0; idx < *len; ++idx) {
      PyObject* elem = decode_object(buf);
      // PyList_SET_ITEM steals the reference even on failure
      if (elem == NULL || PyList_SET_ITEM(obj, idx, elem) < 0) {
        Py_DECREF(obj);
        return NULL;
      }
    }

    return obj;
  }

  case _ESCODE_TYPE_DICT: {
    uint16_t* len = strbuf_pull_type(buf, uint16_t);
    if (len == NULL) { return NULL; }
    PyObject* obj = PyDict_New();
    if (obj == NULL) { return NULL; }

    for (uint16_t idx = 0; idx < *len; ++idx) {
      PyObject* key = decode_object(buf);
      PyObject* val = decode_object(buf);
      if (key == NULL || val == NULL || PyDict_SetItem(obj, key, val) < 0) {
        Py_XDECREF(key);Py_XDECREF(val);Py_DECREF(obj);
        return NULL;
      }

      Py_DECREF(key);Py_DECREF(val);
    }

    return obj;
  }
  }

  PyErr_SetString(ESCODE_EncodeError, "Unrecognized type found: possibly corrupted string");
  return NULL;
}


#endif //__ESCODE_DECODER_H__
