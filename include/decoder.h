/*
 * Copyright (C) 2014 Akhil Wable
 * Author: Akhil Wable <awable@gmail.com>
 *
 * Decoder ESCODE
 *
 */

#ifndef __ESCODE_DECODER_H__
#define __ESCODE_DECODER_H__

#include "escode.h"

PyObject*
decode_object(EscodeReader *buf) {
  ESSIZE_T len;

  byte estype = EscodeReader_pull_num(buf, byte);

  switch (estype) {

  case ESCODE_TYPE_NONE: {
    Py_RETURN_NONE;

  }
  case ESCODE_TYPE_TRUE: {
    Py_RETURN_TRUE;

  }
  case ESCODE_TYPE_FALSE: {
    Py_RETURN_FALSE;

  }
  case ESCODE_TYPE_INT8: {
    uint8_t u8 = EscodeReader_pull_num(buf, uint8_t);
    return PyLong_FromLong((int8_t)u8);
  }

  case ESCODE_TYPE_INT16: {
    uint16_t u16 = EscodeReader_pull_num(buf, uint16_t);
    return PyLong_FromLong((int16_t)ntohs(u16));
  }

  case ESCODE_TYPE_INT32: {
    uint32_t u32 = EscodeReader_pull_num(buf, uint32_t);
    return PyLong_FromLong((int32_t)ntohl(u32));
  }

  case ESCODE_TYPE_INT64: {
    uint64_t u64 = EscodeReader_pull_num(buf, uint64_t);
    return PyLong_FromLongLong((int64_t)ntohll(u64));
  }

  case ESCODE_TYPE_UINT8: {
    uint8_t u8 = EscodeReader_pull_num(buf, uint8_t);
    return PyLong_FromUnsignedLong(u8);
  }

  case ESCODE_TYPE_UINT16: {
    uint16_t u16 = EscodeReader_pull_num(buf, uint16_t);
    return PyLong_FromUnsignedLong(ntohs(u16));
  }

  case ESCODE_TYPE_UINT32: {
    uint32_t u32 = EscodeReader_pull_num(buf, uint32_t);
    return PyLong_FromUnsignedLong(ntohl(u32));
  }

  case ESCODE_TYPE_UINT64: {
    uint64_t u64 = EscodeReader_pull_num(buf, uint64_t);
    return PyLong_FromUnsignedLongLong(ntohll(u64));
  }

  case ESCODE_TYPE_FLOAT: {
    uint64_t u64 = EscodeReader_pull_num(buf, uint64_t);
    u64 = SRINT64_FROMUINT(ntohll(u64));
    return PyFloat_FromDouble(*(double*)&u64);
  }

  case ESCODE_TYPE_DATETIME: {
    uint64_t u64 = EscodeReader_pull_num(buf, uint64_t);
    u64 = ntohll(u64);
    return MyPyDateTime_FROM_TIMESTAMP(u64);
  }

  case ESCODE_TYPE_BYTES: {
    len = ESSIZE_T_ntoh(EscodeReader_pull_num(buf, ESSIZE_T));
    const byte* contents = EscodeReader_pull(buf, len);
    return PyBytes_FromStringAndSize((char*)contents, len);
  }

  case ESCODE_TYPE_UNICODE: {
    len = ESSIZE_T_ntoh(EscodeReader_pull_num(buf, ESSIZE_T));
    const byte* contents = EscodeReader_pull(buf, len);
    return PyUnicode_DecodeUTF8((char*)contents, len, "strict");
  }

  case ESCODE_TYPE_LIST: {
    len = ESSIZE_T_ntoh(EscodeReader_pull_num(buf, ESSIZE_T));
    PyObject* obj = PyList_New(len);
    if (obj == NULL) { return NULL; }

    for (ESSIZE_T idx = 0; idx < len; ++idx) {
      PyObject* elem = decode_object(buf);
      if (elem == NULL || PyList_SET_ITEM(obj, idx, elem) < 0) {
        Py_XDECREF(obj);
        return NULL;
      }
    }
    return obj;
  }

  case ESCODE_TYPE_TUPLE: {
    len = ESSIZE_T_ntoh(EscodeReader_pull_num(buf, ESSIZE_T));
    PyObject* obj = PyTuple_New(len);
    if (obj == NULL) { return NULL; }

    for (ESSIZE_T idx = 0; idx < len; ++idx) {
      PyObject* elem = decode_object(buf);
      if (elem == NULL || PyTuple_SET_ITEM(obj, idx, elem) < 0) {
        Py_XDECREF(obj);
        return NULL;
      }
    }
    return obj;
  }

  case ESCODE_TYPE_SET: {
    len = ESSIZE_T_ntoh(EscodeReader_pull_num(buf, ESSIZE_T));
    PyObject* obj = PySet_New(NULL);
    if (obj == NULL) { return NULL; }

    for (ESSIZE_T idx = 0; idx < len; ++idx) {
      PyObject* item = decode_object(buf);
      if (item == NULL || PySet_Add(obj, item) < 0) {
        Py_XDECREF(item);Py_DECREF(obj);
        return NULL;
      }

      Py_DECREF(item);
    }
    return obj;
  }

  case ESCODE_TYPE_DICT: {
    len = ESSIZE_T_ntoh(EscodeReader_pull_num(buf, ESSIZE_T));
    PyObject* obj = PyDict_New();
    if (obj == NULL) { return NULL; }

    for (ESSIZE_T idx = 0; idx < len; ++idx) {
      PyObject* key = decode_object(buf);
      PyObject* val = decode_object(buf);
      if (key == NULL || val == NULL || PyDict_SetItem(obj, key, val) < 0) {
        Py_XDECREF(key);Py_XDECREF(val);Py_DECREF(obj);
        return NULL;
      }
      Py_DECREF(key);Py_DECREF(val);
    }
    return obj;
  }}

  PyErr_SetString(ESCODE_EncodeError, "Unrecognized type found: possibly corrupted string");
  return NULL;
}


#endif //__ESCODE_DECODER_H__
