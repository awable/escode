/*
 * Copyright (C) 2014 Akhil Wable
 * Author: Akhil Wable <awable@gmail.com>
 *
 * Encoder ESCODE
 *
 */

#ifndef __ESCODE_ENCODER_H__
#define __ESCODE_ENCODER_H__

#include "escode.h"

int
encode_object(PyObject *object, EscodeWriter* buf) {

  EscodeItem enc;

  if (object == Py_None) {
    enc = (EscodeItem){.type=ESCODE_TYPE_NONE};
    EscodeWriter_put_item(buf, enc);

  } else if (object == Py_True) {
    enc = (EscodeItem){.type=ESCODE_TYPE_TRUE, .ops=ITEMOP_INDEXTYPE};
    EscodeWriter_put_item(buf, enc);

  } else if (object == Py_False) {
    enc = (EscodeItem){.type=ESCODE_TYPE_FALSE, .ops=ITEMOP_INDEXTYPE};
    EscodeWriter_put_item(buf, enc);

  } else if (PyInt_CheckExact(object) || PyLong_CheckExact(object)) {
    int32_t ofl;
    int64_t i64;
    uint64_t u64 = 0; //Only set if > INT64_MAX

    i64 = PyLong_AsLongLongAndOverflow(object, &ofl);
    if (ofl > 0) {
      u64 = PyLong_AsUnsignedLongLong(object);
    }

    if (ofl < 0 || PyErr_Occurred()) {
      PyErr_SetString(ESCODE_EncodeError, "Number oout of bounds");
      return 0;
    }

    uint64_t bigendian = htonll(ofl > 0 ? u64 : i64);
    byte* num = (byte*)&bigendian;

    if (ofl > 0 || i64 > UINT32_MAX) {
      // if there was an overflow we cant trust i64 so we do this first
      enc = (EscodeItem){.type=ESCODE_TYPE_UINT64, .str=num, .len=sizeof(uint64_t)};
    } else if (i64 < 0) {
      if (i64 < INT32_MIN) {
        enc = (EscodeItem){.type=ESCODE_TYPE_INT64, .str=num, .len=sizeof(uint64_t)};
      } else if (i64 < INT16_MIN) {
        enc = (EscodeItem){.type=ESCODE_TYPE_INT32, .str=num+4, .len=sizeof(uint32_t)};
      } else if (i64 < INT8_MIN) {
        enc = (EscodeItem){.type=ESCODE_TYPE_INT16, .str=num+6, .len=sizeof(uint16_t)};
      } else {
        enc = (EscodeItem){.type=ESCODE_TYPE_INT8, .str=num+7, .len=sizeof(uint8_t)};
      }
    } else {
      if (i64 <= UINT8_MAX) {
        enc = (EscodeItem){.type=ESCODE_TYPE_UINT8, .str=num+7, .len=sizeof(uint8_t)};
      } else if (i64 <= UINT16_MAX) {
        enc = (EscodeItem){.type=ESCODE_TYPE_UINT16, .str=num+6, .len=sizeof(uint16_t)};
      } else {
        enc = (EscodeItem){.type=ESCODE_TYPE_UINT32, .str=num+4, .len=sizeof(uint32_t)};
      }
    }

    // All numbers have the type as a part of their index encoding
    enc.ops = ITEMOP_INDEXTYPE;
    EscodeWriter_put_item(buf, enc);

  } else if (PyFloat_CheckExact(object)) {
    double val = PyFloat_AS_DOUBLE(object);
    // Doubles are essentially Sign Represented INTs for ordering
    uint64_t u64 = htonll(SRINT64_TOUINT(*(uint64_t*)&val));
    enc = (EscodeItem){.type=ESCODE_TYPE_FLOAT, .str=(byte*)&u64, .len=sizeof(uint64_t)};
    EscodeWriter_put_item(buf, enc);

  } else if (PyDateTime_CheckExact(object)) {
    uint64_t u64 =  MyPyDateTime_GET_TIMESTAMP(object);
    u64 = htonll(u64);
    enc = (EscodeItem){.type=ESCODE_TYPE_DATETIME, .str=(byte*)&u64, .len=sizeof(uint64_t)};
    EscodeWriter_put_item(buf, enc);

  } else if (PyBytes_CheckExact(object)) {
    enc = (EscodeItem){.type=ESCODE_TYPE_BYTES,
           .len=PyBytes_GET_SIZE(object),
           .str=(byte*)PyBytes_AS_STRING(object),
           .ops=ITEMOP_WRITELEN};
    EscodeWriter_put_item(buf, enc);

  } else if (PyUnicode_CheckExact(object)) {
    PyObject* bobject = PyUnicode_AsUTF8String(object);
    if (bobject == NULL) { return 0; }
    enc = (EscodeItem){.type=ESCODE_TYPE_UNICODE,
           .len=PyBytes_GET_SIZE(bobject),
           .str=(byte*)PyBytes_AS_STRING(bobject),
           .ops=ITEMOP_WRITELEN};
    EscodeWriter_put_item(buf, enc);
    Py_DECREF(bobject);

  } else if (buf->ops ^ WRITEROP_INDEX) {

    // COLLECTIONS START HERE: LIST/TUPLE/SET/DICT

    if(PyList_CheckExact(object)) {
      Py_ssize_t len = PyList_GET_SIZE(object);
      enc = (EscodeItem){.type=ESCODE_TYPE_LIST, .len=len, .ops=ITEMOP_WRITELEN};
      EscodeWriter_put_item(buf, enc);

      for (ESSIZE_T idx = 0; idx < len; ++idx) {
        if (!encode_object(PyList_GET_ITEM(object, idx), buf)) { return 0; }
      }

    } else if (PyTuple_CheckExact(object)) {
      Py_ssize_t len = PyTuple_GET_SIZE(object);
      enc = (EscodeItem){.type=ESCODE_TYPE_TUPLE, .len=len, .ops=ITEMOP_WRITELEN};

      EscodeWriter_put_item(buf, enc);
      for (ESSIZE_T idx = 0; idx < len; ++idx) {
        if (!encode_object(PyTuple_GET_ITEM(object, idx), buf)) { return 0; }
      }

    } else if (PyAnySet_CheckExact(object)) {
      enc = (EscodeItem){.type=ESCODE_TYPE_SET, .len=PySet_GET_SIZE(object), .ops=ITEMOP_WRITELEN};
      EscodeWriter_put_item(buf, enc);

      PyObject *item;
      Py_ssize_t pos = 0;
      long hash;
      while (_PySet_NextEntry(object, &pos, &item, &hash)) {
        if (!encode_object(item, buf)) { return 0; }
      }

    } else if (PyDict_CheckExact(object)) {
      enc = (EscodeItem){.type=ESCODE_TYPE_DICT, .len=PyDict_GET_SIZE(object), .ops=ITEMOP_WRITELEN};
      EscodeWriter_put_item(buf, enc);

      PyObject *key, *value;
      Py_ssize_t pos = 0;
      while (PyDict_Next(object, &pos, &key, &value)) {
        if (!encode_object(key, buf)) { return 0; }
        if (!encode_object(value, buf)) { return 0; }
      }
    } else {
      PyErr_SetString(ESCODE_UnsupportedError, Py_TYPE(object)->tp_name);
      return 0;
    }

  } else {
    PyErr_SetString(ESCODE_UnsupportedError, Py_TYPE(object)->tp_name);
    return 0;
  }

  return 1;
}


#endif //__ESCODE_ENCODER_H__
