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
decode_object(ESReader* buf) {

  eshead_t _eshead; // Allocate on stack
  eshead_t* eshead = &_eshead;
  PyObject *obj = NULL;

  byte headbyte = ESReader_readtype(buf, byte);
  ESHEAD_INITDECODE(eshead, headbyte);

  const byte* bytes;

  switch (ESHEAD_GETTYPE(eshead)) {

  case ESTYPE_NONE:
    Py_RETURN_NONE;
  case ESTYPE_BOOL:
    if (ESHEAD_GETBOOL(eshead)) Py_RETURN_TRUE; Py_RETURN_FALSE;

  case ESTYPE_INT: {
    bool ispos = ESHEAD_GETBIT(eshead);
    bytes = ESReader_read(buf, ESHEAD_GETNUMWIDTH(eshead, ispos));
    ESHEAD_DECODEINT(eshead, bytes);
    return (ispos ?
            PyLong_FromUnsignedLongLong(eshead->val.u64) :
            PyLong_FromLongLong(eshead->val.i64));

#if PY_VERSION_HEX >= 0x03030000
  case ESTYPE_DEC: {
    ESDecimal esdec = {};
    esdec.sign = !ESHEAD_GETBIT(eshead);
    uint8_t ebit = ESHEAD_GETEXPBIT(eshead);
    uint8_t width = ESHEAD_GETEXPWIDTH(eshead);
    bytes = ESReader_read(buf, 1);

    if (width == 0x08) {
      if (!ebit && *bytes == 0x00) {
        esdec.ops = esdec.sign ? ESDEC_Inf : ESDEC_Zero;
        return MyPyDec_FromESDecimal(&esdec);
      } else if (ebit && *bytes == 0xFF) {
        esdec.ops = esdec.sign ? ESDEC_Zero : ESDEC_Inf;
        return MyPyDec_FromESDecimal(&esdec);
      }
    }

    ESReader_read(buf, width-1);
    esdec.sign = !ESHEAD_DECODEEXP(eshead, bytes);
    esdec.exp = eshead->val.i64;
    esdec.digits = 2;

    // bytes = ESReader_read(buf, 1);
    // while (*bytes & 1) { ESReader_read(buf, 1); esdec.digits += 2; }
    esdec.data = bytes;

    return MyPyDec_FromESDecimal(&esdec);
  }
#endif

  case ESTYPE_FLOAT: {
    bytes = ESReader_read(buf, sizeof(double));
    ESHEAD_DECODEFLOAT(eshead, bytes);
    return PyFloat_FromDouble(eshead->val.flt);
  }

  case ESTYPE_STRING: {
    bytes = ESReader_read(buf, ESHEAD_GETNUMWIDTH(eshead, 1));
    bool isunicode = ESHEAD_DECODELEN(eshead, bytes);
    const byte* contents = ESReader_read(buf, eshead->val.u64);
    return (isunicode ?
            PyUnicode_DecodeUTF8((char*)contents, eshead->val.u64, "strict") :
            PyBytes_FromStringAndSize((char*)contents, eshead->val.u64));
  }

  case ESTYPE_LIST: {
    bytes = ESReader_read(buf, ESHEAD_GETNUMWIDTH(eshead, 1));
    bool istuple = ESHEAD_DECODELEN(eshead, bytes);

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
    bytes = ESReader_read(buf, ESHEAD_GETNUMWIDTH(eshead, 1));
    bool isdict = ESHEAD_DECODELEN(eshead, bytes);

    if (isdict) {
      obj = _PyDict_NewPresized(eshead->val.u64); assert(obj);
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

  PyErr_Format(ESCODE_DecodeError, "Unrecognized type in headbyte: %02x", headbyte);
  return NULL;
}


#endif //__ESCODE_DECODER_H__
