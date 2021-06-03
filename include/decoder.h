/*
 * Copyright (C) 2014 Akhil Wable
 * Author: Akhil Wable <awable@gmail.com>
 *
 * Decoder ESCODE
 *
 */

#ifndef __ESCODE_DECODER_H__
#define __ESCODE_DECODER_H__

#include "core/strbuf.h"
#include "core/constants.h"
#include "core/eshead.h"
#include "escode.h"

PyObject*
decode_object(EscodeReader* buf) {
  eshead_t _eshead; // Allocate on stack
  eshead_t* eshead = &_eshead;

  const byte* bytes;

  bool headbyte = EscodeReader_readtype(buf, bool);
  ESHEAD_INITDECODE(eshead, headbyte);

  switch (ESHEAD_GETTYPE(eshead)) {

  case ESTYPE_NONE:
    Py_RETURN_NONE;
  case ESTYPE_BOOL:
    if (ESHEAD_GETBOOL(eshead)) Py_RETURN_TRUE; Py_RETURN_FALSE;

  case ESTYPE_INT: {
    bytes = EscodeReader_read(buf, ESHEAD_GETINTWIDTH(eshead));
    bool ispos = ESHEAD_DECODEINT(eshead, bytes);
    return (ispos ?
            PyLong_FromUnsignedLongLong(eshead->val.u64) :
            PyLong_FromLongLong(eshead->val.i64));

  case ESTYPE_FLOAT: {
    bytes = EscodeReader_read(buf, sizeof(double));
    ESHEAD_DECODEFLOAT(eshead, bytes);
    return PyFloat_FromDouble(eshead->val.flt);
  }

  case ESTYPE_STRING: {
    bytes = EscodeReader_read(buf, ESHEAD_GETWIDTH(eshead));
    bool isunicode = ESHEAD_DECODELEN(eshead, bytes);
    const byte* contents = EscodeReader_read(buf, eshead->val.u64);
    return (isunicode ?
            PyUnicode_DecodeUTF8((char*)contents, eshead->val.u64, "strict") :
            PyBytes_FromStringAndSize((char*)contents, eshead->val.u64));
  }

  case ESTYPE_LIST: {
    bytes = EscodeReader_read(buf, ESHEAD_GETWIDTH(eshead));
    bool istuple = ESHEAD_DECODELEN(eshead, bytes);
    PyObject *obj;

    if (istuple) {
      obj = PyTuple_New(eshead->val.u64); assert(obj);
      for (uint64_t idx = 0; idx < eshead->val.u64; ++idx) {
        PyObject* elem = decode_object(buf);
        if (elem == NULL || PyTuple_SET_ITEM(obj, idx, elem) < 0) {
          Py_XDECREF(obj);
          return NULL;
        }
      }
    } else {
      obj = PyList_New(eshead->val.u64); assert(obj);
      for (uint64_t idx = 0; idx < eshead->val.u64; ++idx) {
        PyObject* elem = decode_object(buf);
        if (elem == NULL || PyList_SET_ITEM(obj, idx, elem) < 0) {
          Py_XDECREF(obj);
          return NULL;
        }
      }
    }

    return obj;
  }

  case ESTYPE_SET: {
    bytes = EscodeReader_read(buf, ESHEAD_GETWIDTH(eshead));
    bool isdict = ESHEAD_DECODELEN(eshead, bytes);
    PyObject *obj;

    if (isdict) {
      obj = PyDict_New(); assert(obj);
      for (uint64_t idx = 0; idx < eshead->val.u64; ++idx) {
        PyObject* key = decode_object(buf);
        PyObject* val = decode_object(buf);
        if (key == NULL || val == NULL || PyDict_SetItem(obj, key, val) < 0) {
          Py_XDECREF(key);Py_XDECREF(val);Py_DECREF(obj);
          return NULL;
        }
        Py_DECREF(key);Py_DECREF(val);
      }
    } else {
      obj = PySet_New(NULL); assert(obj);
      for (uint64_t idx = 0; idx < eshead->val.u64; ++idx) {
        PyObject* item = decode_object(buf);
        if (item == NULL || PySet_Add(obj, item) < 0) {
          Py_XDECREF(item);Py_DECREF(obj);
          return NULL;
        }
        Py_DECREF(item);
      }
    }

    return obj;
  }
  }}

  printf("Unrecognized type %02x", headbyte);
  PyErr_SetString(ESCODE_EncodeError, "Unrecognized type found: possibly corrupted string");
  return NULL;
}


#endif //__ESCODE_DECODER_H__
